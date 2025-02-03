// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

import MODEL_BUILDER from 'main/beBuilders/cbdi/cbdiModelBuilder/cbdiModelBuilderNonFlatBuffer';
import INTENTION_BUILDER from 'main/beBuilders/cbdi/intentionBuilder/cbdiIntentionBuilderNonFlatBuffer';
import LOG_BUILDER from 'main/beBuilders/cbdi/logBuilder/cbdiLogBuilderNonFlatBuffer';
import LOGGER from 'misc/addons/logger/LoggerSingleton';
import { eventListeners, response } from 'misc/events/common/cmEvents';
import { Event } from 'misc/types/cbdi/cbdi-types-non-flatbuffer';
import { ipcMain } from 'electron';
import { secondToMillisecond } from './helper';
import { DebugManager } from './debugManager/debugManager';

const PLAYBACK_INTERVAL_TIME = 1000;
const SNAPSHOT_INTERVAL = 60;
const SNAPSHOT_LIMIT = 1;

export enum PlaybackState {
  PAUSE = 'pause',
  PLAY = 'play',
  STOP = 'stop',
  LIVE = 'live',
  DEBUG_BY_STEP = 'debug-by-step',
  DEBUG_BY_STEP_RUN = 'debug-by-step-run',
  DEBUG_AUTORUN = 'debug-auto-run',
}

export type Snapshot = {
  model: string;
  log: string;
  intention: string;
};

/**
 * Delta time in second unit
 */
const deltaTimeInSecond = 1 / 60; // target 60 fps microsecond

class PlayBackManager {
  /**
   * Records of raw events, only used in playback mode (not live playback)
   */
  private records: Event[];

  /**
   * Playback manager's state
   */
  private state: PlaybackState;

  /**
   * Snapshots of builders states
   */
  private snapshots: Record<string, Snapshot>;

  /**
   * Next frame index in second to be played.
   */
  private nextFrameIndexBySecond: number;

  /**
   * Timer to check live is alive
   */
  private liveTimeout: NodeJS.Timer | null;

  /**
   * Live mode alive state
   */
  private isLiveModeStale: boolean;

  /**
   * Playback gameloop by deltaTime
   */
  private _gameLoopInterval: NodeJS.Timer | null;

  /**
   * Playback actual clock in seconds
   */
  private _clockInSecond: number;

  /**
   * Record index for playback
   */
  private _recordIndex = 0;

  /**
   * Previous record index (before going to frame)
   */
  private _previousRecordIndex = 0;

  /**
   * Live playback debug mode
   */
  private _liveDebug = false;

  /**
   * Control debug step
   */
  private _tracer: DebugManager;

  constructor() {
    this.state = PlaybackState.STOP;
    this.snapshots = {};
    this.records = [];
    this.nextFrameIndexBySecond = 0;
    this.liveTimeout = null;
    this.isLiveModeStale = false;
    this._clockInSecond = 0;
    this._gameLoopInterval = null;
    this._liveDebug = false;
    this._tracer = new DebugManager();
  }

  /* --------------------------------- Private -------------------------------- */

  /**
   * Reset the live mode stale timer
   * @param window
   */
  private _resetLiveTimeout(window?: Electron.BrowserWindow) {
    if (this.liveTimeout) {
      clearTimeout(this.liveTimeout);
      this.liveTimeout = null;
    }

    this.isLiveModeStale = false;

    this.liveTimeout = setTimeout(() => {
      if (!this.isLiveModeStale) {
        this.isLiveModeStale = true;

        // Send stale state
        if (window) {
          window.webContents.send(response.playback.stale);
        }
      }

      if (this.liveTimeout) {
        clearTimeout(this.liveTimeout);
        this.liveTimeout = null;
      }
    }, PLAYBACK_INTERVAL_TIME * 2);
  }

  /**
   * Get total frames length
   * @returns
   */
  private _getTotalFramesBySecond() {
    if (this.records.length < 1) {
      return 0;
    }

    const totalFrames =
      Math.floor(this.records[this.records.length - 1].timestampUs / 1000000) +
      1;
    return totalFrames;
  }

