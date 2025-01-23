import LOGGER from 'addons/logger/LoggerSingleton';
import { BaseBuilder } from 'main/beBuilders/baseBuilder/BaseBuilder';
import {
  BDILogIntentionTask,
  BDILogAgentIntentions,
  BDILogIntentionModel,
  BDILogIntentionsModel,
  TCBDIIntentionForNotification,
  GoalInfoItem,
  AgentSummaryGoalInfo,
  BDILogAgentIntentionOverviews,
  BDILogIntentionOverviewsModel,
  BDILogAgentGoals,
  ReasoningItem,
} from 'misc/types/cbdi/cbdi-models-non-flatbuffer';
import {
  BDILogBody,
  BDILogGoalBody,
  BDILogGoalIntentionResult,
  BDILogLevel,
  BDILogType,
  Event,
  EventType,
  BDILogIntentionBody,
  BDILogActionBody,
  BDILogSleepBody,
  BDILogConditionBody,
  MessageData,
} from 'misc/types/cbdi/cbdi-types-non-flatbuffer';
import MODEL_BUILDER from 'main/beBuilders/cbdi/cbdiModelBuilder/cbdiModelBuilderNonFlatBuffer';
import { TaskStatus } from 'misc/constant/common/cmConstants';
import { PLAN_INSPECTOR } from 'main/listeners/cbdi/planInspector/PlanInspector';
import { ipcMain } from 'electron';
import { eventListeners } from 'misc/events/common/cmEvents';
import { getHighestLevel } from './getHighestLevelUtil/getHighestLevel';

const TASK_LIMIT_PER_INTENTION = 64;

// TODO: Check if in the agent, are there any goals/desires that are started,
// but there are no plans to do it or waiting to do...
// pendingGoals and droppedGoals
export class CBDIIntentionBuilder extends BaseBuilder {
  /**
   * Intentions records
   */
  private _intentions: BDILogAgentIntentions;

  /**
   * Goals/desires records
   */
  private _agentGoals: BDILogAgentGoals;

  private _intentionOverviews: BDILogAgentIntentionOverviews;

  private _originalAgentIntentionOverviews: BDILogIntentionOverviewsModel | null = null;

  /* -------------------------------------------------------------------------- */
  /*                                 CONSTRUCTOR                                */
  /* -------------------------------------------------------------------------- */

  constructor(playback = false) {
    super('CBDIIntentionBuilder', playback);
    this._intentions = {};
    this._agentGoals = {};
    this._intentionOverviews = {};
  }

  /* -------------------------------------------------------------------------- */
  /*                                   PRIVATE                                  */
  /* -------------------------------------------------------------------------- */
  /**
   * Generate task from goal
   * @param agentId string agent name
   * @param payload BDILogGoal
   * @param bdiLog BDILog
   * @param timestampUs number
   * @returns BDILogIntentionModel
   */
  private _getTaskFromGoal = (agentId: string, payload: BDILogGoalBody, bdiLog: BDILogBody, timestampUs: number) => {
    const { goalId, goal, result: goalResult, intentionId, taskId, dropReason } = payload;
    const { level, logType } = bdiLog;

    const intention = this._intentions[agentId][intentionId];

    this._processDesires(agentId, logType, bdiLog as BDILogGoalBody);

    const task: BDILogIntentionTask = {
      goal,
      goalId,
      logType,
      level,
      timestamp: timestampUs,
      goalResult,
      taskId,
      dropReason,
    };

    const result: BDILogIntentionModel | null = {
      ...intention,
      tasks: [...intention.tasks, task],
      level: getHighestLevel(bdiLog.level, intention.level),
    };

    result.tasks.sort((a, b) => a.timestamp - b.timestamp);

    return result;
  };

