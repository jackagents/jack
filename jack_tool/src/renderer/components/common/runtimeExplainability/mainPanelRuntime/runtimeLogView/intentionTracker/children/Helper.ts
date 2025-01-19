import { ProcessedIntentionTask } from 'types/cbdi/cbdi-models-non-flatbuffer';
import { IntentionSeverityColorArr, nodeIcon } from 'misc/icons/cbdi/cbdiIcons';
import { BDILogLevel } from 'types/cbdi/cbdi-types-non-flatbuffer';
import { CBDIEditorPlanNodeType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { removeBeforeFirstDotAndDot } from 'misc/utils/common/commonUtils';

/**
 * get intention task info
 * @param task ProcessedIntentionTask
 * @returns intention task info
 */
export function getTaskInfo(task: ProcessedIntentionTask) {
  const { action, sleepMs, taskType, level, conditionSuccess, goal } = task;
  const severityColor = IntentionSeverityColorArr[level || BDILogLevel.NORMAL];
  let taskIcon;
  let taskContent = '';

  switch (taskType) {
    case CBDIEditorPlanNodeType.ActionPlanNodeType:
      taskContent = removeBeforeFirstDotAndDot(action) || '';
      taskIcon = nodeIcon[CBDIEditorPlanNodeType.ActionPlanNodeType];
      break;

    case CBDIEditorPlanNodeType.GoalPlanNodeType:
      taskContent = removeBeforeFirstDotAndDot(goal) || '';
      taskIcon = nodeIcon[CBDIEditorPlanNodeType.GoalPlanNodeType];
      break;

    case CBDIEditorPlanNodeType.SleepPlanNodeType:
      taskContent = `Sleep ${Math.floor(sleepMs! / 1000)}`;
      taskIcon = nodeIcon[CBDIEditorPlanNodeType.SleepPlanNodeType];
      break;

    case CBDIEditorPlanNodeType.ConditionPlanNodeType:
      // TODO add meaningful content for condition task
      taskContent = conditionSuccess ? 'Condition succeeded' : 'Condition failed';
      taskIcon = nodeIcon[CBDIEditorPlanNodeType.ConditionPlanNodeType];
      break;

    default:
      break;
  }

  return {
    severityColor,
    taskIcon,
    taskContent,
  };
}
