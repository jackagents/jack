import { useInterval } from 'hooks/common/useInterval';
import { LatLng } from 'leaflet';
import { iwdRequest, iwdResponse } from 'projectEvents/iwd/iwdEvents';
import React from 'react';
import { BuildMode, DrivingMode, IwdIconButtonId } from 'types/iwd/iwdTypes';
import { GeoPosition, IwdVehicleStatus } from 'types/iwd/iwdVehicleModel';
import ConfigGeoSpatialContent from 'components/common/configGeoSpatialContent/ConfigGeoSpatialContent';
import { useActiveElement } from 'hooks/common/useActiveElement';
import { CONSTANT_STRING } from 'constant/common/cmConstants';
import { IpcRendererEvent } from 'electron';
import MissionConfig from 'components/iwd/configEntity/Vehicle/children/missionConfig/MissionConfig';
import VehicleControl from 'components/iwd/configEntity/Vehicle/children/vehicleControl/VehicleControl';
import MissionDrawer from 'components/iwd/configEntity/Vehicle/children/missionDrawer/MissionDrawer';
import { useDispatch } from 'react-redux';
import {
  iwdActions,
  IwdClientState,
} from 'projectRedux/reducers/iwd/iwdClientReducer';
import { useSliceSelector } from 'projectRedux/sliceProvider/SliceProvider';
import VehicleConfig from './children/vehicleConfig/VehicleConfig';

interface Props {
  vehicle: { id: string };
}

enum Focus {
  none = 'none',
  lat = 'lat',
  lng = 'lng',
  name = 'name',
}

const defaultInterval = 1000;

function ConfigVehicleContent({ vehicle }: Props) {
  const { id } = vehicle;
  const [position, setPosition] = React.useState<LatLng>(new LatLng(0, 0, 0));
  const [interval, setInterval] = React.useState<number | null>(
    defaultInterval
  );

  const [focus, setFocus] = React.useState<Focus>(Focus.none);

  const [drawer, setDrawer] = React.useState(false);

  const { currentBuildMode } = useSliceSelector() as IwdClientState;

  const { changeBuildMode } = iwdActions;

  const dispatch = useDispatch();

  const currBtnId = React.useMemo(() => {
    switch (currentBuildMode) {
      case BuildMode.PATH_MISSION:
        return IwdIconButtonId.path;
      case BuildMode.INTRUDER_MISSION:
        return IwdIconButtonId.intruder;
      case BuildMode.SCOUT_MISSION:
        return IwdIconButtonId.scout;
      case BuildMode.NONE:
      default:
        return IwdIconButtonId.none;
    }
  }, [currentBuildMode]);

  const focusedElement = useActiveElement();

  React.useEffect(() => {
    if (!focusedElement) {
      console.log('focusedElement is null');
      return;
    }

    const element = focusedElement as HTMLInputElement;
    //
    if (element.name === 'lat') {
      setFocus(Focus.lat);
    }
    //
    else if (element.name === 'lng') {
      setFocus(Focus.lng);
    }
    //
    else if (element.name === 'name') {
      setFocus(Focus.name);
    }
    //
    else if (
      element.name !== CONSTANT_STRING.BTN_SAVE &&
      element.name !== CONSTANT_STRING.BTN_CANCEL
    ) {
      setFocus(Focus.none);
    }
  }, [focusedElement]);

  /**
   * Callback on request current vehicle position, update the ui.
   */
  const onResponseReqCurrentVehiclePosition = React.useCallback(
    (_e: IpcRendererEvent, pos: string, status: IwdVehicleStatus) => {
      // update ui position from backend position
      const p = JSON.parse(pos) as GeoPosition;
      setPosition(new LatLng(p.lat, p.lng, 0));

      // If status = idle stop the loop of requesting for new pos
      if (status === IwdVehicleStatus.idle) {
        setInterval(null);
      }
    },
    []
  );

  /**
   * Callback on update status event, update the interval to default number.
   */
  const onResponseUpdateStatus = React.useCallback(
    (_e: IpcRendererEvent, _id: string, status: IwdVehicleStatus) => {
      if (_id === id && status === IwdVehicleStatus.moving) {
        setInterval(defaultInterval);
      }
    },
    []
  );

  React.useEffect(() => {
    window.ipcRenderer.send(
      iwdRequest.vehicle.requestCurrentVehiclePosition,
      id
    );

    window.ipcRenderer.on(
      iwdResponse.vehicle.requestCurrentVehiclePosition,
      onResponseReqCurrentVehiclePosition
    );

    window.ipcRenderer.on(
      iwdResponse.vehicle.updateStatus,
      onResponseUpdateStatus
    );

    return () => {
      window.ipcRenderer.removeAllListeners(
        iwdResponse.vehicle.requestCurrentVehiclePosition
      );

      window.ipcRenderer.removeAllListeners(iwdResponse.vehicle.updateStatus);
    };
  }, []);

  // request for new position by - interval
  useInterval(() => {
    window.ipcRenderer.send(
      iwdRequest.vehicle.requestCurrentVehiclePosition,
      id
    );
  }, interval);

  const handleClickVehicleControl = React.useCallback(() => {
    // TODO: click vehicle control
  }, []);

  const handleClickMissionConfig = React.useCallback(
    (event: React.MouseEvent<HTMLButtonElement>) => {
      switch (event.currentTarget.id) {
        case IwdIconButtonId.path: {
          // May use to add other options in the future
          // openDrawer();

          // Trigger create path mission
          dispatch(changeBuildMode(BuildMode.PATH_MISSION));
          break;
        }
        case IwdIconButtonId.intruder: {
          // Trigger create path mission
          dispatch(changeBuildMode(BuildMode.INTRUDER_MISSION));
          break;
        }
        case IwdIconButtonId.scout: {
          // Trigger create path mission
          dispatch(changeBuildMode(BuildMode.SCOUT_MISSION));
          break;
        }
        default: {
          break;
        }
      }
    },
    []
  );

  const openDrawer = React.useCallback(() => {
    setDrawer(true);
  }, []);

  const closeDrawer = React.useCallback(() => {
    setDrawer(false);
  }, []);

  return (
    <>
      <VehicleControl
        activated
        returnToBase
        mode={DrivingMode.AUTO}
        onClick={handleClickVehicleControl}
      />
      <MissionConfig current={currBtnId} onClick={handleClickMissionConfig} />
      <VehicleConfig focus={focus} id={id} />
      <ConfigGeoSpatialContent focus={focus} position={position} />
      <MissionDrawer
        drawer={drawer}
        onClose={closeDrawer}
        current={currBtnId}
      />
    </>
  );
}

export default React.memo(ConfigVehicleContent);