  private _findKeyWithLargestTimestamp = (obj: Record<string, number>): string | null => {
    let largestTimestamp = -Infinity;
    let largestKey: string | null = null;
    Object.keys(obj).forEach((key) => {
      if (Object.prototype.hasOwnProperty.call(obj, key)) {
        const timestamp = obj[key];
        if (timestamp > largestTimestamp) {
          largestTimestamp = timestamp;
          largestKey = key;
        }
      }
    });

    return largestKey;
  };

  // TODO: Maybe do this in a different builder
  /**
   * Process goals/desires and put it in _agentGoals
   */
  private _processDesires(agentId: string, logType: BDILogType, goalBody: BDILogGoalBody) {
    switch (logType) {
      case BDILogType.GOAL_STARTED:
      case BDILogType.SUB_GOAL_STARTED:
        this._agentGoals[agentId][goalBody.goalId] = {
          goalId: goalBody.goalId,
          goalTemplateName: goalBody.goal,
          goalFinished: false,
          level: BDILogLevel.NORMAL,
          reasoningDic: {},
          allIntentions: [],
        };
        break;

      case BDILogType.GOAL_FINISHED:
      case BDILogType.SUB_GOAL_FINISHED:
        if (this._agentGoals[agentId][goalBody.goalId] === undefined) {
          this._agentGoals[agentId][goalBody.goalId] = {
            goalId: goalBody.goalId,
            goalTemplateName: goalBody.goal,
            goalFinished: true,
            level: BDILogLevel.NORMAL,
            reasoningDic: {},
            allIntentions: [],
          };
        } else {
          this._agentGoals[agentId][goalBody.goalId].goalFinished = true;
          // TODO: Because cbdi reuse the intention id, for each intention goal id
          // all intentions will only keep the latest intention for a intention id
          // later need to either not reuse intention id or store multiple intention sharing same intention id
          this._agentGoals[agentId][goalBody.goalId].latestIntentionWithHighestLevel =
            this._agentGoals[agentId][goalBody.goalId].allIntentions.reverse()[0];
          this._agentGoals[agentId][goalBody.goalId].goalResult = goalBody.result;
        }
        break;
      default:
        break;
    }
  }

  getTaskReasoningText = (task: BDILogIntentionTask) => {
    if (
      // task.logType === BDILogType.ACTION_STARTED ||
      task.logType === BDILogType.ACTION_FINISHED
    ) {
      return task.reasoning;
    }
    if (
      // task.logType === BDILogType.SUB_GOAL_STARTED ||
      task.logType === BDILogType.SUB_GOAL_FINISHED
    ) {
      if (!task.dropReason) {
        return undefined;
      }
      return `Drop reason: ${task.dropReason}`;
    }
    return undefined;
  };

  /**
   * Update _agentGoals with agentId and intentionModel
   * @param agentId
   * @param intentionModel
   */
  private _updateAgentGoalWithIntention(agentId: string, intentionModel: BDILogIntentionModel) {
    if (this._agentGoals[agentId][intentionModel.goalId] === undefined) {
      LOGGER.error('something wrong');
      return false;
    }

    const currentGoalIntentionItem = {
      ...this._agentGoals[agentId][intentionModel.goalId],
    };
    const existingIntentionModelArr = [...this._agentGoals[agentId][intentionModel.goalId].allIntentions];

    const reasoningItem: ReasoningItem = {};
    intentionModel.tasks.forEach((el) => {
      const reasoningText = this.getTaskReasoningText(el);
      if (reasoningText) {
        reasoningItem[el.taskId] = {
          reasoningText,
          level: el.level,
        };
      }
    });
    if (Object.keys(reasoningItem).length > 0) {
      currentGoalIntentionItem.reasoningDic[intentionModel.intentionId] = {
        ...currentGoalIntentionItem.reasoningDic[intentionModel.intentionId],
        ...reasoningItem,
      };
    }

    const index = existingIntentionModelArr.findIndex((el) => el.intentionId === intentionModel.intentionId);

    const level = getHighestLevel(intentionModel.level, currentGoalIntentionItem.level);
    currentGoalIntentionItem.level = level;
    if (index < 0) {
      existingIntentionModelArr.push(intentionModel);
    } else {
      existingIntentionModelArr[index] = intentionModel;
    }
    this._agentGoals[agentId][intentionModel.goalId] = {
      ...currentGoalIntentionItem,
      allIntentions: existingIntentionModelArr,
    };

    return true;
  }

