import CircleIcon from '@mui/icons-material/Circle';
import { Grid, styled, Typography } from '@mui/material';
import React from 'react';

const StatusContainer = styled(Grid)(() => ({
  width: '80px',
  alignItems: 'center',
  margin: '0 10px 0 10px',
  display: 'flex',
}));

const ConnectedIcon = styled(CircleIcon)(() => ({
  color: 'green',
  marginRight: '5px',
}));

const DisconnectedIcon = styled(CircleIcon)(() => ({
  color: 'red',
  marginRight: '5px',
}));

const ConnectionLabel = styled(Typography)(({ theme }) => ({
  color: theme.custom?.text.color,
}));

interface Props {
  connected: boolean;
  showLabel?: boolean;
  labelStyle?: React.CSSProperties;
}

function ConnectionStatus({ labelStyle, showLabel = true, connected }: Props) {
  // TODO: Add pending status
  return (
    <StatusContainer>
      {connected ? <ConnectedIcon /> : <DisconnectedIcon />}
      {showLabel && <ConnectionLabel style={labelStyle}>Status</ConnectionLabel>}
    </StatusContainer>
  );
}

export default React.memo(ConnectionStatus);
