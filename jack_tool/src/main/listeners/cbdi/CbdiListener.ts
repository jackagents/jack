/* eslint-disable class-methods-use-this */
import WebSocketObj from 'main/websocketClient/WebSocketObj';
import { eventListeners, request, response } from 'projectEvents/common/cmEvents';
import { CBDIEditorProject } from 'misc/types/cbdiEdit/cbdiEditModel';
import LOG_BUILDER from 'main/beBuilders/cbdi/logBuilder/cbdiLogBuilderNonFlatBuffer';
import INTENTION_BUILDER from 'main/beBuilders/cbdi/intentionBuilder/cbdiIntentionBuilderNonFlatBuffer';
import { dialog, ipcMain } from 'electron';
import { compressData } from 'misc/utils/common/dataCompression/dataCompression';
import { BDILogAgentIntentionOverviews, BDILogIntentionOverviewsModel, TCBDILogsDict } from 'misc/types/cbdi/cbdi-models-non-flatbuffer';
import { createExplainabilityGraphData } from 'main/helpers/computeExpGraphData/computeExpGraphData';
import { Event } from 'misc/types/cbdi/cbdi-types-non-flatbuffer';
import playbackManager from 'main/playbackManager/playBackManager';
import path from 'path';
import fs from 'fs';
import LOGGER from 'misc/addons/logger/LoggerSingleton';
import MODEL_BUILDER from 'main/beBuilders/cbdi/cbdiModelBuilder/cbdiModelBuilderNonFlatBuffer';
import readline from 'readline';
import { LOADING_PAGE_CLOSE_BUTTON_NAME } from 'misc/constant/cbdiEdit/cbdiEditConstant';
import { InspectAgentGoal } from 'components/common/runtimeExplainability/context/explainabilityContext';
import BaseListener from '../base/BaseListener';
import { PLAN_INSPECTOR } from './planInspector/PlanInspector';

export default class CbdiListener extends BaseListener {
  protected websocketObj?: WebSocketObj;

  private _cbdiEditModel: CBDIEditorProject | null = null;

  private _broadcastModel: boolean = false;

  private _readFileStream: fs.ReadStream | null = null;

  /* --------------------------------- PRIVATE -------------------------------- */

  private onResetExplainability = (_e: Electron.IpcMainInvokeEvent) => {
    this.websocketObj?.clearBuilders();
    playbackManager.clear();
    playbackManager.goLive();
    playbackManager.runLoop((time) => {
      this._window.webContents.send(response.playback.tick, time);
    });
  };

  private onSetEditModel = (_e: Electron.IpcMainInvokeEvent, project: CBDIEditorProject | null) => {
    this._cbdiEditModel = project;
  };

  /**
   * Callback on request agent summary
   * @param _e IpcMainInvokeEvent
   * @param agentId agent id from cbdi
   * @returns summary intentions
   */
  private onRequestAgentSummaryIntentionInfo(_e: Electron.IpcMainInvokeEvent, agentId: string) {
    if (agentId && agentId.length > 0) {
      const agentSummaryIntentionInfo = INTENTION_BUILDER.getAgentSummaryIntentionInfoByAgentId(agentId);
      if (agentSummaryIntentionInfo !== undefined) {
        this._window.webContents.send(response.bdilog.agentSummaryIntentionInfo, JSON.stringify(agentSummaryIntentionInfo));
      }
    }
  }

  /**
   * Callback on request logs
   * @param _e IpcMainInvokeEvent
   * @param agentId agent id from cbdi
   * @returns bdi logs messages
   */
  private onRequestLogs = (_e: Electron.IpcMainInvokeEvent, agentId?: string) => {
    // if agentId undefined send all logs
    // let logs: BDILogEventMessage[] | TCBDILogsDict | null;
    let logs: Event[] | TCBDILogsDict | null;
    if (agentId) {
      logs = LOG_BUILDER.getAgentLogs(agentId);
    } else {
      logs = LOG_BUILDER.getAllLogs();
    }

    this._window.webContents.send(response.bdilog.logs, JSON.stringify(logs));
  };