  /* -------------------------------------------------------------------------- */
  /*                                   PUBLIC                                   */
  /* -------------------------------------------------------------------------- */

  clear() {
    this._intentions = {};
    this._agentGoals = {};
    this._intentionOverviews = {};
  }

  createSnapshot() {
    return JSON.stringify({
      intentions: this._intentions,
      agentGoals: this._agentGoals,
      intentionOverviews: this._intentionOverviews,
      originalAgentIntentionOverviews: this._originalAgentIntentionOverviews,
    });
  }

  restoreSnapshot(snapshotString: string) {
    this.clear();
    const { intentions, agentGoals, intentionOverviews, originalAgentIntentionOverviews } = JSON.parse(snapshotString) as {
      intentions: BDILogAgentIntentions;
      agentGoals: BDILogAgentGoals;
      intentionOverviews: BDILogAgentIntentionOverviews;
      originalAgentIntentionOverviews: BDILogIntentionOverviewsModel | null;
    };

    this._intentions = intentions;
    this._agentGoals = agentGoals;
    this._intentionOverviews = intentionOverviews;
    this._originalAgentIntentionOverviews = originalAgentIntentionOverviews;
  }

  buildNonFlatBuffer = (data: Event) => {
    const { eventId, recipient, sender, senderNode, timestampUs, type, ...bdiLog } = data;

    if (type !== EventType.BDI_LOG) {
      return this;
    }

    const { id: agentId } = sender;

    // Create dictionary
    if (!this._intentions[agentId]) {
      this._intentions[agentId] = {};
    }
    if (this._intentionOverviews[agentId] === undefined) {
      this._intentionOverviews[agentId] = {};
    }
    if (this._agentGoals[agentId] === undefined) {
      this._agentGoals[agentId] = {};
    }
    // Parse body to BDILogEvent
    const { logType, level, ...payload } = bdiLog as BDILogBody;

    if (!payload) {
      throw new Error('Payload is null or undefined');
    }

    const { intentionId, goal, goalId } = payload;

    // Goal started / finished do not have intention id, goalId of these events should be identical to goalId of intentionStarted
    if (logType === BDILogType.GOAL_STARTED || logType === BDILogType.GOAL_FINISHED) {
      // Add to agentGoals
      this._processDesires(agentId, logType, bdiLog as BDILogGoalBody);

      return this;
    }

    if (!intentionId) {
      LOGGER.error('IntentionBuilder: Message is ignored as intentionId is empty');
      return this;
    }

    let intentionModel: BDILogIntentionModel | null = null;

    if (logType === BDILogType.INTENTION_STARTED) {
      const { plan } = payload as BDILogIntentionBody;

      const task: BDILogIntentionTask = {
        goal,
        goalId,
        plan,
        logType,
        level,
        timestamp: timestampUs,
        taskId: 'start',
      };

      intentionModel = {
        startingTimestamp: timestampUs,
        planTemplateName: plan,
        tasks: [task],
        level,
        goalId,
        intentionId,
      };

      // implement intention overviews
      this._intentionOverviews[agentId][intentionId] = {
        goalId,
        level,
        startingTimestamp: timestampUs,
        planTemplateName: plan,
        goalTemplateName: goal,
        intentionStatus: TaskStatus.STARTED,
      };
    }
    // Not intention started
    else {
      if (!this._intentions[agentId][intentionId]) {
        LOGGER.error(`agent ${agentId} does not have ${intentionId} at timestamp ${timestampUs}`);
        return this;
      }

      switch (logType) {
        case BDILogType.INTENTION_FINISHED: {
          const { plan, result: intentionResult } = payload as BDILogIntentionBody;

          const task: BDILogIntentionTask = {
            goal,
            goalId,
            plan,
            logType,
            level,
            timestamp: timestampUs,
            intentionResult,
            taskId: 'end',
          };

          const intention = this._intentions[agentId][intentionId];

          intentionModel = {
            ...intention,
            tasks: [...intention.tasks, task],
            level: getHighestLevel(level, intention.level),
          };

          break;
        }

        case BDILogType.ACTION_STARTED: {
          const { plan, action, reasoning, taskId } = payload as BDILogActionBody;

          const task: BDILogIntentionTask = {
            goal,
            goalId,
            plan,
            logType,
            level,
            timestamp: timestampUs,
            action,
            reasoning,
            taskId,
          };

          const intention = this._intentions[agentId][intentionId];

          intentionModel = {
            ...intention,
            tasks: [...intention.tasks, task],
            level: getHighestLevel(level, intention.level),
          };

          break;
        }

        case BDILogType.ACTION_FINISHED: {
          const { plan, action, reasoning, success, taskId } = payload as BDILogActionBody;

          const task: BDILogIntentionTask = {
            goal,
            goalId,
            plan,
            logType,
            level,
            timestamp: timestampUs,
            action,
            actionSuccess: success,
            reasoning,
            taskId,
          };

          const intention = this._intentions[agentId][intentionId];

          intentionModel = {
            ...intention,
            tasks: [...intention.tasks, task],
            level: getHighestLevel(level, intention.level),
          };

          break;
        }

        case BDILogType.SLEEP_STARTED: {
          const { plan, sleepMs, taskId } = payload as BDILogSleepBody;

          const task: BDILogIntentionTask = {
            goal,
            goalId,
            plan,
            logType,
            level,
            timestamp: timestampUs,
            sleepMs,
            taskId,
          };

          const intention = this._intentions[agentId][intentionId];

          intentionModel = {
            ...intention,
            tasks: [...intention.tasks, task],
            level: getHighestLevel(level, intention.level),
          };

          break;
        }

        case BDILogType.SLEEP_FINISHED: {
          const { plan, sleepMs, taskId } = payload as BDILogSleepBody;

          const task: BDILogIntentionTask = {
            goal,
            goalId,
            plan,
            logType,
            level,
            timestamp: timestampUs,
            sleepMs,
            taskId,
          };

          const intention = this._intentions[agentId][intentionId];

          intentionModel = {
            ...intention,
            tasks: [...intention.tasks, task],
            level: getHighestLevel(level, intention.level),
          };
          break;
        }

        case BDILogType.CONDITION: {
          const { plan, condition, success: conditionSuccess, taskId } = payload as BDILogConditionBody;

          const task: BDILogIntentionTask = {
            goal,
            goalId,
            plan,
            logType,
            level,
            timestamp: timestampUs,
            condition,
            conditionSuccess,
            taskId,
          };

          const intention = this._intentions[agentId][intentionId];

          intentionModel = {
            ...intention,
            tasks: [...intention.tasks, task],
            level: getHighestLevel(level, intention.level),
          };
          break;
        }

        case BDILogType.SUB_GOAL_STARTED: {
          // LOGGER.debug(JSON.stringify(data));
          // Parent goal id = intention goal id

          intentionModel = this._getTaskFromGoal(agentId, payload as BDILogGoalBody, bdiLog as BDILogBody, timestampUs);

          break;
        }

        case BDILogType.SUB_GOAL_FINISHED: {
          intentionModel = this._getTaskFromGoal(agentId, payload as BDILogGoalBody, bdiLog as BDILogBody, timestampUs);
          break;
        }

        default:
          break;
      }
    }

    if (intentionId && intentionModel) {
      // update intention id
      this._intentions[agentId][intentionId] = intentionModel;

      // update agent goal
      if (!this._updateAgentGoalWithIntention(agentId, intentionModel)) {
        LOGGER.error('updateAgentGoalWithIntention failed', JSON.stringify(data));
      }

      // update intention overview
      const lastTask = intentionModel.tasks[intentionModel.tasks.length - 1];

      if (lastTask) {
        const intentionStatus = (() => {
          switch (lastTask.intentionResult) {
            case BDILogGoalIntentionResult.FAILED:
              return TaskStatus.FAILED;
            case BDILogGoalIntentionResult.SUCCESS:
              return TaskStatus.SUCCESS;
            case BDILogGoalIntentionResult.DROPPED:
              return TaskStatus.DROPPED;
            default:
              return TaskStatus.STARTED;
          }
        })();

        this._intentionOverviews[agentId][intentionId].intentionStatus = intentionStatus;
        this._intentionOverviews[agentId][intentionId].level = intentionModel.level;
      }
      // TODO: Rename function/ change comments when feature has changed
      // TODO: Optimize as this is too many messages
      // Check if intention is being inspected to send updates to frontend
      if (
        PLAN_INSPECTOR.isIntentionBeingInspected(agentId, intentionModel.goalId) &&
        (logType === BDILogType.INTENTION_STARTED ||
          logType === BDILogType.ACTION_FINISHED ||
          logType === BDILogType.SUB_GOAL_FINISHED ||
          logType === BDILogType.CONDITION ||
          logType === BDILogType.SLEEP_FINISHED ||
          logType === BDILogType.INTENTION_FINISHED)
      ) {
        ipcMain.emit(
          eventListeners.cbdi.inspectedPlanIntentionChanged,
          JSON.stringify(this.getIntentionWithContextByAgentGoalId(agentId, intentionModel.goalId)),
        );
      }
    }

    return this;
  };

