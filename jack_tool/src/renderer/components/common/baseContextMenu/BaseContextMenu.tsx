import { PropsWithChildren } from 'react';
import './BaseContextMenu.css';
import { Stack } from '@mui/material';
import { Z_INDEX } from 'misc/constant/common/cmConstants';

interface ContextMenuProps extends PropsWithChildren {
  id: string;
  top?: number;
  left?: number;
  right?: number;
  bottom?: number;
  height?: number | string;
  width?: number | string;
}

export default function BaseContextMenu({ id, top, left, right, bottom, width, height, children }: ContextMenuProps) {
  return (
    <Stack
      id={`context-menu-${id}`}
      direction="column"
      style={{
        top,
        left,
        right,
        bottom,
        width,
        height,
        zIndex: Z_INDEX.MENU,
      }}
      className="context-menu"
    >
      {children}
    </Stack>
  );
}
