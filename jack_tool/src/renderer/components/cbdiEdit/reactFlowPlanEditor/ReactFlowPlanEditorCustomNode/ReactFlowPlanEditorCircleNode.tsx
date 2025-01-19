import { Handle, NodeProps, Position } from 'reactflow';
import { CBDIEditorPlanEditorEdgeCondition, CBDIEditorPlanNodeType, PlanNodeData } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { useTheme } from '@mui/material';
import {
  CIRCLE_DIAMETER,
  CIRCLE_ICON_LABEL_GAP,
  CIRCLE_NODE_HEIGHT,
  CIRCLE_NODE_WIDTH,
  DASHED_NODE_BORDER,
  NODE_BORDER,
  NODE_FONT_SIZE,
  NODE_HANDLE_BORDER,
  NODE_HANDLE_DIAMETER,
  NODE_HANDLE_LABEL_BACKGROUND_COLOR,
  NODE_LABEL_BACKGROUND_COLOR,
  NODE_LABEL_BORDER,
  NODE_LABEL_BORDER_RADIUS,
  NODE_GAP,
} from '../reactFlowPlanEditorConstant';
import useCustomNodeHook from './useCustomNodeHook';

export default function ReactFlowPlanEditorCircleNode({ id, data }: NodeProps<PlanNodeData>) {
  /* ----------------------------- useTheme hooks ----------------------------- */
  const theme = useTheme();
  /* ------------------------- useCustomNodeHook hook ------------------------- */
  const isTarget = useCustomNodeHook(id);

  if (data.type === CBDIEditorPlanNodeType.StartPlanNodeType) {
    return (
      <div
        style={{
          display: 'flex',
          justifyContent: 'center',
          alignItems: 'center',
          width: CIRCLE_NODE_WIDTH,
          height: CIRCLE_NODE_HEIGHT,
        }}
      >
        <div
          style={{
            display: 'flex',
            flexDirection: 'column',
            alignItems: 'center',
            color: 'black',
            gap: CIRCLE_ICON_LABEL_GAP,
          }}
        >
          <span
            style={{
              fontSize: NODE_FONT_SIZE,
              background: NODE_LABEL_BACKGROUND_COLOR,
              padding: NODE_GAP,
              borderRadius: NODE_LABEL_BORDER_RADIUS,
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
              backgroundColor: theme.editor.graphView.graphNodeColor,
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
            width: NODE_HANDLE_DIAMETER,
            height: NODE_HANDLE_DIAMETER,
            bottom: -1.5 * NODE_HANDLE_DIAMETER,
            border: NODE_HANDLE_BORDER,
            borderColor: theme.editor.graphView.textColor,
          }}
          isConnectableStart={true}
          isConnectableEnd={false}
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
          width: CIRCLE_NODE_WIDTH,
          height: CIRCLE_NODE_HEIGHT,
        }}
      >
        <div
          style={{
            display: 'flex',
            flexDirection: 'column',
            alignItems: 'center',
            color: 'black',
            gap: CIRCLE_ICON_LABEL_GAP,
          }}
        >
          <div
            className="customShape"
            style={{
              width: CIRCLE_DIAMETER,
              height: CIRCLE_DIAMETER,
              backgroundColor: isTarget ? 'orange' : theme.editor.graphView.graphNodeColor,
              borderRadius: '50%',
              border: isTarget ? DASHED_NODE_BORDER : NODE_BORDER,
            }}
          />
          <span
            style={{
              fontSize: NODE_FONT_SIZE,
              background: NODE_LABEL_BACKGROUND_COLOR,
              padding: NODE_GAP,
              borderRadius: NODE_LABEL_BORDER_RADIUS,
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
            width: NODE_HANDLE_DIAMETER,
            height: NODE_HANDLE_DIAMETER,
            top: -1.5 * NODE_HANDLE_DIAMETER,
            border: NODE_HANDLE_BORDER,
            borderColor: theme.editor.graphView.textColor,
          }}
          isConnectableEnd={true}
          isConnectableStart={false}
        />
      </div>
    );
  }
  return null;
}