  /**
   * Get intention overviews
   * @param agentId string
   * @returns BDILogIntentionOverview
   */
  getAgentIntentionOverviews = (agentId: string) => {
    if (this._intentionOverviews[agentId]) {
      const keys = Object.keys(this._intentions[agentId]);
      const result: BDILogIntentionOverviewsModel = {};

      // reservese order latest first
      keys.sort((a, b) => this._intentions[agentId][b].startingTimestamp - this._intentions[agentId][a].startingTimestamp);

      for (let i = 0; i < keys.length; i++) {
        const intentionId = keys[i];

        result[intentionId] = this._intentionOverviews[agentId][intentionId];
      }

      return result;
    }

    LOGGER.info('Cannot find intention overveiws of', agentId);
    return null;
  };

  requestAgentIntentionOverviewsChanges(agentId: string, grabFresh: boolean) {
    if (grabFresh) {
      this._originalAgentIntentionOverviews = null;
    }

    if (agentId in this._intentionOverviews) {
      const memoryAgentIntentionOverviews = this._intentionOverviews[agentId];

      if (this._originalAgentIntentionOverviews) {
        const changes = this.calculateDifferences(this._originalAgentIntentionOverviews, memoryAgentIntentionOverviews);

        this._originalAgentIntentionOverviews = JSON.parse(JSON.stringify(memoryAgentIntentionOverviews));

        return changes;
      }

      this._originalAgentIntentionOverviews = JSON.parse(JSON.stringify(memoryAgentIntentionOverviews));

      return memoryAgentIntentionOverviews;
    }

    return null;
  }