  /**
   * Callback on request intention overviews
   * @param _e IpcMainInvokeEvent
   * @param agentId agent id from cbdi
   * @returns intention logs
   */
  private onRequestIntentionOverviews(_e: Electron.IpcMainInvokeEvent, agentId?: string, grabFresh = false) {
    // if agentId undefined send all intentions
    let intentions: BDILogIntentionOverviewsModel | BDILogAgentIntentionOverviews | null = null;
    if (agentId) {
      const changes = INTENTION_BUILDER.requestAgentIntentionOverviewsChanges(agentId, grabFresh);

      if (changes && Object.keys(changes).length > 0) {
        intentions = INTENTION_BUILDER.getAgentIntentionOverviews(agentId);
      }
    } else {
      intentions = INTENTION_BUILDER.getAllIntentionOverviews();
    }

    if (intentions) {
      const compressedData = compressData(intentions);
      this._window.webContents.send(response.bdilog.intentionOverviews, compressedData);
    }
  }

  private onGetIntentionTasksByIds(_e: Electron.IpcMainInvokeEvent, intentionIds: string[], agentId: string | undefined) {
    const intentionModel = INTENTION_BUILDER.getIntentionTasksByIds(intentionIds, agentId);
    if (intentionModel) {
      this._window.webContents.send(response.bdilog.intentionTasksByIds, JSON.stringify(intentionModel));
    }
  }

  private onRequestNodeInfo() {
    const nodeInfo = MODEL_BUILDER.getNodeInfo();

    this._window.webContents.send(response.cbdi.nodeInfo, JSON.stringify(nodeInfo));
  }

  private onAgentOrServiceUpdated() {
    if (!this._broadcastModel) {
      this._broadcastModel = true;

      setTimeout(() => {
        this._broadcastModel = false;
        const changes = MODEL_BUILDER.requestModelChanges(false);

        if (changes && Object.keys(changes).length > 0 && this._cbdiEditModel) {
          const models = MODEL_BUILDER.getModels();

          const graphData = createExplainabilityGraphData(this._cbdiEditModel, Object.values(models), INTENTION_BUILDER);

          const compressedData = compressData(graphData);

          this._window.webContents.send(response.cbdi.nodeModel, compressedData);
        }
      }, 1000);
    }
  }

  private onRequestAgentModel(
    e: Electron.IpcMainInvokeEvent,
    agentId: string,
    // path = [],
    // range?: [number, number]
  ) {
    // const data = MODEL_BUILDER.inspectAgent(agentId, path, range);

    // if (data) {
    //   const compressedData = compressData(data);
    //   this._window.webContents.send(response.cbdi.agentModel, compressedData);
    // }

    const agent = MODEL_BUILDER.getAgent(agentId);

    if (agent) {
      const compressedData = compressData(agent);
      this._window.webContents.send(response.cbdi.agentModel, compressedData);
    }
  }

  private getWsStatus() {
    this._window.webContents.send(response.websocket.status, this.websocketObj?.status());
  }

  private onDiscoverAgents() {
    if (this._cbdiEditModel) {
      const models = MODEL_BUILDER.getModels();
      const graphData = createExplainabilityGraphData(this._cbdiEditModel, Object.values(models), INTENTION_BUILDER);

      const compressedData = compressData(graphData);

      this._window.webContents.send(response.cbdi.nodeModel, compressedData);
    }
  }

  private onGetIntentionWithContextByAgentGoalId(_e: Electron.IpcMainInvokeEvent, inspectAgentGoal: InspectAgentGoal | undefined) {
    if (inspectAgentGoal && inspectAgentGoal.agentId && inspectAgentGoal.goalId) {
      const intention = INTENTION_BUILDER.getIntentionWithContextByAgentGoalId(inspectAgentGoal.agentId, inspectAgentGoal.goalId);
      this._window.webContents.send(response.cbdi.getIntentionForNotificationById, JSON.stringify(intention));
    }
  }

