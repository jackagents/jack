import { IconButton, useTheme } from '@mui/material';
import React, { PropsWithChildren } from 'react';

interface Props extends PropsWithChildren {
  title: string;
  disabled?: boolean;
  onClick?: React.MouseEventHandler<HTMLButtonElement>;
}
export default function IconBtnOnRightToolTitle({ title, children, disabled = false, onClick }: Props) {
  const theme = useTheme();

  return (
    <IconButton
      sx={{
        borderRadius: 0,
        boxShadow: 0,
        color: theme.custom?.setting.textColor,
        '&:hover': { color: theme.custom?.text.color },
      }}
      style={{ fontSize: '2rem' }}
      title={title}
      onClick={onClick}
      disabled={disabled}
    >
      {children}
    </IconButton>
  );
}