  /**
   * Get intention by intentionId
   * @param intentionId string
   * @returns null | {intentionId: BDILogIntentionModel}
   */
  getIntentionById = (intentionId: string) => {
    // Get all intentions
    const intentions = Object.values(this._intentions);

    // Scan through intentions
    for (let i = 0; i < intentions.length; i++) {
      const intentionObj = intentions[i];

      // Check for intention id
      if (intentionId in intentionObj) {
        // const type = agentId.toLowerCase().includes('team') ? 'team' : 'agent';
        const intention = intentionObj[intentionId];

        return intention;
      }
    }

    // Return null if not found
    LOGGER.info('cannot find intention with intentionId=[', intentionId, ']');
    return null;
  };

  /**
   * Get intention with context by agent Id and goal Id
   * @param agentId string
   * @param goalId string
   * @returns TCBDIIntentionForNotification | null
   */
  getIntentionWithContextByAgentGoalId = (agentId: string, goalId: string) => {
    const goalIntentionObj = this._agentGoals[agentId];
    if (goalIntentionObj) {
      const agent = MODEL_BUILDER.getAgent(agentId);
      const goalIntentionItem = goalIntentionObj[goalId];
      if (goalIntentionItem) {
        let goalContextMsg: MessageData[] | undefined;
        const pursues = MODEL_BUILDER.getPursueGoalContext(agentId, goalId);

        if (pursues && pursues.length > 0) {
          // TODO: BUG when change progress bar the purses can be duplicated items in array
          // Deduplicate the array
          const uniqueArray = Array.from(new Set(pursues.map((el) => JSON.stringify(el)))).map((el) => JSON.parse(el));
          goalContextMsg = uniqueArray.map((x) => x.message);
        }

        // if goal is finished, return the latest intention with highest level
        if (goalIntentionItem.goalFinished) {
          const intentionWithHightestLevel = goalIntentionItem.latestIntentionWithHighestLevel;
          return {
            intentionData: intentionWithHightestLevel,
            goalId,
            reasoningDic: goalIntentionItem.reasoningDic,
            goalContextMsg,
            planTemplateName: intentionWithHightestLevel?.planTemplateName,
            goalTemplateName: goalIntentionItem.goalTemplateName,
            agentAddress: agent?.address,
          } as TCBDIIntentionForNotification;
        }

        // if goal is not finished, return the latest intention
        const lastItention: BDILogIntentionModel | undefined = goalIntentionItem.allIntentions.slice(-1)[0];
        return {
          intentionData: lastItention,
          goalId,
          reasoningDic: goalIntentionItem.reasoningDic,
          goalContextMsg,
          planTemplateName: lastItention?.planTemplateName,
          goalTemplateName: goalIntentionItem.goalTemplateName,
          agentAddress: agent?.address,
        } as TCBDIIntentionForNotification;
      }
    }

    // Return null if not found
    LOGGER.info('cannot find intention with agentId=[', agentId, '] goalId=[', goalId, ']');
    return null;
  };

