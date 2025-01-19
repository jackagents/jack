/* eslint-disable no-param-reassign */
/* eslint-disable react/jsx-props-no-spreading */
import React, { useCallback, useEffect, useRef, useState } from 'react';
import ReactFlow, {
  addEdge,
  MiniMap,
  Controls,
  Background,
  useNodesState,
  useEdgesState,
  BackgroundVariant,
  Connection,
  MarkerType,
  Node,
  Edge,
  useReactFlow,
  ReactFlowProvider,
  Panel,
  getRectOfNodes,
  getTransformForBounds,
  Viewport,
  useOnViewportChange,
  ReactFlowInstance,
} from 'reactflow';
import {
  Mod,
  ModuleConcept,
  IPlanEdge,
  IPlanNode,
  PlanEdgeData,
  PlanEditorNodeType,
  defaultEdgeControlPoints,
  PlanNodeData,
  IMapping,
  CBDIEditorRootConceptType,
  CBDIEditorPlanNodeType,
  CBDIEditorPlanEditorEdgeCondition,
} from 'misc/types/cbdiEdit/cbdiEditTypes';
import { CBDIEditorProjectPlan } from 'misc/types/cbdiEdit/cbdiEditModel';
import { Rnd } from 'react-rnd';
import { areModuleConceptsEqual } from 'misc/utils/common/commonUtils';
import { useTheme } from '@mui/material';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { RootState } from 'projectRedux/Store';
import { useDispatch, useSelector } from 'react-redux';
import { getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { editorRendererToRendererEvents, request } from 'misc/events/cbdiEdit/editEvents';
import { v4 } from 'uuid';
import { toPng } from 'html-to-image';
import ReactFlowPlanEditorRectangleNode from './ReactFlowPlanEditorCustomNode/ReactFlowPlanEditorRectangleNode';
import ReactFlowPlanEditorCircleNode from './ReactFlowPlanEditorCustomNode/ReactFlowPlanEditorCircleNode';
import FloatingConnectionLine from './ReactFlowPlanEditorEdge/FloatingConnectionLine';
import './reactFlowPlanEditor.css';
import { BACKGOUND_GRID_SIZE, CONTEXT_MENU_HEIGHT, CONTEXT_MENU_WIDTH, NODE_HANDLE_DIAMETER } from './reactFlowPlanEditorConstant';
import { getCenterPositionWithEdge, getLayoutedElements, getNodesAndEdgesFromPlan } from './utils';
import ReactFlowPlanEditorContextMenu, { ContextMenuProps } from './ReactFlowPlanEditorContextMenu/ReactFlowPlanEditorContextMenu';
import PlanContext from './PlanContext/PlanContext';
import PositionableEdge from './ReactFlowPlanEditorEdge/BezierEdge/BezierEdge';
import { useCbdiReactflowContext } from '../CbdiEditReactflowContext/CbdiEditReactflowContext';
import { getConceptDefaultMappings } from '../universalSearch/helper';

const minimapStyle = {
  height: 120,
};

const edgeTypes = {
  custom: PositionableEdge,
};

const fitViewOptions = { duration: 100, maxZoom: 0.8 };

function PlanEditor() {
  /* ----------------------------- Redux ----------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);

  const selectedTreeNodeConcept = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.selectedTreeNodeConcept);

  const graphSelectedNodeConcept = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.graph.selectedNodeConcept);

  const dispatch = useDispatch();
  const { setGraphSelectedNode, updatePlan, setSelectedTreeNodeConcept } = cbdiEditActions;

  /* ----------------------------- useContext hooks ----------------------------- */
  const { viewportDataDic, draggingModuleConcept, setViewportDataDic, setDraggingModuleConcept } = useCbdiReactflowContext();
  /* ----------------------------- useState hooks ----------------------------- */
  const [menu, setMenu] = useState<ContextMenuProps | null>(null);
  const [reactFlowInstance, setReactFlowInstance] = React.useState<ReactFlowInstance | null>(null);
  /* ------------------------------ useMemo hooks ----------------------------- */

  const selectedPlan = React.useMemo(
    () => getObjectByModuleConcept(current, selectedTreeNodeConcept) as CBDIEditorProjectPlan | undefined,
    [current, selectedTreeNodeConcept],
  );

  const [layoutedNodes, layoutedEdges] = React.useMemo(() => {
    const { nodes: mlayoutedNodes, edges: mlayoutedEdges } = getNodesAndEdgesFromPlan(selectedPlan, true);

    return [mlayoutedNodes, mlayoutedEdges];
  }, [selectedPlan]);

  const nodeTypes = React.useMemo(
    () => ({
      [PlanEditorNodeType.Rectangle]: ReactFlowPlanEditorRectangleNode,
      [PlanEditorNodeType.Circle]: ReactFlowPlanEditorCircleNode,
    }),
    [],
  );

  /* ----------------------------- reactflow hooks ---------------------------- */
  const [nodes, setNodes, onNodesChange] = useNodesState<PlanNodeData>(layoutedNodes);
  const [edges, setEdges, onEdgesChange] = useEdgesState<PlanEdgeData>(layoutedEdges);
  const { project, getNodes, getViewport, setViewport } = useReactFlow<PlanNodeData, PlanEdgeData>();
  /* ------------------------------ useRef hooks ------------------------------ */
  const ref = useRef<HTMLDivElement>(null);
  const edgeUpdateSuccessful = useRef(true);

  /* ------------------------------ useCallbacks ------------------------------ */

  const isValidConnection = useCallback(
    (connection: Connection) => {
      const matchedEdge = edges.find(
        (edge) => edge.source === connection.source && edge.sourceHandle === connection.sourceHandle && edge.target === connection.target,
      );

      if (matchedEdge) {
        return false;
      }
      return true;
    },
    [edges],
  );

  const handleUpdatePlan = useCallback(
    ({ newNodes, newEdges }: { newNodes?: Node[]; newEdges?: Edge[] }) => {
      let newPlanNodes: IPlanNode[] | undefined;
      let newPlanEdges: IPlanEdge[] | undefined;
      if (newNodes) {
        newPlanNodes = [];
        newNodes.forEach((node) => {
          const planNode: IPlanNode = {
            ...node,
            nodeId: node.id,
            type: node.type as PlanEditorNodeType,
            nodeData: node.data,
            metaData: {
              position: node.position,
              timestamp: node.data.updateTimestamp,
            },
          };
          newPlanNodes!.push(planNode);
        });
      }
      if (newEdges) {
        newPlanEdges = [];
        newEdges.forEach((edge) => {
          const planEdge: IPlanEdge = {
            ...edge,
            source: edge.source,
            target: edge.target,
            edgeData: edge.data as PlanEdgeData,
          };
          newPlanEdges!.push(planEdge);
        });
      }
      dispatch(
        updatePlan({
          planRerferConcept: selectedTreeNodeConcept!,
          nodes: newPlanNodes,
          edges: newPlanEdges,
        }),
      );
    },
    [selectedTreeNodeConcept],
  );

  const getNewNodesDragStop = useCallback((originalNodes: Node[], movingNodes: Node[]) => {
    const newNodes = originalNodes.map((node) => {
      // find moving node and update updateTimestamp of node position
      const isMovingNode = movingNodes.some((movingNode) => movingNode.id === node.id);
      if (isMovingNode) {
        node.data = {
          ...node.data,
          updateTimestamp: Date.now(),
        };
      }
      return node;
    });
    return newNodes;
  }, []);

  const onLayout = useCallback(
    (direction: 'TB' | 'LR') => {
      if (reactFlowInstance) {
        const { nodes: mlayoutedNodes, edges: mlayoutedEdges } = getLayoutedElements(nodes, edges, direction);
        handleUpdatePlan({ newNodes: mlayoutedNodes, newEdges: mlayoutedEdges });
        setTimeout(() => {
          reactFlowInstance.fitView(fitViewOptions);
          setTimeout(() => {
            const currentViewport = getViewport();
            setViewportDataDic((prev) => {
              prev[selectedTreeNodeConcept!.uuid] = currentViewport;
              return prev;
            });
          }, 200);
        }, 200);
      }
    },
    [nodes, edges, handleUpdatePlan, reactFlowInstance, setViewportDataDic],
  );

  const exportToPng = async () => {
    const imageWidth = 1024;
    const imageHeight = 768;
    const nodesBounds = getRectOfNodes(getNodes());
    const transform = getTransformForBounds(nodesBounds, imageWidth, imageHeight, 0.5, 2);

    try {
      const viewportElement = document.querySelector('.react-flow__viewport') as HTMLElement | undefined;
      if (viewportElement) {
        const dataUrl = await toPng(viewportElement, {
          width: imageWidth,
          height: imageHeight,
          style: {
            width: imageWidth.toString(),
            height: imageHeight.toString(),
            transform: `translate(${transform[0]}px, ${transform[1]}px) scale(${transform[2]})`,
          },
        });
        // Show a save dialog to get the file path

        const resultMessage = await window.ipcRenderer.invoke(request.graph.generatePlan, dataUrl, selectedPlan!.name);
        console.log(resultMessage);
      }
    } catch (error) {
      console.log(error);
    }
  };

  const onNodeClick = useCallback(
    (event: React.MouseEvent, node: Node<PlanNodeData>) => {
      event.stopPropagation();
      event.preventDefault();
      // create a moduleConcept for this task
      const graphNodeModuleConcept: ModuleConcept = {
        uuid: node.id,
        module: '',
        name: node.data.type,
      };
      // if circle node is selected or reselect the current graph selected node
      // make graph selected node to be plan node
      if (node.type === PlanEditorNodeType.Circle || areModuleConceptsEqual(graphNodeModuleConcept, graphSelectedNodeConcept)) {
        dispatch(setGraphSelectedNode(selectedTreeNodeConcept));
      }
      // make selected task to be graph selected node
      else {
        dispatch(setGraphSelectedNode(graphNodeModuleConcept));
      }
    },
    [graphSelectedNodeConcept, selectedTreeNodeConcept],
  );

  /**
   * if current plan node has action/goal
   * jump to its concept
   */
  const onNodeDoubleClick = useCallback(
    (event: React.MouseEvent, node: Node<PlanNodeData>) => {
      event.stopPropagation();
      event.preventDefault();
      const moduleConcept = node.data.action || node.data.goal || null;

      const obj = getObjectByModuleConcept(current!, moduleConcept);

      if (obj && obj._mod !== Mod.Deletion) {
        dispatch(setGraphSelectedNode(moduleConcept));
        dispatch(setSelectedTreeNodeConcept(moduleConcept));
      }
    },
    [current],
  );

  const onNodeDragStop = useCallback(
    (_event: React.MouseEvent, _movingNode: Node, movingNodes: Node[]) => {
      const newNodes = getNewNodesDragStop(nodes, movingNodes);
      handleUpdatePlan({ newNodes });
    },
    [nodes],
  );

  const onSelectionDragStop = useCallback(
    (event: React.MouseEvent, movingNodes: Node[]) => {
      const newNodes = getNewNodesDragStop(nodes, movingNodes);
      handleUpdatePlan({ newNodes });
    },
    [nodes],
  );

  const onNodesDelete = useCallback(
    (deleteNodes: Node<PlanNodeData>[]) => {
      // Create updated nodes
      // Delete nodes other than start and end node
      const newNodes = nodes.filter(
        (node) =>
          node.data.type === CBDIEditorPlanNodeType.StartPlanNodeType ||
          node.data.type === CBDIEditorPlanNodeType.EndPlanNodeType ||
          !deleteNodes.some((el) => el.id === node.id),
      );
      handleUpdatePlan({ newNodes });
    },
    [nodes],
  );

  // gets called after connect to node
  const onConnect = useCallback(
    (params: Connection) => {
      // removing existing edge which have same source handle and source
      const filteredEdges = edges.filter((edge) => edge.source !== params.source || edge.sourceHandle !== params.sourceHandle);

      const newEdges = addEdge(
        {
          ...params,
          type: 'custom',
          markerEnd: { type: MarkerType.ArrowClosed },
          data: {
            condition: params.sourceHandle,
            controlPoints: defaultEdgeControlPoints,
            timestamp: Date.now(),
          },
        },
        filteredEdges,
      );
      handleUpdatePlan({ newEdges });
    },
    [edges, handleUpdatePlan],
  );

  const onEdgesDelete = useCallback(
    (deleteEdges: Edge[]) => {
      const newEdges = edges.filter((edge) => !deleteEdges.some((el) => el.id === edge.id));
      handleUpdatePlan({ newEdges });
    },
    [edges, handleUpdatePlan],
  );

  // gets called after end of edge gets dragged to another source or target
  const onEdgeUpdate = React.useCallback(
    (oldEdge: Edge, newConnection: Connection) => {
      edgeUpdateSuccessful.current = true;
      const filteredEdges = edges.filter((edge) => edge.id !== oldEdge.id);
      const newEdges = addEdge(
        {
          ...newConnection,
          type: 'custom',
          markerEnd: { type: MarkerType.ArrowClosed },
          data: {
            ...oldEdge.data,
            condition: newConnection.sourceHandle,
            controlPoints: defaultEdgeControlPoints,
            timestamp: Date.now(),
          },
        },
        filteredEdges,
      );

      handleUpdatePlan({ newEdges });
    },
    [edges, handleUpdatePlan],
  );

  const onEdgeUpdateStart = React.useCallback(() => {
    edgeUpdateSuccessful.current = false;
  }, []);

  const onEdgeUpdateEnd = React.useCallback(
    (_e: any, edge: Edge) => {
      if (!edgeUpdateSuccessful.current) {
        const newEdges = edges.filter((e) => e.id !== edge.id);
        handleUpdatePlan({ newEdges });
      }

      edgeUpdateSuccessful.current = true;
    },
    [edges, handleUpdatePlan],
  );

  const onNodeContextMenu = useCallback(
    (event: React.MouseEvent, node: Node) => {
      if (!ref.current) {
        return;
      }
      // Prevent native context menu from showing
      event.preventDefault();
      event.stopPropagation();
      // Calculate position of the context menu. We want to make sure it
      // doesn't get positioned off-screen.
      const pane = ref.current.getBoundingClientRect();

      setMenu({
        id: node.id,
        top: event.clientY < pane.height + pane.y - CONTEXT_MENU_HEIGHT ? event.clientY - pane.y : undefined,
        left: event.clientX < pane.width + pane.x - CONTEXT_MENU_WIDTH ? event.clientX - pane.x : undefined,
        right: event.clientX >= pane.width + pane.x - CONTEXT_MENU_WIDTH ? pane.width + pane.x - event.clientX : undefined,
        bottom: event.clientY >= pane.height + pane.y - CONTEXT_MENU_HEIGHT ? pane.height + pane.y - event.clientY : undefined,
      });
    },
    [setMenu],
  );

  const onPaneClick = useCallback(() => {
    dispatch(setGraphSelectedNode(selectedTreeNodeConcept));
    setMenu(null);
    // unselect all edges
    const newEdges = edges.map((edge) => ({
      source: edge.source,
      target: edge.target,
      selected: false,
      edgeData: edge.data!,
    }));
    dispatch(
      updatePlan({
        planRerferConcept: selectedTreeNodeConcept!,
        edges: newEdges,
      }),
    );
  }, [setMenu, selectedTreeNodeConcept, edges]);

  // create a temporary task base on adding task type
  // put it on the center of pane
  const onEditorAddingPlanTask = React.useCallback(
    (event: Electron.IpcRendererEvent, newNode: IPlanNode) => {
      if (!ref.current) {
        return;
      }
      const pane = ref.current.getBoundingClientRect();
      const paneCenter = project({ x: pane.width / 2, y: pane.height / 2 });

      const newGraphNode: Node<PlanNodeData, PlanEditorNodeType> = {
        id: newNode.nodeId,
        position: paneCenter,
        data: { ...newNode.nodeData, updateTimestamp: Date.now() },
        type: newNode.type,
      };

      const newNodes = nodes.concat(newGraphNode);
      handleUpdatePlan({ newNodes });
    },
    [nodes],
  );

  // insert task base on adding task and edge
  const onEditorAddingPlanTaskWithEdge = React.useCallback(
    (event: Electron.IpcRendererEvent, newNode: IPlanNode, edge: Edge<PlanEdgeData>) => {
      const centerPosition = getCenterPositionWithEdge(nodes, edge);
      const newGraphNode: Node<PlanNodeData, PlanEditorNodeType> = {
        id: newNode.nodeId,
        position: centerPosition,
        data: { ...newNode.nodeData, updateTimestamp: Date.now() },
        type: newNode.type,
      };
      const newNodes = nodes.concat(newGraphNode);
      let newEdges = edges.filter((el) => el.id !== edge.id);
      newEdges = newEdges.concat(
        {
          id: `${v4()}`,
          source: edge.source,
          target: newNode.nodeId,
          data: {
            condition: edge.data!.condition,
            controlPoints: defaultEdgeControlPoints,
            timestamp: Date.now(),
          },
          type: 'custom',
          markerEnd: { type: MarkerType.ArrowClosed },
        },
        {
          id: `${v4()}`,
          source: newNode.nodeId,
          target: edge.target,
          data: {
            condition: CBDIEditorPlanEditorEdgeCondition.True,
            controlPoints: defaultEdgeControlPoints,
            timestamp: Date.now(),
          },
          type: 'custom',
          markerEnd: { type: MarkerType.ArrowClosed },
        },
      );
      handleUpdatePlan({ newNodes, newEdges });
    },
    [nodes, edges],
  );

  /**
   * handle tree item drop on plan editor
   * add dragging action/goal to new node in plan editor
   * @param event
   * @returns
   */
  const onDropTreeItem = (event: React.DragEvent<HTMLDivElement>) => {
    event.preventDefault();
    if (!draggingModuleConcept) {
      return;
    }
    const newNodeObj = getObjectByModuleConcept(current, draggingModuleConcept);
    if (newNodeObj) {
      const defaultMappingDic: IMapping[] | undefined = getConceptDefaultMappings(current!, newNodeObj._objectType, draggingModuleConcept);
      const id = v4();
      const position = reactFlowInstance!.screenToFlowPosition({
        x: event.clientX,
        y: event.clientY,
      });

      const newGraphNode: Node<PlanNodeData, PlanEditorNodeType> = {
        id,
        position,
        type: PlanEditorNodeType.Rectangle,
        data: {
          id,
          note: '',
          type:
            newNodeObj._objectType === CBDIEditorRootConceptType.ActionConceptType
              ? CBDIEditorPlanNodeType.ActionPlanNodeType
              : CBDIEditorPlanNodeType.GoalPlanNodeType,
          async: false,
          mappings: defaultMappingDic,
          action: newNodeObj._objectType === CBDIEditorRootConceptType.ActionConceptType ? draggingModuleConcept : undefined,
          goal: newNodeObj._objectType === CBDIEditorRootConceptType.GoalConceptType ? draggingModuleConcept : undefined,
          updateTimestamp: Date.now(),
        },
      };
      const newNodes = nodes.concat(newGraphNode);
      handleUpdatePlan({ newNodes });
    }
    setDraggingModuleConcept(null);
  };

  useOnViewportChange({
    onEnd: (viewport: Viewport) => {
      setViewportDataDic((prev) => {
        prev[selectedTreeNodeConcept!.uuid] = viewport;
        return prev;
      });
    },
  });

  /**
   * when click the fitView in control
   * wait for 200ms and set viewport data dic
   */
  const handleFitViewControl = () => {
    setTimeout(() => {
      const currentViewport = getViewport();
      setViewportDataDic((prev) => {
        prev[selectedTreeNodeConcept!.uuid] = currentViewport;
        return prev;
      });
    }, 200);
  };

  /* ----------------------------- useEffect hooks ---------------------------- */
  // add task
  useEffect(() => {
    const onEditorAddingPlanTaskCleanup = window.ipcRenderer.setupIpcListener(
      editorRendererToRendererEvents.editorAddingPlanTask,
      onEditorAddingPlanTask,
    );

    const onEditorAddingPlanTaskWithEdgeCleanup = window.ipcRenderer.setupIpcListener(
      editorRendererToRendererEvents.editorAddingPlanTaskWithEdge,
      onEditorAddingPlanTaskWithEdge,
    );

    return () => {
      onEditorAddingPlanTaskCleanup();
      onEditorAddingPlanTaskWithEdgeCleanup();
    };
  }, [nodes, edges]);

  // export plan graph to png
  useEffect(() => {
    const onEditorExportPlanPngGraphCleanup = window.ipcRenderer.setupIpcListener(
      editorRendererToRendererEvents.editorExportPlanPngGraph,
      exportToPng,
    );

    return () => {
      onEditorExportPlanPngGraphCleanup();
    };
  }, []);

  /**
   * when selected tree node concept change
   * try to get the viewport data from context
   * create default one if not exists
   */
  useEffect(() => {
    if (selectedTreeNodeConcept && reactFlowInstance) {
      if (!viewportDataDic[selectedTreeNodeConcept.uuid]) {
        reactFlowInstance.fitView(fitViewOptions);
        // wait 200ms for fit view and save the viewport to viewportDataDic
        setTimeout(() => {
          const currentViewport = getViewport();
          setViewportDataDic((prev) => {
            prev[selectedTreeNodeConcept.uuid] = currentViewport;
            return prev;
          });
        }, 200);
      } else {
        setViewport(viewportDataDic[selectedTreeNodeConcept.uuid]);
      }
    }
  }, [selectedTreeNodeConcept, reactFlowInstance]);

  // update nodes and edges in graph when layouted nodes edges change
  useEffect(() => {
    setNodes(layoutedNodes);
  }, [layoutedNodes]);

  useEffect(() => {
    setEdges(layoutedEdges);
  }, [layoutedEdges]);

  /* ----------------------------- useTheme hooks ----------------------------- */
  const theme = useTheme();
  /* ------------------------------- Components ------------------------------- */
  return (
    <ReactFlow
      ref={ref}
      className="reactFlowPlanEdtitor"
      nodes={nodes}
      edges={edges}
      connectionLineComponent={FloatingConnectionLine}
      onNodesChange={onNodesChange}
      onEdgesChange={onEdgesChange}
      onNodeContextMenu={onNodeContextMenu}
      onConnect={onConnect}
      isValidConnection={isValidConnection}
      nodeTypes={nodeTypes}
      edgeTypes={edgeTypes}
      selectNodesOnDrag={false}
      snapToGrid
      snapGrid={[BACKGOUND_GRID_SIZE, BACKGOUND_GRID_SIZE]}
      onPaneClick={onPaneClick}
      onNodeDragStop={onNodeDragStop}
      onSelectionDragStop={onSelectionDragStop}
      nodeDragThreshold={50}
      onNodeClick={onNodeClick}
      onNodeDoubleClick={onNodeDoubleClick}
      onNodesDelete={onNodesDelete}
      onEdgesDelete={onEdgesDelete}
      onEdgeUpdate={onEdgeUpdate}
      onEdgeUpdateStart={onEdgeUpdateStart}
      onEdgeUpdateEnd={onEdgeUpdateEnd}
      connectionRadius={NODE_HANDLE_DIAMETER * 4}
      deleteKeyCode={['Delete', 'Backspace']}
      defaultViewport={selectedTreeNodeConcept ? viewportDataDic[selectedTreeNodeConcept.uuid] : undefined}
      onInit={setReactFlowInstance}
      onDragOver={(e) => {
        e.preventDefault();
        e.dataTransfer.dropEffect = 'copy';
      }}
      onDrop={onDropTreeItem}
    >
      <MiniMap style={minimapStyle} zoomable pannable />
      <Controls onFitView={handleFitViewControl} fitViewOptions={fitViewOptions} />
      <Background
        variant={BackgroundVariant.Dots}
        color={theme.editor.graphView.gridLinearColor}
        gap={BACKGOUND_GRID_SIZE}
        style={{ backgroundColor: theme.editor.graphView.bgColor }}
      />
      <Panel position="top-right" style={{ gap: 5, display: 'flex' }}>
        <button style={{ display: 'block', fontSize: 18 }} type="button" onClick={() => onLayout('TB')}>
          Vertical layout
        </button>
        <button style={{ display: 'block', fontSize: 18 }} type="button" onClick={() => onLayout('LR')}>
          Horizontal layout
        </button>
        <button style={{ display: 'block', fontSize: 18 }} type="button" onClick={exportToPng}>
          Export to png
        </button>
      </Panel>
      {menu && <ReactFlowPlanEditorContextMenu onClick={onPaneClick} {...menu} handleUpdatePlan={handleUpdatePlan} />}
    </ReactFlow>
  );
}

export default function ReactFlowPlanEditor() {
  const [planContextSize, setPlanContextSize] = React.useState({
    width: 300,
    height: 350,
  });
  const [planContextPosition, setPlanContextPosition] = React.useState({
    x: 50,
    y: 150,
  });

  return (
    <ReactFlowProvider>
      <PlanEditor />
      {/* plan context table */}
      <Rnd
        minWidth={300}
        minHeight={350}
        maxWidth={600}
        style={{ zIndex: 9, overflow: 'hidden' }}
        size={{
          width: planContextSize.width,
          height: planContextSize.height,
        }}
        position={{
          x: planContextPosition.x,
          y: planContextPosition.y,
        }}
        onDragStop={(_e, d) => {
          setPlanContextPosition({ x: d.x, y: d.y });
        }}
        onResizeStop={(_e, _direction, ref, _delta, position) => {
          setPlanContextSize({
            width: +ref.style.width,
            height: +ref.style.height,
          });
          setPlanContextPosition(position);
        }}
      >
        <PlanContext />
      </Rnd>
    </ReactFlowProvider>
  );
}
