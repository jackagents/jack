import { Flex } from 'components/common/base/BaseContainer';
import { styled } from '@mui/material';
import Placeholder from 'components/common/placeholder/Placeholder';

const Root = styled(Flex)({
  backgroundColor: '#252525',
});

/* ------------------------------ ProjectPanel ------------------------------ */
export default function Search() {
  return (
    <Root>
      <Placeholder label="Search" />
    </Root>
  );
}
