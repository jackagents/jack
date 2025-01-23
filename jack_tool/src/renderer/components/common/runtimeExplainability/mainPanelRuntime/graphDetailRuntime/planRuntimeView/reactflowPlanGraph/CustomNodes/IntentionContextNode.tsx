import React from 'react';
import { Handle, NodeProps, Position } from 'reactflow';
import { nodeColor, nodeIcon } from 'misc/icons/cbidEdit/cbdiEditIcon';
import {
  RECTANGLE_NODE_WIDTH,
  RECTANGLE_TASK_ICON_HEIGHT,
  RECTANGLE_TASK_ICON_WIDTH,
  NODE_FONT_SIZE,
  NODE_LABEL_BORDER_RADIUS,
  NODE_GAP,
  NODE_HANDLE_BORDER,
  NODE_PADDING,
  NODE_HANDLE_DIAMETER,
  NODE_HANDLE_LABEL_BACKGROUND_COLOR,
} from 'components/cbdiEdit/reactFlowPlanEditor/reactFlowPlanEditorConstant';
import { CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { useExplainabilityContext } from 'components/common/runtimeExplainability/context/explainabilityContext';

export const IntentionContextNodeType = 'intentionContextNode';

export type IntentionContextNodeData = {
  type: CBDIEditorRootConceptType;
  label: string;
};

export default function IntentionContextNode({ data }: NodeProps<IntentionContextNodeData>) {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { planGraphTaskScale } = useExplainabilityContext();

  /* ------------------------------ useMemo hooks ----------------------------- */
  const [customNodeIcon, customNodeBgColor] = React.useMemo(() => {
    let mcustomNodeIcon = nodeIcon[CBDIEditorRootConceptType.AgentConceptType];
    let mcustomNodeBgColor = nodeColor[CBDIEditorRootConceptType.AgentConceptType];
    switch (data.type) {
      case CBDIEditorRootConceptType.AgentConceptType:
        mcustomNodeIcon = nodeIcon[CBDIEditorRootConceptType.AgentConceptType];
        mcustomNodeBgColor = nodeColor[CBDIEditorRootConceptType.AgentConceptType];
        break;
      case CBDIEditorRootConceptType.GoalConceptType:
        mcustomNodeIcon = nodeIcon[CBDIEditorRootConceptType.GoalConceptType];
        mcustomNodeBgColor = nodeColor[CBDIEditorRootConceptType.GoalConceptType];
        break;
      case CBDIEditorRootConceptType.PlanConceptType:
        mcustomNodeIcon = nodeIcon[CBDIEditorRootConceptType.PlanConceptType];
        mcustomNodeBgColor = nodeColor[CBDIEditorRootConceptType.PlanConceptType];
        break;
      default:
        break;
    }
    return [mcustomNodeIcon, mcustomNodeBgColor];
  }, [data.type]);

  /* ------------------------------- Components ------------------------------- */
  return (
    <div
      className="intentionContextNode"
      style={{
        width: RECTANGLE_NODE_WIDTH * planGraphTaskScale,
        borderRadius: NODE_LABEL_BORDER_RADIUS * planGraphTaskScale,
        border: NODE_HANDLE_BORDER,
        background: customNodeBgColor,
        display: 'flex',
        alignItems: 'center',
        flexDirection: 'column',
        padding: NODE_PADDING * planGraphTaskScale,
        gap: NODE_GAP * planGraphTaskScale,
      }}
    >
      {/* target handler */}
      <Handle
        type="target"
        position={Position.Top}
        style={{
          background: NODE_HANDLE_LABEL_BACKGROUND_COLOR,
          width: NODE_HANDLE_DIAMETER * planGraphTaskScale,
          height: NODE_HANDLE_DIAMETER * planGraphTaskScale,
          top: -1.5 * NODE_HANDLE_DIAMETER * planGraphTaskScale,
        }}
        isConnectableEnd={false}
        isConnectableStart={false}
      />
      {/* node image */}
      <img
        style={{
          width: RECTANGLE_TASK_ICON_WIDTH * planGraphTaskScale,
          height: RECTANGLE_TASK_ICON_HEIGHT * planGraphTaskScale,
        }}
        src={customNodeIcon}
        alt={data.label}
      />
      {/* label */}
      <div
        title={data.label}
        style={{
          fontSize: NODE_FONT_SIZE * planGraphTaskScale,
          color: 'black',
          textAlign: 'center',
          wordWrap: 'break-word',
          wordBreak: 'break-word',
        }}
      >
        {data.label}
      </div>
      {/* source handlers */}
      <Handle
        type="source"
        position={Position.Bottom}
        style={{
          background: NODE_HANDLE_LABEL_BACKGROUND_COLOR,
          width: NODE_HANDLE_DIAMETER * planGraphTaskScale,
          height: NODE_HANDLE_DIAMETER * planGraphTaskScale,
          bottom: -1.5 * NODE_HANDLE_DIAMETER * planGraphTaskScale,
        }}
        isConnectableEnd={false}
        isConnectableStart={false}
      />
    </div>
  );
}
