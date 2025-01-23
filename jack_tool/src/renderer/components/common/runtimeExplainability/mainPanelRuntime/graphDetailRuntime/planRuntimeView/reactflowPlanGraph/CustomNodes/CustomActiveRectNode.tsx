import React from 'react';
import { NodeProps } from 'reactflow';
import { nodeColor, nodeIcon } from 'misc/icons/cbidEdit/cbdiEditIcon';
import { getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { RootState } from 'projectRedux/Store';
import { useSelector } from 'react-redux';
import {
  RECTANGLE_NODE_HEIGHT,
  RECTANGLE_NODE_WIDTH,
  RECTANGLE_TASK_ICON_HEIGHT,
  RECTANGLE_TASK_ICON_WIDTH,
  NODE_LABEL_BORDER_RADIUS,
  NODE_GAP,
  NODE_HANDLE_BORDER,
  NODE_PADDING,
  NODE_FONT_SIZE,
} from 'components/cbdiEdit/reactFlowPlanEditor/reactFlowPlanEditorConstant';
import { CBDIEditorPlanNodeType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { BottomSourceHandlers, TopTargetHandler } from 'components/cbdiEdit/reactFlowPlanEditor/ReactFlowPlanEditorCustomNode/CustomHandlers';
import { useExplainabilityContext } from 'components/common/runtimeExplainability/context/explainabilityContext';
import { ActiveNodeData } from './NodeDataType';

export default function CustomActiveRectNode({ data }: NodeProps<ActiveNodeData>) {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { planGraphTaskScale } = useExplainabilityContext();

  /* ----------------------------- Redux ----------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);

  /* ------------------------------ useMemo hooks ----------------------------- */
  // node image
  const [customNodeIcon, customNodeBgColor] = React.useMemo(() => {
    let mcustomNodeIcon = nodeIcon[CBDIEditorPlanNodeType.ActionPlanNodeType];
    let mcustomNodeBgColor = nodeColor[CBDIEditorPlanNodeType.ActionPlanNodeType];
    switch (data.type) {
      case CBDIEditorPlanNodeType.ActionPlanNodeType:
        mcustomNodeIcon = nodeIcon[CBDIEditorPlanNodeType.ActionPlanNodeType];
        mcustomNodeBgColor = nodeColor[CBDIEditorPlanNodeType.ActionPlanNodeType];
        break;
      case CBDIEditorPlanNodeType.GoalPlanNodeType:
        mcustomNodeIcon = nodeIcon[CBDIEditorPlanNodeType.GoalPlanNodeType];
        mcustomNodeBgColor = nodeColor[CBDIEditorPlanNodeType.GoalPlanNodeType];
        break;
      case CBDIEditorPlanNodeType.SleepPlanNodeType:
        mcustomNodeIcon = nodeIcon[CBDIEditorPlanNodeType.SleepPlanNodeType];
        mcustomNodeBgColor = nodeColor[CBDIEditorPlanNodeType.SleepPlanNodeType];
        break;
      case CBDIEditorPlanNodeType.ConditionPlanNodeType:
        mcustomNodeIcon = nodeIcon[CBDIEditorPlanNodeType.ConditionPlanNodeType];
        mcustomNodeBgColor = nodeColor[CBDIEditorPlanNodeType.ConditionPlanNodeType];
        break;
      default:
        break;
    }
    return [mcustomNodeIcon, mcustomNodeBgColor];
  }, [data.type]);

  // node label
  const customNodeLabel = React.useMemo(() => {
    const messageObj = getObjectByModuleConcept(current, data.action || data.goal);
    if (messageObj) {
      return messageObj.name;
    }
    switch (data.type) {
      case CBDIEditorPlanNodeType.ActionPlanNodeType:
        return 'Action';
      case CBDIEditorPlanNodeType.GoalPlanNodeType:
        return 'Goal';
      case CBDIEditorPlanNodeType.SleepPlanNodeType:
        if (data.duration !== undefined) {
          return `Sleep for ${data.duration} ms`;
        }
        return 'Sleep';
      case CBDIEditorPlanNodeType.ConditionPlanNodeType:
        if (data.conditiontext && data.conditiontext !== '') {
          return data.conditiontext;
        }
        return 'Condition';
      default:
        return 'Unknown';
    }
  }, [current, data.action, data.goal, data.type, data.duration, data.conditiontext]);

  // node tag filled color and isActive
  /* ------------------------------- Components ------------------------------- */
  return (
    <div
      style={{
        width: RECTANGLE_NODE_WIDTH * planGraphTaskScale,
        height: RECTANGLE_NODE_HEIGHT * planGraphTaskScale,
        borderRadius: NODE_LABEL_BORDER_RADIUS * planGraphTaskScale,
        border: NODE_HANDLE_BORDER,
        background: customNodeBgColor,
        display: 'flex',
        alignItems: 'center',
        flexDirection: 'column',
        padding: NODE_PADDING * planGraphTaskScale,
        gap: NODE_GAP,
        opacity: data.taskStatus ? 1 : 0.5,
        cursor: data.subGoalId ? 'pointer' : 'default',
      }}
    >
      {/* Comment out status tag for now */}
      {/* status tag */}
      {/* {tagFilledColor && (
        <div
          style={{
            position: 'absolute',
            top: RECTANGLE_NODE_HEIGHT / 4*planGraphTaskScale,
            left: -RECTANGLE_NODE_WIDTH*planGraphTaskScale,
            backgroundColor: tagFilledColor,
            borderRadius: NODE_LABEL_BORDER_RADIUS*planGraphTaskScale,
            padding: NODE_GAP*planGraphTaskScale,
            color: 'white',
            boxShadow: data.taskStatus ? '0px 4px 24px 4px rgba(0,128,0,0.8)' : undefined,
          }}
        >
          {data.taskStatus}
        </div>
      )} */}
      {/* target handler */}
      <TopTargetHandler scale={planGraphTaskScale} isConnectableEnd={false} />
      {/* node image */}
      <img
        style={{
          width: RECTANGLE_TASK_ICON_WIDTH * planGraphTaskScale,
          height: RECTANGLE_TASK_ICON_HEIGHT * planGraphTaskScale,
        }}
        src={customNodeIcon}
        alt={customNodeLabel}
      />
      {/* label */}
      <div
        title={customNodeLabel}
        style={{
          color: 'black',
          fontSize: NODE_FONT_SIZE * planGraphTaskScale,
          textAlign: 'center',
          wordWrap: 'break-word',
          wordBreak: 'break-word',
        }}
      >
        {customNodeLabel}
      </div>
      {/* source handlers */}
      <BottomSourceHandlers dataType={data.type} scale={planGraphTaskScale} trueHanderConnectable={false} falseHanderConnectable={false} />
    </div>
  );
}
