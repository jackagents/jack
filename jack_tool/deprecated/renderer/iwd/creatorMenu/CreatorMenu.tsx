import { Stack } from '@mui/material';
import React from 'react';
import MapOutlinedIcon from '@mui/icons-material/MapOutlined';
import DirectionsCarFilledOutlinedIcon from '@mui/icons-material/DirectionsCarFilledOutlined';
import { CREATOR_BUTTON_ID, CREATOR_MENU } from 'constant/common/cmConstants';
import FunctionLabel from 'components/common/functionLabel/FunctionLabel';
import {
  ContentBox,
  GridContainer,
  StyledCustomList,
} from 'components/common/base/BaseContainer';
import { useDispatch, useSelector } from 'react-redux';
import { RootState } from 'projectRedux/Store';
import { iwdActions } from 'projectRedux/reducers/iwd/iwdClientReducer';
import { BuildMode } from 'types/iwd/iwdTypes';
import { IWD_NOTI_MESS } from 'constant/iwd/iwdConstant';
import { useSnackbar } from 'notistack';
import MapList from 'components/common/mapList/MapList';
import SimMenuButton from '../../../../../src/renderer/components/common/simulationMenuButton/SimMenuButton';

/**
 * IWatchdog create menu
 * @returns
 */
function CreatorMenu() {
  const [anchorEl, setAnchorEl] = React.useState<null | HTMLElement>(null);
  const { enqueueSnackbar, closeSnackbar } = useSnackbar();
  const open = Boolean(anchorEl);

  const { currentBuildMode } = useSelector((state: RootState) => state.iwd);
  const { changeBuildMode, setZoneType } = iwdActions;
  const dispatch = useDispatch();

  /**
   * Call back on open map list menu
   */
  const openMenuList = React.useCallback(
    (event: React.MouseEvent<HTMLElement>) => {
      setAnchorEl(event.currentTarget);
    },
    []
  );

  /**
   * Callback on click simulation button
   */
  const handleClick = React.useCallback(
    (event: React.MouseEvent<HTMLElement>, id: number) => {
      if (currentBuildMode !== BuildMode.NONE) return;

      switch (id) {
        case CREATOR_BUTTON_ID.MAP: {
          // Open map type menu
          openMenuList(event);
          enqueueSnackbar(IWD_NOTI_MESS.CHOOSE_ZONE, { variant: 'warning' });
          break;
        }

        case CREATOR_BUTTON_ID.VEHICLE: {
          // Allow add vehicle in VehicleBuilder.tsx
          dispatch(changeBuildMode(BuildMode.VEHICLE));
          enqueueSnackbar(IWD_NOTI_MESS.PLACE_VEHICLE, { variant: 'warning' });
          break;
        }

        default:
          break;
      }
    },
    [currentBuildMode]
  );

  /**
   * Callback on close map list menu
   */
  const handleClose = React.useCallback(() => {
    dispatch(changeBuildMode(BuildMode.NONE));
    setAnchorEl(null);
    closeSnackbar();
  }, []);

  /**
   * Callback on click map list menu
   */
  const handleMenuListClick = React.useCallback(
    (_event: React.MouseEvent<HTMLElement>, index: number) => {
      dispatch(changeBuildMode(BuildMode.ZONE));
      dispatch(setZoneType(index));
      setAnchorEl(null);
      closeSnackbar();
    },
    []
  );

  return (
    <GridContainer container>
      <StyledCustomList>
        <Stack direction="column">
          {/* Create map menu */}
          <ContentBox>
            <FunctionLabel text={CREATOR_MENU.MAP} />
            <SimMenuButton
              id={CREATOR_BUTTON_ID.MAP}
              icon={MapOutlinedIcon}
              text="Add Zone"
              onClick={(event) => {
                handleClick(event, CREATOR_BUTTON_ID.MAP);
              }}
              elements={[
                // Map list menu
                <MapList
                  anchorEl={anchorEl}
                  open={open}
                  onClose={handleClose}
                  onMenuListClick={handleMenuListClick}
                />,
              ]}
            />
          </ContentBox>

          {/* Create vehicle menu */}
          <ContentBox>
            <FunctionLabel text={CREATOR_MENU.ASSETS} />
            <SimMenuButton
              id={CREATOR_BUTTON_ID.VEHICLE}
              icon={DirectionsCarFilledOutlinedIcon}
              text="Add Vehicle"
              onClick={(event) => {
                handleClick(event, CREATOR_BUTTON_ID.VEHICLE);
              }}
            />
          </ContentBox>
        </Stack>
      </StyledCustomList>
    </GridContainer>
  );
}

export default React.memo(CreatorMenu);