  /**
   * Update loop that runs every deltaTime
   * @param tickCallback
   */
  private _updateLoop(tickCallback?: (...args: any) => void) {
    if (this.state === PlaybackState.LIVE) {
      const startIndex = Math.max(0, this._recordIndex);
      const len = this.records.length;

      for (let i = startIndex; i < len; i++) {
        const record = this.records[i];
        // build records with timestamp <= clock
        MODEL_BUILDER.buildNonFlatBuffer(record);
        LOG_BUILDER.buildNonFlatBuffer(record);
        INTENTION_BUILDER.buildNonFlatBuffer(record);
      }
      this._recordIndex = this.records.length - 1;

      if (tickCallback && this._clockInSecond >= this.nextFrameIndexBySecond) {
        // send tick event to frontend
        tickCallback(this.nextFrameIndexBySecond);
        this.nextFrameIndexBySecond++;
      }

      if (startIndex < len - 1) {
        this._clockInSecond = this.records[len - 1].timestampUs / 1000000;
      }
    }
    //
    else if (this.state === PlaybackState.PLAY) {
      const startIndex = Math.max(0, this._recordIndex);
      const len = this.records.length;

      for (let i = startIndex; i < len; i++) {
        const record = this.records[i];

        // Check if it is break point
        if (
          this._tracer.isActivated() &&
          this._tracer.isDebugBreakPoint(record) &&
          this._tracer.CurrentEventIndex < i
        ) {
          this.stopByBreakPoint(i);
          break;
        }

        if (record.timestampUs > secondToMillisecond(this._clockInSecond)) {
          this._recordIndex = i;
          break;
        }

        // build records with timestamp <= clock
        MODEL_BUILDER.buildNonFlatBuffer(record);
        LOG_BUILDER.buildNonFlatBuffer(record);
        INTENTION_BUILDER.buildNonFlatBuffer(record);
      }

      // tick every second
      if (tickCallback && this._clockInSecond >= this.nextFrameIndexBySecond) {
        // send tick event to frontend
        tickCallback(this.nextFrameIndexBySecond);

        // create snapshot if reaches checkpoint
        if (
          this.nextFrameIndexBySecond > 0 &&
          this.nextFrameIndexBySecond % SNAPSHOT_INTERVAL === 0
        ) {
          this.createSnapshot(this.nextFrameIndexBySecond);
        }

        // Finish playback
        const totalFrames = this._getTotalFramesBySecond();
        const lastFrameIndex = totalFrames - 1;
        if (this.nextFrameIndexBySecond > lastFrameIndex) {
          this.stop();
        }

        this.nextFrameIndexBySecond++;
      }

      if (startIndex < len - 1) {
        this._clockInSecond += deltaTimeInSecond;
      }
    }
    //
    else if (this.state === PlaybackState.PAUSE) {
      if (this._tracer.isAutoRun) {
        this._tracer.updateDebugCounter(deltaTimeInSecond);

        if (!this._tracer.isCoolingDown()) {
          this._tracer.resetCounter();
          this.nextTrace();
        }
      }
    }
  }

  /* --------------------------------- Public --------------------------------- */
  /**
   * Reset playback file
   */
  resetPlaybackFile() {
    this.stop();
    this.clearModel();
  }

  /**
   * Clear playback manager
   */
  clear() {
    this.stop();
    this.records = [];
    this.snapshots = {};
    this.isLiveModeStale = false;
    if (this.liveTimeout) {
      clearTimeout(this.liveTimeout);
      this.liveTimeout = null;
    }

    this._tracer.reset();
  }

  /**
   * Open a playback file.
   * @param rawEvents
   */
  openRawEvts(rawEvents: Event[]) {
    this.resetPlaybackFile();
    this.records = rawEvents;

    const totalFrames = this._getTotalFramesBySecond();

    LOGGER.info('Total frames', totalFrames);
  }

