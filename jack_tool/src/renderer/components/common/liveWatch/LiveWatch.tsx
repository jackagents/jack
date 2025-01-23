import { Stack, styled, Typography } from '@mui/material';
import React from 'react';
import { Container } from '../base/BaseContainer';

const CustomText = styled(Typography)(({ theme }) => ({
  color: theme.custom?.text.color,
  fontSize: '.8em',
}));

const CustomStack = styled(Stack)({
  marginLeft: '10px',
  alignItems: 'flex-end',
  height: '100%',
  justifyContent: 'center',
});

const LiveWatchContainer = styled(Container)({
  width: '100%',
  justifyContent: 'flex-end',
  alignItems: 'flex-end',
  marginRight: '10px',
});

interface Props {
  direction?: 'row' | 'column';
  labelStyle?: React.CSSProperties;
}
function LiveWatch({ direction = 'column', labelStyle }: Props) {
  const [dateState, setDateState] = React.useState(new Date());

  React.useEffect(() => {
    setInterval(() => setDateState(new Date()), 1000);
  }, []);

  return (
    <LiveWatchContainer container>
      <CustomStack
        direction={direction}
        spacing={direction === 'row' ? 1 : undefined}
        style={direction === 'row' ? { alignItems: 'center' } : undefined}
      >
        <CustomText style={labelStyle} color="primary">
          {dateState.toLocaleDateString('en-AU', {
            day: 'numeric',
            month: 'numeric',
            year: 'numeric',
          })}
        </CustomText>
        <CustomText style={labelStyle}>
          {dateState.toLocaleTimeString('en-US', {
            hour: 'numeric',
            minute: 'numeric',
            timeZoneName: 'short',
          })}
        </CustomText>
      </CustomStack>
    </LiveWatchContainer>
  );
}

export default React.memo(LiveWatch);