  /**
   * On inspected plan goal id changed (drop or change new)
   * @param _e
   * @param goalId
   */
  private onInspectPlanGoalIdChanged = (_e: Electron.IpcMainInvokeEvent, inspectAgentGoal: InspectAgentGoal | undefined) => {
    if (inspectAgentGoal && inspectAgentGoal.agentId !== undefined && inspectAgentGoal.goalId !== undefined) {
      PLAN_INSPECTOR.startInspectPlan(inspectAgentGoal.agentId, inspectAgentGoal.goalId);
      LOGGER.debug(`inspecting plan with agent id =${inspectAgentGoal?.agentId} goal id =${inspectAgentGoal?.goalId}`);
    } else {
      PLAN_INSPECTOR.stopInspectPlan();
      LOGGER.debug(`stop inspecting plan`);
    }
  };

  /**
   * On inspecting plan's intention has changed
   * @param data
   */
  private onInspectingPlanIntentionChanged = (data: string) => {
    this._window.webContents.send(response.cbdi.getIntentionForNotificationById, data);
  };

  /* -------------------------------------------------------------------------- */
  /*                                  PROTECTED                                 */
  /* -------------------------------------------------------------------------- */

  protected onWindowClose() {
    this.websocketObj?.clearBuilders();
    playbackManager.clear();
    this.closeWs();
  }

  protected onWindowReadyToShow() {
    this.websocketObj?.clearBuilders();
    playbackManager.clear();
    this.closeWs();
    this.setMainWindowId();
  }

  /**
   * Callback on close websocket
   */
  protected closeWs() {
    if (this.websocketObj) {
      this.websocketObj.stop();
      this.websocketObj.cleanup();
      this._window.webContents.send(response.window.setAppLoading, false);
    }
  }

  /**
   * Callback on start websocket
   * @param e IpcMainInvokeEvent
   * @param address (optional) ip address: string
   */
  protected startWs(_e: Electron.IpcMainInvokeEvent, address?: string) {
    if (this.websocketObj && !this.websocketObj.isConnected()) {
      this.websocketObj.start(address);
      this._window.webContents.send(
        response.window.setAppLoading,
        true, // is loading
        true, // is closable
        LOADING_PAGE_CLOSE_BUTTON_NAME.DISCONNECT_WS, // close button name
      );
    }
  }

  /**
   * Callback on send message through Websocket
   * @param _e IpcMainInvokeEvent
   * @param msg string
   */
  protected sendMsgWs(_e: Electron.IpcMainInvokeEvent, msg: string) {
    if (this.websocketObj && this.websocketObj.isConnected()) {
      this.websocketObj.send(msg);
    }
  }

  private onPlaybackOpenFile(evt: Electron.IpcMainInvokeEvent) {
    // Show loading
    this._window.webContents.send(response.window.setAppLoading, true);
    // Open file dialog
    this.openFileDialogSync(
      {
        filters: [
          { name: 'JSON', extensions: ['json'] },
          { name: 'All Files', extensions: ['*'] },
        ],
      },
      (filePaths: string[]) => {
        const filePath = filePaths[0];

        // Get file name
        const fileName = path.basename(filePath);

        // Init readfilestream
        this._readFileStream = fs.createReadStream(filePath);

        // Parsed events array
        const nonFlatbufferEvents: Event[] = [];

        // Readline interface
        const rl = readline.createInterface({
          input: this._readFileStream,
          crlfDelay: Infinity, // Handle different line endings
        });

        // Get the total file size in bytes
        const fileSize = fs.statSync(filePath).size;
        let bytesRead = 0;

        // Progress interval to send to frontend every 1s
        const progressInterval = setInterval(() => {
          const percentage = (bytesRead / fileSize) * 100;

          this._window.webContents.send(response.window.setAppLoadingPercentage, percentage.toFixed(2));
        }, 1000);

        rl.on('line', (line) => {
          // Process each line as it is read
          const nonFlatBufferEvent = JSON.parse(line) as Event;
          nonFlatbufferEvents.push(nonFlatBufferEvent);

          // Update the bytes read
          bytesRead += Buffer.from(line).length;
        });

        // On finish reading
        rl.on('close', () => {
          // Stop the progress logging interval
          clearInterval(progressInterval);

          // trycatch block to in case user load wrong file
          try {
            // Open with playback manager
            playbackManager.openRawEvts(nonFlatbufferEvents);

            // Ready to play
            this._window.webContents.send(response.playback.openFile, filePath, fileName, playbackManager.getPlaybackLength());
            // Hide loading
            this._window.webContents.send(response.window.setAppLoading, false);
            // Reset read file stream
            this._readFileStream = null;
          } catch (error: any) {
            LOGGER.error(error.stack ? error.stack : error);
            dialog.showErrorBox('Load playback file error', error.stack);
            this._window.webContents.send(response.playback.openFile);
            // Hide loading
            this._window.webContents.send(response.window.setAppLoading, false);
          }
        });

        rl.on('error', (error) => {
          // Handle any errors that occur while reading the file
          LOGGER.error('Error reading file:', error);
        });
      },
      () => {
        // Cancel to play
        this._window.webContents.send(response.playback.openFile);
        // Hide loading
        this._window.webContents.send(response.window.setAppLoading, false);
      },
    );
  }

