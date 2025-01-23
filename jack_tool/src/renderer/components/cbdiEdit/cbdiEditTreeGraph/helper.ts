/* eslint-disable no-param-reassign */
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
} from 'misc/types/cbdiEdit/cbdiEditModel';
import {
  CBDIEditorObject,
  CBDIEditorRootConceptType,
  CBDIEditorSharedMessage,
  EmptyModuleConcept,
  Mod,
  ModuleConcept,
} from 'misc/types/cbdiEdit/cbdiEditTypes';
import { copy, getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { Edge, Position, XYPosition } from 'reactflow';
import dagre from 'dagre';
import { areModuleConceptsEqual, isModuleConceptOverview } from 'misc/utils/common/commonUtils';
import { v4 } from 'uuid';
import { CbdiEditTreeGraphNode } from './types';
import {
  drawActionModuleConceptGraph,
  drawGoalModuleConceptGraph,
  drawProjectOverviewGraph,
  drawPlanModuleConceptGraph,
  drawRoleModuleConceptGraph,
  drawServiceModuleConceptGraph,
  drawAgentModuleConceptGraph,
  drawModuleProjectOverviewGraph,
  drawResourceModuleConceptGraph,
  drawMessageModuleConceptGraph,
  drawEntityModuleConceptGraph,
  drawEventModuleConceptGraph,
  drawEnumModuleConceptGraph,
  drawTacticModuleConceptGraph,
} from './drawModuleConceptTreeGraph';
import {
  CBDI_EDIT_TREE_GRAPH_EDGE_SEP,
  CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT,
  CBDI_EDIT_TREE_GRAPH_NODE_SEP,
  CBDI_EDIT_TREE_GRAPH_NODE_WIDTH,
  CBDI_EDIT_TREE_GRAPH_RANK_SEP,
} from './constants';

type Item = {
  id: string;
};

function removeDuplicatesById<T extends Item>(array: T[]): T[] {
  return array.reduce((accumulator: T[], current: T) => {
    const duplicate = accumulator.find((item) => item.id === current.id);
    if (!duplicate) {
      accumulator.push(current);
    }
    return accumulator;
  }, []);
}

type NonLayoutGroupNodeDic = {
  [groupId: string]: CbdiEditTreeGraphNode[];
};

const getLayoutedElements = (
  nodes: CbdiEditTreeGraphNode[],
  edges: Edge[],
  direction = 'TB',
): {
  nodes: CbdiEditTreeGraphNode[];
  edges: Edge[];
} => {
  const dagreGraph = new dagre.graphlib.Graph();
  dagreGraph.setDefaultEdgeLabel(() => ({}));
  const isHorizontal = direction === 'LR';
  dagreGraph.setGraph({
    rankdir: direction,
    ranksep: CBDI_EDIT_TREE_GRAPH_RANK_SEP,
    edgesep: CBDI_EDIT_TREE_GRAPH_EDGE_SEP,
    nodesep: CBDI_EDIT_TREE_GRAPH_NODE_SEP,
  });

  nodes.forEach((node) => {
    // only do layout for not notLayout node
    if (!node.data.isNotReLayout) {
      dagreGraph.setNode(node.id, {
        width: node.width,
        height: node.height,
      });
    }
  });

  edges.forEach((edge) => {
    // only do layout for not notLayout edge
    const sourceNode = nodes.find((el) => el.id === edge.source);
    const targetNode = nodes.find((el) => el.id === edge.target);
    if (sourceNode && targetNode && !sourceNode.data.isNotReLayout && !targetNode.data.isNotReLayout) {
      dagreGraph.setEdge(edge.source, edge.target);
    }
  });

  dagre.layout(dagreGraph);

  nodes.forEach((node) => {
    // only do layout for not notLayout node
    if (!node.data.isNotReLayout) {
      const nodeWithPosition = dagreGraph.node(node.id);
      node.targetPosition = isHorizontal ? Position.Left : Position.Top;
      node.sourcePosition = isHorizontal ? Position.Right : Position.Bottom;

      // We are shifting the dagre node position (anchor=center center) to the top left
      // so it matches the React Flow node anchor point (top left).
      node.position = {
        x: nodeWithPosition.x - node.width! / 2,
        y: nodeWithPosition.y - node.height! / 2,
      };
    }

    return node;
  });

  // relocate the group node which is not layout using dagre
  const nonLayoutGroupNodeDic: NonLayoutGroupNodeDic = {};
  nodes.forEach((node) => {
    if (node.data.groupId) {
      if (!nonLayoutGroupNodeDic[node.data.groupId]) {
        nonLayoutGroupNodeDic[node.data.groupId] = [];
      }
      nonLayoutGroupNodeDic[node.data.groupId].push(node);
    }
  });

  // get minX and maxX for all non layout group nodes
  // to make all layout group node have same width
  let minX = Infinity;
  let maxX = -Infinity;

  Object.values(nonLayoutGroupNodeDic).forEach((roleNodes) => {
    roleNodes.forEach((roleNode) => {
      if (roleNode.position.x < minX) {
        minX = roleNode.position.x;
      }
      if (roleNode.position.x > maxX) {
        maxX = roleNode.position.x;
      }
    });
  });

  // relocate horizontal group node which has child nodes
  Object.entries(nonLayoutGroupNodeDic).forEach(([groupId, roleNodes]) => {
    let minY = Infinity;
    let maxY = -Infinity;
    roleNodes.forEach((roleNode) => {
      if (roleNode.position.y < minY) {
        minY = roleNode.position.y;
      }
      if (roleNode.position.x > maxY) {
        maxY = roleNode.position.y;
      }
    });
    const groupNode = nodes.find((node) => node.id === groupId)!;
    groupNode.position = {
      x: minX - CBDI_EDIT_TREE_GRAPH_NODE_WIDTH * 4,
      y: minY - CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT * 0.6,
    };
    groupNode.width = maxX - minX + CBDI_EDIT_TREE_GRAPH_NODE_WIDTH * 9;
    groupNode.height = maxY - minY + CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT * 3;
    groupNode.style = {
      ...groupNode.style,
      width: groupNode.width,
      height: groupNode.height,
    };
  });

  // relocate horizontal group node which does not has child nodes
  // set minX and maxX to 0 if they are Infinity/-Infinity
  if (minX === Infinity) {
    minX = 0;
  }
  if (maxX === -Infinity) {
    maxX = 0;
  }
  const horizontalGroupWithChildren = Object.keys(nonLayoutGroupNodeDic);
  const horizontalGroupWithoutChildren = nodes
    .map((node) => {
      if (node.data.yDistanceFromCenter) {
        if (!horizontalGroupWithChildren.includes(node.id)) {
          return node;
        }
      }
      return undefined;
    })
    .filter((el) => el !== undefined) as CbdiEditTreeGraphNode[];
  // use center node and y distance from center to reposition the group node which does not have child nodes
  horizontalGroupWithoutChildren.forEach((groupNode) => {
    const centerNode = nodes.find((el) => el.id === groupNode.data.centerNodeId)!;
    groupNode.position = {
      x: minX - CBDI_EDIT_TREE_GRAPH_NODE_WIDTH * 4,
      y:
        centerNode.position.y + 3.5 * CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT * groupNode.data.yDistanceFromCenter! - CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT * 0.6,
    };
    groupNode.width = maxX - minX + CBDI_EDIT_TREE_GRAPH_NODE_WIDTH * 9;
    groupNode.height = CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT * 3;
    groupNode.style = {
      ...groupNode.style,
      width: groupNode.width,
      height: groupNode.height,
    };
  });

  // relocate the node which use other node's relative position
  nodes.forEach((node, index) => {
    if (node.data.relativeNodeId && node.data.relativePosition) {
      const relativeNode = nodes.find((el) => el.id === node.data.relativeNodeId);
      if (relativeNode) {
        if (node.data.relativePosition === 'left') {
          nodes[index].position = {
            x: relativeNode.position.x - CBDI_EDIT_TREE_GRAPH_NODE_WIDTH * 4,
            y: relativeNode.position.y,
          };
        } else if (node.data.relativePosition === 'right') {
          nodes[index].position = {
            x: relativeNode.position.x + CBDI_EDIT_TREE_GRAPH_NODE_WIDTH * 4,
            y: relativeNode.position.y,
          };
        }
      }
    }
  });

  return { nodes, edges };
};

export const getNodesAndEdgesByModuleConcept = (
  project: CBDIEditorProject | null,
  moduleConcept: ModuleConcept | null,
  overviewMaxDepth: number,
): { nodes: CbdiEditTreeGraphNode[]; edges: Edge[] } => {
  let nodes: CbdiEditTreeGraphNode[] = [];
  let edges: Edge[] = [];
  if (project) {
    const currentObj = getObjectByModuleConcept(project, moduleConcept);
    if (currentObj && currentObj._mod !== Mod.Deletion) {
      switch (currentObj._objectType) {
        case CBDIEditorRootConceptType.TeamConceptType:
          ({ nodes, edges } = drawAgentModuleConceptGraph(project, currentObj as CBDIEditorProjectTeam));
          break;

        case CBDIEditorRootConceptType.AgentConceptType:
          ({ nodes, edges } = drawAgentModuleConceptGraph(project, currentObj as CBDIEditorProjectAgent));
          break;

        case CBDIEditorRootConceptType.RoleConceptType:
          ({ nodes, edges } = drawRoleModuleConceptGraph(project, currentObj as CBDIEditorProjectRole));
          break;

        case CBDIEditorRootConceptType.GoalConceptType:
          ({ nodes, edges } = drawGoalModuleConceptGraph(project, currentObj as CBDIEditorProjectGoal));
          break;

        case CBDIEditorRootConceptType.PlanConceptType:
          ({ nodes, edges } = drawPlanModuleConceptGraph(project, currentObj as CBDIEditorProjectPlan));
          break;

        case CBDIEditorRootConceptType.TacticConceptType:
          ({ nodes, edges } = drawTacticModuleConceptGraph(project, currentObj as CBDIEditorProjectTactic));
          break;

        case CBDIEditorRootConceptType.ActionConceptType:
          ({ nodes, edges } = drawActionModuleConceptGraph(project, currentObj as CBDIEditorProjectAction));
          break;

        case CBDIEditorRootConceptType.ServiceConceptType:
          ({ nodes, edges } = drawServiceModuleConceptGraph(project, currentObj as CBDIEditorProjectService));
          break;

        case CBDIEditorRootConceptType.ResourceConceptType:
          ({ nodes, edges } = drawResourceModuleConceptGraph(project, currentObj as CBDIEditorProjectResource));
          break;

        case CBDIEditorRootConceptType.MessageConceptType:
          ({ nodes, edges } = drawMessageModuleConceptGraph(project, currentObj as CBDIEditorProjectMessage));
          break;

        case CBDIEditorRootConceptType.EntityConceptType:
          ({ nodes, edges } = drawEntityModuleConceptGraph(project, currentObj as CBDIEditorProjectEntity));
          break;

        case CBDIEditorRootConceptType.EventConceptType:
          ({ nodes, edges } = drawEventModuleConceptGraph(project, currentObj as CBDIEditorProjectEvent));
          break;

        case CBDIEditorRootConceptType.EnumConceptType:
          ({ nodes, edges } = drawEnumModuleConceptGraph(project, currentObj as CBDIEditorProjectEnum));
          break;

        default:
          break;
      }
    } else if (isModuleConceptOverview(moduleConcept)) {
      if (moduleConcept!.module === '') {
        ({ nodes, edges } = drawProjectOverviewGraph(project, overviewMaxDepth));
      } else {
        ({ nodes, edges } = drawModuleProjectOverviewGraph(project, moduleConcept!.module, overviewMaxDepth));
      }
    }
  }

  // remove duplicated nodes and edges
  nodes = removeDuplicatesById(nodes);
  edges = removeDuplicatesById(edges);

  return getLayoutedElements(nodes, edges);
};

/**
 * get landing group node when drag tree item to CbdiEditTreeGraph
 * @param dropPosition
 * @param groupNodes
 * @returns
 */
export const getDropGroupNode = (dropPosition: XYPosition, groupNodes: CbdiEditTreeGraphNode[]) => {
  const dropGroupNode = groupNodes.find((node) => {
    if (
      dropPosition.x > node.position.x &&
      dropPosition.x < node.position.x + node.width! &&
      dropPosition.y > node.position.y &&
      dropPosition.y < node.position.y + node.height!
    ) {
      return true;
    }
    return false;
  });

  return dropGroupNode;
};

const getCbdiEditorObjectFieldKey = (fieldObjectType: CBDIEditorRootConceptType, cbdiEditorObject: CBDIEditorObject) => {
  switch (cbdiEditorObject._objectType) {
    case CBDIEditorRootConceptType.TeamConceptType:
    case CBDIEditorRootConceptType.AgentConceptType:
      if (fieldObjectType === CBDIEditorRootConceptType.ActionConceptType) {
        return 'action_handlers';
      }
      if (fieldObjectType === CBDIEditorRootConceptType.MessageConceptType) {
        return 'beliefs';
      }
      return `${fieldObjectType}s`;

    case CBDIEditorRootConceptType.TacticConceptType:
      if (fieldObjectType === CBDIEditorRootConceptType.PlanConceptType) {
        return 'plan_list';
      }
      return `${fieldObjectType}s`;

    case CBDIEditorRootConceptType.EntityConceptType:
      if (fieldObjectType === CBDIEditorRootConceptType.EntityConceptType) {
        return 'children';
      }
      return `${fieldObjectType}s`;

    case CBDIEditorRootConceptType.ServiceConceptType:
      if (fieldObjectType === CBDIEditorRootConceptType.ActionConceptType) {
        return 'action_handlers';
      }
      if (fieldObjectType === CBDIEditorRootConceptType.MessageConceptType) {
        return 'topics';
      }

      return `${fieldObjectType}s`;

    default:
      return `${fieldObjectType}s`;
  }
};

export const handleAddModuleConceptByDrag = (
  addingModuleConcept: ModuleConcept,
  addingItemType: CBDIEditorRootConceptType,
  seletedTreeNodeObj: CBDIEditorObject,
  project: CBDIEditorProject,
) => {
  const newObject = copy(seletedTreeNodeObj);
  let newItem: any = addingModuleConcept;
  const cbdiEditorObjectFieldKey = getCbdiEditorObjectFieldKey(addingItemType, seletedTreeNodeObj);

  if (
    seletedTreeNodeObj._objectType === CBDIEditorRootConceptType.RoleConceptType &&
    addingItemType === CBDIEditorRootConceptType.MessageConceptType
  ) {
    newItem = {
      ...addingModuleConcept,
      write: false,
      read: false,
    } as CBDIEditorSharedMessage;
  }
  // if adding plan's handle goal is not included in agent/team 's goals, add it
  else if (
    (seletedTreeNodeObj._objectType === CBDIEditorRootConceptType.AgentConceptType ||
      seletedTreeNodeObj._objectType === CBDIEditorRootConceptType.TeamConceptType) &&
    addingItemType === CBDIEditorRootConceptType.PlanConceptType
  ) {
    const newPlan = getObjectByModuleConcept(project, newItem as ModuleConcept) as CBDIEditorProjectPlan;
    if (!(newObject as CBDIEditorProjectAgent | CBDIEditorProjectTeam).goals.some((goalItem) => areModuleConceptsEqual(goalItem, newPlan.handles))) {
      (newObject as CBDIEditorProjectAgent | CBDIEditorProjectTeam).goals.unshift({
        ...newPlan.handles,
        startup_goal: false,
        startup_tactic: EmptyModuleConcept,
      });
    }
  }
  // if adding role's goals are not included in agent/team 's goals, add them
  else if (
    (seletedTreeNodeObj._objectType === CBDIEditorRootConceptType.AgentConceptType ||
      seletedTreeNodeObj._objectType === CBDIEditorRootConceptType.TeamConceptType) &&
    addingItemType === CBDIEditorRootConceptType.RoleConceptType
  ) {
    const newRole = getObjectByModuleConcept(project, newItem as ModuleConcept) as CBDIEditorProjectRole;
    newRole.goals.forEach((goalModuleConcept) => {
      if (
        !(newObject as CBDIEditorProjectAgent | CBDIEditorProjectTeam).goals.some((goalItem) => areModuleConceptsEqual(goalItem, goalModuleConcept))
      ) {
        (newObject as CBDIEditorProjectAgent | CBDIEditorProjectTeam).goals.unshift({
          ...goalModuleConcept,
          startup_goal: false,
          startup_tactic: EmptyModuleConcept,
        });
      }
    });
  }
  // if adding goal into agent/team, add agentGoal
  else if (
    (seletedTreeNodeObj._objectType === CBDIEditorRootConceptType.AgentConceptType ||
      seletedTreeNodeObj._objectType === CBDIEditorRootConceptType.TeamConceptType) &&
    addingItemType === CBDIEditorRootConceptType.GoalConceptType
  ) {
    newItem = {
      ...newItem,
      startup_goal: false,
      startup_tactic: EmptyModuleConcept,
    };
  }

  // if adding plan into tactic, add into plan list
  else if (
    seletedTreeNodeObj._objectType === CBDIEditorRootConceptType.TacticConceptType &&
    addingItemType === CBDIEditorRootConceptType.PlanConceptType
  ) {
    newItem = {
      id: v4(),
      moduleConcept: newItem,
    };
    newObject[cbdiEditorObjectFieldKey].push(newItem);
    // if use plan list is false, make it true
    (newObject as CBDIEditorProjectTactic).use_plan_list = true;
    return newObject;
  }

  // if adding message into service, add topic
  else if (
    seletedTreeNodeObj._objectType === CBDIEditorRootConceptType.ServiceConceptType &&
    addingItemType === CBDIEditorRootConceptType.MessageConceptType
  ) {
    newItem = {
      name: `newTopic${v4().slice(0, 4)}`,
      message: newItem,
    };
    newObject[cbdiEditorObjectFieldKey].push(newItem);
    // if use plan list is false, make it true
    (newObject as CBDIEditorProjectTactic).use_plan_list = true;
    return newObject;
  }

  // if adding entity into entity, make sure entity is not itself
  else if (
    seletedTreeNodeObj._objectType === CBDIEditorRootConceptType.EntityConceptType &&
    addingItemType === CBDIEditorRootConceptType.EntityConceptType
  ) {
    if (areModuleConceptsEqual(seletedTreeNodeObj, addingModuleConcept)) {
      return seletedTreeNodeObj;
    }
  }

  if (newObject[cbdiEditorObjectFieldKey] && !newObject[cbdiEditorObjectFieldKey].some((el: any) => el.uuid === newItem.uuid)) {
    newObject[cbdiEditorObjectFieldKey].unshift(newItem);
  }
  return newObject;
};