  /**
   * Only use this in live mode to feed event to playback records
   * @param event
   */
  feedLiveEvent(event: Event, window: Electron.BrowserWindow) {
    // Reset check live stale timer
    this._resetLiveTimeout(window);

    // Only build asap when is live and islivedebug = false
    if (this.isLive() && !this._liveDebug) {
      // build records with timestamp <= clock
      MODEL_BUILDER.buildNonFlatBuffer(event);
      LOG_BUILDER.buildNonFlatBuffer(event);
      INTENTION_BUILDER.buildNonFlatBuffer(event);
    }

    this.records.push(event);

    const totalFrames = this._getTotalFramesBySecond();

    // Current event index by second
    const index = Math.floor(event.timestampUs / 1000000);

    // Previous event index by second
    const previous =
      this.records.length > 2
        ? Math.floor(
            this.records[this.records.length - 2].timestampUs / 1000000
          )
        : 0;

    // First event in timeframe
    if (previous < index) {
      window.webContents.send(response.playback.feedLiveProgress, totalFrames);

      // Only store events if `isLiveDebug` is on
      if (!this._liveDebug) {
        this.records.pop();

        // If debug live is off we need to send tick event otherwise client won't be able to update playback bar time
        window.webContents.send(response.playback.tick, totalFrames);
      }
      // Create snapshot at frame 30, 60,... (by SNAPSHOT_INTERVAL)
      else if (
        // The builders are only run when in live mode
        this.isLive() &&
        Math.floor(totalFrames / SNAPSHOT_INTERVAL) > 0 &&
        totalFrames % SNAPSHOT_INTERVAL === 1
      ) {
        const lastFrameIndex = totalFrames - 1;
        this.createSnapshot(lastFrameIndex);
      }
    }
    // Error
    else if (previous > index) {
      throw new Error('Event is not in order');
    }
  }

  /**
   * Start playback
   * @param cb
   * @returns
   */
  play(cb?: (...args: any) => void) {
    if (this.state === PlaybackState.PLAY) {
      return;
    }

    this.state = PlaybackState.PLAY;

    this.runLoop(cb);
  }

  runLoop(cb?: (...args: any) => void) {
    if (!this._gameLoopInterval) {
      this._updateLoop(cb);

      this._gameLoopInterval = setInterval(
        this._updateLoop.bind(this, cb),
        deltaTimeInSecond * 1000
      );
    }
  }

  /**
   * Pause playback
   */
  pause() {
    this.state = PlaybackState.PAUSE;
    ipcMain.emit(eventListeners.playback.commandFrontendPausePlayback);
  }

  /**
   * Stop playback
   */
  stop() {
    this.nextFrameIndexBySecond = 0;
    this.state = PlaybackState.STOP;
    this._clockInSecond = 0;
    this._recordIndex = 0;

    if (this._gameLoopInterval && !this.isTracing()) {
      clearInterval(this._gameLoopInterval);
      this._gameLoopInterval = null;
    }

    if (!this.isTracing) {
      this._tracer.reset();
    }
  }

