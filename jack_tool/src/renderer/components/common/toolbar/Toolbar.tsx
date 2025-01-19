import { Stack, styled } from '@mui/material';
import React, { PropsWithChildren } from 'react';

const ToolbarStack = styled(Stack)(({ theme }) => ({
  height: 'inherit',
  backgroundColor: theme.custom?.toolbarBgColor,
}));

interface Props extends PropsWithChildren {
  toolbarStyle?: React.CSSProperties;
}

function Toolbar({ toolbarStyle, children }: Props) {
  return (
    <ToolbarStack direction="row" style={toolbarStyle}>
      {children}
    </ToolbarStack>
  );
}

export default React.memo(Toolbar);
