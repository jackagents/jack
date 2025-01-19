import { TaskStatus } from 'constant/common/cmConstants';
import startedIcon from 'assets/cbdi/icons/started.png';
import failedIcon from 'assets/cbdi/icons/failed.png';
import droppedIcon from 'assets/cbdi/icons/dropped.png';
import successIcon from 'assets/cbdi/icons/success.png';

import teamSvg from 'assets/cbdi/icons/team_icon.svg';
import agentSvg from 'assets/cbdi/icons/agent_icon.svg';
import tacticSvg from 'assets/cbdi/icons/tactic_icon.svg';
import roleSvg from 'assets/cbdi/icons/role_icon.svg';
import planSvg from 'assets/cbdi/icons/plan_icon.svg';
import goalSvg from 'assets/cbdi/icons/goal_icon.svg';
import resourceSvg from 'assets/cbdi/icons/resource_icon.svg';
import actionSvg from 'assets/cbdi/icons/action_icon.svg';
import paramSvg from 'assets/cbdi/icons/param_icon.svg';
import conditionIcon from 'assets/cbdi/icons/condition.png';
import sleepIcon from 'assets/cbdi/icons/sleep.png';
import { BDILogLevel } from 'misc/types/cbdi/cbdi-types-non-flatbuffer';
import {
  CBDIEditorOtherCategoryType,
  CBDIEditorPlanNodeType,
  CBDIEditorRootConceptType,
} from 'misc/types/cbdiEdit/cbdiEditTypes';

/**
 * list item color dic in intention tracker for different severity level
 */
export const IntentionSeverityColorArr: Record<BDILogLevel, string> = {
  [BDILogLevel.NORMAL]: '#adb5bd',
  [BDILogLevel.IMPORTANT]: '#fee440',
  [BDILogLevel.CRITICAL]: '#fb8b24',
};
/**
 * icons in intention tracker for different bdi status
 */

export const bdiStatusIcon: { [type: string]: string } = {};
bdiStatusIcon[TaskStatus.STARTED] = startedIcon;
bdiStatusIcon[TaskStatus.FAILED] = failedIcon;
bdiStatusIcon[TaskStatus.DROPPED] = droppedIcon;
bdiStatusIcon[TaskStatus.SUCCESS] = successIcon;

export const nodeIcon: { [type: string]: string } = {};
nodeIcon[CBDIEditorRootConceptType.TeamConceptType] = teamSvg;
nodeIcon[CBDIEditorRootConceptType.AgentConceptType] = agentSvg;
nodeIcon[CBDIEditorRootConceptType.TacticConceptType] = tacticSvg;
nodeIcon[CBDIEditorRootConceptType.RoleConceptType] = roleSvg;
nodeIcon[CBDIEditorRootConceptType.PlanConceptType] = planSvg;
nodeIcon[CBDIEditorRootConceptType.GoalConceptType] = goalSvg;
nodeIcon[CBDIEditorRootConceptType.ResourceConceptType] = resourceSvg;
nodeIcon[CBDIEditorRootConceptType.ActionConceptType] = actionSvg;

nodeIcon[CBDIEditorOtherCategoryType.MessageFieldType] = paramSvg;

nodeIcon[CBDIEditorPlanNodeType.ConditionPlanNodeType] = conditionIcon;
nodeIcon[CBDIEditorPlanNodeType.SleepPlanNodeType] = sleepIcon;
nodeIcon[CBDIEditorPlanNodeType.ActionPlanNodeType] = actionSvg;
nodeIcon[CBDIEditorPlanNodeType.GoalPlanNodeType] = goalSvg;

export const nodeColor: { [type: string]: string } = {};
nodeColor[CBDIEditorRootConceptType.TeamConceptType] = '#f8cecc';
nodeColor[CBDIEditorRootConceptType.AgentConceptType] = '#ffe6cc';
nodeColor[CBDIEditorRootConceptType.TacticConceptType] = '#edc68e';
nodeColor[CBDIEditorRootConceptType.RoleConceptType] = '#32e3d1';
nodeColor[CBDIEditorRootConceptType.PlanConceptType] = '#dbf1ff';
nodeColor[CBDIEditorRootConceptType.ResourceConceptType] = '#d1b3ff';
nodeColor[CBDIEditorRootConceptType.ActionConceptType] = '#f1faee';
nodeColor[CBDIEditorRootConceptType.GoalConceptType] = '#fff5db';
nodeColor[CBDIEditorRootConceptType.ServiceConceptType] = '#ffffff';

nodeColor[CBDIEditorOtherCategoryType.MessageFieldType] = '#ffffff';

nodeColor[CBDIEditorPlanNodeType.SleepPlanNodeType] = '#e07a5f';
nodeColor[CBDIEditorPlanNodeType.ConditionPlanNodeType] = '#ffffff';
nodeColor[CBDIEditorPlanNodeType.ActionPlanNodeType] = '#f1faee';
nodeColor[CBDIEditorPlanNodeType.GoalPlanNodeType] = '#fff5db';
