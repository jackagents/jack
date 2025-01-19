import { styled } from '@mui/material';
import { nodeColor, nodeIcon } from 'misc/icons/cbidEdit/cbdiEditIcon';
import { IPlanNode } from 'misc/types/cbdiEdit/cbdiEditTypes';

/* --------------------------------- Styles --------------------------------- */
const Root = styled('div')({
  width: 50,
  height: 50,
  borderRadius: 5,
  position: 'relative',
});

/* --------------------------------- TaskAvatar --------------------------------- */
interface Props {
  task: IPlanNode;
}

export default function TaskAvatar({ task }: Props) {
  const { nodeData } = task;
  return (
    <Root style={{ backgroundColor: nodeColor[nodeData.type] }}>
      <img alt="" src={nodeIcon[nodeData.type]} style={{ width: 50, height: 50, padding: 10 }} />
    </Root>
  );
}
