import React, { useEffect, useMemo } from 'react';
import { Fluid } from 'components/common/base/BaseContainer';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { useDispatch, useSelector } from 'react-redux';
import { Allotment } from 'allotment';
import DebugConsole from 'components/cbdiEdit/debugConsole/DebugConsole';
import { RootState } from 'projectRedux/Store';
import { defaultEdgeControlPoints, IPlanEdge, IPlanNode, ModuleConcept, PlanEdgeData, PlanEditorNodeType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { CBDIEditorProjectPlan, IModelWarning } from 'misc/types/cbdiEdit/cbdiEditModel';
import { styled } from '@mui/material';
import GraphEditView from '../graphEditView/GraphEditView';
import { getNodesAndEdgesFromPlan, getLayoutedElements } from '../reactFlowPlanEditor/utils';

/* --------------------------------- Styles --------------------------------- */
const DetailRow = styled('div')({
  cursor: 'pointer',
  display: 'flex',
  justifyContent: 'space-between',
  alignItems: 'center',
  padding: '2px 10px',
  '&:hover': {
    color: 'black',
    backgroundColor: 'yellow',
  },
});

/* -------------------------------- MainPanel ------------------------------- */
function MainPanel() {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const debugConsole = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.debugConsole);
  const dispatch = useDispatch();
  const { setDebugConsoleIsVisible, updatePlan, setProjectWarning, setSelectedTreeNodeConcept } = cbdiEditActions;
  /* ------------------------------ useMemo hooks ----------------------------- */
  const unlayoutedPlanModuelConcepts = useMemo(() => {
    const munlayoutedPlanModuelConcepts: ModuleConcept[] = [];
    if (current) {
      for (let i = 0; i < current.plans.length; i++) {
        const planModuleConcept = current.plans[i];
        const plan = getObjectByModuleConcept(current!, planModuleConcept) as CBDIEditorProjectPlan;
        let isLayouted = false;

        for (let j = 0; j < plan.tasks.length; j++) {
          const task = plan.tasks[j];
          if (task.metaData) {
            isLayouted = true;
            break;
          }
        }

        for (let k = 0; k < plan.edges.length; k++) {
          const edge = plan.edges[k];
          if (JSON.stringify(edge.edgeData.controlPoints) !== JSON.stringify(defaultEdgeControlPoints)) {
            isLayouted = true;
            break;
          }
        }

        if (!isLayouted) {
          munlayoutedPlanModuelConcepts.push(planModuleConcept);
        }
      }
    }
    return munlayoutedPlanModuelConcepts;
  }, [current?.plans]);

  /* -------------------------------- Callbacks ------------------------------- */
  const onVisibleChange = (index: number, value: boolean) => {
    if (index === 1) {
      // setIsConsoleVisible(value);
      if (!value) {
        dispatch(setDebugConsoleIsVisible(false));
      } else {
        dispatch(setDebugConsoleIsVisible(true));
      }
    }
  };

  const handleLayoutPlan = (planModuleConcept: ModuleConcept) => {
    const planObj = getObjectByModuleConcept(current, planModuleConcept) as CBDIEditorProjectPlan | undefined;

    if (planObj) {
      const direction = 'TB';
      const { nodes, edges } = getNodesAndEdgesFromPlan(planObj, true);
      const { nodes: mlayoutedNodes, edges: mlayoutedEdges } = getLayoutedElements(nodes, edges, direction);
      const newPlanNodes: IPlanNode[] = [];
      const newPlanEdges: IPlanEdge[] = [];
      mlayoutedNodes.forEach((node) => {
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
        newPlanNodes.push(planNode);
      });

      mlayoutedEdges.forEach((edge) => {
        const planEdge: IPlanEdge = {
          ...edge,
          source: edge.source,
          target: edge.target,
          edgeData: edge.data as PlanEdgeData,
        };
        newPlanEdges.push(planEdge);
      });

      dispatch(
        updatePlan({
          planRerferConcept: planModuleConcept,
          nodes: newPlanNodes,
          edges: newPlanEdges,
        }),
      );
    }
  };

  const handleAutoLayout = () => {
    unlayoutedPlanModuelConcepts.forEach((planModuleConcept) => {
      handleLayoutPlan(planModuleConcept);
    });
  };
  /* -------------------------------- useEffect hooks ------------------------------- */
  useEffect(() => {
    if (unlayoutedPlanModuelConcepts.length > 0) {
      const detail = (
        <div style={{ display: 'flex', flexDirection: 'column', gap: 4 }}>
          {unlayoutedPlanModuelConcepts.map((moduleConcept, index) => {
            const planObj = getObjectByModuleConcept(current, moduleConcept);
            if (planObj) {
              return (
                <DetailRow
                  onClick={() => {
                    dispatch(setSelectedTreeNodeConcept(moduleConcept));
                  }}
                  title={`Open ${planObj.name}`}
                  key={index as number}
                >
                  <div>{planObj.name}</div>
                  <button
                    type="button"
                    style={{ padding: '2px 10px', fontSize: 12, border: '1px solid black' }}
                    onClick={(e) => {
                      e.stopPropagation();
                      handleLayoutPlan(moduleConcept);
                    }}
                    title={`Fix ${planObj.name}`}
                  >
                    Fix
                  </button>
                </DetailRow>
              );
            }
            return null;
          })}
        </div>
      );

      const warning: IModelWarning = {
        title: `Plan Layout Missing (${unlayoutedPlanModuelConcepts.length} Plan${unlayoutedPlanModuelConcepts.length > 1 ? 's' : ''})`,
        detail,
        fixHandler: handleAutoLayout,
      };
      dispatch(setProjectWarning([warning]));
      dispatch(setDebugConsoleIsVisible(true));
    } else {
      dispatch(setProjectWarning([]));
    }
  }, [unlayoutedPlanModuelConcepts]);

  return (
    <Fluid>
      <Allotment defaultSizes={[80, 20]} vertical onVisibleChange={onVisibleChange}>
        <Allotment.Pane preferredSize="80%">
          <GraphEditView />
        </Allotment.Pane>
        <Allotment.Pane snap minSize={200} maxSize={600} visible={debugConsole.isVisible}>
          <DebugConsole />
        </Allotment.Pane>
      </Allotment>
    </Fluid>
  );
}

export default React.memo(MainPanel);
