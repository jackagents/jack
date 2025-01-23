import { IpcRendererEvent } from 'electron';
import { LatLng, LeafletMouseEvent } from 'leaflet';
import { iwdRequest, iwdResponse } from 'projectEvents/iwd/iwdEvents';
import { iwdActions } from 'projectRedux/reducers/iwd/iwdClientReducer';
import { RootState } from 'projectRedux/Store';
import React from 'react';
import { useMapEvents } from 'react-leaflet';
import { useDispatch, useSelector } from 'react-redux';
import { IwdPathMissionModel } from 'types/iwd/iwdMissionModel';
import { BuildMode } from 'types/iwd/iwdTypes';
import { IwdVehicleModel } from 'types/iwd/iwdVehicleModel';
import BeVehicle from 'main/beBuilders/iwd/vehicle/BeVehicle';
import RebaseLayer from './children/RebaseLayer';
import VehicleLayer from './children/VehicleLayer';

interface RebaseProps {
  rebaseVehicle?: BeVehicle;
  newPosition?: LatLng;
}

function VehicleBuilder() {
  const { currentBuildMode, saveMode } = useSelector(
    (state: RootState) => state.iwd
  );
  const { changeBuildMode, setSaveMode } = iwdActions;

  const dispatch = useDispatch();

  const [rebase, setRebase] = React.useState<RebaseProps>();

  const drawable = React.useMemo(() => {
    if (
      currentBuildMode === BuildMode.VEHICLE ||
      currentBuildMode === BuildMode.REBASE_VEHICLE
    )
      return true;

    return false;
  }, [currentBuildMode]);

  const [vehicles, setVehicles] = React.useState<BeVehicle[]>([]);

  /**
   * Callback on create vehicle
   */
  const handleCreateVehicle = React.useCallback(
    (_e: IpcRendererEvent, vehicle: string) => {
      const newVehicle = JSON.parse(vehicle) as BeVehicle;

      setVehicles([...vehicles, newVehicle]);
    },
    [vehicles]
  );

  /**
   * Callback on click on map in vehicle build mode or rebase mode
   */
  const handleClick = React.useCallback(
    (e: LeafletMouseEvent) => {
      if (!drawable) return;

      // Build new vehicle
      if (currentBuildMode === BuildMode.VEHICLE) {
        // Add Vehicle

        // Send latlng to backend
        window.ipcRenderer.invoke(
          iwdRequest.vehicle.create,
          JSON.stringify(e.latlng)
        );

        // Reset to normal
        dispatch(iwdActions.changeBuildMode(BuildMode.NONE));
      }

      // Set base position
      else if (
        currentBuildMode === BuildMode.REBASE_VEHICLE &&
        rebase &&
        rebase.rebaseVehicle
      ) {
        // Render a marker represent new position
        setRebase({ ...rebase, newPosition: e.latlng });
      }
    },
    [drawable]
  );

  useMapEvents({
    click: handleClick,
  });

  /**
   * Callback on start rebase, search for vehicle in Array of vehicles.
   */
  const onRebase = React.useCallback(
    (_event: IpcRendererEvent, data: string) => {
      const vehicle = JSON.parse(data) as IwdVehicleModel;
      const index = vehicles.findIndex((x) => x.model?.id === vehicle.id);

      if (index >= 0) {
        setRebase({
          rebaseVehicle: vehicles[index],
        });
      }
    },
    [vehicles]
  );

  /**
   * Callback on vehicles updated.
   */
  const onUpdateVehicles = React.useCallback(
    (_event: IpcRendererEvent, data: string) => {
      const newVehicle = JSON.parse(data) as BeVehicle;

      const newVehicles = vehicles.map((v) => {
        if (v.model?.id === newVehicle.model?.id) {
          return newVehicle;
        }
        return v;
      });

      setVehicles([...newVehicles]);
    },
    [vehicles]
  );

  const onNewPathMissionCreated = React.useCallback(
    (_event: IpcRendererEvent, data: string) => {
      // path mission model
      const pathMission = JSON.parse(data) as IwdPathMissionModel;

      // new vehicles array
      const newVehicles = vehicles.map((v) => {
        if (v.model?.id === pathMission.vehicleId) {
          // add mission id to associated vehicle
          const newVehicle = {
            ...v,
            model: {
              ...v.model,
              missionId: pathMission.id,
            },
          };

          return newVehicle;
        }

        return v;
      });

      setVehicles([...newVehicles]);
    },
    [vehicles]
  );

  /**
   * Listeners
   */
  React.useEffect(() => {
    window.ipcRenderer.on(iwdResponse.vehicle.create, handleCreateVehicle);
    window.ipcRenderer.on(iwdResponse.vehicle.rebase, onRebase);
    window.ipcRenderer.on(iwdResponse.vehicle.update, onUpdateVehicles);
    window.ipcRenderer.on(
      iwdResponse.mission.newPathMission,
      onNewPathMissionCreated
    );

    return () => {
      window.ipcRenderer.removeAllListeners(iwdResponse.vehicle.create);
      window.ipcRenderer.removeAllListeners(iwdResponse.vehicle.rebase);
      window.ipcRenderer.removeListener(
        iwdResponse.vehicle.update,
        onUpdateVehicles
      );
      window.ipcRenderer.removeAllListeners(iwdResponse.mission.newPathMission);
    };
  }, [vehicles]);

  /**
   * Trigger rebase mode
   */
  React.useEffect(() => {
    if (
      currentBuildMode !== BuildMode.REBASE_VEHICLE &&
      rebase &&
      rebase.rebaseVehicle
    ) {
      dispatch(changeBuildMode(BuildMode.REBASE_VEHICLE));
    }
  }, [rebase]);

  /**
   * Trigger when current build mode changed
   */
  React.useEffect(() => {
    if (currentBuildMode === BuildMode.NONE) {
      // Reset
      setRebase({ rebaseVehicle: undefined, newPosition: undefined });
    }
  }, [currentBuildMode]);

  /**
   * Trigger on click save button from SaveModal.tsx
   */
  React.useEffect(() => {
    if (saveMode === BuildMode.REBASE_VEHICLE) {
      // Send request to change base (finalised)
      window.ipcRenderer.send(
        iwdRequest.vehicle.rebaseFinalised,
        rebase?.rebaseVehicle?.model?.id,
        JSON.stringify(rebase?.newPosition)
      );

      // Reset save mode
      dispatch(setSaveMode(BuildMode.NONE));
    }
  }, [saveMode]);

  return (
    <div className="vehicle-builder">
      <VehicleLayer vehicles={vehicles} />
      <RebaseLayer rebaseLocation={rebase?.newPosition} />
    </div>
  );
}

export default React.memo(VehicleBuilder);
