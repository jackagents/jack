import React, { useMemo } from 'react';
import { EdgeProps, EdgeLabelRenderer } from 'reactflow';
import {
  EDGD_LABEL_PADDING,
  EDGE_LABEL_BACKGROUND_COLOR,
  EDGE_LABEL_BORDER,
  EDGE_LABEL_BORDER_RADIUS,
  EDGE_LABEL_FONT_SIZE,
} from 'components/cbdiEdit/reactFlowPlanEditor/reactFlowPlanEditorConstant';
import { PlanEdgeData } from 'misc/types/cbdiEdit/cbdiEditTypes';
import ClickableBaseEdge from 'components/cbdiEdit/reactFlowPlanEditor/ReactFlowPlanEditorEdge/ClickableBaseEdge';
import { getBezierPathWithControlPoints } from 'components/cbdiEdit/reactFlowPlanEditor/ReactFlowPlanEditorEdge/helper';

export type ActiveEdgeData = {
  count: number;
  toLastIndex?: number;
} & PlanEdgeData;

export default function CustomActiveEdge({ markerEnd, sourceX, sourceY, targetX, targetY, style, data }: EdgeProps<ActiveEdgeData>) {
  /* ------------------------------ useMemo hooks ----------------------------- */
  const pathClassName = useMemo(() => {
    if (data && data.toLastIndex !== undefined && data.toLastIndex < 3) {
      // Temporarily remove animated effect for last two active edge
      // return 'react-flow__edge-path animated';
      return 'react-flow__edge-path';
    }
    return 'react-flow__edge-path';
  }, [data]);

  const controlPoints = data?.controlPoints ?? [];

  const { path, labelX, labelY } = getBezierPathWithControlPoints({
    sourceX,
    sourceY,
    targetX,
    targetY,
    controlPoints,
  });

  /* ------------------------------- Components ------------------------------- */

  return (
    <>
      <ClickableBaseEdge
        path={path}
        markerEnd={markerEnd}
        style={{
          ...style,
          stroke: data && data.count > 0 ? 'green' : 'gray',
          strokeWidth: 5,
          opacity: data && data.count > 0 ? 1 : 0.5,
        }}
        className={pathClassName}
      />
      <EdgeLabelRenderer>
        <div
          style={{
            opacity: data && data.count > 0 ? 1 : 0.5,
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
            hidden={!data || data?.count < 2}
            style={{
              backgroundColor: EDGE_LABEL_BACKGROUND_COLOR,
              padding: EDGD_LABEL_PADDING,
              borderRadius: EDGE_LABEL_BORDER_RADIUS,
              border: EDGE_LABEL_BORDER,
            }}
          >
            {data?.count}
          </div>
        </div>
      </EdgeLabelRenderer>
    </>
  );
}
