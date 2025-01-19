/* eslint-disable react/jsx-props-no-spreading */
import React from 'react';
import {
  ClickAwayListener,
  Grow,
  Paper,
  Popper,
  styled,
  Button,
  MenuList,
} from '@mui/material';
import { NestedMenuItem } from 'mui-nested-menu';
import { v4 } from 'uuid';
import MyMenuItem from 'components/common/menuItem/MenuItem';
import MenuAction from 'types/common/cmTypes';

const TopMenuButton = styled(Button)(({ theme }) => ({
  padding: '0 0.5em',
  margin: '0.2em',
  color: theme.custom?.text.color,
  textTransform: 'none',
  '&:hover': {
    backgroundColor: '#d2d2d2',
    borderRadius: '0.5em',
  },
  '&:disabled': {
    color: 'grey',
  },
}));

const NoDragDiv = styled('div')({
  WebkitAppRegion: 'no-drag',
});
/* ************************************************************************************************
 * MenuButton
 * *********************************************************************************************** */
interface Props {
  label: string | JSX.Element;
  index: number;
  submenu: MenuAction[];
  disabled?: boolean;
  isMenuActive: boolean;
  setIsMenuActive: (activeStatus: boolean) => void;
  activeMenuIndex: number;
  setActiveMenuIndex: (activeIndex: number) => void;
  style?: React.CSSProperties;
}

export default function MenuButton({
  label,
  index,
  submenu,
  style,
  isMenuActive,
  setIsMenuActive,
  activeMenuIndex,
  setActiveMenuIndex,
  disabled = false,
}: Props) {
  /* ----------------------------- useRef hooks ----------------------------- */
  const anchorRef = React.useRef(null);
  /* -------------------------------- Callbacks ------------------------------- */
  const handleMenuClick = () => {
    setActiveMenuIndex(index);
    setIsMenuActive(true);
  };

  const handleMouseEnter = () => {
    if (isMenuActive) {
      setActiveMenuIndex(index);
    }
  };

  const handleClose = () => {
    setActiveMenuIndex(-1);
    setIsMenuActive(false);
  };

  return (
    <NoDragDiv>
      <TopMenuButton
        ref={anchorRef}
        variant="text"
        onClick={handleMenuClick}
        onMouseEnter={handleMouseEnter}
        style={style || undefined}
        disabled={disabled}
      >
        {label}
      </TopMenuButton>
      <Popper
        open={index === activeMenuIndex}
        anchorEl={anchorRef.current}
        role={undefined}
        placement="bottom-start"
        transition
        disablePortal
        style={{ zIndex: 1300 }}
      >
        {({ TransitionProps, placement }) => (
          <Grow
            {...TransitionProps}
            style={{
              transformOrigin:
                placement === 'bottom-start' ? 'left top' : 'left bottom',
            }}
          >
            <Paper>
              <ClickAwayListener onClickAway={handleClose}>
                <MenuList autoFocusItem={index === activeMenuIndex}>
                  {submenu.map((action: MenuAction) => {
                    if (
                      action.submenu &&
                      action.submenu.length > 0 &&
                      !action.disabled
                    ) {
                      return (
                        <NestedMenuItem
                          key={v4()}
                          label={action.label}
                          parentMenuOpen={index === activeMenuIndex}
                          onKeyDown={(e) => {
                            e.stopPropagation();
                          }}
                          disabled={action.disabled || false}
                        >
                          {action.submenu.map((subaction: MenuAction) => {
                            return (
                              <MyMenuItem
                                key={v4()}
                                label={subaction.label!}
                                onClick={subaction.onClick!}
                                onClose={handleClose}
                              />
                            );
                          })}
                        </NestedMenuItem>
                      );
                    }
                    return (
                      <MyMenuItem
                        key={v4()}
                        label={action.label!}
                        onClick={action.onClick!}
                        onClose={handleClose}
                        disabled={action.disabled || false}
                      />
                    );
                  })}
                </MenuList>
              </ClickAwayListener>
            </Paper>
          </Grow>
        )}
      </Popper>
    </NoDragDiv>
  );
}
