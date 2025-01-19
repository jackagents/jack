import { Stack, styled } from '@mui/material';

const StyledStack = styled(Stack)({
  margin: '.5vw',
});

export default function ConfigBoxStack(props: any) {
  return (
    <StyledStack children={props.children} direction="column" spacing="1vw" />
  );
}
