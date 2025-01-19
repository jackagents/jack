import { Edge, MarkerType, Node, Position, XYPosition } from 'reactflow';
import { CBDIEditorProjectPlan } from 'misc/types/cbdiEdit/cbdiEditModel';
import { CBDIEditorJSONPlanNode, defaultEdgeControlPoints, PlanEdgeData, PlanEditorNodeType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import dagre from 'dagre';
import { CIRCLE_NODE_HEIGHT, CIRCLE_NODE_WIDTH, DAGRE_GRAPH_SEP, RECTANGLE_NODE_HEIGHT, RECTANGLE_NODE_WIDTH } from './reactFlowPlanEditorConstant';

// get self loop path for edge's source and target are same
export const getSelfLoopPath = (spacing: number, sourceX: number, sourceY: number, targetX: number, targetY: number): [string, number, number] => {
  const radiusX = spacing;
  const radiusY = (sourceY - targetY) * 0.8;
  const edgePath = `M ${sourceX} ${sourceY + 5} A ${radiusX} ${radiusY} 0 1 0 ${targetX} ${targetY - 5}`;
  const labelX = targetX + radiusX * 1.5;
  const labelY = (sourceY + targetY) / 2;
  return [edgePath, labelX, labelY];
};

export const getCenterPositionWithEdge = (nodes: Node<CBDIEditorJSONPlanNode>[], edge: Edge<PlanEdgeData>): XYPosition => {
  const sourceNode = nodes.find((nd) => nd.id === edge.source);
  const targetNode = nodes.find((nd) => nd.id === edge.target);

  if (!sourceNode || !targetNode) {
    return { x: 0, y: 0 };
  }

  const centerX = (sourceNode.position.x + targetNode.position.x) / 2;
  const centerY = (sourceNode.position.y + targetNode.position.y) / 2;

  return { x: centerX, y: centerY };
};

/**
 * get nodes and edges from selected plan
 * @param selectedPlan
 * @param draggable if node is draggable
 * @returns
 */
export const getNodesAndEdgesFromPlan = (selectedPlan: CBDIEditorProjectPlan | undefined, draggable: boolean) => {
  const mnodes: Node[] = [];
  const medges: Edge[] = [];
  if (selectedPlan) {
    // add tasks from plan into initial nodes
    selectedPlan.tasks.forEach((task) => {
      const { width, height } = (() => {
        switch (task.type) {
          case PlanEditorNodeType.Circle:
            return { width: CIRCLE_NODE_WIDTH, height: CIRCLE_NODE_HEIGHT };
          case PlanEditorNodeType.Rectangle:
            return {
              width: RECTANGLE_NODE_WIDTH,
              height: RECTANGLE_NODE_HEIGHT,
            };
          default:
            return { width: 0, height: 0 };
        }
      })();
      const initialNode: Node<CBDIEditorJSONPlanNode> = {
        id: task.nodeId,
        draggable,
        position: task.metaData?.position || { x: 0, y: 0 },
        width,
        height,
        data: { ...task.nodeData, updateTimestamp: task.metaData?.timestamp },
        type: task.type,
        selected: task.selected,
      };
      mnodes.push(initialNode);
    });
    // add edges from plan into initial edges
    selectedPlan.edges.forEach((edge, index) => {
      const initialEdge: Edge<PlanEdgeData> = {
        id: `${edge.source}-${edge.target}-${edge.edgeData.condition}-${index}`,
        source: edge.source,
        target: edge.target,
        data: {
          ...edge.edgeData,
        },
        type: 'custom',
        markerEnd: { type: MarkerType.ArrowClosed },
        sourceHandle: edge.edgeData.condition,
        selected: edge.selected,
      };
      medges.push(initialEdge);
    });
  }
  return { nodes: mnodes, edges: medges };
};

/**
 * use dagre to auto layout nodes and edges
 * @param unLayoutedNodes
 * @param unLayoutedEdges
 * @param direction
 * @returns
 */
export const getLayoutedElements = (unLayoutedNodes: Node[], unLayoutedEdges: Edge[], direction = 'TB') => {
  const dagreGraph = new dagre.graphlib.Graph();
  dagreGraph.setDefaultEdgeLabel(() => ({}));

  const isHorizontal = direction === 'LR';
  dagreGraph.setGraph({
    rankdir: direction,
    ranksep: DAGRE_GRAPH_SEP,
    edgesep: DAGRE_GRAPH_SEP,
    nodesep: DAGRE_GRAPH_SEP,
  });

  const currentTimestamp = Date.now();

  unLayoutedNodes.forEach((node) => {
    dagreGraph.setNode(node.id, { width: node.width, height: node.height });
    // update updateTimestamp
    node.data.updateTimestamp = currentTimestamp;
  });

  unLayoutedEdges.forEach((edge) => {
    dagreGraph.setEdge(edge.source, edge.target);
    // reset edge controlPoints and update updateTimestamp
    edge.data = { ...edge.data, controlPoints: defaultEdgeControlPoints, timestamp: currentTimestamp };
  });

  dagre.layout(dagreGraph);

  unLayoutedNodes.forEach((node) => {
    const nodeWithPosition = dagreGraph.node(node.id);
    node.targetPosition = isHorizontal ? Position.Left : Position.Top;
    node.sourcePosition = isHorizontal ? Position.Right : Position.Bottom;

    // We are shifting the dagre node position (anchor=center center) to the top left
    // so it matches the React Flow node anchor point (top left).
    node.position = {
      x: nodeWithPosition.x - (node.width || 0) / 2,
      y: nodeWithPosition.y - (node.height || 0) / 2,
    };

    return node;
  });

  return { nodes: unLayoutedNodes, edges: unLayoutedEdges };
};
