/* eslint-disable no-param-reassign */
import { Mod, CBDIEditorRootConceptType, CBDIEditorObject, ModuleConcept, EmptyModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { areModuleConceptsEqual } from 'misc/utils/common/commonUtils';
import {
  CBDIEditorProject,
  CBDIEditorProjectAction,
  CBDIEditorProjectAgent,
  CBDIEditorProjectEntity,
  CBDIEditorProjectEnum,
  CBDIEditorProjectEvent,
  CBDIEditorProjectGoal,
  CBDIEditorProjectMessage,
  CBDIEditorProjectPlan,
  CBDIEditorProjectResource,
  CBDIEditorProjectRole,
  CBDIEditorProjectService,
  CBDIEditorProjectTactic,
  CBDIEditorProjectTeam,
} from 'types/cbdiEdit/cbdiEditModel';
import { Edge, Position } from 'reactflow';
import { CbdiEditTreeGraphNode } from './types';
import { CBDI_EDIT_TREE_GRAPH_EDGE_EDGE_MARKER_TYPE, CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT, CBDI_EDIT_TREE_GRAPH_NODE_WIDTH } from './constants';

const getCbdiEditTreeGraphNodeByObject = (object: CBDIEditorObject, prefixId?: string): CbdiEditTreeGraphNode => ({
  id: (prefixId || '') + object.uuid,
  position: { x: 0, y: 0 },
  width: CBDI_EDIT_TREE_GRAPH_NODE_WIDTH,
  height: CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT,
  data: object,
  type: 'custom',
});

const getCbdiEditTreeGraphNodeByModuleConcept = (
  project: CBDIEditorProject,
  moduleConcept: ModuleConcept,
  objectType: CBDIEditorRootConceptType,
  prefixId?: string,
): CbdiEditTreeGraphNode => {
  const obj = getObjectByModuleConcept(project, moduleConcept);
  if (obj) {
    return getCbdiEditTreeGraphNodeByObject(obj, prefixId);
  }
  return {
    id: (prefixId || '') + moduleConcept.uuid,
    position: { x: 0, y: 0 },
    width: CBDI_EDIT_TREE_GRAPH_NODE_WIDTH,
    height: CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT,
    data: {
      ...moduleConcept,
      _mod: Mod.Deletion,
      _objectType: objectType,
      note: '',
    },
    type: 'custom',
  };
};

const getGroupCbdiEditTreeGraphNode = (
  id: string,
  childNumber: number,
  childObjectTypes: CBDIEditorRootConceptType[],
  direction?: 'vertical' | 'horizontal',
): CbdiEditTreeGraphNode => {
  const layoutDirection = direction !== undefined ? direction : 'horizontal';
  if (layoutDirection === 'horizontal') {
    return {
      id,
      type: 'group',
      data: {
        _mod: Mod.None,
        _objectType: CBDIEditorRootConceptType.ActionConceptType,
        module: '',
        name: '',
        note: '',
        uuid: '',
        childObjectTypes,
      },
      position: { x: 0, y: 0 },
      width: CBDI_EDIT_TREE_GRAPH_NODE_WIDTH * 3 * Math.max(childNumber, 1),
      height: CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT * 2,
      style: {
        width: CBDI_EDIT_TREE_GRAPH_NODE_WIDTH * 3 * Math.max(childNumber, 1),
        height: CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT * 2,
      },
    };
  }
  return {
    id,
    type: 'group',
    data: {
      _mod: Mod.None,
      _objectType: CBDIEditorRootConceptType.ActionConceptType,
      module: '',
      name: '',
      note: '',
      uuid: '',
      childObjectTypes,
    },
    position: { x: 0, y: 0 },
    width: CBDI_EDIT_TREE_GRAPH_NODE_WIDTH * 3,
    height: CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT * 2 * Math.max(childNumber + 0.5, 1.5),
    style: {
      width: CBDI_EDIT_TREE_GRAPH_NODE_WIDTH * 3,
      height: CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT * 2 * Math.max(childNumber + 0.5, 1.5),
    },
  };
};

const getRePositionedChildNode = (childNode: CbdiEditTreeGraphNode, index: number, direction?: 'vertical' | 'horizontal') => {
  const layoutDirection = direction !== undefined ? direction : 'horizontal';
  if (layoutDirection === 'horizontal') {
    childNode.position = {
      x: CBDI_EDIT_TREE_GRAPH_NODE_WIDTH * 3 * index + CBDI_EDIT_TREE_GRAPH_NODE_WIDTH,
      y: CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT / 8,
    };
  } else {
    childNode.position = {
      x: CBDI_EDIT_TREE_GRAPH_NODE_WIDTH,
      y: CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT * 2 * index + CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT,
    };
  }
  childNode.data = { ...childNode.data, isNotReLayout: true };
  return childNode;
};

const getCbdiEditTreeGraphEdge = (
  sourceNode: CbdiEditTreeGraphNode,
  targetNode: CbdiEditTreeGraphNode,
  edgePosition?: 'TB' | 'LR',
  edgeLabel?: string,
): Edge => {
  const position = edgePosition || 'TB';
  const sourceHandle = position === 'TB' ? Position.Bottom : Position.Right;
  const targetHandle = position === 'TB' ? Position.Top : Position.Left;
  return {
    id: `${sourceNode.id}-${targetNode.id}`,
    source: sourceNode.id,
    target: targetNode.id,
    type: 'bezier',
    markerEnd: CBDI_EDIT_TREE_GRAPH_EDGE_EDGE_MARKER_TYPE,
    sourceHandle,
    targetHandle,
    label: edgeLabel,
    labelStyle: { fontSize: 18 },
  };
};

type AgentRoleItem = {
  upstreamRoles: ModuleConcept[];
  downstreamRoles: ModuleConcept[];
  noUpstreamRolePlans: ModuleConcept[];
  agent: ModuleConcept;
  type: CBDIEditorRootConceptType.TeamConceptType | CBDIEditorRootConceptType.AgentConceptType;
};

/**
 * convert team object to team role item
 * @param teamObj
 * @param allRoleObjs
 * @param project
 * @returns
 */
const convertTeamRoleItem = (teamObj: CBDIEditorProjectTeam, allRoleObjs: CBDIEditorProjectRole[], project: CBDIEditorProject) => {
  const upstreamRoles: ModuleConcept[] = [];
  const teamRoleItem: AgentRoleItem = {
    agent: teamObj,
    upstreamRoles: [],
    downstreamRoles: [],
    noUpstreamRolePlans: [],
    type: CBDIEditorRootConceptType.TeamConceptType,
  };
  teamObj.plans.forEach((planModuleConcept) => {
    const planObj = getObjectByModuleConcept(project, planModuleConcept) as CBDIEditorProjectPlan | undefined;
    if (planObj) {
      let isPlanNoUpstreamRolePlan = true;
      allRoleObjs.forEach((roleObj) => {
        if (roleObj.goals.find((goalModuleConcept) => areModuleConceptsEqual(goalModuleConcept, planObj.handles))) {
          isPlanNoUpstreamRolePlan = false;
          upstreamRoles.push(roleObj);
        }
      });
      if (isPlanNoUpstreamRolePlan) {
        teamRoleItem.noUpstreamRolePlans.push(planModuleConcept);
      }
    }
  });
  const uniqueUpstreamRoles = Array.from(new Set(upstreamRoles.map((item) => JSON.stringify(item)))).map((item) => JSON.parse(item));
  const downstreamRoles = teamObj.roles.filter((roleModuleConcept) => {
    if (uniqueUpstreamRoles.some((el) => areModuleConceptsEqual(el, roleModuleConcept))) {
      return false;
    }
    return true;
  });
  teamRoleItem.upstreamRoles = uniqueUpstreamRoles;
  teamRoleItem.downstreamRoles = downstreamRoles;

  return teamRoleItem;
};

/**
 * convert agent object to agent role item
 * @param agentObj
 * @param allRoleObjs
 * @param project
 * @returns
 */
const convertAgentRoleItem = (agentObj: CBDIEditorProjectAgent, allRoleObjs: CBDIEditorProjectRole[], project: CBDIEditorProject) => {
  const agentRoleItem: AgentRoleItem = {
    agent: agentObj,
    upstreamRoles: [],
    downstreamRoles: [],
    noUpstreamRolePlans: [],
    type: CBDIEditorRootConceptType.AgentConceptType,
  };
  const upstreamRoles: ModuleConcept[] = [];
  agentObj.plans.forEach((planModuleConcept) => {
    const planObj = getObjectByModuleConcept(project, planModuleConcept) as CBDIEditorProjectPlan | undefined;
    if (planObj) {
      let isPlanNoUpstreamRolePlan = true;
      allRoleObjs.forEach((roleObj) => {
        if (roleObj.goals.find((goalModuleConcept) => areModuleConceptsEqual(goalModuleConcept, planObj.handles))) {
          isPlanNoUpstreamRolePlan = false;
          upstreamRoles.push(roleObj);
        }
      });
      if (isPlanNoUpstreamRolePlan) {
        agentRoleItem.noUpstreamRolePlans.push(planModuleConcept);
      }
    }
  });
  const uniqueUpstreamRoles = Array.from(new Set(upstreamRoles.map((item) => JSON.stringify(item)))).map((item) => JSON.parse(item));
  agentRoleItem.upstreamRoles = uniqueUpstreamRoles;

  return agentRoleItem;
};

/**
 * convert all teams and agent to agent role item and return array
 * @param project
 * @returns
 */
const getAgentRoleArray = (project: CBDIEditorProject) => {
  const agentRoleArray: AgentRoleItem[] = [];
  // 1. get all role objects
  const allRoleObjs = project.roles
    .map((moduleConcept) => {
      const roleObj = getObjectByModuleConcept(project, moduleConcept);
      if (roleObj && roleObj._mod !== Mod.Deletion) {
        return roleObj;
      }
      return undefined;
    })
    .filter((el) => el !== undefined) as CBDIEditorProjectRole[];
  // 2. process team, add upstream roles and downstream roles to team
  project.teams.forEach((moduleConcept) => {
    const teamObj = getObjectByModuleConcept(project, moduleConcept) as CBDIEditorProjectTeam | undefined;
    if (teamObj && teamObj._mod !== Mod.Deletion) {
      const teamRoleItem = convertTeamRoleItem(teamObj, allRoleObjs, project);
      agentRoleArray.push(teamRoleItem);
    }
  });

  // 3. process agent, add upstream roles to agent
  project.agents.forEach((moduleConcept) => {
    const agentObj = getObjectByModuleConcept(project, moduleConcept) as CBDIEditorProjectAgent | undefined;
    if (agentObj && agentObj._mod !== Mod.Deletion) {
      const agentRoleItem = convertAgentRoleItem(agentObj, allRoleObjs, project);
      agentRoleArray.push(agentRoleItem);
    }
  });

  return agentRoleArray;
};

/**
 * add horizontal group node
 * @param groupNodeId
 * @param childrenCount
 * @param groupLabel
 * @param nodes
 * @param childObjectTypes
 */
const addHorizontalConceptGroupNode = (
  groupNodeId: string,
  childrenCount: number,
  groupLabel: string,
  nodes: CbdiEditTreeGraphNode[],
  childObjectTypes: CBDIEditorRootConceptType[],
  xDistanceFromCenter: number,
  centerNodeId: string,
) => {
  const groupNode = getGroupCbdiEditTreeGraphNode(groupNodeId, childrenCount, childObjectTypes);

  groupNode.data = {
    ...groupNode.data,
    isNotReLayout: true,
    groupLabel,
    yDistanceFromCenter: xDistanceFromCenter,
    centerNodeId,
  };
  nodes.push(groupNode);
};

/**
 * add side vertical concept group node to nodes
 * @param groupNodeId
 * @param moduleConcepts
 * @param cbdiEditorRootConceptType
 * @param xDistanceFromLeft
 * @param nodes
 * @param project
 */
const addSideVerticalConceptGroupNode = (
  groupNodeId: string,
  moduleConcepts: ModuleConcept[],
  cbdiEditorRootConceptType: CBDIEditorRootConceptType,
  xDistanceFromLeft: number,
  nodes: CbdiEditTreeGraphNode[],
  project: CBDIEditorProject,
  groupLabel: string,
  childObjectTypes: CBDIEditorRootConceptType[],
  customNodeIconType?: 'beliefset' | 'messageField' | 'enumItem',
) => {
  const groupNode = getGroupCbdiEditTreeGraphNode(groupNodeId, moduleConcepts.length, childObjectTypes, 'vertical');
  groupNode.position = {
    x: -2 * xDistanceFromLeft * groupNode.width! - 3 * groupNode.width!,
    y: 3 * CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT,
  };
  groupNode.data = {
    ...groupNode.data,
    isNotReLayout: true,
    groupLabel,
  };
  nodes.push(groupNode);
  moduleConcepts.forEach((moduleConcept, index) => {
    let childNode = getCbdiEditTreeGraphNodeByModuleConcept(project, moduleConcept, cbdiEditorRootConceptType, index.toString());
    childNode.data = {
      ...childNode.data,
      customNodeIconType,
      name: moduleConcept.name,
    };
    childNode.parentNode = groupNode.id;
    childNode = getRePositionedChildNode(childNode, index, 'vertical');
    nodes.push(childNode);
  });
};

export function drawAgentModuleConceptGraph(
  project: CBDIEditorProject,
  agentObj: CBDIEditorProjectTeam | CBDIEditorProjectAgent,
): {
  nodes: CbdiEditTreeGraphNode[];
  edges: Edge[];
} {
  const nodes: CbdiEditTreeGraphNode[] = [];
  const edges: Edge[] = [];

  // get all perform plan objects
  const allPerformPlanObjs = agentObj.plans
    .map((planModuleConcept) => {
      const planObj = getObjectByModuleConcept(project, planModuleConcept);
      if (planObj && planObj._mod !== Mod.Deletion) {
        return planObj;
      }
      return undefined;
    })
    .filter((el) => el !== undefined) as CBDIEditorProjectPlan[];

  // convert all team and agent to a team role item with upstream and downstream roles
  const agentRoleArray = getAgentRoleArray(project);
  // get the item for current agent obj

  const agentRoleItem = agentRoleArray.find((item) => areModuleConceptsEqual(item.agent, agentObj))!;
  // 1. add hidden middle node so upstream and downstream and hook
  const hiddenMiddleNode = getCbdiEditTreeGraphNodeByModuleConcept(project, agentObj, agentRoleItem.type, 'hiddenMiddleNode');
  hiddenMiddleNode.hidden = true;
  nodes.push(hiddenMiddleNode);

  const upstreamGoalList: ModuleConcept[] = [];

  // 2. add upstream

  // 2.1 add upstream group node

  // upstream role group
  const upstreamRoleGroupNodeId = `${hiddenMiddleNode.id}upstreamRoleGroup`;
  addHorizontalConceptGroupNode(
    upstreamRoleGroupNodeId,
    agentRoleItem.upstreamRoles.length,
    'Performing Roles',
    nodes,
    [CBDIEditorRootConceptType.RoleConceptType],
    -3,
    hiddenMiddleNode.id,
  );

  // upstream goal group
  const upstreamGoalGroupNodeId = `${hiddenMiddleNode.id}upstreamGoalGroup`;
  addHorizontalConceptGroupNode(
    upstreamGoalGroupNodeId,
    agentObj.goals.length,
    'Performing Goals',
    nodes,
    [CBDIEditorRootConceptType.GoalConceptType],
    -2,
    hiddenMiddleNode.id,
  );

  // upstream plan group
  const upstreamPlanGroupNodeId = `${hiddenMiddleNode.id}upstreamPlanGroup`;
  // add upstream plan group node even there is no plans in agent
  // because there might be empty plan
  addHorizontalConceptGroupNode(
    upstreamPlanGroupNodeId,
    agentObj.plans.length,
    'Performing Plans',
    nodes,
    [CBDIEditorRootConceptType.PlanConceptType],
    -1,
    hiddenMiddleNode.id,
  );

  // 2.2 add upstream roles
  agentRoleItem.upstreamRoles.forEach((roleModuleConcept) => {
    const roleObj = getObjectByModuleConcept(project, roleModuleConcept) as CBDIEditorProjectRole | undefined;
    if (roleObj) {
      const roleNode = getCbdiEditTreeGraphNodeByObject(roleObj, `${hiddenMiddleNode.id}upstream`);
      roleNode.data = {
        ...roleNode.data,
        groupId: upstreamRoleGroupNodeId,
      };
      nodes.push(roleNode);

      // 2.2.1 add upstream teams
      agentRoleArray.forEach((ar) => {
        if (!areModuleConceptsEqual(agentObj, ar.agent) && ar.downstreamRoles.some((el) => areModuleConceptsEqual(el, roleModuleConcept))) {
          const upstreamstreamAgentNode = getCbdiEditTreeGraphNodeByModuleConcept(project, ar.agent, ar.type, roleNode.id);
          const agentEdge = getCbdiEditTreeGraphEdge(upstreamstreamAgentNode, roleNode);
          nodes.push(upstreamstreamAgentNode);
          edges.push(agentEdge);
        }
      });

      // 2.2.2 add upstream goals
      roleObj.goals.forEach((goalModuleConcept) => {
        const matchedPlans = allPerformPlanObjs.filter((planObj) => areModuleConceptsEqual(planObj.handles, goalModuleConcept));
        if (matchedPlans.length > 0) {
          const goalNode = getCbdiEditTreeGraphNodeByModuleConcept(
            project,
            goalModuleConcept,
            CBDIEditorRootConceptType.GoalConceptType,
            `${roleNode.id}upstream`,
          );
          goalNode.data = {
            ...goalNode.data,
            groupId: upstreamGoalGroupNodeId,
          };
          const matchedAgentGoal = agentObj.goals.find((agentGoal) => areModuleConceptsEqual(agentGoal, goalModuleConcept));
          if (!matchedAgentGoal) {
            goalNode.data = { ...goalNode.data, nodeBadgeType: 'question' };
          } else if (matchedAgentGoal.startup_goal) {
            goalNode.data = { ...goalNode.data, nodeBadgeType: 'star' };
          }
          const upstreamGoalEdge = getCbdiEditTreeGraphEdge(roleNode, goalNode);
          upstreamGoalList.push(goalModuleConcept);
          nodes.push(goalNode);
          edges.push(upstreamGoalEdge);

          // 2.2.3 add upstream plans
          matchedPlans.forEach((planObj) => {
            const planNode = getCbdiEditTreeGraphNodeByObject(planObj, goalNode.id);

            planNode.data = {
              ...planNode.data,
              groupId: upstreamPlanGroupNodeId,
            };

            const planEdge = getCbdiEditTreeGraphEdge(goalNode, planNode);
            const teamUpstreamEdge = getCbdiEditTreeGraphEdge(planNode, hiddenMiddleNode);
            nodes.push(planNode);
            edges.push(planEdge, teamUpstreamEdge);
          });
        }
      });
    }
  });

  // 2.3 add noRole plans and goals
  agentRoleItem.noUpstreamRolePlans.forEach((planModuleConcept) => {
    const planNode = getCbdiEditTreeGraphNodeByModuleConcept(
      project,
      planModuleConcept,
      CBDIEditorRootConceptType.PlanConceptType,
      'noUpstreamRolePlan',
    );

    planNode.data = {
      ...planNode.data,
      groupId: upstreamPlanGroupNodeId,
    };

    nodes.push(planNode);
    const planEdge = getCbdiEditTreeGraphEdge(planNode, hiddenMiddleNode);
    edges.push(planEdge);
    const planObj = getObjectByModuleConcept(project, planModuleConcept) as CBDIEditorProjectPlan | undefined;
    if (planObj && planObj._mod !== Mod.Deletion) {
      const goalNode = getCbdiEditTreeGraphNodeByModuleConcept(
        project,
        planObj.handles,
        CBDIEditorRootConceptType.GoalConceptType,
        'noUpstreamRoleGoal',
      );
      goalNode.data = {
        ...goalNode.data,
        groupId: upstreamGoalGroupNodeId,
      };
      const matchedAgentGoal = agentObj.goals.find((agentGoal) => areModuleConceptsEqual(agentGoal, planObj.handles));
      if (!matchedAgentGoal) {
        goalNode.data = { ...goalNode.data, nodeBadgeType: 'question' };
      } else if (matchedAgentGoal.startup_goal) {
        goalNode.data = { ...goalNode.data, nodeBadgeType: 'star' };
      }
      upstreamGoalList.push(planObj.handles);
      nodes.push(goalNode);
      const goalEdge = getCbdiEditTreeGraphEdge(goalNode, planNode);
      edges.push(goalEdge);
    }
  });

  // 3. add downstream
  // 3.1 add downstream group

  // downstream role group
  const downstreamRoleGroupNodeId = `${hiddenMiddleNode.id}downstreamRoleGroup`;
  addHorizontalConceptGroupNode(
    downstreamRoleGroupNodeId,
    agentRoleItem.downstreamRoles.length,
    'Required Roles',
    nodes,
    [CBDIEditorRootConceptType.RoleConceptType],
    1,
    hiddenMiddleNode.id,
  );

  // downstream goal group
  const downstreamGoalGroupNodeId = `${hiddenMiddleNode.id}downstreamGoalGroup`;

  addHorizontalConceptGroupNode(
    downstreamGoalGroupNodeId,
    agentRoleItem.downstreamRoles.length,
    'Delegated Goals',
    nodes,
    [CBDIEditorRootConceptType.GoalConceptType],
    2,
    hiddenMiddleNode.id,
  );

  // 3.2 add downstream roles
  agentRoleItem.downstreamRoles.forEach((roleModuleConcept) => {
    const roleObj = getObjectByModuleConcept(project, roleModuleConcept) as CBDIEditorProjectRole | undefined;
    if (roleObj) {
      const roleNode = getCbdiEditTreeGraphNodeByObject(roleObj, hiddenMiddleNode.id);
      roleNode.data = {
        ...roleNode.data,
        groupId: downstreamRoleGroupNodeId,
      };
      nodes.push(roleNode);
      // 3.2 add goals
      if (roleObj.goals.length > 0) {
        roleObj.goals.forEach((goalModuleConcept) => {
          const goalNode = getCbdiEditTreeGraphNodeByModuleConcept(
            project,
            goalModuleConcept,
            CBDIEditorRootConceptType.GoalConceptType,
            `${roleNode.id}downstream`,
          );
          goalNode.data = {
            ...goalNode.data,
            groupId: downstreamGoalGroupNodeId,
          };
          const downstreamGoalEdge = getCbdiEditTreeGraphEdge(hiddenMiddleNode, goalNode);
          const downstreamRoleEdge = getCbdiEditTreeGraphEdge(goalNode, roleNode);
          nodes.push(goalNode);
          edges.push(downstreamGoalEdge, downstreamRoleEdge);
        });
      } else {
        const emptyGoalNode = getCbdiEditTreeGraphNodeByModuleConcept(
          project,
          EmptyModuleConcept,
          CBDIEditorRootConceptType.GoalConceptType,
          `${roleNode.id}emptyPGoal`,
        );
        emptyGoalNode.data = {
          ...emptyGoalNode.data,
          nodeBadgeType: 'question',
          groupId: downstreamGoalGroupNodeId,
        };

        nodes.push(emptyGoalNode);
        const emptyGoalAgentEdge = getCbdiEditTreeGraphEdge(hiddenMiddleNode, emptyGoalNode);
        const emptyGoalRoleEdge = getCbdiEditTreeGraphEdge(emptyGoalNode, roleNode);
        edges.push(emptyGoalAgentEdge, emptyGoalRoleEdge);
      }
      // 3.3 add downstream agents
      agentRoleArray.forEach((ar) => {
        if (!areModuleConceptsEqual(agentObj, ar.agent) && ar.upstreamRoles.some((el) => areModuleConceptsEqual(el, roleModuleConcept))) {
          const downstreamAgentNode = getCbdiEditTreeGraphNodeByModuleConcept(project, ar.agent, ar.type, roleNode.id);
          const agentEdge = getCbdiEditTreeGraphEdge(roleNode, downstreamAgentNode);
          nodes.push(downstreamAgentNode);
          edges.push(agentEdge);
        }
      });
    }
  });

  // 4. add beliefs
  addSideVerticalConceptGroupNode(
    `${hiddenMiddleNode.id}beliefGroup`,
    agentObj.beliefs,
    CBDIEditorRootConceptType.MessageConceptType,
    1,
    nodes,
    project,
    'Beliefs',
    [CBDIEditorRootConceptType.MessageConceptType],
    'beliefset',
  );

  // 5. add services
  addSideVerticalConceptGroupNode(
    `${hiddenMiddleNode.id}serviceGroup`,
    agentObj.services,
    CBDIEditorRootConceptType.ServiceConceptType,
    0,
    nodes,
    project,
    'Services',
    [CBDIEditorRootConceptType.ServiceConceptType],
  );

  return { nodes, edges };
}

export function drawRoleModuleConceptGraph(
  project: CBDIEditorProject,
  roleObj: CBDIEditorProjectRole,
): {
  nodes: CbdiEditTreeGraphNode[];
  edges: Edge[];
} {
  const nodes: CbdiEditTreeGraphNode[] = [];
  const edges: Edge[] = [];
  // 1. add role node
  const roleNode = getCbdiEditTreeGraphNodeByObject(roleObj);
  nodes.push(roleNode);

  // 2. get upstream and downstream agent role items
  const agentRoleArray = getAgentRoleArray(project);
  const upstreamAgents: AgentRoleItem[] = [];
  const downstreamAgents: AgentRoleItem[] = [];

  agentRoleArray.forEach((agentRoleItem) => {
    if (agentRoleItem.upstreamRoles.some((el) => areModuleConceptsEqual(el, roleObj))) {
      upstreamAgents.push(agentRoleItem);
    }
    if (agentRoleItem.downstreamRoles.some((el) => areModuleConceptsEqual(el, roleObj))) {
      downstreamAgents.push(agentRoleItem);
    }
  });

  // 3. add upstream agent group and nodes

  // 3.1 add upstream agent group node
  const upstreamAgentGroupNodeId = `${roleNode.id}upstreamAgentGroup`;
  addHorizontalConceptGroupNode(
    upstreamAgentGroupNodeId,
    upstreamAgents.length,
    'Requiring Agents',
    nodes,
    [CBDIEditorRootConceptType.AgentConceptType, CBDIEditorRootConceptType.TeamConceptType],
    -1,
    roleNode.id,
  );

  // 3.2 add upstream agent node
  upstreamAgents.forEach((upstreamAgentItem) => {
    const upstreamAgentNode = getCbdiEditTreeGraphNodeByModuleConcept(
      project,
      upstreamAgentItem.agent,
      upstreamAgentItem.type,
      `${roleNode.id}upstream`,
    );
    upstreamAgentNode.data = {
      ...upstreamAgentNode.data,
      groupId: upstreamAgentGroupNodeId,
    };
    nodes.push(upstreamAgentNode);
    const upstreamEdge = getCbdiEditTreeGraphEdge(upstreamAgentNode, roleNode);
    edges.push(upstreamEdge);
  });

  // 4. add downstream group and agent nodes

  // 4.1 add upstream agent group node
  const downstreamAgentGroupNodeId = `${roleNode.id}downstreamAgentGroup`;
  addHorizontalConceptGroupNode(
    downstreamAgentGroupNodeId,
    downstreamAgents.length,
    'Performing Agents',
    nodes,
    [CBDIEditorRootConceptType.AgentConceptType, CBDIEditorRootConceptType.TeamConceptType],
    1,
    roleNode.id,
  );

  // 4.2 add downstream agent node
  downstreamAgents.forEach((downstreamAgentItem) => {
    const downstreamAgentNode = getCbdiEditTreeGraphNodeByModuleConcept(
      project,
      downstreamAgentItem.agent,
      downstreamAgentItem.type,
      `${roleNode.id}downstream`,
    );
    downstreamAgentNode.data = {
      ...downstreamAgentNode.data,
      groupId: downstreamAgentGroupNodeId,
    };
    nodes.push(downstreamAgentNode);
    const downstreamEdge = getCbdiEditTreeGraphEdge(roleNode, downstreamAgentNode);
    edges.push(downstreamEdge);
  });

  // 5. add goals
  addSideVerticalConceptGroupNode(`${roleNode.id}goalGroup`, roleObj.goals, CBDIEditorRootConceptType.GoalConceptType, 1, nodes, project, 'Goals', [
    CBDIEditorRootConceptType.GoalConceptType,
  ]);

  // 6. add messages
  addSideVerticalConceptGroupNode(
    `${roleNode.id}messageGroup`,
    roleObj.messages,
    CBDIEditorRootConceptType.MessageConceptType,
    0,
    nodes,
    project,
    'Messages',
    [CBDIEditorRootConceptType.MessageConceptType],
  );

  return { nodes, edges };
}

export function drawGoalModuleConceptGraph(
  project: CBDIEditorProject,
  goalObj: CBDIEditorProjectGoal,
  selectedTacticObj?: CBDIEditorProjectTactic,
): {
  nodes: CbdiEditTreeGraphNode[];
  edges: Edge[];
} {
  const nodes: CbdiEditTreeGraphNode[] = [];
  const edges: Edge[] = [];
  // 1. add goal
  const goalNode: CbdiEditTreeGraphNode = getCbdiEditTreeGraphNodeByObject(goalObj);
  nodes.push(goalNode);

  // 2. add left message
  if (!areModuleConceptsEqual(goalObj.message, EmptyModuleConcept)) {
    const messageNode = getCbdiEditTreeGraphNodeByModuleConcept(
      project,
      goalObj.message,
      CBDIEditorRootConceptType.MessageConceptType,
      `${goalNode.id}message`,
    );
    messageNode.data = {
      ...messageNode.data,
      relativeNodeId: goalNode.id,
      relativePosition: 'left',
      isNotReLayout: true,
    };
    const messageEdge = getCbdiEditTreeGraphEdge(messageNode, goalNode, 'LR', 'Message');
    nodes.push(messageNode);
    edges.push(messageEdge);
  }

  // 3. add role
  project.roles.forEach((moduleConcept) => {
    const roleObj = getObjectByModuleConcept(project, moduleConcept) as CBDIEditorProjectRole | undefined;

    if (roleObj && roleObj._mod !== Mod.Deletion && roleObj.goals.some((el) => areModuleConceptsEqual(el, goalObj))) {
      const roleNode = getCbdiEditTreeGraphNodeByObject(roleObj, goalNode.id);
      nodes.push(roleNode);
      const roleEdge = getCbdiEditTreeGraphEdge(roleNode, goalNode);
      edges.push(roleEdge);
    }
  });

  // 4. add upstream plan
  project.plans.forEach((moduleConcept) => {
    const planObj = getObjectByModuleConcept(project, moduleConcept) as CBDIEditorProjectPlan | undefined;

    const isGoalInPlan =
      planObj &&
      planObj._mod !== Mod.Deletion &&
      planObj.tasks.some((task) => task.nodeData.goal && areModuleConceptsEqual(task.nodeData.goal, goalObj));

    if (isGoalInPlan) {
      const planNode: CbdiEditTreeGraphNode = getCbdiEditTreeGraphNodeByObject(planObj, `${goalNode.id}upstream`);

      nodes.push(planNode);
      const upstreamPlanEdge = getCbdiEditTreeGraphEdge(planNode, goalNode);
      edges.push(upstreamPlanEdge);

      const upstreamGoalNode = getCbdiEditTreeGraphNodeByModuleConcept(
        project,
        planObj.handles,
        CBDIEditorRootConceptType.GoalConceptType,
        'upstreamGoal',
      );
      nodes.push(upstreamGoalNode);
      const upstreamGoalEdge = getCbdiEditTreeGraphEdge(upstreamGoalNode, planNode);
      edges.push(upstreamGoalEdge);
    }
  });

  // 5. add downstream plans

  const allDownstreamPlans = project.plans
    .map((moduleConcept) => {
      const planObj = getObjectByModuleConcept(project, moduleConcept) as CBDIEditorProjectPlan | undefined;
      if (planObj && planObj._mod !== Mod.Deletion && areModuleConceptsEqual(planObj.handles, goalObj)) {
        return planObj;
      }
      return undefined;
    })
    .filter((el) => el !== undefined) as CBDIEditorProjectPlan[];

  // downstream plan group
  const downstreamPlanGroupNodeId = `${goalNode.id}downstreamPlanGroup`;
  addHorizontalConceptGroupNode(
    downstreamPlanGroupNodeId,
    allDownstreamPlans.length,
    'Performing Plans',
    nodes,
    [CBDIEditorRootConceptType.PlanConceptType],
    1,
    goalNode.id,
  );

  allDownstreamPlans.forEach((downstreamPlanObj) => {
    const planNode: CbdiEditTreeGraphNode = getCbdiEditTreeGraphNodeByObject(downstreamPlanObj, goalNode.id);

    planNode.data = { ...planNode.data, groupId: downstreamPlanGroupNodeId };

    // if has selected tactic obj
    // fade downstream plans which is not in tactic plan list
    if (selectedTacticObj && !selectedTacticObj.plan_list.some((el) => areModuleConceptsEqual(el.moduleConcept, downstreamPlanObj))) {
      planNode.data = { ...planNode.data, faded: true };
    }

    nodes.push(planNode);
    const downstreamPlanEdge = getCbdiEditTreeGraphEdge(goalNode, planNode);
    edges.push(downstreamPlanEdge);
  });

  // 6. add tactics
  const allTactics = project.tactics
    .map((moduleConcept) => {
      const tacticObj = getObjectByModuleConcept(project, moduleConcept) as CBDIEditorProjectTactic | undefined;
      if (tacticObj && tacticObj._mod !== Mod.Deletion && areModuleConceptsEqual(tacticObj.goal, goalObj)) {
        return tacticObj;
      }
      return undefined;
    })
    .filter((el) => el !== undefined) as CBDIEditorProjectTactic[];

  addSideVerticalConceptGroupNode(
    `${goalNode.id}tacticGroup`,
    allTactics,
    CBDIEditorRootConceptType.TacticConceptType,
    0,
    nodes,
    project,
    'Tactics',
    [CBDIEditorRootConceptType.TacticConceptType],
  );

  // if has selected tactic obj
  // fade other tactic node
  if (selectedTacticObj) {
    nodes.forEach((node) => {
      if (node.data._objectType === CBDIEditorRootConceptType.TacticConceptType) {
        if (!areModuleConceptsEqual(node.data, selectedTacticObj)) {
          node.data = { ...node.data, faded: true };
        }
      }
    });
  }

  return { nodes, edges };
}

export function drawPlanModuleConceptGraph(
  project: CBDIEditorProject,
  planObj: CBDIEditorProjectPlan,
): {
  nodes: CbdiEditTreeGraphNode[];
  edges: Edge[];
} {
  const nodes: CbdiEditTreeGraphNode[] = [];
  const edges: Edge[] = [];
  // 1. add plan
  const planNode: CbdiEditTreeGraphNode = getCbdiEditTreeGraphNodeByObject(planObj);
  nodes.push(planNode);

  // 2. add goal
  if (planObj.handles) {
    const goalNode = getCbdiEditTreeGraphNodeByModuleConcept(project, planObj.handles, CBDIEditorRootConceptType.GoalConceptType);

    nodes.push(goalNode);
    const goalEdge = getCbdiEditTreeGraphEdge(goalNode, planNode);
    edges.push(goalEdge);

    // 3. add agent
    project.agents.forEach((moduleConcept) => {
      const agentObj = getObjectByModuleConcept(project, moduleConcept) as CBDIEditorProjectAgent | undefined;
      if (agentObj && agentObj.goals.some((goal) => areModuleConceptsEqual(goal, planObj.handles))) {
        const agentNode: CbdiEditTreeGraphNode = getCbdiEditTreeGraphNodeByObject(agentObj);
        nodes.push(agentNode);
        const handleGoalNode = getCbdiEditTreeGraphNodeByModuleConcept(project, planObj.handles!, CBDIEditorRootConceptType.GoalConceptType);
        const agentEdge = getCbdiEditTreeGraphEdge(agentNode, handleGoalNode);
        edges.push(agentEdge);
      }
    });
  }

  // 4. add subgoal and action task
  planObj.tasks.forEach((task) => {
    const taskModuleConcept = task.nodeData.action || task.nodeData.goal;
    if (taskModuleConcept) {
      const taskObjectType = task.nodeData.action ? CBDIEditorRootConceptType.ActionConceptType : CBDIEditorRootConceptType.GoalConceptType;
      const taskNode = getCbdiEditTreeGraphNodeByModuleConcept(project, taskModuleConcept, taskObjectType);

      nodes.push(taskNode);
      const taskLink = getCbdiEditTreeGraphEdge(planNode, taskNode);
      edges.push(taskLink);
    }
  });

  return { nodes, edges };
}

export function drawTacticModuleConceptGraph(
  project: CBDIEditorProject,
  tacticObj: CBDIEditorProjectTactic,
): {
  nodes: CbdiEditTreeGraphNode[];
  edges: Edge[];
} {
  const goalObj = getObjectByModuleConcept(project, tacticObj.goal) as CBDIEditorProjectGoal;
  if (goalObj) {
    return drawGoalModuleConceptGraph(project, goalObj, tacticObj);
  }
  return { nodes: [], edges: [] };
}

export function drawActionModuleConceptGraph(
  project: CBDIEditorProject,
  actionObj: CBDIEditorProjectAction,
): {
  nodes: CbdiEditTreeGraphNode[];
  edges: Edge[];
} {
  const nodes: CbdiEditTreeGraphNode[] = [];
  const edges: Edge[] = [];
  // 1. add action
  const actionNode: CbdiEditTreeGraphNode = getCbdiEditTreeGraphNodeByObject(actionObj);
  nodes.push(actionNode);

  // 2. add plan

  // upstream plan group
  const upstreamPlanGroupNodeId = `${actionNode.id}upstreamPlanGroup`;
  const upstreamPlanObjs: CBDIEditorProjectPlan[] = [];
  project.plans.forEach((moduleConcept) => {
    const planObj = getObjectByModuleConcept(project, moduleConcept) as CBDIEditorProjectPlan | undefined;
    if (
      planObj &&
      planObj.tasks.some((task) => {
        const taskModuleConcept = task.nodeData.action || task.nodeData.goal;
        if (taskModuleConcept && areModuleConceptsEqual(taskModuleConcept, actionObj)) {
          return true;
        }
        return false;
      })
    ) {
      upstreamPlanObjs.push(planObj);
    }
  });

  addHorizontalConceptGroupNode(
    upstreamPlanGroupNodeId,
    upstreamPlanObjs.length,
    'Used In',
    nodes,
    [CBDIEditorRootConceptType.PlanConceptType],
    -1,
    actionNode.id,
  );
  upstreamPlanObjs.forEach((planObj) => {
    const planNode: CbdiEditTreeGraphNode = getCbdiEditTreeGraphNodeByObject(planObj);
    planNode.data = { ...planNode.data, groupId: upstreamPlanGroupNodeId };
    nodes.push(planNode);
    const planEdge = getCbdiEditTreeGraphEdge(planNode, actionNode);
    edges.push(planEdge);
  });

  // 3. add downstream nodes
  const downstreamPlanGroupNodeId = `${actionNode.id}downstreamNodeGroup`;
  const downstreamNodeObjs: CBDIEditorObject[] = [];

  // 3.1 add agent
  project.agents.forEach((moduleConcept) => {
    const agentObj = getObjectByModuleConcept(project, moduleConcept) as CBDIEditorProjectAgent | undefined;
    if (agentObj && agentObj.action_handlers.some((action) => areModuleConceptsEqual(action, actionObj))) {
      downstreamNodeObjs.push(agentObj);
    }
  });

  // 3.2 add team
  project.teams.forEach((moduleConcept) => {
    const teamObj = getObjectByModuleConcept(project, moduleConcept) as CBDIEditorProjectTeam | undefined;
    if (teamObj && teamObj.action_handlers.some((action) => areModuleConceptsEqual(action, actionObj))) {
      downstreamNodeObjs.push(teamObj);
    }
  });

  // 3.3 add service
  project.services.forEach((moduleConcept) => {
    const serviceObj = getObjectByModuleConcept(project, moduleConcept) as CBDIEditorProjectService | undefined;
    if (serviceObj && serviceObj.action_handlers.some((action) => areModuleConceptsEqual(action, actionObj))) {
      downstreamNodeObjs.push(serviceObj);
    }
  });

  // 3.4 add downstream group node
  addHorizontalConceptGroupNode(
    downstreamPlanGroupNodeId,
    downstreamNodeObjs.length,
    'Used By',
    nodes,
    [CBDIEditorRootConceptType.AgentConceptType, CBDIEditorRootConceptType.TeamConceptType, CBDIEditorRootConceptType.ServiceConceptType],
    1,
    actionNode.id,
  );
  downstreamNodeObjs.forEach((obj) => {
    const downstreamNode: CbdiEditTreeGraphNode = getCbdiEditTreeGraphNodeByObject(obj);
    downstreamNode.data = {
      ...downstreamNode.data,
      groupId: downstreamPlanGroupNodeId,
    };
    nodes.push(downstreamNode);
    const downstreamEdge = getCbdiEditTreeGraphEdge(actionNode, downstreamNode);
    edges.push(downstreamEdge);
  });

  // 4. add request message
  if (!areModuleConceptsEqual(actionObj.request, EmptyModuleConcept)) {
    const requestMessageNode = getCbdiEditTreeGraphNodeByModuleConcept(
      project,
      actionObj.request,
      CBDIEditorRootConceptType.MessageConceptType,
      `${actionNode.id}requestMessage`,
    );
    requestMessageNode.data = {
      ...requestMessageNode.data,
      relativeNodeId: actionNode.id,
      relativePosition: 'left',
      isNotReLayout: true,
    };
    const requestMessageEdge = getCbdiEditTreeGraphEdge(requestMessageNode, actionNode, 'LR', 'Request Message');
    nodes.push(requestMessageNode);
    edges.push(requestMessageEdge);
  }

  // 5. add reply message
  if (!areModuleConceptsEqual(actionObj.reply, EmptyModuleConcept)) {
    const requestMessageNode = getCbdiEditTreeGraphNodeByModuleConcept(
      project,
      actionObj.reply,
      CBDIEditorRootConceptType.MessageConceptType,
      `${actionNode.id}replyMessage`,
    );
    requestMessageNode.data = {
      ...requestMessageNode.data,
      relativeNodeId: actionNode.id,
      relativePosition: 'right',
      isNotReLayout: true,
    };
    const requestMessageEdge = getCbdiEditTreeGraphEdge(actionNode, requestMessageNode, 'LR', 'Reply Message');
    nodes.push(requestMessageNode);
    edges.push(requestMessageEdge);
  }

  return { nodes, edges };
}

export function drawServiceModuleConceptGraph(
  project: CBDIEditorProject,
  serviceObj: CBDIEditorProjectService,
): {
  nodes: CbdiEditTreeGraphNode[];
  edges: Edge[];
} {
  const nodes: CbdiEditTreeGraphNode[] = [];
  const edges: Edge[] = [];

  // 1. add service
  const serviceNode: CbdiEditTreeGraphNode = getCbdiEditTreeGraphNodeByObject(serviceObj);

  nodes.push(serviceNode);

  // 2. add team and agent
  const agentGroupNodeId = `${serviceNode.id}agentGroup`;
  const agentObjs: (CBDIEditorProjectAgent | CBDIEditorProjectTeam)[] = [];
  const allAgentIds = [...project.teams, ...project.agents];
  allAgentIds.forEach((moduleConcept) => {
    const agentObj = getObjectByModuleConcept(project, moduleConcept) as CBDIEditorProjectAgent | CBDIEditorProjectTeam | undefined;
    if (agentObj && agentObj._mod !== Mod.Deletion) {
      if (agentObj.services.some((el) => areModuleConceptsEqual(el, serviceObj))) {
        agentObjs.push(agentObj);
      }
    }
  });

  addHorizontalConceptGroupNode(
    agentGroupNodeId,
    allAgentIds.length,
    'Agents',
    nodes,
    [CBDIEditorRootConceptType.AgentConceptType, CBDIEditorRootConceptType.TeamConceptType],
    -1,
    serviceNode.id,
  );
  if (agentObjs.length > 0) {
    agentObjs.forEach((agentObj) => {
      const agentNode = getCbdiEditTreeGraphNodeByObject(agentObj, serviceNode.id);
      agentNode.data = {
        ...agentNode.data,
        groupId: agentGroupNodeId,
      };
      const agentEdge = getCbdiEditTreeGraphEdge(agentNode, serviceNode);
      nodes.push(agentNode);
      edges.push(agentEdge);
    });
  }
  // if no team/agent use service, create a hidden agent node, so the service node and agent group node will not overlap
  else {
    const hiddenAgentNode = getCbdiEditTreeGraphNodeByModuleConcept(
      project,
      EmptyModuleConcept,
      CBDIEditorRootConceptType.AgentConceptType,
      `${serviceNode.id}emptyPlan`,
    );
    hiddenAgentNode.hidden = true;
    hiddenAgentNode.data = {
      ...hiddenAgentNode.data,
      groupId: agentGroupNodeId,
    };
    const agentEdge = getCbdiEditTreeGraphEdge(hiddenAgentNode, serviceNode);
    nodes.push(hiddenAgentNode);
    edges.push(agentEdge);
  }

  // 3. add action
  const actionGroupNodeId = `${serviceNode.id}downstreamActionGroup`;
  addHorizontalConceptGroupNode(
    actionGroupNodeId,
    serviceObj.action_handlers.length,
    'Actions',
    nodes,
    [CBDIEditorRootConceptType.ActionConceptType],
    1,
    serviceNode.id,
  );
  serviceObj.action_handlers.forEach((actionConcept) => {
    const actionNode = getCbdiEditTreeGraphNodeByModuleConcept(project, actionConcept, CBDIEditorRootConceptType.ActionConceptType, serviceNode.id);
    actionNode.data = { ...actionNode.data, groupId: actionGroupNodeId };
    nodes.push(actionNode);
    const planEdge = getCbdiEditTreeGraphEdge(serviceNode, actionNode);
    edges.push(planEdge);
  });

  // 4. add topic
  const topicGroupNodeId = `${serviceNode.id}topicGroup`;

  const topicModuleConcepts = serviceObj.topics.map((topic) => ({
    ...topic.message,
    name: topic.name,
  }));

  addSideVerticalConceptGroupNode(topicGroupNodeId, topicModuleConcepts, CBDIEditorRootConceptType.MessageConceptType, 0, nodes, project, 'Topics', [
    CBDIEditorRootConceptType.MessageConceptType,
  ]);

  return { nodes, edges };
}

export function drawResourceModuleConceptGraph(
  project: CBDIEditorProject,
  resourceObj: CBDIEditorProjectResource,
): {
  nodes: CbdiEditTreeGraphNode[];
  edges: Edge[];
} {
  const nodes: CbdiEditTreeGraphNode[] = [];
  const edges: Edge[] = [];

  // 1. add resource
  const resourceNode: CbdiEditTreeGraphNode = getCbdiEditTreeGraphNodeByObject(resourceObj);

  nodes.push(resourceNode);

  return { nodes, edges };
}

export function drawMessageModuleConceptGraph(
  project: CBDIEditorProject,
  messageObj: CBDIEditorProjectMessage,
): {
  nodes: CbdiEditTreeGraphNode[];
  edges: Edge[];
} {
  const nodes: CbdiEditTreeGraphNode[] = [];
  const edges: Edge[] = [];

  // 1. add message
  const messageNode: CbdiEditTreeGraphNode = getCbdiEditTreeGraphNodeByObject(messageObj);

  nodes.push(messageNode);

  // 2. add message field
  messageObj.fields.forEach((field) => {
    const fieldNode = getCbdiEditTreeGraphNodeByObject(messageObj, field.id);
    fieldNode.data = {
      ...fieldNode.data,
      name: field.name,
      customNodeIconType: 'messageField',
    };
    const fieldEdge = getCbdiEditTreeGraphEdge(messageNode, fieldNode);
    nodes.push(fieldNode);
    edges.push(fieldEdge);
  });

  return { nodes, edges };
}

export function drawEntityModuleConceptGraph(
  project: CBDIEditorProject,
  entityObj: CBDIEditorProjectEntity,
): {
  nodes: CbdiEditTreeGraphNode[];
  edges: Edge[];
} {
  const nodes: CbdiEditTreeGraphNode[] = [];
  const edges: Edge[] = [];

  // 1. add entity
  const entityNode: CbdiEditTreeGraphNode = getCbdiEditTreeGraphNodeByObject(entityObj);
  if (entityObj.agent) {
    entityNode.data = { ...entityNode.data, nodeBadgeType: 'agent' };
  }
  nodes.push(entityNode);

  // 2. add components
  const upstreamComponentGroupId = `${entityNode.id}upstreamComponentGroup`;
  addHorizontalConceptGroupNode(
    upstreamComponentGroupId,
    entityObj.messages.length + entityObj.services.length,
    'Components',
    nodes,
    [CBDIEditorRootConceptType.MessageConceptType, CBDIEditorRootConceptType.ServiceConceptType],
    -1,
    entityNode.id,
  );

  entityObj.messages.forEach((messageModuleConcept) => {
    const componentNode = getCbdiEditTreeGraphNodeByModuleConcept(
      project,
      messageModuleConcept,
      CBDIEditorRootConceptType.MessageConceptType,
      entityNode.id,
    );
    componentNode.data = {
      ...componentNode.data,
      groupId: upstreamComponentGroupId,
    };
    const componentEdge = getCbdiEditTreeGraphEdge(componentNode, entityNode);
    nodes.push(componentNode);
    edges.push(componentEdge);
  });

  entityObj.services.forEach((serviceModuleConcept) => {
    const componentNode = getCbdiEditTreeGraphNodeByModuleConcept(
      project,
      serviceModuleConcept,
      CBDIEditorRootConceptType.ServiceConceptType,
      entityNode.id,
    );
    componentNode.data = {
      ...componentNode.data,
      groupId: upstreamComponentGroupId,
    };

    const componentEdge = getCbdiEditTreeGraphEdge(componentNode, entityNode);
    nodes.push(componentNode);
    edges.push(componentEdge);
  });

  // 3. add child entities
  const downstreamEntityChildrenGroupId = `${entityNode.id}downstreamEntityChildrenGroup`;
  addHorizontalConceptGroupNode(
    downstreamEntityChildrenGroupId,
    entityObj.children.length,
    'Child Entities',
    nodes,
    [CBDIEditorRootConceptType.EntityConceptType],
    1,
    entityNode.id,
  );
  entityObj.children.forEach((childEntity) => {
    const childEntityNode = getCbdiEditTreeGraphNodeByModuleConcept(project, childEntity, CBDIEditorRootConceptType.EntityConceptType, entityNode.id);
    const childEntityObj = getObjectByModuleConcept(project, childEntity) as CBDIEditorProjectEntity | undefined;
    if (childEntityObj?.agent) {
      childEntityNode.data = {
        ...childEntityNode.data,
        nodeBadgeType: 'agent',
      };
    }

    childEntityNode.data = {
      ...childEntityNode.data,
      groupId: downstreamEntityChildrenGroupId,
    };

    const childEntityEdge = getCbdiEditTreeGraphEdge(entityNode, childEntityNode);
    nodes.push(childEntityNode);
    edges.push(childEntityEdge);
  });

  return { nodes, edges };
}

export function drawEventModuleConceptGraph(
  project: CBDIEditorProject,
  eventObj: CBDIEditorProjectEvent,
): {
  nodes: CbdiEditTreeGraphNode[];
  edges: Edge[];
} {
  const nodes: CbdiEditTreeGraphNode[] = [];
  const edges: Edge[] = [];

  // 1. add event
  const eventNode: CbdiEditTreeGraphNode = getCbdiEditTreeGraphNodeByObject(eventObj);

  nodes.push(eventNode);

  // 2. add message
  const messageNode = getCbdiEditTreeGraphNodeByModuleConcept(project, eventObj.message, CBDIEditorRootConceptType.MessageConceptType, eventNode.id);
  const messageEdge = getCbdiEditTreeGraphEdge(eventNode, messageNode);
  nodes.push(messageNode);
  edges.push(messageEdge);

  return { nodes, edges };
}

export function drawEnumModuleConceptGraph(
  project: CBDIEditorProject,
  enumObj: CBDIEditorProjectEnum,
): {
  nodes: CbdiEditTreeGraphNode[];
  edges: Edge[];
} {
  const nodes: CbdiEditTreeGraphNode[] = [];
  const edges: Edge[] = [];

  // 1. add enum
  const enumNode: CbdiEditTreeGraphNode = getCbdiEditTreeGraphNodeByObject(enumObj);

  nodes.push(enumNode);

  return { nodes, edges };
}

/**
 * populate downstream roles and agents
 * @param agentRoleItem targeting agent role item
 * @param agentRoleArray all agent role items
 * @param nodes
 * @param edges
 * @param project
 * @param maxDepth
 * @param prefixId
 */
const populateOverviewAgentRole = (
  agentRoleItem: AgentRoleItem,
  agentRoleArray: AgentRoleItem[],
  nodes: CbdiEditTreeGraphNode[],
  edges: Edge[],
  project: CBDIEditorProject,
  maxDepth: number,
  prefixId?: string,
) => {
  const agentNode = getCbdiEditTreeGraphNodeByModuleConcept(project, agentRoleItem.agent, agentRoleItem.type, prefixId);
  nodes.push(agentNode);
  agentRoleItem.downstreamRoles.forEach((downstreamRole) => {
    const roleNode = getCbdiEditTreeGraphNodeByModuleConcept(project, downstreamRole, CBDIEditorRootConceptType.RoleConceptType, agentNode.id);
    nodes.push(roleNode);
    const roleEdge = getCbdiEditTreeGraphEdge(agentNode, roleNode);
    edges.push(roleEdge);
    const downstreamAgents = agentRoleArray.filter((el) => {
      if (
        !areModuleConceptsEqual(el.agent, agentRoleItem.agent) &&
        el.upstreamRoles.some((upstreamRole) => areModuleConceptsEqual(upstreamRole, downstreamRole))
      ) {
        return true;
      }
      return false;
    });

    downstreamAgents.forEach((downstreamAgent) => {
      const downstreamAgentNode = getCbdiEditTreeGraphNodeByModuleConcept(project, downstreamAgent.agent, downstreamAgent.type, roleNode.id);
      nodes.push(downstreamAgentNode);
      const downstreamAgentEdge = getCbdiEditTreeGraphEdge(roleNode, downstreamAgentNode);
      edges.push(downstreamAgentEdge);
      if (maxDepth > 1) {
        populateOverviewAgentRole(downstreamAgent, agentRoleArray, nodes, edges, project, maxDepth - 1, roleNode.id);
      }
    });
  });
};

export function drawProjectOverviewGraph(project: CBDIEditorProject, overviewMaxDepth: number) {
  const nodes: CbdiEditTreeGraphNode[] = [];
  const edges: Edge[] = [];

  // 1. get agent role array
  const agentRoleArray: AgentRoleItem[] = getAgentRoleArray(project);

  // 2. get the starting agents
  const startingAgents = agentRoleArray.filter((el) => el.upstreamRoles.length === 0 && el.type === CBDIEditorRootConceptType.TeamConceptType);

  // 3. populate downstream role and agent base on max depth
  startingAgents.forEach((startingAgent) => {
    populateOverviewAgentRole(startingAgent, agentRoleArray, nodes, edges, project, overviewMaxDepth);
  });

  return { nodes, edges };
}

export function drawModuleProjectOverviewGraph(project: CBDIEditorProject, moduleName: string, overviewMaxDepth: number) {
  const nodes: CbdiEditTreeGraphNode[] = [];
  const edges: Edge[] = [];

  // 1. get agent role array where agent are from provided module
  const allAgentRoleArray: AgentRoleItem[] = getAgentRoleArray(project);
  const agentRoleArray: AgentRoleItem[] = allAgentRoleArray.filter((el) => el.agent.module === moduleName);

  // 2. get the starting agents
  const startingAgents = agentRoleArray.filter((el) => el.upstreamRoles.length === 0 && el.type === CBDIEditorRootConceptType.TeamConceptType);

  // 3. populate downstream role and agent base on max depth
  startingAgents.forEach((startingAgent) => {
    populateOverviewAgentRole(startingAgent, agentRoleArray, nodes, edges, project, overviewMaxDepth);
  });

  return { nodes, edges };
}
