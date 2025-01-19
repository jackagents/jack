import { IPlanNode } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { Fluid } from 'components/common/base/BaseContainer';
import TaskDetailView from './TaskDetailView';

/* ----------------------------- TaskDetailBody ----------------------------- */
interface Props {
  task: IPlanNode;
}

export default function TaskDetailBody({ task }: Props) {
  return (
    <Fluid>
      <TaskDetailView task={task} />
    </Fluid>
  );
}
