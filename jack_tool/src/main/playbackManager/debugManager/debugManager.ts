import { PLAN_INSPECTOR } from 'main/listeners/cbdi/planInspector/PlanInspector';
import {
  BDILog,
  BDILogType,
  Event,
  EventType,
} from 'misc/types/cbdi/cbdi-types-non-flatbuffer';

const MIN_COOLDOWN = 1; // In second

enum DebugState {
  INACTIVATED = 'inactivated',
  ACTIVATED = 'activated',
}

export class DebugManager {
  private debugState = DebugState.INACTIVATED;

  private timerCounter: number;

  public CurrentEventIndex: number;

  public isAutoRun = false;

  constructor() {
    this.timerCounter = 0;
    this.CurrentEventIndex = 0;
    this.isAutoRun = false;
  }

  private _isBdiLog = (evt: Event): evt is BDILog => {
    return evt.type === EventType.BDI_LOG;
  };

  activate() {
    this.debugState = DebugState.ACTIVATED;
    return this;
  }

  deactivate() {
    this.debugState = DebugState.INACTIVATED;
    return this;
  }

  isActivated() {
    return this.debugState === DebugState.ACTIVATED;
  }

  updateDebugCounter(deltaTime: number) {
    this.timerCounter += deltaTime;
  }

  isCoolingDown() {
    return this.timerCounter < MIN_COOLDOWN;
  }

  resetCounter() {
    // Reset timerCounter
    this.timerCounter = 0;
  }

  reset() {
    this.timerCounter = 0;
    this.CurrentEventIndex = 0;
    this.isAutoRun = false;
  }

  isDebugBreakPoint(record: Event) {
    if (
      this._isBdiLog(record) &&
      record.goalId === PLAN_INSPECTOR.getInspectingPlanGoalId() &&
      // Step by task
      // TODO: Missing subgoal
      (record.logType === BDILogType.INTENTION_STARTED ||
        record.logType === BDILogType.ACTION_FINISHED ||
        record.logType === BDILogType.SUB_GOAL_FINISHED ||
        record.logType === BDILogType.CONDITION ||
        record.logType === BDILogType.SLEEP_FINISHED ||
        record.logType === BDILogType.INTENTION_FINISHED)
    ) {
      return true;
    }

    return false;
  }
}
