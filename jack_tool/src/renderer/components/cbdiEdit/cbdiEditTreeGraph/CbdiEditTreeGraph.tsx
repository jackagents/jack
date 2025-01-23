/* eslint-disable consistent-return */
/* eslint-disable no-param-reassign */
import React, { useEffect } from 'react';
import type { MouseEvent } from 'react';
import ReactFlow, {
  Node,
  MiniMap,
  Controls,
  Background,
  ReactFlowProvider,
  BackgroundVariant,
  useReactFlow,
  useOnViewportChange,
  Viewport,
  Panel,
  ReactFlowInstance,
} from 'reactflow';
import { useTheme } from '@mui/material';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { RootState } from 'projectRedux/Store';
import { useDispatch, useSelector } from 'react-redux';
import 'reactflow/dist/style.css';
import { CBDIEditorObject, CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { areModuleConceptsEqual, isModuleConceptOverview } from 'misc/utils/common/commonUtils';
import './CbdiEditTreeGraph.css';
import Select from 'react-select';
import { getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { CBDIEditorProjectTactic } from 'misc/types/cbdiEdit/cbdiEditModel';
import CustomGroupCbdiEditTreeGraphNode from './custom/CustomGroupCbdiEditTreeGraphNode';
import { useCbdiReactflowContext } from '../CbdiEditReactflowContext/CbdiEditReactflowContext';
import CustomCbdiEditTreeGraphNode from './custom/CustomCbdiEditTreeGraphNode';
import { getDropGroupNode, getNodesAndEdgesByModuleConcept, handleAddModuleConceptByDrag } from './helper';

const BACKGOUND_GRID_SIZE = 20;
const minimapStyle = {
  height: 120,
};
const fitViewOptions = { duration: 100, maxZoom: 0.8 };

interface DepthOptionType {
  value: number;
  label: string;
}
const depthOptions: DepthOptionType[] = [
  { value: 1, label: 'Depth: 1' },
  { value: 2, label: 'Depth: 2' },
  { value: 3, label: 'Depth: 3' },
  { value: 4, label: 'Depth: 4' },
  { value: 5, label: 'Depth: 5' },
];

function CbdiEditTreeGraphFlow() {
  /* ----------------------------- Redux ----------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);

  const selectedTreeNodeConcept = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.selectedTreeNodeConcept);

  const dispatch = useDispatch();
  const { setGraphSelectedNode, setSelectedTreeNodeConcept, updateObjects } = cbdiEditActions;
  /* ----------------------------- constant ----------------------------- */
  const viewportDicId = (() => {
    const selectedTreeNodeObj = getObjectByModuleConcept(current, selectedTreeNodeConcept);
    // if it is tactic
    // use tactic's goal graph's viewport
    if (selectedTreeNodeObj && selectedTreeNodeObj._objectType === CBDIEditorRootConceptType.TacticConceptType) {
      return (selectedTreeNodeObj as CBDIEditorProjectTactic).goal.uuid;
    }

    return selectedTreeNodeConcept!.uuid;
  })();
  /* ----------------------------- useContext hooks ----------------------------- */
  const { viewportDataDic, draggingModuleConcept, setViewportDataDic, setSelectedNodeId, setDraggingModuleConcept } = useCbdiReactflowContext();
  /* ----------------------------- useReactFlow hooks ----------------------------- */
  const { setViewport, getViewport, fitView } = useReactFlow();

  useOnViewportChange({
    onEnd: (viewport: Viewport) => {
      setViewportDataDic((prev) => {
        prev[viewportDicId] = viewport;
        return prev;
      });
    },
  });

  /* ----------------------------- useTheme hooks ----------------------------- */
  const theme = useTheme();
  /* ------------------------------ useState hooks ----------------------------- */
  const [overviewMaxDepth, setOverviewMaxDepth] = React.useState<DepthOptionType>({ value: 3, label: 'Depth: 3' });
  const [reactFlowInstance, setReactFlowInstance] = React.useState<ReactFlowInstance | null>(null);
  const [highlightedGroupNode, setHighlightedGroupNode] = React.useState<{
    [nodeId: string]: string;
  }>({});
  /* ------------------------------ useMemo hooks ----------------------------- */

  const [layoutedNodes, layoutedEdges] = React.useMemo(() => {
    const { nodes, edges } = getNodesAndEdgesByModuleConcept(current, selectedTreeNodeConcept, overviewMaxDepth.value);
    Object.entries(highlightedGroupNode).forEach(([nodeId, borderColor]) => {
      nodes.forEach((node) => {
        if (node.id === nodeId) {
          node.style = {
            ...node.style,
            boxShadow: `0 0 10px ${borderColor}, 0 0 15px ${borderColor}, 0 0 20px ${borderColor}, 0 0 25px ${borderColor}`,
          };
        }
      });
    });
    return [nodes, edges];
  }, [current, selectedTreeNodeConcept, overviewMaxDepth, highlightedGroupNode]);

  const nodeTypes = React.useMemo(
    () => ({
      custom: CustomCbdiEditTreeGraphNode,
      group: CustomGroupCbdiEditTreeGraphNode,
    }),
    [],
  );

  const maxPopulateDepthControl = React.useMemo(() => {
    if (isModuleConceptOverview(selectedTreeNodeConcept)) {
      return (
        <Panel position="top-right" style={{ gap: 5, display: 'flex' }}>
          <Select
            options={depthOptions}
            placeholder="Max depth"
            value={overviewMaxDepth}
            onChange={(newValue) => {
              if (newValue) {
                setOverviewMaxDepth(newValue);
              }
            }}
            styles={{
              option: (provided) => ({
                ...provided,
                color: 'black',
              }),
              control: (provided) => ({
                ...provided,
                width: 120,
              }),
            }}
          />
        </Panel>
      );
    }
    return null;
  }, [selectedTreeNodeConcept, overviewMaxDepth, setOverviewMaxDepth]);

  const groupNodes = React.useMemo(() => layoutedNodes.filter((node) => node.type === 'group'), [layoutedNodes]);

  /* ------------------------------ callbacks ----------------------------- */
  const handleNodeClick = (event: MouseEvent, node: Node<CBDIEditorObject>) => {
    // if click tactic node
    // if will jump to tactic object graph to filter goal plan
    if (node.data._objectType === CBDIEditorRootConceptType.TacticConceptType) {
      if (!areModuleConceptsEqual(node.data, selectedTreeNodeConcept)) {
        dispatch(setSelectedTreeNodeConcept(node.data));
        dispatch(setGraphSelectedNode(node.data));
      }
    } else {
      setSelectedNodeId(node.id);
      dispatch(setGraphSelectedNode(node.data));
    }
  };

  const handleNodeDoubleClick = (event: MouseEvent, node: Node<CBDIEditorObject>) => {
    if (!areModuleConceptsEqual(node.data, selectedTreeNodeConcept)) {
      dispatch(setSelectedTreeNodeConcept(node.data));
      dispatch(setGraphSelectedNode(node.data));
    }
  };

  const handlePaneClick = () => {
    setSelectedNodeId(undefined);
    dispatch(setGraphSelectedNode(selectedTreeNodeConcept));
  };

  /**
   * when click the fitView in control
   * wait for 200ms and set viewport data dic
   */
  const handleFitViewControl = () => {
    setTimeout(() => {
      const currentViewport = getViewport();
      setViewportDataDic((prev) => {
        prev[viewportDicId] = currentViewport;
        return prev;
      });
    }, 200);
  };

  const handleDrop = (event: React.DragEvent<HTMLDivElement>) => {
    if (!draggingModuleConcept) {
      return;
    }
    setHighlightedGroupNode({});
    const newNodeObj = getObjectByModuleConcept(current, draggingModuleConcept);
    const seletedTreeNodeObj = getObjectByModuleConcept(current, selectedTreeNodeConcept);
    if (newNodeObj && seletedTreeNodeObj) {
      const position = reactFlowInstance!.screenToFlowPosition({
        x: event.clientX,
        y: event.clientY,
      });

      const dropGroupNode = getDropGroupNode(position, groupNodes);
      if (dropGroupNode && dropGroupNode.data.childObjectTypes && dropGroupNode.data.childObjectTypes.includes(newNodeObj._objectType)) {
        const updatedObj = handleAddModuleConceptByDrag(draggingModuleConcept, newNodeObj._objectType, seletedTreeNodeObj, current!);
        if (JSON.stringify(updatedObj) !== JSON.stringify(seletedTreeNodeObj)) {
          dispatch(updateObjects(updatedObj));
        }
      }
    }
    setDraggingModuleConcept(null);
  };

  const handleDragOver = (event: React.DragEvent<HTMLDivElement>) => {
    event.preventDefault();
    event.dataTransfer.dropEffect = 'copy';
    if (!draggingModuleConcept) {
      return;
    }
    const newNodeObj = getObjectByModuleConcept(current, draggingModuleConcept);
    const seletedTreeNodeObj = getObjectByModuleConcept(current, selectedTreeNodeConcept);
    if (newNodeObj && seletedTreeNodeObj) {
      const position = reactFlowInstance!.screenToFlowPosition({
        x: event.clientX,
        y: event.clientY,
      });

      const dropGroupNode = getDropGroupNode(position, groupNodes);
      if (dropGroupNode) {
        if (dropGroupNode.data.childObjectTypes && dropGroupNode.data.childObjectTypes.includes(newNodeObj._objectType)) {
          const updatedObj = handleAddModuleConceptByDrag(draggingModuleConcept, newNodeObj._objectType, seletedTreeNodeObj, current!);
          if (JSON.stringify(updatedObj) !== JSON.stringify(seletedTreeNodeObj)) {
            return setHighlightedGroupNode({ [dropGroupNode.id]: '#90ee90' });
          }
          return setHighlightedGroupNode({ [dropGroupNode.id]: '#ff7f7f' });
        }
        return setHighlightedGroupNode({ [dropGroupNode.id]: '#ff7f7f' });
      }
    }
    return setHighlightedGroupNode({});
  };

  /* ------------------------------ useEffect hooks ----------------------------- */
  /**
   * when selected tree node concept change
   * try to get the viewport data from context
   * create default one if not exists
   */
  useEffect(() => {
    if (selectedTreeNodeConcept && reactFlowInstance) {
      if (!viewportDataDic[viewportDicId]) {
        reactFlowInstance.fitView(fitViewOptions);
        // wait 200ms for fit view and save the viewport to viewportDataDic
        setTimeout(() => {
          const currentViewport = getViewport();
          setViewportDataDic((prev) => {
            prev[viewportDicId] = currentViewport;
            return prev;
          });
        }, 200);
      } else {
        setViewport(viewportDataDic[viewportDicId]);
      }
    }
  }, [selectedTreeNodeConcept, reactFlowInstance]);
  return (
    <ReactFlow
      className="cbdiEditTreeGraph"
      nodes={layoutedNodes}
      edges={layoutedEdges}
      nodeTypes={nodeTypes}
      onNodeClick={handleNodeClick}
      onNodeDoubleClick={handleNodeDoubleClick}
      onPaneClick={handlePaneClick}
      defaultViewport={selectedTreeNodeConcept ? viewportDataDic[viewportDicId] : undefined}
      minZoom={0.1}
      onInit={setReactFlowInstance}
      onDragOver={handleDragOver}
      onDrop={handleDrop}
    >
      <MiniMap style={minimapStyle} zoomable pannable />
      <Controls onFitView={handleFitViewControl} fitViewOptions={fitViewOptions} />
      {maxPopulateDepthControl}
      <Background
        variant={BackgroundVariant.Dots}
        color={theme.editor.graphView.gridLinearColor}
        gap={BACKGOUND_GRID_SIZE}
        style={{ backgroundColor: theme.editor.graphView.bgColor }}
      />
    </ReactFlow>
  );
}

function CbdiEditTreeGraph() {
  return (
    <ReactFlowProvider>
      <CbdiEditTreeGraphFlow />
    </ReactFlowProvider>
  );
}

export default CbdiEditTreeGraph;