  private onPlaybackResetFile = (evt: Electron.IpcMainInvokeEvent) => {
    playbackManager.resetPlaybackFile();
  };

  private onPlaybackPlay = () => {
    // Play
    playbackManager.play(this.sendTick.bind(this));
  };

  private onPlaybackPause = (evt: Electron.IpcMainInvokeEvent) => {
    // Pause
    playbackManager.pause();
  };

  /**
   * On go to live callback
   * @param evt
   */
  private onPlayBackGoLive = (evt: Electron.IpcMainInvokeEvent) => {
    this._window.webContents.send(response.window.setAppLoading, true);

    LOGGER.info('Requesting data from ws thread');

    playbackManager.pause();

    // https://gitlab.aosgrp.net/applications/aewcf/-/issues/975
    // Get event data from ws thread
    // The mainthread will wait for data callback to proceed
    // this.websocketObj?.getDataFromWsThread(playbackManager.previousRecordIndex());
  };

  /**
   * Callback when data from ws thread is available
   * @param data
   */
  private onGetEventDataFromWsThread = (data: Event[]) => {
    LOGGER.info('Received data from ws thread', data.length);

    // Set records from ws thread to playback manager
    playbackManager.setRecords(data);

    // Live
    playbackManager.goLive();
    playbackManager.runLoop((time) => {
      this._window.webContents.send(response.playback.tick, time);
    });
    this._window.webContents.send(response.playback.goLive);
    this._window.webContents.send(response.window.setAppLoading, false);
  };

  private onPlaybackStop = (evt: Electron.IpcMainInvokeEvent) => {
    // Stop
    playbackManager.stop();
    playbackManager.clearModel();
  };

  private onPlaybackGoToFrame = (evt: Electron.IpcMainInvokeEvent, timeframe: number) => {
    // Show loading
    this._window.webContents.send(response.window.setAppLoading, true);

    // Save previous index
    playbackManager.setPreviousRecordIndex();

    // Go to timeframe
    playbackManager.goToFrame(timeframe);

    // Check inspecting intention
    const inspectingIntentionStarted = INTENTION_BUILDER.isInspectingIntentionStarted();
    if (!inspectingIntentionStarted) {
      this._window.webContents.send(response.playback.inspectingIntentionUnavailable);
    }

    // Hide loading
    this._window.webContents.send(response.window.setAppLoading, false);

    // Send response as go to frame success
    this._window.webContents.send(response.playback.goToFrame);
  };

  private sendTick(tick: number) {
    this._window.webContents.send(response.playback.tick, tick);
  }

  /**
   * On client switches XAI Live Playback debug mode
   * @param evt
   * @param xaiLivePlayback
   */
  private onSwitchXaiLivePlayback = (evt: Electron.IpcMainInvokeEvent, xaiLivePlayback: boolean) => {
    // switch xai live playback
    playbackManager.setLiveDebug(xaiLivePlayback);

    this._window.webContents.send(response.playback.switchXaiLivePlayback, xaiLivePlayback);
  };

