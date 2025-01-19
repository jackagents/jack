import { Menu, MenuItem, styled } from '@mui/material';
import { NestedMenuItem } from 'mui-nested-menu';
import React from 'react';
import { v4 } from 'uuid';

interface MenuAction {
  label?: string;
  submenu?: MenuAction[];
  accelerator?: string;
  onClick?: () => void;
}

interface Props {
  cursor: { x: number; y: number } | null;
  menu: MenuAction[];
  offSetX: number;
  offSetY: number;
  onClose: () => void;
}

const StyledNestedMenu = styled(NestedMenuItem)({
  marginLeft: '5px',
});

const StyledMenuItem = styled(MenuItem)({
  marginLeft: '5px',
  padding: '6px 12px',
});

export function ContextMenu(props: Props) {
  const [contextMenu, setContextMenu] = React.useState<{
    mouseX: number;
    mouseY: number;
  } | null>(null);

  React.useEffect(() => {
    if (!props.cursor) return;

    setContextMenu(
      contextMenu === null
        ? {
            mouseX: props.cursor.x - props.offSetX,
            mouseY: props.cursor.y - props.offSetY,
          }
        : null
    );
  }, [props.cursor]);

  const createMenu = (menu: MenuAction) => {
    return menu.submenu ? (
      <StyledNestedMenu
        className="context-nested-menu-item"
        key={v4()}
        label={menu.label}
        onClick={() => {
          if (menu.onClick) menu.onClick();
        }}
        autoFocus={false}
        parentMenuOpen
        onKeyDown={(e) => {
          e.stopPropagation();
        }}
      >
        {menu.submenu.map((submenu: MenuAction) => {
          return createMenu(submenu);
        })}
      </StyledNestedMenu>
    ) : (
      <StyledMenuItem
        autoFocus={false}
        className="context-menu-item"
        key={v4()}
        onClick={menu.onClick}
      >
        {menu.label}
      </StyledMenuItem>
    );
  };

  const handleClose = () => {
    setContextMenu(null);
    props.onClose();
  };

  return (
    <Menu
      className="context-menu"
      open={contextMenu !== null}
      disableAutoFocus
      disableAutoFocusItem
      onClose={handleClose}
      anchorReference="anchorPosition"
      anchorPosition={
        contextMenu !== null
          ? { top: contextMenu.mouseY, left: contextMenu.mouseX }
          : undefined
      }
    >
      {props.menu.map((submenu: MenuAction) => {
        // Create Nested menu until no submenu
        return createMenu(submenu);
      })}
    </Menu>
  );
}
