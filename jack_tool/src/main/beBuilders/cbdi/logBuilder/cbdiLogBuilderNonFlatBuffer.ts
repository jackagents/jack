// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

import { TCBDILogsDict } from 'misc/types/cbdi/cbdi-models-non-flatbuffer';
import { Event, EventType } from 'misc/types/cbdi/cbdi-types-non-flatbuffer';
import { BaseBuilder } from 'main/beBuilders/baseBuilder/BaseBuilder';
import LOGGER from 'misc/addons/logger/LoggerSingleton';

// TODO: This needs to be improved as it does not do a lot of processing right now
export class CBDILogBuilder extends BaseBuilder {
  // private _logs: TCBDILogsDict;
  private _logs: TCBDILogsDict;

  /* -------------------------------------------------------------------------- */
  /*                                 CONSTRUCTOR                                */
  /* -------------------------------------------------------------------------- */

  constructor(playback = false) {
    super('CBDILogBuilder', playback);
    this._logs = {};
  }

  /* -------------------------------------------------------------------------- */
  /*                                   PRIVATE                                  */
  /* -------------------------------------------------------------------------- */

  /* -------------------------------------------------------------------------- */
  /*                                   PUBLIC                                   */
  /* -------------------------------------------------------------------------- */

  clear() {
    this._logs = {};
  }

  createSnapshot() {
    return JSON.stringify({
      logs: this._logs,
    });
  }

  restoreSnapshot(snapshotString: string) {
    this.clear();
    const { logs } = JSON.parse(snapshotString) as {
      logs: TCBDILogsDict;
    };

    this._logs = logs;
  }

  buildNonFlatBuffer(data: Event) {
    const { type } = data;

    if (type !== EventType.BDI_LOG) {
      return;
    }

    const { id } = data.sender;

    // Check if this agent key existed in logs
    if (!Object.keys(this._logs).includes(id)) {
      this._logs[id] = [];
    }

    // Add log to array
    this._logs[id].push(data);
  }

  getAgentLogs(id: string) {
    if (this._logs[id] === undefined) {
      LOGGER.info('Cannot find logs of', id);
      return null;
    }
    // sort by latest first then return the logs
    return this._logs[id]
      .sort((a, b) => b.timestampUs - a.timestampUs)
      .slice(0, 100);
  }

  getAllLogs() {
    return this._logs;
  }
}

const LOG_BUILDER = new CBDILogBuilder();
export default LOG_BUILDER;
