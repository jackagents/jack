import { BDILogIntentionTask } from 'types/cbdi/cbdi-models-non-flatbuffer';
import { TaskStatus } from 'constant/common/cmConstants';
import { BDILogGoalIntentionResult, BDILogLevel, BDILogType } from 'types/cbdi/cbdi-types-non-flatbuffer';
import { getLevelValue } from 'main/beBuilders/cbdi/intentionBuilder/getHighestLevelUtil/getHighestLevel';
import { Node, Edge, MarkerType } from 'reactflow';
import { CIRCLE_NODE_WIDTH, RECTANGLE_NODE_HEIGHT, RECTANGLE_NODE_WIDTH } from 'components/cbdiEdit/reactFlowPlanEditor/reactFlowPlanEditorConstant';
import { DELEGATED_PLAN_NAME } from 'components/common/runtimeExplainability/constants';
import { v4 } from 'uuid';
import { CBDIEditorPlanEditorEdgeCondition, CBDIEditorPlanNodeType, CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { removeBeforeFirstDotAndDot } from 'misc/utils/common/commonUtils';
import { IntentionContextNodeType } from './CustomNodes/IntentionContextNode';
import { ActiveNodeData } from './CustomNodes/NodeDataType';
import { ActiveEdgeData } from './CustomEdges/CustomActiveEdge';

export interface ActiveTaskDic {
  [taskId: string]: {
    status: TaskStatus;
    bdiLogLevel: BDILogLevel;
    isCurrentTask?: boolean;
    remark?: string | undefined;
    subGoalId?: string | undefined;
  };
}

export interface ActiveEdge {
  sourceTaskId: string;
  targetTaskId: string;
  toLastIndex: number;
  count: number;
}

// if there are multiple active task with same task id
// if the bdi log level is same or higher, update level and remark of task
const getBdiLogLevelAndReasoningText = (activeTaskDic: ActiveTaskDic, task: BDILogIntentionTask) => {
  if (activeTaskDic[task.taskId]) {
    const currentbdiLogLevel = activeTaskDic[task.taskId].bdiLogLevel;
    const inComingbdiLogLevel = task.level;
    if (getLevelValue(currentbdiLogLevel) > getLevelValue(inComingbdiLogLevel)) {
      return {
        level: currentbdiLogLevel,
        remark: activeTaskDic[task.taskId].remark,
      };
    }
  }
  if (task.logType === BDILogType.ACTION_STARTED || task.logType === BDILogType.ACTION_FINISHED) {
    return { level: task.level, remark: task.reasoning };
  }
  if (task.logType === BDILogType.SUB_GOAL_STARTED || task.logType === BDILogType.SUB_GOAL_FINISHED) {
    return {
      level: task.level,
      remark: task.dropReason ? `Drop reason: ${task.dropReason}` : undefined,
    };
  }
  return { level: task.level, remark: undefined };
};

const END_TASK_LOG_TYPES = [BDILogType.ACTION_FINISHED, BDILogType.SUB_GOAL_FINISHED, BDILogType.CONDITION, BDILogType.SLEEP_FINISHED];

export function getActiveTaskAndEdge(bdiTasks: BDILogIntentionTask[], planId: string) {
  const activeTaskDic: ActiveTaskDic = {};

  const activeEdgeArray: ActiveEdge[] = [];

  let previousTask: BDILogIntentionTask;

  bdiTasks.forEach((task, index) => {
    const { level, remark } = getBdiLogLevelAndReasoningText(activeTaskDic, task);

    switch (task.logType) {
      case BDILogType.INTENTION_STARTED:
        activeTaskDic[task.taskId] = {
          status: TaskStatus.STARTED,
          bdiLogLevel: level,
        };
        break;

      case BDILogType.INTENTION_FINISHED: {
        let status = TaskStatus.FAILED;
        switch (task.intentionResult) {
          case BDILogGoalIntentionResult.DROPPED:
            status = TaskStatus.DROPPED;
            break;
          case BDILogGoalIntentionResult.SUCCESS:
            status = TaskStatus.SUCCESS;
            break;
          default:
            break;
        }

        const intentionStatus = status;
        activeTaskDic[task.taskId] = {
          status: intentionStatus,
          bdiLogLevel: level,
        };
        break;
      }

      case BDILogType.ACTION_STARTED:
        activeTaskDic[task.taskId] = {
          status: TaskStatus.STARTED,
          bdiLogLevel: level,
          remark,
        };
        break;

      case BDILogType.ACTION_FINISHED: {
        const actionStatus = task.actionSuccess ? TaskStatus.SUCCESS : TaskStatus.FAILED;

        activeTaskDic[task.taskId] = {
          status: actionStatus,
          bdiLogLevel: level,
          remark,
        };
        break;
      }

      case BDILogType.SUB_GOAL_STARTED:
        activeTaskDic[task.taskId] = {
          status: TaskStatus.STARTED,
          bdiLogLevel: level,
          subGoalId: task.goalId,
          remark,
        };
        break;

      case BDILogType.SUB_GOAL_FINISHED: {
        const goalStatus = task.goalResult ? TaskStatus.SUCCESS : TaskStatus.FAILED;

        activeTaskDic[task.taskId] = {
          status: goalStatus,
          bdiLogLevel: level,
          subGoalId: task.goalId,
          remark,
        };
        break;
      }

      case BDILogType.CONDITION: {
        const conditionStatus = task.conditionSuccess ? TaskStatus.SUCCESS : TaskStatus.FAILED;
        activeTaskDic[task.taskId] = {
          status: conditionStatus,
          bdiLogLevel: level,
        };
        break;
      }

      case BDILogType.SLEEP_STARTED:
        activeTaskDic[task.taskId] = {
          status: TaskStatus.STARTED,
          bdiLogLevel: level,
        };
        break;

      case BDILogType.SLEEP_FINISHED:
        activeTaskDic[task.taskId] = {
          status: TaskStatus.SUCCESS,
          bdiLogLevel: level,
        };
        break;

      default:
        break;
    }
    if (index === bdiTasks.length - 1) {
      activeTaskDic[task.taskId].isCurrentTask = true;
    } else {
      activeTaskDic[task.taskId].isCurrentTask = false;
    }
    const toLastIndex = bdiTasks.length - index - 1;

    if (previousTask) {
      // if previous node is start node
      if (previousTask.taskId === CBDIEditorPlanNodeType.StartPlanNodeType) {
        activeEdgeArray.push({
          sourceTaskId: `${CBDIEditorPlanNodeType.StartPlanNodeType}/${planId.replace(/-/g, '')}`,
          targetTaskId: task.taskId,
          toLastIndex,
          count: 1,
        });
      }
      // if current node is end node
      else if (task.taskId === CBDIEditorPlanNodeType.EndPlanNodeType) {
        activeEdgeArray.push({
          sourceTaskId: previousTask.taskId,
          targetTaskId: `${CBDIEditorPlanNodeType.EndPlanNodeType}/${planId.replace(/-/g, '')}`,
          toLastIndex,
          count: 1,
        });
      }
      // only update activeEdgeArray when the previous task is 'finished' task
      else if (END_TASK_LOG_TYPES.includes(previousTask.logType)) {
        const edgeIndex = activeEdgeArray.findIndex((el) => el.sourceTaskId === previousTask.taskId && el.targetTaskId === task.taskId);
        if (edgeIndex > -1) {
          activeEdgeArray[edgeIndex].toLastIndex = toLastIndex;
          activeEdgeArray[edgeIndex].count += 1;
        } else {
          activeEdgeArray.push({
            sourceTaskId: previousTask.taskId,
            targetTaskId: task.taskId,
            toLastIndex,
            count: 1,
          });
        }
      }
    }
    previousTask = task;
  });
  return { activeTaskDic, activeEdgeArray };
}

export const capitalize = (str: string) => str.charAt(0).toUpperCase() + str.slice(1);

// add intention context nodes into initial nodes
export const addIntentionContextIntoNodes = (
  nodes: Node[],
  edges: Edge[],
  agentName: string | undefined,
  planName: string | undefined,
  goalName: string,
) => {
  const mnodes = [...nodes];
  const medges = [...edges];
  // get start node position
  const startNodePosition = mnodes.find((node) => node.data.type === CBDIEditorPlanNodeType.StartPlanNodeType)?.position || { x: 0, y: 0 };

  const DISTANCE_BETWEEN_NODE = RECTANGLE_NODE_HEIGHT * 2;
  const positionX = startNodePosition.x - (RECTANGLE_NODE_WIDTH - CIRCLE_NODE_WIDTH) / 2;
  // if plan name is DELEGATED_PLAN_NAME
  if (planName === DELEGATED_PLAN_NAME) {
    const goalNode = {
      id: v4(),
      position: {
        x: positionX,
        y: startNodePosition.y - 2 * DISTANCE_BETWEEN_NODE,
      },

      data: {
        type: CBDIEditorRootConceptType.GoalConceptType,
        label: removeBeforeFirstDotAndDot(goalName),
      },
      type: IntentionContextNodeType,
    };

    const agentNode = {
      id: v4(),
      position: {
        x: positionX,

        y: startNodePosition.y - DISTANCE_BETWEEN_NODE,
      },
      data: {
        type: CBDIEditorRootConceptType.AgentConceptType,
        label: removeBeforeFirstDotAndDot(agentName),
      },
      type: IntentionContextNodeType,
    };
    mnodes.push(goalNode, agentNode);
    const goalAgentEdge: Edge = {
      id: `${goalNode.id}-${agentNode.id}`,
      source: goalNode.id,
      target: agentNode.id,
      data: { condition: CBDIEditorPlanEditorEdgeCondition.True, count: 1 },
    };
    medges.push(goalAgentEdge);
  } else {
    const goalNode = {
      id: v4(),
      position: {
        x: positionX,
        y: startNodePosition.y - 3 * DISTANCE_BETWEEN_NODE,
      },
      data: {
        type: CBDIEditorRootConceptType.GoalConceptType,
        label: removeBeforeFirstDotAndDot(goalName),
      },
      type: IntentionContextNodeType,
    };

    const agentNode = {
      id: v4(),
      position: {
        x: positionX,
        y: startNodePosition.y - 2 * DISTANCE_BETWEEN_NODE,
      },
      data: {
        type: CBDIEditorRootConceptType.AgentConceptType,
        label: removeBeforeFirstDotAndDot(agentName),
      },
      type: IntentionContextNodeType,
    };

    const planNode = {
      id: v4(),
      position: {
        x: positionX,
        y: startNodePosition.y - DISTANCE_BETWEEN_NODE,
      },
      data: {
        type: CBDIEditorRootConceptType.PlanConceptType,
        label: removeBeforeFirstDotAndDot(planName),
      },
      type: IntentionContextNodeType,
    };
    mnodes.push(goalNode, agentNode, planNode);
    const goalAgentEdge: Edge = {
      id: `${goalNode.id}-${agentNode.id}`,
      source: goalNode.id,
      target: agentNode.id,
      data: { condition: CBDIEditorPlanEditorEdgeCondition.True, count: 1 },
    };
    const agentPlanEdge: Edge = {
      id: `${agentNode.id}-${planNode.id}`,
      source: agentNode.id,
      target: planNode.id,
      data: { condition: CBDIEditorPlanEditorEdgeCondition.True, count: 1 },
    };
    medges.push(goalAgentEdge, agentPlanEdge);
  }
  return { nodes: mnodes, edges: medges };
};

// get active nodes and edges
export const getActiveNodesAndEdges = (nodes: Node[], edges: Edge[], activeTaskDic: ActiveTaskDic, activeEdgeArr: ActiveEdge[]) => {
  const activeNodes: Node<ActiveNodeData>[] = nodes.map((node) => {
    // find the corresponding task
    // taskId in bdiLog removes the -
    const activeNodeId = Object.keys(activeTaskDic).find((activeTaskId) => {
      // if node is start or end, matching nodeId start_t or end_t in activeTaskDic
      if (node.data.type === CBDIEditorPlanNodeType.StartPlanNodeType) {
        return activeTaskId === CBDIEditorPlanNodeType.StartPlanNodeType;
      }
      if (node.data.type === CBDIEditorPlanNodeType.EndPlanNodeType) {
        return activeTaskId === CBDIEditorPlanNodeType.EndPlanNodeType;
      }
      return activeTaskId.replace(/-/g, '') === node.id.replace(/-/g, '');
    });
    if (activeNodeId) {
      const activeNode = activeTaskDic[activeNodeId];
      const newNode = {
        ...node,
        data: {
          ...node.data,
          taskStatus: activeNode.status,
          bdiLogLevel: activeNode.bdiLogLevel,
          remark: activeNode.remark,
          subGoalId: activeNode.subGoalId,
          isCurrentTask: activeNode.isCurrentTask || false,
        },
      };

      return newNode;
    }
    return node;
  });
  const activeEdges: Edge<ActiveEdgeData>[] = edges.map((edge) => {
    const activeEdge = activeEdgeArr.find(
      (el) =>
        el.sourceTaskId.replace(/-/g, '') === edge.source.replace(/-/g, '') && el.targetTaskId.replace(/-/g, '') === edge.target.replace(/-/g, ''),
    );
    // if edge itself has count (goal context graph edge), use it
    // if it is activeEdge, use its count
    // if it is not activeEdge, make edge count 0
    const count = edge.data.count ? edge.data.count : activeEdge?.count || 0;
    const newEdge = {
      ...edge,
      data: { ...edge.data, count, toLastIndex: activeEdge?.toLastIndex },
      markerEnd: count > 0 ? { type: MarkerType.ArrowClosed, color: 'green' } : edge.markerEnd,
    };
    return newEdge;
  });

  return { activeNodes, activeEdges };
};