  /**
   * Compute builders states to timeframe
   * @param targetTimeframe next frame to be played
   */
  goToFrame(targetTimeframe: number, records = this.records, live = false) {
    if (
      targetTimeframe !== 0 &&
      this.state === PlaybackState.LIVE &&
      !this._liveDebug
    ) {
      LOGGER.error('Error: should not playback when live debug is off');
      return;
    }

    LOGGER.debug('previousIndex', this._previousRecordIndex);
    LOGGER.debug('Go to frame', targetTimeframe);

    // Have to perform this to store playbackIndex
    const pbIndex =
      this.nextFrameIndexBySecond > targetTimeframe
        ? 0
        : this.nextFrameIndexBySecond;

    const lastRecordIndex = Math.max(this._recordIndex, 0);
    // Stop the playback before performing goToFrame
    this.stop();

    let currFrame = 0;

    if (targetTimeframe > 0) {
      // Restore snapshot and move current frame
      currFrame = this.restoreSnapshot(targetTimeframe, pbIndex);

      // If currFrame = 0 means there is no snapshot, then continue from playbackIndex
      currFrame = currFrame > 0 ? currFrame : pbIndex;
    }

    // If start from frame 0, then clear model
    if (currFrame === 0) {
      this.clearModel();
    }

    // Find record index
    // If last record index is smaller, use last record index (otherwise we will lose events)
    const millisecondCurrFrame = secondToMillisecond(currFrame);
    this._recordIndex = Math.min(
      records.findIndex((x) => x.timestampUs >= millisecondCurrFrame),
      lastRecordIndex
    );
    this._recordIndex = Math.max(0, this._recordIndex); // Make sure it is positive

    if (
      targetTimeframe > -1 &&
      records.length > 0 &&
      secondToMillisecond(targetTimeframe) <=
        records[records.length - 1].timestampUs // last record timestamp
    ) {
      const targetTimeframeInMicrosecond = secondToMillisecond(targetTimeframe);

      const len = records.length;
      // Compute the builder state to that frame in worker thread

      for (let i = this._recordIndex; i < len; i++) {
        const record = records[i];

        this._recordIndex = i;

        if (!live && record.timestampUs >= targetTimeframeInMicrosecond) {
          break;
        }

        MODEL_BUILDER.buildNonFlatBuffer(record);
        LOG_BUILDER.buildNonFlatBuffer(record);
        INTENTION_BUILDER.buildNonFlatBuffer(record);
      }

      this._clockInSecond = targetTimeframe;

      // Update next frame to be played
      this.nextFrameIndexBySecond = targetTimeframe + 1;

      // Update tracer
      this._tracer.CurrentEventIndex = this._recordIndex;

      LOGGER.info(
        'Go to frame successfully, current frame is',
        targetTimeframe
      );
    }
    // Out of range
    else if (targetTimeframe !== 0) {
      throw new Error('Playback error, timeframe is out of range');
    }

    // timeframe = 0, do nothing, this is just triggered at the start from live mode
  }

  /**
   * Create snapshot in playback manager
   * @param timeframe index by second, starts from 0.
   */
  createSnapshot = (timeframe: number) => {
    // Do not create snapshot when not live or not live debug
    if (!this.isLive() || !this._liveDebug) {
      return;
    }

    LOGGER.info('Create snapshot at frame', timeframe);

    const snapshot: Snapshot = {
      model: MODEL_BUILDER.createSnapshot(),
      log: LOG_BUILDER.createSnapshot(),
      intention: INTENTION_BUILDER.createSnapshot(),
    };

    // Get timeframes and sort by order
    const oldKeys = Object.keys(this.snapshots)
      .map((x) => parseInt(x, 10))
      .sort((a, b) => {
        return a - b > 0 ? 1 : -1;
      });

    if (oldKeys.length > 0 && oldKeys[oldKeys.length - 1] < timeframe) {
      // Add snapshot
      this.snapshots[timeframe.toString()] = snapshot;

      // Get timeframes and sort by order
      const timeframes = Object.keys(this.snapshots)
        .map((x) => parseInt(x, 10))
        .sort((a, b) => {
          return a - b > 0 ? 1 : -1;
        });

      // remove the first when reach limit
      // (the smallest timeframe because the later timeframe is more expensive to recompute)
      if (timeframes.length > SNAPSHOT_LIMIT && timeframes.length > 0) {
        const toBeRemoved = timeframes[0];
        delete this.snapshots[toBeRemoved];

        LOGGER.info('Remove snapshot at frame', toBeRemoved);
      }
    }
  };

