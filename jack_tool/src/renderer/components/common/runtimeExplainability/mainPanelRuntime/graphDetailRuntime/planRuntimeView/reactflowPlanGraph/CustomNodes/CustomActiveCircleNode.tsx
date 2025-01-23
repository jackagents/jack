import { Handle, NodeProps, Position } from 'reactflow';
import {
  CIRCLE_DIAMETER,
  CIRCLE_ICON_LABEL_GAP,
  CIRCLE_NODE_HEIGHT,
  CIRCLE_NODE_WIDTH,
  NODE_BORDER,
  NODE_FONT_SIZE,
  NODE_HANDLE_BORDER,
  NODE_HANDLE_DIAMETER,
  NODE_HANDLE_LABEL_BACKGROUND_COLOR,
  NODE_LABEL_BACKGROUND_COLOR,
  NODE_LABEL_BORDER,
  NODE_LABEL_BORDER_RADIUS,
  NODE_GAP,
} from 'components/cbdiEdit/reactFlowPlanEditor/reactFlowPlanEditorConstant';
import React from 'react';
import { TaskStatus, TaskStatusColorEnum } from 'misc/constant/common/cmConstants';
import { CBDIEditorPlanEditorEdgeCondition, CBDIEditorPlanNodeType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { useExplainabilityContext } from 'components/common/runtimeExplainability/context/explainabilityContext';
import { ActiveNodeData } from './NodeDataType';

export default function CustomActiveCircleNode({ data }: NodeProps<ActiveNodeData>) {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { planGraphTaskScale } = useExplainabilityContext();

  // node tag filled color and isActive
  const tagFilledColor = React.useMemo(() => {
    if (!data.taskStatus) {
      return undefined;
    }
    let filledColor;
    switch (data.taskStatus) {
      // dismiss status tag for started and success

      case TaskStatus.STARTED:
        filledColor = TaskStatusColorEnum.STARTED;
        break;

      case TaskStatus.SUCCESS:
        filledColor = TaskStatusColorEnum.SUCCESS;
        break;

      case TaskStatus.FAILED:
        // TODO: Remove hack
        // HACK: Hide fail task status for end node
        // filledColor = TaskStatusColorEnum.FAILED;
        break;

      case TaskStatus.DROPPED:
        filledColor = TaskStatusColorEnum.DROPPED;
        break;

      default:
        break;
    }
    return filledColor;
  }, [data.taskStatus]);

  if (data.type === CBDIEditorPlanNodeType.StartPlanNodeType) {
    return (
      <div
        style={{
          display: 'flex',
          justifyContent: 'center',
          alignItems: 'center',
          width: CIRCLE_NODE_WIDTH * planGraphTaskScale,
          height: CIRCLE_NODE_HEIGHT * planGraphTaskScale,
          opacity: data.taskStatus ? 1 : 0.5,
        }}
      >
        {/* status tag */}
        {tagFilledColor && (
          <div
            style={{
              position: 'absolute',
              top: 0,
              left: -CIRCLE_NODE_WIDTH * planGraphTaskScale,
              backgroundColor: tagFilledColor,
              borderRadius: NODE_LABEL_BORDER_RADIUS * planGraphTaskScale,
              padding: NODE_GAP * planGraphTaskScale,
              color: 'white',
              boxShadow: data.taskStatus ? '0px 4px 24px 4px rgba(0,128,0,0.8)' : undefined,
            }}
          >
            {data.taskStatus}
          </div>
        )}
        <div
          style={{
            display: 'flex',
            flexDirection: 'column',
            alignItems: 'center',
            color: 'black',
            gap: CIRCLE_ICON_LABEL_GAP * planGraphTaskScale,
          }}
        >
          <span
            style={{
              fontSize: NODE_FONT_SIZE * planGraphTaskScale,
              background: NODE_LABEL_BACKGROUND_COLOR,
              padding: NODE_GAP * planGraphTaskScale,
              borderRadius: NODE_LABEL_BORDER_RADIUS * planGraphTaskScale,
              border: NODE_LABEL_BORDER,
            }}
          >
            {CBDIEditorPlanNodeType.StartPlanNodeType}
          </span>
          <div
            className="customShape"
            style={{
              width: CIRCLE_DIAMETER,
              height: CIRCLE_DIAMETER,
              backgroundColor: 'black',
              borderRadius: '50%',
              border: NODE_BORDER,
            }}
          />
        </div>
        <Handle
          id={CBDIEditorPlanEditorEdgeCondition.True}
          type="source"
          position={Position.Bottom}
          style={{
            background: NODE_HANDLE_LABEL_BACKGROUND_COLOR,
            width: NODE_HANDLE_DIAMETER * planGraphTaskScale,
            height: NODE_HANDLE_DIAMETER * planGraphTaskScale,
            bottom: -NODE_HANDLE_DIAMETER * planGraphTaskScale,
            border: NODE_HANDLE_BORDER,
          }}
          isConnectable={false}
        />
      </div>
    );
  }

  if (data.type === CBDIEditorPlanNodeType.EndPlanNodeType) {
    return (
      <div
        style={{
          display: 'flex',
          justifyContent: 'center',
          alignItems: 'center',
          width: CIRCLE_NODE_WIDTH * planGraphTaskScale,
          height: CIRCLE_NODE_HEIGHT * planGraphTaskScale,
          opacity: data.taskStatus ? 1 : 0.5,
        }}
      >
        {/* status tag */}
        {tagFilledColor && (
          <div
            style={{
              position: 'absolute',
              top: 0,
              left: -CIRCLE_NODE_WIDTH * planGraphTaskScale,
              backgroundColor: tagFilledColor,
              borderRadius: NODE_LABEL_BORDER_RADIUS * planGraphTaskScale,
              padding: NODE_GAP * planGraphTaskScale,
              color: 'white',
              boxShadow: data.taskStatus ? '0px 4px 24px 4px rgba(0,128,0,0.8)' : undefined,
            }}
          >
            {data.taskStatus}
          </div>
        )}
        <div
          style={{
            display: 'flex',
            flexDirection: 'column',
            alignItems: 'center',
            color: 'black',
            gap: CIRCLE_ICON_LABEL_GAP * planGraphTaskScale,
          }}
        >
          <div
            className="customShape"
            style={{
              width: CIRCLE_DIAMETER * planGraphTaskScale,
              height: CIRCLE_DIAMETER * planGraphTaskScale,
              backgroundColor: 'black',
              borderRadius: '50%',
              border: NODE_BORDER,
            }}
          />
          <span
            style={{
              fontSize: NODE_FONT_SIZE * planGraphTaskScale,
              background: NODE_LABEL_BACKGROUND_COLOR,
              padding: NODE_GAP * planGraphTaskScale,
              borderRadius: NODE_LABEL_BORDER_RADIUS * planGraphTaskScale,
              border: NODE_LABEL_BORDER,
            }}
          >
            {CBDIEditorPlanNodeType.EndPlanNodeType}
          </span>
        </div>
        <Handle
          type="target"
          position={Position.Top}
          style={{
            background: NODE_HANDLE_LABEL_BACKGROUND_COLOR,
            width: NODE_HANDLE_DIAMETER * planGraphTaskScale,
            height: NODE_HANDLE_DIAMETER * planGraphTaskScale,
            top: -NODE_HANDLE_DIAMETER * planGraphTaskScale,
            border: NODE_HANDLE_BORDER,
          }}
          isConnectable={false}
        />
      </div>
    );
  }
  return null;
}
