import React from 'react';
import { Button, styled } from '@mui/material';
import { DescriptionOutlined as DescriptionOutlinedIcon, Adb as AdbIcon, SettingsOutlined as SettingsOutlinedIcon } from '@mui/icons-material';
import { FlexCol } from 'components/common/base/BaseContainer';
import { MODE_PROJECT, MODE_RUNTIME, MODE_SETTINGS } from 'constant/cbdiEdit/cbdiEditConstant';
import { useDispatch, useSelector } from 'react-redux';
import { RootState } from 'projectRedux/Store';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';

/* --------------------------------- Styles --------------------------------- */
const Root = styled(FlexCol)(({ theme }) => ({
  backgroundColor: theme.editor.menu.bgColor,
  justifyContent: 'space-between',
}));

const SideMenuButton = styled(Button)(({ theme }) => ({
  width: '100%',
  height: 50,
  padding: 0,
  minWidth: '100%',
  border: '2px solid transparent',
  color: theme.editor.menu.textColor,
  '& .MuiSvgIcon-root': {
    fontSize: '30px',
  },
  '&:hover': {
    color: theme.editor.menu.activeTextColor,
    backgroudColor: theme.editor.menu.hoveringBgColor,
  },
  '&.checked': {
    color: theme.editor.menu.activeTextColor,
    borderLeft: `2px solid ${theme.editor.menu.activeTextColor}`,
  },
}));

interface Props {
  modelLoaded?: boolean;
}

/* -------------------------------- SideMenu -------------------------------- */
function SideMenu({ modelLoaded = false }: Props) {
  /* ---------------------------------- Redux --------------------------------- */
  const mode = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.explorer.mode);
  const dispatch = useDispatch();
  const { setExplorerMode } = cbdiEditActions;

  /* -------------------------------- Callbacks ------------------------------- */

  const switchMode: React.MouseEventHandler<HTMLButtonElement> | undefined = (event) => {
    dispatch(setExplorerMode(event.currentTarget.id));
  };

  /* ------------------------------- Components ------------------------------- */

  return (
    <Root>
      <div>
        <SideMenuButton
          className={mode === MODE_PROJECT ? 'checked' : ''}
          variant="text"
          color="secondary"
          id={MODE_PROJECT}
          onClick={switchMode} // If we use inline arrow function, it will cause rerendering
        >
          <DescriptionOutlinedIcon />
        </SideMenuButton>

        <SideMenuButton
          disabled={!modelLoaded}
          className={mode === MODE_RUNTIME ? 'checked' : ''}
          variant="text"
          color="secondary"
          id={MODE_RUNTIME}
          onClick={switchMode}
        >
          <AdbIcon />
        </SideMenuButton>
      </div>

      <div>
        <SideMenuButton className={mode === MODE_SETTINGS ? 'checked' : ''} variant="text" color="secondary" id={MODE_SETTINGS} onClick={switchMode}>
          <SettingsOutlinedIcon />
        </SideMenuButton>
      </div>
    </Root>
  );
}

export default React.memo(SideMenu);
