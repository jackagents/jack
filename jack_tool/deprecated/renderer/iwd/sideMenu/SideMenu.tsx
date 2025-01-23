import React from 'react';
import {
  SearchOutlined as SearchOutlinedIcon,
  SettingsOutlined as SettingsOutlinedIcon,
  SportsEsportsOutlined as SportsEsportsOutlinedIcon,
  PsychologyOutlined as PsychologyOutlinedIcon,
  RadarOutlined as RadarOutlinedIcon,
} from '@mui/icons-material';
import ConstructionOutlinedIcon from '@mui/icons-material/ConstructionOutlined';
import { useDispatch, useSelector } from 'react-redux';
import {
  SideMenuButton,
  SideMenuButtonGroup,
  SideMenuRoot,
} from 'components/common/base/BaseContainer';
import { iwdActions } from 'projectRedux/reducers/iwd/iwdClientReducer';
import { MODE } from 'constant/iwd/iwdConstant';
import { RootState } from 'projectRedux/Store';
import { CustomMenu } from 'types/iwd/iwdTypes';

const defaultMenu: CustomMenu = {
  creatorMenu: false,
  searchMenu: false,
  settingMenu: false,
  simMapMenu: false,
  explainability: false,
  scenarioEditor: false,
};

function SideMenu() {
  const { explorer } = useSelector((state: RootState) => state.iwd);

  const dispatch = useDispatch();

  /* ********************************************************************************************
   * Callbacks
   * ******************************************************************************************* */

  const handleClick = (newMode: string) => {
    if (explorer.mode === newMode) {
      dispatch(iwdActions.setExplorerMode(MODE.MODE_NONE));
      dispatch(
        iwdActions.setMenu({
          ...defaultMenu,
        })
      );
      return;
    }

    dispatch(iwdActions.setExplorerMode(newMode));
    switch (newMode) {
      case MODE.MODE_SCENARIO_CREATION:
        dispatch(iwdActions.setMenu({ ...defaultMenu, creatorMenu: true }));
        break;
      case MODE.MODE_SETTINGS:
        dispatch(iwdActions.setMenu({ ...defaultMenu, settingMenu: true }));
        break;
      case MODE.MODE_SEARCH:
        dispatch(iwdActions.setMenu({ ...defaultMenu, searchMenu: true }));
        break;
      case MODE.MODE_SIM_MAP:
        dispatch(iwdActions.setMenu({ ...defaultMenu, simMapMenu: true }));
        break;
      case MODE.MODE_RUNTIME_EXPLAINABILITY:
        dispatch(iwdActions.setMenu({ ...defaultMenu, explainability: true }));
        break;
      case MODE.MODE_SCENARIO_EDITOR:
        dispatch(iwdActions.setMenu({ ...defaultMenu, scenarioEditor: true }));
        break;
      default:
        break;
    }
  };

  /* ********************************************************************************************
   * Functions
   * ******************************************************************************************* */

  return (
    <SideMenuRoot>
      <SideMenuButtonGroup orientation="vertical">
        <SideMenuButton
          className={
            explorer.mode === MODE.MODE_SCENARIO_CREATION ? 'checked' : ''
          }
          variant="text"
          color="secondary"
          onClick={() => {
            handleClick(MODE.MODE_SCENARIO_CREATION);
          }}
        >
          <SportsEsportsOutlinedIcon />
        </SideMenuButton>

        <SideMenuButton
          className={explorer.mode === MODE.MODE_SEARCH ? 'checked' : ''}
          variant="text"
          color="secondary"
          onClick={() => {
            handleClick(MODE.MODE_SEARCH);
          }}
        >
          <SearchOutlinedIcon />
        </SideMenuButton>

        <SideMenuButton
          className={explorer.mode === MODE.MODE_SIM_MAP ? 'checked' : ''}
          variant="text"
          color="secondary"
          onClick={() => {
            handleClick(MODE.MODE_SIM_MAP);
          }}
        >
          <RadarOutlinedIcon />
        </SideMenuButton>

        <SideMenuButton
          className={
            explorer.mode === MODE.MODE_RUNTIME_EXPLAINABILITY ? 'checked' : ''
          }
          variant="text"
          color="secondary"
          onClick={() => {
            handleClick(MODE.MODE_RUNTIME_EXPLAINABILITY);
          }}
        >
          <PsychologyOutlinedIcon />
        </SideMenuButton>

        <SideMenuButton
          className={
            explorer.mode === MODE.MODE_SCENARIO_EDITOR ? 'checked' : ''
          }
          variant="text"
          color="secondary"
          onClick={() => {
            handleClick(MODE.MODE_SCENARIO_EDITOR);
          }}
        >
          <ConstructionOutlinedIcon />
        </SideMenuButton>
      </SideMenuButtonGroup>

      <SideMenuButtonGroup orientation="vertical">
        <SideMenuButton
          className={explorer.mode === MODE.MODE_SETTINGS ? 'checked' : ''}
          variant="text"
          color="secondary"
          onClick={() => {
            handleClick(MODE.MODE_SETTINGS);
          }}
        >
          <SettingsOutlinedIcon />
        </SideMenuButton>
      </SideMenuButtonGroup>
    </SideMenuRoot>
  );
}

/* ************************************************************************************************
 * Connect to Redux Store
 * *********************************************************************************************** */
export default React.memo(SideMenu);