  private onDebugStart = () => {
    playbackManager.pause();
  };

  private onDebugStep = () => {
    playbackManager.trace(this.sendTick.bind(this));
  };

  private onDebugExit = () => {
    playbackManager.stopTrace();
  };

  private onDebugAuto = () => {
    playbackManager.debugAuto(this.sendTick.bind(this));
  };

  private onDebugAutoPause = () => {
    playbackManager.pauseDebugAuto();
  };

  private onCommandFrontendPausePlayback = () => {
    this._window.webContents.send(response.playback.commandFrontendPause);
  };

  registerEventHandlers() {
    super.registerEventHandlers();

    /* ------------------------- Websocket status events ------------------------ */
    ipcMain.handle(request.websocket.connect, this.startWs.bind(this));
    ipcMain.handle(request.websocket.disconnect, this.closeWs.bind(this));
    ipcMain.handle(request.websocket.send, this.sendMsgWs.bind(this));
    ipcMain.handle(request.websocket.status, this.getWsStatus.bind(this));

    /* ------------------------- CBDI edit model events ------------------------- */
    ipcMain.handle(request.cbdi.setEditModel, this.onSetEditModel.bind(this));

    /* ---------------------- Builders data request events ---------------------- */
    ipcMain.handle(request.cbdi.reset, this.onResetExplainability.bind(this));
    ipcMain.handle(request.bdilog.agentSummaryIntentionInfo, this.onRequestAgentSummaryIntentionInfo.bind(this));
    ipcMain.handle(request.bdilog.logs, this.onRequestLogs.bind(this));
    ipcMain.handle(request.bdilog.intentionOverviews, this.onRequestIntentionOverviews.bind(this));
    ipcMain.handle(request.bdilog.intentionTasksByIds, this.onGetIntentionTasksByIds.bind(this));
    ipcMain.handle(request.cbdi.nodeInfo, this.onRequestNodeInfo.bind(this));
    ipcMain.handle(request.cbdi.agentModel, this.onRequestAgentModel.bind(this));
    ipcMain.handle(request.cbdi.discoverModels, this.onDiscoverAgents.bind(this));
    ipcMain.handle(request.cbdi.getIntentionByAgentGoalId, this.onGetIntentionWithContextByAgentGoalId.bind(this));
    ipcMain.handle(request.cbdi.inspectPlanGoalIdChanged, this.onInspectPlanGoalIdChanged.bind(this));
    /* --------------------------- Event listener -------------------------- */
    ipcMain.addListener(eventListeners.cbdi.modelUpdated, this.onAgentOrServiceUpdated.bind(this));
    ipcMain.addListener(eventListeners.cbdi.nodeInfo, this.onRequestNodeInfo.bind(this));
    ipcMain.addListener(eventListeners.cbdi.inspectedPlanIntentionChanged, this.onInspectingPlanIntentionChanged.bind(this));
    ipcMain.addListener(eventListeners.playback.commandFrontendPausePlayback, this.onCommandFrontendPausePlayback.bind(this));
    // https://gitlab.aosgrp.net/applications/aewcf/-/issues/975
    // ipcMain.addListener('get-event-data-from-ws-thread', this.onGetEventDataFromWsThread.bind(this));

    /* -------------------------------- Playback -------------------------------- */
    ipcMain.handle(request.playback.openFile, this.onPlaybackOpenFile.bind(this));
    ipcMain.handle(request.playback.resetFile, this.onPlaybackResetFile.bind(this));
    ipcMain.handle(request.playback.play, this.onPlaybackPlay.bind(this));
    ipcMain.handle(request.playback.pause, this.onPlaybackPause.bind(this));
    ipcMain.handle(request.playback.stop, this.onPlaybackStop.bind(this));
    ipcMain.handle(request.playback.goToFrame, this.onPlaybackGoToFrame.bind(this));
    ipcMain.handle(request.playback.goLive, this.onPlayBackGoLive.bind(this));
    ipcMain.handle(request.playback.switchXaiLivePlayback, this.onSwitchXaiLivePlayback.bind(this));
    ipcMain.handle(request.playback.debug.enter, this.onDebugStart.bind(this));
    ipcMain.handle(request.playback.debug.stepNext, this.onDebugStep.bind(this));
    ipcMain.handle(request.playback.debug.exit, this.onDebugExit.bind(this));
    ipcMain.handle(request.playback.debug.autoPlay, this.onDebugAuto.bind(this));
    ipcMain.handle(request.playback.debug.autoPlayPause, this.onDebugAutoPause.bind(this));

    /* ----------------- This is to close connection on refresh ----------------- */
    this._window.on('close', this.onWindowClose.bind(this));
    this._window.on('ready-to-show', this.onWindowReadyToShow.bind(this));
  }