  /**
   * Get intention tasks by intentionId
   * @param intentionIds string[]
   * @returns null | {intentionTasks: BDILogIntentionTask[]}
   */
  getIntentionTasksByIds = (intentionIds: string[], agentId: string | undefined) => {
    if (agentId) {
      // Get all intentions
      const intentions = this._intentions[agentId];
      if (intentions) {
        const intentionModel: BDILogIntentionsModel = {};

        intentionIds.forEach((intentionId) => {
          if (intentions[intentionId].tasks.length > TASK_LIMIT_PER_INTENTION) {
            intentions[intentionId].tasks.splice(0, intentions[intentionId].tasks.length - TASK_LIMIT_PER_INTENTION);
          }

          intentionModel[intentionId] = intentions[intentionId];
        });

        return intentionModel;
      }
    }
    // Return null if not found
    LOGGER.info('cannot find intention tasks with intentionIds=', intentionIds);
    return null;
  };

  /**
   * Get agent summary intentions info by agent Id
   * @param agentId string
   */
  getAgentSummaryIntentionInfoByAgentId = (agentId: string): AgentSummaryGoalInfo | undefined => {
    const agentBeliefsets = MODEL_BUILDER.getAgent(agentId)?.beliefSets;
    const goals = this._agentGoals[agentId];
    if (goals !== undefined) {
      const currentGoals: GoalInfoItem[] = [];
      const recentImportantGoals: GoalInfoItem[] = [];
      Object.entries(goals).forEach(([goalId, goalIntentionItem]) => {
        let goalContextMsg: MessageData[] | undefined;
        const pursues = MODEL_BUILDER.getPursueGoalContext(agentId, goalId);

        if (pursues && pursues.length > 0) {
          // TODO: BUG when change progress bar the purses can be duplicated items in array
          // Deduplicate the array
          const uniqueArray = Array.from(new Set(pursues.map((el) => JSON.stringify(el)))).map((el) => JSON.parse(el));
          goalContextMsg = uniqueArray.map((x) => x.message);
        }

        // if goal is finished and level is more than normal, put into recent important goals
        if (goalIntentionItem.goalFinished && goalIntentionItem.level !== BDILogLevel.NORMAL) {
          const intentionFinishedTask = goalIntentionItem.latestIntentionWithHighestLevel?.tasks.find(
            (task) => task.logType === BDILogType.INTENTION_FINISHED,
          );
          const importantGoal: GoalInfoItem = {
            agentId,
            goalId,
            goalTemplateName: goalIntentionItem.goalTemplateName,
            bdiLogLevel: goalIntentionItem.level,
            intentionId: goalIntentionItem.latestIntentionWithHighestLevel?.intentionId,
            planTemplateName: goalIntentionItem.latestIntentionWithHighestLevel?.planTemplateName,
            intentionResult: intentionFinishedTask?.intentionResult,
            goalResult: goalIntentionItem.goalResult,
            goalContextMsg,
          };
          recentImportantGoals.push(importantGoal);
        } else if (!goalIntentionItem.goalFinished) {
          const lastIntention: BDILogIntentionModel | undefined = goalIntentionItem.allIntentions.slice(-1)[0];
          const intentionFinishedTask = lastIntention?.tasks.find((task) => task.logType === BDILogType.INTENTION_FINISHED);

          const currentGoal: GoalInfoItem = {
            agentId,
            goalId,
            goalTemplateName: goalIntentionItem.goalTemplateName,
            bdiLogLevel: goalIntentionItem.level,
            intentionId: lastIntention?.intentionId,
            planTemplateName: lastIntention?.planTemplateName,
            intentionResult: intentionFinishedTask?.intentionResult,
            goalResult: goalIntentionItem.goalResult,
            goalContextMsg,
          };
          currentGoals.push(currentGoal);
        }
      });

      return {
        agentBeliefsets,
        currentIntentions: currentGoals,
        recentImportantIntentions: recentImportantGoals,
      };
    }
    return undefined;
  };

