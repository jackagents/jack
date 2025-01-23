import { IPlanNode } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { Fluid } from 'components/common/base/BaseContainer';
import DetailField from '../../children/DetailField';
import TaskDetailBody from './TaskDetailBody';

/* ---------------------------- TaskDetailContent --------------------------- */
interface Props {
  task: IPlanNode;
}

export default function TaskDetailContent({ task }: Props) {
  return (
    <Fluid>
      <DetailField fieldName="body" content={<TaskDetailBody task={task} />} />
    </Fluid>
  );
}
