import React from 'react';
import { styled, Button, ButtonGroup, useTheme } from '@mui/material';
import MenuAction from 'types/common/cmTypes';
import MenuButton from 'components/common/menuButton/MenuButton';
import { DragRegion, Flex } from 'components/common/base/BaseContainer';
import { request, response } from 'projectEvents/common/cmEvents';
import { IpcRendererEvent } from 'electron';
import imgs from 'misc/images';

/* ************************************************************************************************
 * Styles
 * *********************************************************************************************** */
const Root = styled(Flex)(({ theme }) => ({
  justifyContent: 'space-between',
  alignItems: 'center',
  backgroundColor: theme.custom?.headerBgColor,
}));

const Left = styled('div')({
  height: '100%',
  display: 'flex',
  justifyContent: 'left',
  alignItems: 'center',
});

const Right = styled(Left)({
  justifyContent: 'right',
});

const Logo = styled('img')({
  height: '100%',
  padding: '5px 10px',
  boxSizing: 'border-box',
  objectFit: 'contain',
});

const MenubarButtonGroup = styled(ButtonGroup)({
  height: '100%',
});
const MenubarButton = styled(Button)({
  width: 40,
  height: '100%',
  WebkitAppRegion: 'no-drag',
  '&:hover': {
    backgroundColor: '#d2d2d2',
  },
});

const CloseButton = styled(MenubarButton)(({ theme }) => ({
  '&:hover': {
    backgroundColor: theme.custom?.closeBtnHoveredColor,
  },
}));

/* ************************************************************************************************
 * MenuBar
 * *********************************************************************************************** */
interface Props {
  labelStyle?: React.CSSProperties;
  menuAction?: MenuAction;
  onCloseWindow?: () => void;
  onClickLogo?: () => void;
}

interface States {
  windowMaximised: boolean;
  dark: boolean;
}

function TitleMenu({ labelStyle, menuAction, onCloseWindow, onClickLogo }: Props) {
  /* ----------------------------- useTheme hooks ----------------------------- */
  const theme = useTheme();
  /* ----------------------------- useState hooks ----------------------------- */
  const [isWindowMaximised, setIsWindowMaximised] = React.useState(false);
  const [isMenuActive, setIsMenuActive] = React.useState(false);
  const [activeMenuIndex, setActiveMenuIndex] = React.useState(-1);

  const onWindowMaximizedChanged = (_event: IpcRendererEvent, isMaximized: boolean) => {
    setIsWindowMaximised(isMaximized);
  };

  // TODO add argument into close
  const closeWindow = () => {
    window.ipcRenderer.invoke(request.window.close);
  };

  const minimizeWindow = () => {
    window.ipcRenderer.invoke(request.window.minimize);
  };

  const maximizeWindow = () => {
    window.ipcRenderer.invoke(request.window.maximize);
  };

  const getImage = React.useCallback(() => {
    if (isWindowMaximised) {
      if (theme.theme === 'dark') {
        return <img alt="restore" src={imgs.wRestoreImg} />;
      }

      return <img alt="restore" src={imgs.bRestoreImg} />;
    }

    if (theme.theme === 'dark') {
      return <img alt="maximize" src={imgs.wMaxImg} />;
    }

    return <img alt="maximize" src={imgs.bMaxImg} />;
  }, [isWindowMaximised, theme.theme]);

  /* ********************************************************************************************
   * Functions
   * ******************************************************************************************* */

  return (
    <DragRegion>
      <Root>
        <Left role="menu">
          <Logo src={imgs.iconImg} onClick={onClickLogo} style={{ cursor: onClickLogo && 'pointer' }} />
          <MenubarButtonGroup>
            {menuAction?.submenu!.map((action: MenuAction, key: number) => (
              <MenuButton
                key={key as number}
                index={key}
                label={action.label!}
                submenu={action.submenu!}
                isMenuActive={isMenuActive}
                setIsMenuActive={(activeStatus: boolean) => {
                  setIsMenuActive(activeStatus);
                }}
                activeMenuIndex={activeMenuIndex}
                setActiveMenuIndex={(activeIndex: number) => {
                  setActiveMenuIndex(activeIndex);
                }}
                disabled={action.disabled}
                style={{
                  color: theme.custom?.button.textColor,
                  ...labelStyle,
                }}
              />
            ))}
          </MenubarButtonGroup>
        </Left>
      </Root>
    </DragRegion>
  );
}

export default React.memo(TitleMenu);
