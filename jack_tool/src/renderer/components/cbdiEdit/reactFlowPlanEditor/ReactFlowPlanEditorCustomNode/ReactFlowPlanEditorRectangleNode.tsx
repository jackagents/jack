import React from 'react';
import { NodeProps } from 'reactflow';
import { nodeColor, nodeIcon } from 'misc/icons/cbidEdit/cbdiEditIcon';
import { styled } from '@mui/material';
import HelpOutlineIcon from '@mui/icons-material/HelpOutline';
import { getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { RootState } from 'projectRedux/Store';
import { useSelector } from 'react-redux';
import { PlanNodeData, CBDIEditorRootConceptType, CBDIEditorPlanNodeType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import {
  BORDER_RADIUS,
  RECTANGLE_NODE_WIDTH,
  RECTANGLE_TASK_ICON_HEIGHT,
  RECTANGLE_TASK_ICON_WIDTH,
  NODE_FONT_SIZE,
  NODE_PADDING,
  NODE_BORDER,
  DASHED_NODE_BORDER,
  NODE_GAP,
} from '../reactFlowPlanEditorConstant';
import { BottomSourceHandlers, TopTargetHandler } from './CustomHandlers';
import useCustomNodeHook from './useCustomNodeHook';

/* --------------------------------- Styles --------------------------------- */
const NoteHelperIconContainer = styled('div')({
  position: 'absolute',
  top: NODE_PADDING / 2,
  right: NODE_PADDING / 2,
  fontSize: NODE_FONT_SIZE,
  color: 'black',
  textAlign: 'center',
  wordWrap: 'break-word',
  wordBreak: 'break-word',
  cursor: 'pointer',
});

const NoteViewContainer = styled('div')({
  position: 'absolute',
  top: NODE_PADDING,
  right: -NODE_PADDING,
  transform: 'translate(100%,-100%)',
  color: 'white',
  width: 200,
  '--r': '20px', // the radius
  '--t': '10px', // the size of the tail
  '--_d': '0%',
  padding: 'calc(2*var(--r)/3)',
  background: '#1384C5',
  borderLeft: 'var(--t) solid #0000',
  marginRight: 'var(--t)',
  placeSelf: 'start',
  WebkitMask: `
    radial-gradient(var(--t) at var(--_d) 0,#0000 98%,#000 102%)
      var(--_d) 100%/calc(100% - var(--r)) var(--t) no-repeat,
    conic-gradient(at var(--r) var(--r),#000 75%,#0000 0)
      calc(var(--r)/-2) calc(var(--r)/-2) padding-box,
    radial-gradient(50% 50%,#000 98%,#0000 101%)
      0 0/var(--r) var(--r) space padding-box;
 `,
});

export default function ReactFlowPlanEditorRectangleNode({ id, data }: NodeProps<PlanNodeData>) {
  /* ------------------------- useCustomNodeHook hook ------------------------- */
  const isTarget = useCustomNodeHook(id);
  /* ----------------------------- Redux ----------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  /* ----------------------------- useState hooks ----------------------------- */
  const [isNoteViewOpen, setIsNoteViewOpen] = React.useState(false);
  /* ------------------------------ useMemo hooks ----------------------------- */

  const [customNodeIcon, customNodeBgColor] = React.useMemo(() => {
    let mcustomNodeIcon = nodeIcon[CBDIEditorRootConceptType.ActionConceptType];
    let mcustomNodeBgColor = nodeColor[CBDIEditorRootConceptType.ActionConceptType];
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
  /* ------------------------------- Components ------------------------------- */
  return (
    <div
      className="customShape"
      style={{
        width: RECTANGLE_NODE_WIDTH,
        borderRadius: BORDER_RADIUS,
        backgroundColor: isTarget ? 'orange' : customNodeBgColor,
        border: isTarget ? DASHED_NODE_BORDER : NODE_BORDER,
        display: 'flex',
        alignItems: 'center',
        flexDirection: 'column',
        padding: NODE_PADDING,
        gap: NODE_GAP,
      }}
    >
      {data.note !== '' && (
        <NoteHelperIconContainer
          title="click to toggle note"
          onClick={(e) => {
            e.stopPropagation();
            setIsNoteViewOpen((prev) => !prev);
          }}
        >
          <HelpOutlineIcon color={isNoteViewOpen ? 'info' : 'action'} />
        </NoteHelperIconContainer>
      )}

      {isNoteViewOpen && <NoteViewContainer>{data.note}</NoteViewContainer>}
      {/* target handler */}
      <TopTargetHandler isConnectableEnd={isTarget} />
      {/* node image */}
      <img
        style={{
          width: RECTANGLE_TASK_ICON_WIDTH,
          height: RECTANGLE_TASK_ICON_HEIGHT,
        }}
        src={customNodeIcon}
        alt={customNodeLabel}
      />
      {/* label */}
      <div
        title={customNodeLabel}
        style={{
          fontSize: NODE_FONT_SIZE,
          color: 'black',
          textAlign: 'center',
          wordWrap: 'break-word',
          wordBreak: 'break-word',
        }}
      >
        {customNodeLabel}
      </div>
      <BottomSourceHandlers trueHanderConnectable={true} falseHanderConnectable={true} dataType={data.type} />
    </div>
  );
}