  /**
   * Get intention result by subgoal id and delegating agent address
   * @param subGoalId
   * @param delegatingAgentAddress
   * @returns
   */
  getIntentionStatusBySubGoalIdDelegatingAgent = (teamId: string, subGoalId: string) => {
    const goalIntentionItem = this.getIntentionWithContextByAgentGoalId(teamId, subGoalId);

    if (goalIntentionItem?.intentionData) {
      const intentionFinishedTask = goalIntentionItem.intentionData.tasks.find((task) => task.logType === BDILogType.INTENTION_FINISHED);
      return intentionFinishedTask?.intentionResult;
    }

    return undefined;
  };

  getAllIntentions() {
    return this._intentions;
  }

  getAllIntentionOverviews() {
    return this._intentionOverviews;
  }

  /**
   * Check inspecting intention in PLAN_INSPECTOR has been started or not
   * @returns true if it has been started, otherwise false
   */
  isInspectingIntentionStarted() {
    const agentId = PLAN_INSPECTOR.getInspectingAgentId();
    const goalId = PLAN_INSPECTOR.getInspectingPlanGoalId();
    if (!agentId || !goalId) {
      return false;
    }
    return !!this.getIntentionWithContextByAgentGoalId(agentId, goalId);
  }
}

const INTENTION_BUILDER = new CBDIIntentionBuilder();
export default INTENTION_BUILDER;
