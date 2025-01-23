/* eslint-disable jsx-a11y/click-events-have-key-events */
/* eslint-disable react/button-has-type */
/* eslint-disable jsx-a11y/control-has-associated-label */
/* eslint-disable jsx-a11y/no-static-element-interactions */
import React, { useState } from 'react';
import { Edge, EdgeLabelRenderer, EdgeProps, useReactFlow } from 'reactflow';
import { EdgeBezierControlPoint, IPlanEdge, PlanEdgeData, PlanNodeData } from 'misc/types/cbdiEdit/cbdiEditTypes';
import './BezierEdge.css';
import { RootState } from 'projectRedux/Store';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { useSelector, useDispatch } from 'react-redux';
import { copyObj } from 'misc/utils/common/commonUtils';
import Add from '@mui/icons-material/Add';
import { editorRendererToRendererEvents } from 'misc/events/cbdiEdit/editEvents';
import ClickableBaseEdge from '../ClickableBaseEdge';
import { EDGE_LABEL_BACKGROUND_COLOR, EDGE_LABEL_BORDER, EDGE_LABEL_FONT_SIZE } from '../../reactFlowPlanEditorConstant';
import { getBezierPathWithControlPoints } from '../helper';

export default function PositionableEdge({
  id,
  source,
  target,
  sourceX,
  sourceY,
  targetX,
  targetY,
  style = {},
  markerEnd,
  data,
  selected,
}: EdgeProps<PlanEdgeData>) {
  /* ----------------------------- useState hooks ----------------------------- */
  const [move, setMove] = useState(false);

  const reactFlowInstance = useReactFlow<PlanNodeData, PlanEdgeData>();
  const controlPoints = data?.controlPoints ?? [];

  const { path, labelX, labelY } = getBezierPathWithControlPoints({
    sourceX,
    sourceY,
    targetX,
    targetY,
    controlPoints,
  });

  /* ---------------------------------- Redux --------------------------------- */
  const selectedTreeNodeConcept = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.selectedTreeNodeConcept);
  const dispatch = useDispatch();
  const { updatePlan } = cbdiEditActions;

  /* -------------------------------- functions ------------------------------- */

  // update all edges into model
  const updateAllEdges = (allEdges: Edge[]) => {
    const newEdges: IPlanEdge[] = allEdges.map((edge) => ({
      source: edge.source,
      target: edge.target,
      selected: edge.selected,
      edgeData: edge.data,
    }));
    dispatch(
      updatePlan({
        planRerferConcept: selectedTreeNodeConcept!,
        edges: newEdges,
      }),
    );
  };

  /* ------------------------------- Callbacks ------------------------------ */

  const handleClickAdd = (e: React.MouseEvent) => {
    e.stopPropagation();
    e.preventDefault();
    const planEdgeToAdd: Edge<PlanEdgeData> = {
      id,
      source,
      target,
      data: data!,
    };
    if (window.mainWindowId) {
      window.ipcRenderer.sendTo(window.mainWindowId, editorRendererToRendererEvents.editorOpenUniversalSearchWithPlanEdge, planEdgeToAdd);
    }
  };

  // mouse move is used to move the handler when its been mousedowned on
  const handleMouseMove = React.useCallback(
    (active: number | undefined, handlerIndex: number) => (event: React.MouseEvent<HTMLDivElement>) => {
      event.preventDefault();
      event.stopPropagation();
      if (!move || active === undefined || active === -1) {
        return;
      }
      const position = reactFlowInstance.screenToFlowPosition({
        x: event.clientX,
        y: event.clientY,
      });
      let relativeX = position.x;
      let relativeY = position.y;
      if (handlerIndex === 0) {
        relativeX -= sourceX;
        relativeY -= sourceY;
      } else if (handlerIndex === 1) {
        relativeX -= targetX;
        relativeY -= targetY;
      }

      reactFlowInstance.setEdges((edges) => {
        const updated = copyObj(edges);
        if (updated[active].data) {
          updated[active].data!.controlPoints[handlerIndex] = {
            x: relativeX,
            y: relativeY,
            active,
          };
          updated[active].data!.timestamp = Date.now();
        }
        return updated;
      });
    },
    [move],
  );

  // mouse up is used to release all the handlers
  const handleMouseUp = (event: React.MouseEvent<HTMLDivElement, MouseEvent>) => {
    if (!selected) {
      return;
    }
    event.preventDefault();
    event.stopPropagation();
    setMove(false);
    const edges = reactFlowInstance.getEdges();
    const updatedEdges = copyObj(edges);
    for (let i = 0; i < updatedEdges.length; i++) {
      if (updatedEdges[i].data) {
        const handlersLength = updatedEdges[i].data!.controlPoints.length;
        for (let j = 0; j < handlersLength; j++) {
          updatedEdges[i].data!.controlPoints[j].active = -1;
        }
      }
    }
    updateAllEdges(updatedEdges);
  };

  // mouse down is used to activate the handler
  const handleMouseDown = (handlerIndex: number) => (event: React.MouseEvent<HTMLButtonElement, MouseEvent>) => {
    if (!selected) {
      return;
    }
    event.preventDefault();
    event.stopPropagation();
    setMove(true);
    reactFlowInstance.setEdges((edges) => {
      const edgeIndex = edges.findIndex((edge) => edge.id === id);
      const updated = copyObj(edges);
      if (updated[edgeIndex].data) {
        updated[edgeIndex].data!.controlPoints[handlerIndex].active = edgeIndex;
      }
      return updated;
    });
  };

  return (
    <>
      <ClickableBaseEdge path={path} markerEnd={markerEnd} style={style} />
      <EdgeLabelRenderer>
        <div
          className="react-flow_edge_label"
          style={{
            display: 'flex',
            flexDirection: 'column',
            justifyContent: 'center',
            alignItems: 'center',
            gap: 10,
            position: 'absolute',
            transform: `translate(-50%, -50%) translate(${labelX}px,${labelY}px)`,
            fontSize: EDGE_LABEL_FONT_SIZE,
            // everything inside EdgeLabelRenderer has no pointer events by default
            // if you have an interactive element, set pointer-events: all
            pointerEvents: 'all',
            color: 'black',
          }}
        >
          <div
            className="react-flow_edge_add-icon"
            onClick={handleClickAdd}
            style={{
              backgroundColor: EDGE_LABEL_BACKGROUND_COLOR,
              borderRadius: '25%',
              cursor: 'pointer',
              border: EDGE_LABEL_BORDER,
            }}
          >
            <Add />
          </div>
        </div>
      </EdgeLabelRenderer>
      {selected &&
        controlPoints.map(({ x, y, active }: EdgeBezierControlPoint, index: number) => {
          let controlPointPath = '';
          let absoluteX = x;
          let absoluteY = y;
          if (index === 0) {
            absoluteX += sourceX;
            absoluteY += sourceY;
            controlPointPath += `M ${sourceX} ${sourceY} L ${absoluteX} ${absoluteY}`;
          } else if (index === 1) {
            absoluteX += targetX;
            absoluteY += targetY;
            controlPointPath += `M ${absoluteX} ${absoluteY} L ${targetX} ${targetY}`;
          }

          return (
            <React.Fragment key={`edge${id}_handler${index as number}`}>
              <EdgeLabelRenderer>
                <div
                  className="nopan controlPointContainer"
                  style={{
                    transform: `translate(-50%, -50%) translate(${absoluteX}px,${absoluteY}px)`,
                  }}
                >
                  <div
                    className={`controlPointEventContainer ${active} ${`${active ?? -1}` !== '-1' ? 'active' : ''}`}
                    onMouseMove={handleMouseMove(active, index)}
                    onMouseUp={handleMouseUp}
                  >
                    <button className="controlPoint" onMouseDown={handleMouseDown(index)} />
                  </div>
                </div>
              </EdgeLabelRenderer>
              <path d={controlPointPath} fill="red" stroke="red" strokeWidth={3} strokeDasharray="5,5" />
            </React.Fragment>
          );
        })}
    </>
  );
}
