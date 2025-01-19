/* eslint-disable jsx-a11y/click-events-have-key-events */
import React from 'react';
import { getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { TCBDIIntentionForNotification } from 'types/cbdi/cbdi-models-non-flatbuffer';
import { useExplainabilityContext } from 'components/common/runtimeExplainability/context/explainabilityContext';
import ReactFlow, {
  MiniMap,
  Controls,
  Background,
  useNodesState,
  useEdgesState,
  BackgroundVariant,
  ReactFlowProvider,
  Node,
  useReactFlow,
  ReactFlowInstance,
  useOnViewportChange,
  Viewport,
  ControlButton,
} from 'reactflow';
import scaleIcon from 'assets/common/icons/scale.png';
import { BACKGOUND_GRID_SIZE } from 'components/cbdiEdit/reactFlowPlanEditor/reactFlowPlanEditorConstant';
import { getNodesAndEdgesFromPlan } from 'components/cbdiEdit/reactFlowPlanEditor/utils';
import { request } from 'misc/events/common/cmEvents';
import { CBDIEditorRootConceptType, PlanEditorNodeType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { CBDIEditorProjectPlan } from 'misc/types/cbdiEdit/cbdiEditModel';
import { nodeColor } from 'misc/icons/cbdi/cbdiIcons';
import { removeBeforeFirstDotAndDot } from 'misc/utils/common/commonUtils';
import SliderPicker from 'components/common/sliderPicker/SliderPicker';
import { ActiveEdge, ActiveTaskDic, getActiveNodesAndEdges, getActiveTaskAndEdge } from './helper';
import IntentionContextNode, { IntentionContextNodeType } from './CustomNodes/IntentionContextNode';
import CustomActiveRectNode from './CustomNodes/CustomActiveRectNode';
import CustomActiveEdge from './CustomEdges/CustomActiveEdge';
import './reactflowPlanGraphRuntime.css';
import GoalContextTreeView from '../goalContextTreeView/GoalContextTreeView';
import { ReasoningListView } from '../reasoningListView/ReasoningListView';
import CustomActiveCircleNode from './CustomNodes/CustomActiveCircleNode';
/* --------------------------------- Styles --------------------------------- */
const minimapStyle = {
  height: 120,
};

interface PlanGraphProps {
  intentionData: TCBDIIntentionForNotification;
  selectedPlan: CBDIEditorProjectPlan | undefined;
  activeTaskDic: ActiveTaskDic;
  activeEdgeArr: ActiveEdge[];
}

const fitViewOptions = { duration: 100, maxZoom: 0.8 };

function PlanGraph({ intentionData, selectedPlan, activeTaskDic, activeEdgeArr }: PlanGraphProps) {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { viewportDataDic, planGraphTaskScale, setPlanGraphTaskScale, setViewportDataDic, inspectAgentGoal } = useExplainabilityContext();

  /* ---------------------------- useMemo hooks ---------------------------- */
  /**
   * initialNodes and initialEdges, recalculate when selected plan has changed
   */
  const [initialNodes, initialEdges] = React.useMemo(() => {
    const { nodes, edges } = getNodesAndEdgesFromPlan(selectedPlan, false);

    // const { nodes: minitialNodes, edges: minitialEdges } = addIntentionContextIntoNodes(nodes, edges, agentName, planName, goalName);

    return [nodes, edges];
  }, [selectedPlan]);

  const [activeNodes, activeEdges] = React.useMemo(() => {
    const { activeNodes: mactiveNodes, activeEdges: mactiveEdges } = getActiveNodesAndEdges(initialNodes, initialEdges, activeTaskDic, activeEdgeArr);

    return [mactiveNodes, mactiveEdges];
  }, [initialNodes, initialEdges, activeTaskDic, activeEdgeArr]);

  const nodeTypes = React.useMemo(
    () => ({
      [PlanEditorNodeType.Rectangle]: CustomActiveRectNode,
      [PlanEditorNodeType.Circle]: CustomActiveCircleNode,
      [IntentionContextNodeType]: IntentionContextNode,
    }),
    [],
  );

  const edgeTypes = React.useMemo(
    () => ({
      custom: CustomActiveEdge,
    }),
    [],
  );

  /* ----------------------------- reactflow hooks ---------------------------- */
  const [nodes, setNodes, onNodesChange] = useNodesState(initialNodes);
  const [edges, setEdges, onEdgesChange] = useEdgesState(initialEdges);
  const { getViewport, setViewport } = useReactFlow();

  /* ----------------------------- useState hooks ---------------------------- */
  const [reactFlowInstance, setReactFlowInstance] = React.useState<ReactFlowInstance | null>(null);
  const [isFontSizeControlOpen, setIsFontSizeControlOpen] = React.useState<boolean>(false);
  /* ------------------------------ useRef hooks ------------------------------ */
  const prevScrollPosition = React.useRef(0);
  const planContainerRef = React.useRef<HTMLDivElement>(null);

  /* -------------------------------- useCallback hooks ------------------------------- */
  const onNodeClick = React.useCallback((event: React.MouseEvent, node: Node) => {
    event.stopPropagation();
    event.preventDefault();
    // does not allow to select circle node
    if (node.type === PlanEditorNodeType.Rectangle && node.data.type === CBDIEditorRootConceptType.GoalConceptType && node.data.subGoalId) {
      window.ipcRenderer.invoke(request.cbdi.getIntentionByAgentGoalId, { agentId: intentionData.agentAddress?.id, goalId: node.data.subGoalId });
    }
  }, []);
  /* ----------------------------- useEffect hooks ---------------------------- */
  React.useEffect(() => {
    // Save the previous scroll position when the component is about to re-render
    if (planContainerRef.current) {
      prevScrollPosition.current = planContainerRef.current.scrollTop;
    }
  }, []);

  React.useEffect(() => {
    // After the component re-renders, restore the previous scroll position
    if (planContainerRef.current) {
      planContainerRef.current.scrollTop = prevScrollPosition.current;
    }
  });

  // update nodes and edges in graph when active nodes edges change
  React.useEffect(() => {
    setNodes(activeNodes);
    setEdges(activeEdges);
  }, [activeNodes, activeEdges]);

  const handleClickTaskScaleControl = () => {
    setIsFontSizeControlOpen(!isFontSizeControlOpen);
  };

  const handleChangePlanGraphScaleChange = (newValue: number) => {
    setPlanGraphTaskScale(newValue);
  };
  /**
   * when click the fitView in control
   * wait for 200ms and set viewport data dic
   */
  const handleFitViewControl = () => {
    setTimeout(() => {
      if (inspectAgentGoal && inspectAgentGoal.agentId && inspectAgentGoal.goalId) {
        const uniqueId = inspectAgentGoal.agentId + inspectAgentGoal.goalId;
        const currentViewport = getViewport();
        setViewportDataDic((prev) => {
          prev[uniqueId] = currentViewport;
          return prev;
        });
      }
    }, 200);
  };

  useOnViewportChange({
    onEnd: (viewport: Viewport) => {
      if (inspectAgentGoal && inspectAgentGoal.agentId && inspectAgentGoal.goalId) {
        const uniqueId = inspectAgentGoal.agentId + inspectAgentGoal.goalId;
        setViewportDataDic((prev) => {
          prev[uniqueId] = viewport;
          return prev;
        });
      }
    },
  });

  /**
   * when inspectAgentGoal change
   * try to get the viewport data from context
   * create default one if not exists
   */
  React.useEffect(() => {
    if (inspectAgentGoal && inspectAgentGoal.agentId && inspectAgentGoal.goalId && reactFlowInstance) {
      const uniqueId = inspectAgentGoal.agentId + inspectAgentGoal.goalId;
      if (!viewportDataDic[uniqueId]) {
        reactFlowInstance.fitView(fitViewOptions);
        // wait 200ms for fit view and save the viewport to viewportDataDic
        setTimeout(() => {
          const currentViewport = getViewport();
          setViewportDataDic((prev) => {
            prev[uniqueId] = currentViewport;
            return prev;
          });
        }, 200);
      } else {
        setViewport(viewportDataDic[uniqueId]);
      }
    }
  }, [inspectAgentGoal, reactFlowInstance]);

  return (
    <ReactFlow
      className="reactFlowPlanGraphRuntime"
      nodes={nodes}
      edges={edges}
      onNodesChange={onNodesChange}
      onNodeClick={onNodeClick}
      onEdgesChange={onEdgesChange}
      fitView
      nodeTypes={nodeTypes}
      edgeTypes={edgeTypes}
      selectNodesOnDrag={false}
      onInit={setReactFlowInstance}
      snapToGrid
      snapGrid={[BACKGOUND_GRID_SIZE, BACKGOUND_GRID_SIZE]}
    >
      <MiniMap style={minimapStyle} zoomable pannable />
      <Controls onFitView={handleFitViewControl} fitViewOptions={fitViewOptions}>
        <ControlButton title="change task size scale" onClick={handleClickTaskScaleControl} style={{ position: 'relative' }}>
          <img src={scaleIcon} alt="" style={{ width: 12, height: 12 }} />
          <div
            onClick={(e) => {
              e.stopPropagation();
            }}
            role="button"
            tabIndex={0}
            hidden={!isFontSizeControlOpen}
            style={{
              cursor: 'default',
              padding: 0,
              backgroundColor: 'transparent',
              position: 'absolute',
              top: 30,
              right: -20,
              transform: 'translate(100%,-100%)',
            }}
          >
            <SliderPicker
              title="task size scale"
              maxValue={2}
              minValue={0.5}
              step={0.1}
              unitText="x"
              value={planGraphTaskScale}
              onChangeValue={handleChangePlanGraphScaleChange}
            />
          </div>
        </ControlButton>
      </Controls>
      <Background
        variant={BackgroundVariant.Dots}
        // TODO
        // change background color of plan graph
        color="black"
        gap={BACKGOUND_GRID_SIZE}
      />
    </ReactFlow>
  );
}

interface Props {
  intentionData: TCBDIIntentionForNotification;
}

export default function ReactFlowPlanGraph(props: Props) {
  const { intentionData } = props;
  /* ----------------------------- useState hooks ----------------------------- */
  const [activeTaskDic, setActiveTaskDic] = React.useState<ActiveTaskDic>({});
  const [activeEdgeArr, setActiveEdgeArr] = React.useState<ActiveEdge[]>([]);
  /* ---------------------------- useContext hooks ---------------------------- */
  const { project, debugMode } = useExplainabilityContext();
  /* ------------------------------ useMemo hooks ----------------------------- */
  /**
   * Selected plan, recalculate when intention id has changed
   */
  const selectedPlan: CBDIEditorProjectPlan | undefined = React.useMemo(() => {
    if (!project) {
      return undefined;
    }

    const { planTemplateName } = intentionData;

    const planModuleConcept = project.plans.find((moduleConcept) => {
      const planObj = getObjectByModuleConcept(project, moduleConcept);
      return `${planObj?.module}.${planObj?.name}` === planTemplateName;
    });

    if (planModuleConcept) {
      return getObjectByModuleConcept(project, planModuleConcept);
    }

    return undefined;
  }, [intentionData.planTemplateName]);
  /* ----------------------------- useEffect hooks ---------------------------- */
  // update plan graph when select a different plan
  React.useEffect(() => {
    if (project && intentionData.intentionData) {
      const { tasks } = intentionData.intentionData;
      if (selectedPlan !== undefined) {
        const result = getActiveTaskAndEdge(tasks, selectedPlan.uuid);

        // set active task dic
        // if the new edge arr is part of existing edge arr, keep the active task dic and update the tasks in new active task dic
        // if the new edge arr is not part of existing edge arr, set new active task dic
        setActiveTaskDic((prev) => {
          if (debugMode) {
            return result.activeTaskDic;
          }
          for (let i = 0; i < result.activeEdgeArray.length; i++) {
            if (i > activeEdgeArr.length - 1) {
              return result.activeTaskDic;
            }
            if (
              result.activeEdgeArray[i].sourceTaskId !== activeEdgeArr[i].sourceTaskId ||
              result.activeEdgeArray[i].targetTaskId !== activeEdgeArr[i].targetTaskId
            ) {
              return result.activeTaskDic;
            }
          }
          const updatedExistingActiveTaskDic = { ...prev };
          Object.keys(result.activeTaskDic).forEach((el) => {
            updatedExistingActiveTaskDic[el] = result.activeTaskDic[el];
          });
          return updatedExistingActiveTaskDic;
        });

        // set active edge arr
        // if the new edge arr is part of existing edge arr, keep the existing edge arr
        // if the new edge arr is not part of existing edge arr, set new edge arr
        setActiveEdgeArr((prev) => {
          if (debugMode) {
            return result.activeEdgeArray;
          }
          for (let i = 0; i < result.activeEdgeArray.length; i++) {
            if (i > prev.length - 1) {
              return result.activeEdgeArray;
            }
            if (result.activeEdgeArray[i].sourceTaskId !== prev[i].sourceTaskId || result.activeEdgeArray[i].targetTaskId !== prev[i].targetTaskId) {
              return result.activeEdgeArray;
            }
          }
          return prev;
        });
      }
    } else {
      setActiveTaskDic({});
      setActiveEdgeArr([]);
    }
  }, [debugMode, intentionData, project, selectedPlan]);

  return (
    <div style={{ display: 'flex', flexDirection: 'column', height: '100%' }}>
      <div style={{ padding: 5, height: 28, backgroundColor: nodeColor[CBDIEditorRootConceptType.GoalConceptType] }}>
        Goal: {removeBeforeFirstDotAndDot(intentionData.goalTemplateName)}
      </div>
      <div style={{ padding: 5, height: 28, backgroundColor: nodeColor[CBDIEditorRootConceptType.PlanConceptType] }}>
        Plan: {removeBeforeFirstDotAndDot(intentionData.planTemplateName) || 'No Plan'}
      </div>
      <ReactFlowProvider>
        <PlanGraph intentionData={intentionData} selectedPlan={selectedPlan} activeTaskDic={activeTaskDic} activeEdgeArr={activeEdgeArr} />
        {/* Goal Context Tree View */}
        <GoalContextTreeView goalContextMsg={intentionData.goalContextMsg} />

        {/* Reasoning List View */}
        <ReasoningListView reasoningDic={intentionData.reasoningDic} selectedPlan={selectedPlan} />
      </ReactFlowProvider>
    </div>
  );
}
