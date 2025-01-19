import React from 'react';
import { styled, Tooltip } from '@mui/material';
import {
  capitalize,
  getObjectByModuleConcept,
} from 'misc/utils/cbdiEdit/Helpers';
import { useSelector } from 'react-redux';
import { RootState } from 'projectRedux/Store';
import {
  CBDIEditorPlanNodeType,
  IPlanNode,
} from 'misc/types/cbdiEdit/cbdiEditTypes';
import TaskAvatar from './TaskAvatar';

/* --------------------------------- Styles --------------------------------- */
const Root = styled('div')({
  width: '100%',
  height: 60,
  position: 'relative',
});

const AvatarSlot = styled('div')({
  position: 'absolute',
  left: 0,
  width: 50,
  height: 50,
});

const NameAndType = styled('div')({
  position: 'absolute',
  left: 60,
  right: 0,
  height: 50,
});

const TextView = styled('div')({
  width: '100%',
  lineHeight: '25px',
});
/* ------------------------------- TaskHeader ------------------------------- */

interface Props {
  task: IPlanNode;
}

function TaskHeader({ task }: Props) {
  const { nodeData } = task;
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector(
    (state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current
  );

  /* ------------------------------ useMemo hooks ----------------------------- */
  const title = React.useMemo(() => {
    let mtitle: string | undefined;

    switch (nodeData.type) {
      case CBDIEditorPlanNodeType.ActionPlanNodeType:
        if (nodeData.action === undefined) {
          mtitle = 'Action';
        } else {
          const actionObj = getObjectByModuleConcept(current!, nodeData.action);
          mtitle = actionObj?.name;
        }
        break;
      case CBDIEditorPlanNodeType.GoalPlanNodeType:
        if (nodeData.goal === undefined) {
          mtitle = 'Goal';
        } else {
          const goalObj = getObjectByModuleConcept(current!, nodeData.goal);
          mtitle = goalObj?.name;
        }
        break;
      case CBDIEditorPlanNodeType.SleepPlanNodeType:
        mtitle = nodeData.duration ? `Sleep for ${nodeData.duration}` : 'Sleep';
        break;
      case CBDIEditorPlanNodeType.ConditionPlanNodeType:
        mtitle = nodeData.conditiontext || 'Condition';
        break;
      default:
        break;
    }

    return mtitle;
  }, [task, current]);

  /* ------------------------------- Components ------------------------------- */
  return (
    <Root>
      <Tooltip
        title={<span style={{ fontSize: 14 }}>{nodeData.note}</span>}
        placement="left"
        arrow
      >
        <AvatarSlot>
          <TaskAvatar task={task} />
        </AvatarSlot>
      </Tooltip>
      <NameAndType>
        <TextView
          style={{
            fontWeight: 'bold',
          }}
        >
          {title}
        </TextView>
        <TextView>{capitalize(task.type)}</TextView>
      </NameAndType>
    </Root>
  );
}

export default React.memo(TaskHeader);
