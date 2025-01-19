import { CBDIEditorPlanEditorEdgeCondition, CBDIEditorPlanNodeType, CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { Handle, Position } from 'reactflow';
import { useMemo } from 'react';
import { useTheme } from '@mui/material';
import { NODE_HANDLE_LABEL_BACKGROUND_COLOR, NODE_HANDLE_DIAMETER, NODE_HANDLE_BORDER, NODE_HANDLE_FONT_SIZE } from '../reactFlowPlanEditorConstant';

export function BottomSourceHandlers({
  trueHanderConnectable,
  falseHanderConnectable,
  dataType,
  scale = 1,
}: {
  trueHanderConnectable: boolean;
  falseHanderConnectable: boolean;
  dataType: CBDIEditorPlanNodeType | CBDIEditorRootConceptType;
  scale?: number;
}) {
  /* ----------------------------- useTheme hooks ----------------------------- */
  const theme = useTheme();

  const [trueEdgeLabel, falseEdgeLabel] = useMemo(() => {
    if (dataType === CBDIEditorPlanNodeType.ConditionPlanNodeType) {
      return [CBDIEditorPlanEditorEdgeCondition.True, CBDIEditorPlanEditorEdgeCondition.False];
    }
    return ['succeed', 'fail'];
  }, [dataType]);

  if (dataType === CBDIEditorPlanNodeType.SleepPlanNodeType) {
    return (
      <Handle
        id={CBDIEditorPlanEditorEdgeCondition.True}
        type="source"
        position={Position.Bottom}
        style={{
          background: NODE_HANDLE_LABEL_BACKGROUND_COLOR,
          width: NODE_HANDLE_DIAMETER * scale,
          height: NODE_HANDLE_DIAMETER * scale,
          bottom: -1.5 * NODE_HANDLE_DIAMETER * scale,
          border: NODE_HANDLE_BORDER,
          borderColor: theme.editor.graphView.textColor,
        }}
        isConnectableStart={trueHanderConnectable}
        isConnectableEnd={true}
      />
    );
  }

  return (
    <>
      <Handle
        id={CBDIEditorPlanEditorEdgeCondition.True}
        type="source"
        position={Position.Bottom}
        style={{
          background: NODE_HANDLE_LABEL_BACKGROUND_COLOR,
          width: NODE_HANDLE_DIAMETER * scale,
          height: NODE_HANDLE_DIAMETER * scale,
          bottom: -1.5 * NODE_HANDLE_DIAMETER * scale,
          left: NODE_HANDLE_DIAMETER * 2 * scale,
          border: NODE_HANDLE_BORDER,
          borderColor: theme.editor.graphView.textColor,
        }}
        isConnectableStart={trueHanderConnectable}
        isConnectableEnd={true}
      >
        <div
          style={{
            position: 'absolute',
            left: (-NODE_HANDLE_DIAMETER / 2) * scale,
            transform: 'translateX(-100%)',
            color: theme.editor.graphView.textColor,
            fontSize: NODE_HANDLE_FONT_SIZE * scale,
          }}
        >
          {trueEdgeLabel}
        </div>
      </Handle>
      <Handle
        id={CBDIEditorPlanEditorEdgeCondition.False}
        type="source"
        position={Position.Bottom}
        style={{
          background: NODE_HANDLE_LABEL_BACKGROUND_COLOR,
          width: NODE_HANDLE_DIAMETER * scale,
          height: NODE_HANDLE_DIAMETER * scale,
          bottom: -1.5 * NODE_HANDLE_DIAMETER * scale,
          left: 'auto',
          right: NODE_HANDLE_DIAMETER * scale,
          border: NODE_HANDLE_BORDER,
          borderColor: theme.editor.graphView.textColor,
        }}
        isConnectableStart={falseHanderConnectable}
        isConnectableEnd={true}
      >
        <div
          style={{
            position: 'absolute',
            left: 1.5 * NODE_HANDLE_DIAMETER * scale,
            color: theme.editor.graphView.textColor,
            fontSize: NODE_HANDLE_FONT_SIZE * scale,
          }}
        >
          {falseEdgeLabel}
        </div>
      </Handle>
    </>
  );
}

export function TopTargetHandler({ isConnectableEnd, scale = 1 }: { isConnectableEnd: boolean; scale?: number }) {
  /* ----------------------------- useTheme hooks ----------------------------- */
  const theme = useTheme();

  return (
    <Handle
      type="target"
      position={Position.Top}
      style={{
        background: NODE_HANDLE_LABEL_BACKGROUND_COLOR,
        width: NODE_HANDLE_DIAMETER * scale,
        height: NODE_HANDLE_DIAMETER * scale,
        top: -1.5 * NODE_HANDLE_DIAMETER * scale,
        border: NODE_HANDLE_BORDER,
        borderColor: theme.editor.graphView.textColor,
      }}
      isConnectableEnd={isConnectableEnd}
      isConnectableStart={false}
    />
  );
}