  /**
   * Deregister event handlers
   */
  deregisterEventHandlers() {
    super.deregisterEventHandlers();

    /* ------------------------- Websocket status events ------------------------ */
    ipcMain.removeHandler(request.websocket.connect);
    ipcMain.removeHandler(request.websocket.disconnect);
    ipcMain.removeHandler(request.websocket.send);
    ipcMain.removeHandler(request.websocket.status);

    /* ------------------------- CBDI edit model events ------------------------- */
    ipcMain.removeHandler(request.cbdi.setEditModel);

    /* ---------------------- Builders data request events ---------------------- */
    ipcMain.removeHandler(request.cbdi.reset);
    ipcMain.removeHandler(request.bdilog.logs);
    ipcMain.removeHandler(request.bdilog.intentionOverviews);
    ipcMain.removeHandler(request.bdilog.intentionTasksByIds);
    ipcMain.removeHandler(request.bdilog.agentSummaryIntentionInfo);
    ipcMain.removeHandler(request.cbdi.nodeInfo);
    ipcMain.removeHandler(request.cbdi.agentModel);
    ipcMain.removeHandler(request.cbdi.discoverModels);
    ipcMain.removeHandler(request.cbdi.getIntentionByAgentGoalId);
    ipcMain.removeHandler(request.cbdi.inspectPlanGoalIdChanged);
    // https://gitlab.aosgrp.net/applications/aewcf/-/issues/975
    // ipcMain.removeHandler('get-event-data-from-ws-thread');

    /* --------------------------- Event listener -------------------------- */
    ipcMain.removeListener(eventListeners.cbdi.modelUpdated, this.onAgentOrServiceUpdated.bind(this));
    ipcMain.removeListener(eventListeners.cbdi.nodeInfo, this.onRequestNodeInfo.bind(this));
    ipcMain.removeListener(eventListeners.cbdi.inspectedPlanIntentionChanged, this.onInspectingPlanIntentionChanged.bind(this));
    ipcMain.removeListener(eventListeners.playback.commandFrontendPausePlayback, this.onCommandFrontendPausePlayback.bind(this));

    /* -------------------------------- Playback -------------------------------- */
    ipcMain.removeHandler(request.playback.openFile);
    ipcMain.removeHandler(request.playback.resetFile);
    ipcMain.removeHandler(request.playback.play);
    ipcMain.removeHandler(request.playback.pause);
    ipcMain.removeHandler(request.playback.stop);
    ipcMain.removeHandler(request.playback.goToFrame);
    ipcMain.removeHandler(request.playback.goLive);
    ipcMain.removeHandler(request.playback.switchXaiLivePlayback);
    ipcMain.removeHandler(request.playback.debug.enter);
    ipcMain.removeHandler(request.playback.debug.stepNext);
    ipcMain.removeHandler(request.playback.debug.exit);
    ipcMain.removeHandler(request.playback.debug.autoPlay);
    ipcMain.removeHandler(request.playback.debug.autoPlayPause);

    /* ----------------- This is to close connection on refresh ----------------- */
    this._window.off('close', this.onWindowClose.bind(this));
    this._window.off('ready-to-show', this.onWindowReadyToShow.bind(this));
  }
}
