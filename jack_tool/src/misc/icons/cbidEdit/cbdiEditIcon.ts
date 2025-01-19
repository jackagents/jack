import teamIcon from 'assets/cbdiEdit/icons/team_icon.svg';
import agentIcon from 'assets/cbdiEdit/icons/agent_icon.svg';
import tacticIcon from 'assets/cbdiEdit/icons/tactic_icon.svg';
import roleIcon from 'assets/cbdiEdit/icons/role_icon.svg';
import messageIcon from 'assets/cbdiEdit/icons/message_icon.svg';
import messageFieldIcon from 'assets/cbdiEdit/icons/messageField_icon.png';
import goalIcon from 'assets/cbdiEdit/icons/goal_icon.svg';
import planIcon from 'assets/cbdiEdit/icons/plan_icon.svg';
import sleepIcon from 'assets/cbdiEdit/icons/wait_icon.svg';
import conditionIcon from 'assets/cbdiEdit/icons/condition.png';
import resourceIcon from 'assets/cbdiEdit/icons/resource_icon.svg';
import actionIcon from 'assets/cbdiEdit/icons/action_icon.svg';
import serviceIcon from 'assets/cbdiEdit/icons/service.png';
import enumIcon from 'assets/cbdiEdit/icons/enum.png';
import enumOptionIcon from 'assets/cbdiEdit/icons/enumOption.png';
import entityIcon from 'assets/cbdiEdit/icons/entity_icon.png';
import eventIcon from 'assets/cbdiEdit/icons/event_icon.png';
import moduleIcon from 'assets/cbdiEdit/icons/module.png';
import addIcon from 'assets/cbdiEdit/icons/add.png';
import beliefsetIcon from 'assets/cbdiEdit/icons/beliefset_icon.svg';
import reactFlowIcon from 'assets/cbdiEdit/icons/reactFlow.png';
import { CBDIEditorOtherCategoryType, CBDIEditorPlanNodeType, CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';

export const nodeIcon: { [type: string]: string } = {};
nodeIcon[CBDIEditorRootConceptType.TeamConceptType] = teamIcon;
nodeIcon[CBDIEditorRootConceptType.AgentConceptType] = agentIcon;
nodeIcon[CBDIEditorRootConceptType.TacticConceptType] = tacticIcon;
nodeIcon[CBDIEditorRootConceptType.RoleConceptType] = roleIcon;
nodeIcon[CBDIEditorRootConceptType.MessageConceptType] = messageIcon;
nodeIcon[CBDIEditorOtherCategoryType.MessageFieldType] = messageFieldIcon;
nodeIcon[CBDIEditorRootConceptType.GoalConceptType] = goalIcon;
nodeIcon[CBDIEditorRootConceptType.PlanConceptType] = planIcon;
nodeIcon[CBDIEditorRootConceptType.ResourceConceptType] = resourceIcon;
nodeIcon[CBDIEditorRootConceptType.ActionConceptType] = actionIcon;
nodeIcon[CBDIEditorRootConceptType.ServiceConceptType] = serviceIcon;
nodeIcon[CBDIEditorRootConceptType.EnumConceptType] = enumIcon;
nodeIcon[CBDIEditorRootConceptType.EntityConceptType] = entityIcon;
nodeIcon[CBDIEditorRootConceptType.EventConceptType] = eventIcon;

nodeIcon[CBDIEditorPlanNodeType.SleepPlanNodeType] = sleepIcon;
nodeIcon[CBDIEditorPlanNodeType.ConditionPlanNodeType] = conditionIcon;
nodeIcon[CBDIEditorPlanNodeType.ActionPlanNodeType] = actionIcon;
nodeIcon[CBDIEditorPlanNodeType.GoalPlanNodeType] = goalIcon;

nodeIcon[CBDIEditorOtherCategoryType.ModuleType] = moduleIcon;
nodeIcon[CBDIEditorOtherCategoryType.EnumOptionType] = enumOptionIcon;
nodeIcon[CBDIEditorOtherCategoryType.ReactFlowPlan] = reactFlowIcon;
nodeIcon.beliefset = beliefsetIcon;
nodeIcon.add = addIcon;

export const nodeColor: { [type: string]: string } = {};
nodeColor[CBDIEditorRootConceptType.TeamConceptType] = '#f8cecc';
nodeColor[CBDIEditorRootConceptType.AgentConceptType] = '#ffe6cc';
nodeColor[CBDIEditorRootConceptType.TacticConceptType] = '#edc68e';
nodeColor[CBDIEditorRootConceptType.RoleConceptType] = '#32e3d1';
nodeColor[CBDIEditorRootConceptType.MessageConceptType] = '#e7ffb3';
nodeColor[CBDIEditorRootConceptType.PlanConceptType] = '#dbf1ff';
nodeColor[CBDIEditorRootConceptType.ResourceConceptType] = '#d1b3ff';
nodeColor[CBDIEditorRootConceptType.ActionConceptType] = '#f1faee';
nodeColor[CBDIEditorRootConceptType.GoalConceptType] = '#fff5db';
nodeColor[CBDIEditorRootConceptType.ServiceConceptType] = '#ffffff';
nodeColor[CBDIEditorRootConceptType.EnumConceptType] = '#ffffff';
nodeColor[CBDIEditorRootConceptType.EntityConceptType] = '#ffffff';
nodeColor[CBDIEditorRootConceptType.EventConceptType] = '#ffffff';

nodeColor[CBDIEditorPlanNodeType.SleepPlanNodeType] = '#e07a5f';
nodeColor[CBDIEditorPlanNodeType.ConditionPlanNodeType] = '#ffffff';
nodeColor[CBDIEditorPlanNodeType.ActionPlanNodeType] = '#f1faee';
nodeColor[CBDIEditorPlanNodeType.GoalPlanNodeType] = '#fff5db';

export const nodeProps = {
  icon: nodeIcon,
  fillColor: nodeColor,
};
