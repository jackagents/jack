import { TaskStatus } from 'misc/constant/common/cmConstants';
import { BDILogType } from 'misc/types/cbdi/cbdi-types-non-flatbuffer';
import {
  BDILogIntentionTask,
  ProcessedIntentionTask,
} from 'misc/types/cbdi/cbdi-models-non-flatbuffer';
import { CBDIEditorPlanNodeType } from 'misc/types/cbdiEdit/cbdiEditTypes';

export const processTasks = (tasks: BDILogIntentionTask[]) => {
  const newProcessedTasks: ProcessedIntentionTask[] = [];

  tasks.forEach((task) => {
    switch (task.logType) {
      case BDILogType.INTENTION_STARTED:
      case BDILogType.INTENTION_FINISHED:
        break;

      case BDILogType.SUB_GOAL_STARTED: {
        const newGoalTask: ProcessedIntentionTask = {
          ...task,
          taskType: CBDIEditorPlanNodeType.GoalPlanNodeType,
          taskStatus: TaskStatus.STARTED,
        };
        newProcessedTasks.push(newGoalTask);
        break;
      }

      case BDILogType.SUB_GOAL_FINISHED: {
        if (
          newProcessedTasks.length > 0 &&
          task.goal === newProcessedTasks[newProcessedTasks.length - 1].goal
        ) {
          const goalStatus = task.goalResult
            ? TaskStatus.SUCCESS
            : TaskStatus.FAILED;

          newProcessedTasks[newProcessedTasks.length - 1].taskStatus =
            goalStatus;
        }
        break;
      }

      case BDILogType.ACTION_STARTED: {
        const newActionTask: ProcessedIntentionTask = {
          ...task,
          taskType: CBDIEditorPlanNodeType.ActionPlanNodeType,
          taskStatus: TaskStatus.STARTED,
        };
        newProcessedTasks.push(newActionTask);
        break;
      }

      case BDILogType.ACTION_FINISHED: {
        if (
          newProcessedTasks.length > 0 &&
          task.action === newProcessedTasks[newProcessedTasks.length - 1].action
        ) {
          newProcessedTasks[newProcessedTasks.length - 1].taskStatus =
            task.actionSuccess ? TaskStatus.SUCCESS : TaskStatus.FAILED;
          newProcessedTasks[newProcessedTasks.length - 1].reasoning =
            task.reasoning;
        }
        break;
      }

      case BDILogType.SLEEP_STARTED: {
        const newSleepTask: ProcessedIntentionTask = {
          ...task,
          taskType: CBDIEditorPlanNodeType.SleepPlanNodeType,
          taskStatus: TaskStatus.STARTED,
        };
        newProcessedTasks.push(newSleepTask);
        break;
      }
      case BDILogType.SLEEP_FINISHED: {
        if (
          newProcessedTasks.length > 0 &&
          task.sleepMs ===
            newProcessedTasks[newProcessedTasks.length - 1].sleepMs
        ) {
          newProcessedTasks[newProcessedTasks.length - 1].taskStatus =
            TaskStatus.SUCCESS;
        }
        break;
      }

      case BDILogType.CONDITION: {
        const newConditionTask: ProcessedIntentionTask = {
          ...task,
          taskType: CBDIEditorPlanNodeType.ConditionPlanNodeType,
          taskStatus: task.conditionSuccess
            ? TaskStatus.SUCCESS
            : TaskStatus.FAILED,
        };
        newProcessedTasks.push(newConditionTask);
        break;
      }

      default:
        break;
    }
  });

  return newProcessedTasks;
};