  /**
   * Restore snapshot and return the current frame of snapshot.
   * @param targetTimeframe starts from 0
   * @returns
   */
  restoreSnapshot = (targetTimeframe: number, currTimeframe: number) => {
    // List of timeframes snapshot
    const indices = Object.keys(this.snapshots).map((x) => parseInt(x, 10));

    // Find the index closest to a smaller timeframe
    const index = indices.reduce((prev, curr) => {
      if (curr < targetTimeframe) {
        if (targetTimeframe - curr < targetTimeframe - prev) {
          return curr;
        }
      }
      return prev;
    }, 0);

    // Only restore if the snapshoot frame is bigger than playback index
    if (index > 0 && index > currTimeframe) {
      LOGGER.info('Restore snapshot at frame', index);
      MODEL_BUILDER.restoreSnapshot(this.snapshots[index.toString()].model);
      LOG_BUILDER.restoreSnapshot(this.snapshots[index.toString()].log);
      INTENTION_BUILDER.restoreSnapshot(
        this.snapshots[index.toString()].intention
      );
    }

    return index > currTimeframe ? index : currTimeframe;
  };

  /**
   * Clear model
   */
  clearModel = () => {
    MODEL_BUILDER.clear();
    LOG_BUILDER.clear();
    INTENTION_BUILDER.clear();
  };

  /**
   * Get the playback length
   * @returns
   */
  getPlaybackLength() {
    return this._getTotalFramesBySecond();
  }

  lastRecordsIndex() {
    return this.records.length - 1;
  }

  /**
   * Playback goes live.
   */
  goLive = () => {
    // Find the last frame and go to that frame
    this.goToLastFrame();

    // Set state to live
    this.state = PlaybackState.LIVE;

    this._resetLiveTimeout();
  };

  goToLastFrame = () => {
    // Find the last frame and go to that frame
    const lastFrame = Math.max(this._getTotalFramesBySecond() - 1, 0);
    this.goToFrame(lastFrame, this.records, true);
  };

  /**
   * Playback's state is live
   * @returns
   */
  isLive = () => this.state === PlaybackState.LIVE;

  /**
   * Set live debug state
   */
  setLiveDebug = (updatedLiveDebug: boolean) => {
    this._liveDebug = updatedLiveDebug;
  };

  /**
   * Start debug autorun
   */
  debugAuto(cb?: (...args: any) => void) {
    this._tracer.isAutoRun = true;
    this.trace(cb);
  }

  pauseDebugAuto() {
    this._tracer.isAutoRun = false;
    this.pause();
  }

  trace(cb?: (...args: any) => void) {
    this._tracer.CurrentEventIndex = this._recordIndex;

    if (!this._tracer.isActivated()) {
      this._tracer.activate();
      this.play(cb);
    } else {
      this.nextTrace();
    }
  }

  nextTrace() {
    this._tracer.activate();
    this.state = PlaybackState.PLAY;
  }

  stopTrace() {
    this._recordIndex = this._tracer.CurrentEventIndex;
    this._tracer.deactivate();
    this._tracer.reset();
    this.state = PlaybackState.PAUSE;
  }

  isTracing() {
    return this._tracer.isActivated();
  }

  isBreakPoint(mess: Event) {
    return this._tracer.isDebugBreakPoint(mess);
  }

  stopByBreakPoint(index: number) {
    this.pause();
    this._tracer.CurrentEventIndex = index;
    this._recordIndex = index;
  }

  /**
   * Set records from previousRecordIndex to end
   * @param data
   */
  setRecords(data: Event[]) {
    this.records.splice(this._previousRecordIndex + 1);
    // eslint-disable-next-line no-restricted-syntax
    for (const item of data) {
      this.records.push(item);
    }
  }

  /**
   * Get previous record index
   * @returns
   */
  previousRecordIndex() {
    return this._previousRecordIndex;
  }

  /**
   * Set previous record index
   */
  setPreviousRecordIndex() {
    if (this._previousRecordIndex < this._recordIndex) {
      this._previousRecordIndex = this._recordIndex;
    }
  }

  /**
   * Get live debug
   * @returns
   */
  isLiveDebug() {
    return this._liveDebug;
  }
}

/**
 * Playback manager.
 */
const playbackManager = new PlayBackManager();
export default playbackManager;
