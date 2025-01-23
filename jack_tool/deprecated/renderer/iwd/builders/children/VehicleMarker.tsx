import { LatLng, LeafletMouseEvent } from 'leaflet';
import React from 'react';
import {
  CurrentSelectElement,
  CurrentVehicle,
  SelectableElement,
} from 'types/iwd/iwdTypes';
import {
  iwdActions,
  IwdClientState,
} from 'projectRedux/reducers/iwd/iwdClientReducer';
import { useDispatch } from 'react-redux';
import { IwdVehicleModel, IwdVehicleStatus } from 'types/iwd/iwdVehicleModel';
import DriftMarker from 'components/common/driftMarker/ReactDriftMarker';
import { planeIcon } from 'misc/icons/common/cmIcons';
import { Polyline } from 'react-leaflet';
import { iwdRequest, iwdResponse } from 'projectEvents/iwd/iwdEvents';
import { request } from 'projectEvents/common/cmEvents';
import { IpcRendererEvent } from 'electron';
import { IwdPathMissionModel } from 'types/iwd/iwdMissionModel';
import { useSliceSelector } from 'projectRedux/sliceProvider/SliceProvider';

type Props = {
  data: IwdVehicleModel;
};

interface VehicleMarkerProps {
  nextPos: LatLng;
  prevPos: LatLng;
}

function VehicleMarker({ data }: Props) {
  const { id, position, speed, status, missionId } = data;

  const [markerPos, setMarkerPos] = React.useState<VehicleMarkerProps>({
    nextPos: new LatLng(position.lat, position.lng),
    prevPos: new LatLng(position.lat, position.lng),
  });

  const [originPath, setOriginPath] = React.useState<LatLng>(markerPos.prevPos);

  const [historyPath, setHistoryPath] = React.useState<LatLng[]>([
    markerPos.prevPos,
  ]);

  const [path, setPath] = React.useState<LatLng[]>([]);

  const { currentSelectElement } = useSliceSelector() as IwdClientState;

  const dispatch = useDispatch();

  const time = React.useMemo(() => {
    // Calculate distance
    const d = markerPos.prevPos.distanceTo(markerPos.nextPos);

    // to Km
    const t = d / (speed / 1000000);
    return t;
  }, [speed, position, markerPos]);

  const reset = () => {
    setPath([markerPos.nextPos]);
    setHistoryPath([markerPos.nextPos]);
  };

  const handleClick = React.useCallback(() => {
    const payload: CurrentSelectElement = {
      type: SelectableElement.VEHICLE,
      value: { id, position, status } as CurrentVehicle,
    };

    dispatch(iwdActions.changeCurrentSelectElement(JSON.stringify(payload)));
  }, [data]);

  /**
   * Callback on right click vehicle
   */
  const handleContextmenu = React.useCallback((event: LeafletMouseEvent) => {
    event.originalEvent.preventDefault();

    window.ipcRenderer.invoke(
      request.project.contextmenu,

      SelectableElement.VEHICLE,

      JSON.stringify({
        containerPoint: {
          x: event.containerPoint.x,
          y: event.containerPoint.y,
        },
        latlngPoint: event.latlng,
      }),

      JSON.stringify(data)
    );
  }, []);

  /**
   * Callback to update origin
   */
  const updatePath = React.useCallback((latlng: LatLng) => {
    setOriginPath(latlng);
  }, []);

  /**
   * Callback to update position in backend
   */
  const updatePos = React.useCallback((latlng: LatLng) => {
    window.ipcRenderer.send(
      iwdRequest.vehicle.updatePosition,
      id,
      JSON.stringify(latlng)
    );
  }, []);

  /**
   * Callback on receiving mission
   */
  const onGetPathMissionResponsed = React.useCallback(
    (_event: IpcRendererEvent, strData: string) => {
      const mission = JSON.parse(strData) as IwdPathMissionModel;
      const { vehicleId, paths } = mission;

      // Make sure the mission belongs to this vehicle
      if (vehicleId !== id) return;

      // Start moving vehicle if global state !== pause
      if (status === IwdVehicleStatus.paused) return;

      // Update vehicle status in backend
      window.ipcRenderer.send(
        iwdRequest.vehicle.updateStatus,
        id,
        IwdVehicleStatus.moving
      );

      const latlngs = paths.map((x) => new LatLng(x.lat, x.lng));

      setPath([...latlngs]);
    },
    []
  );

  /**
   * Callback on move end
   */
  const handleMoveEnd = React.useCallback(() => {
    // set previous marker to current position
    setMarkerPos({
      ...markerPos,
      prevPos: markerPos.nextPos,
    });

    // add current position to history path
    setHistoryPath([...historyPath, markerPos.nextPos]);

    // remove path
    setPath([...path.slice(1)]);

    // if no more path
    if (path.length === 1) {
      // change status to idle in backend
      window.ipcRenderer.send(
        iwdRequest.vehicle.updateStatus,
        id,
        IwdVehicleStatus.idle
      );

      // reset path
      reset();
    }
  }, [markerPos, historyPath, path]);

  /**
   * Currently unused
   * Trigger when vehicle move.
   */
  // React.useEffect(() => {
  //   if (originPath.distanceTo(markerPos.nextPos) === 0) {
  //     setMarkerPos({
  //       ...markerPos,
  //       prevPos: originPath,
  //     });
  //   }
  // }, [originPath]);

  /**
   * Trigger when mission id changed
   */
  React.useEffect(() => {
    if (missionId) {
      // Get the mission paths
      window.ipcRenderer.send(iwdRequest.mission.getPathMission, missionId);
    }
  }, [missionId]);

  /**
   * Trigger when init position changed
   */
  React.useEffect(() => {
    const latlng = new LatLng(position.lat, position.lng);

    // Reset history path
    setHistoryPath([latlng]);

    // Reset prevpos
    setMarkerPos({ ...markerPos, prevPos: latlng });
  }, [position]);

  React.useEffect(() => {
    window.ipcRenderer.on(
      `${iwdResponse.mission.getPathMission}_${id}`,
      onGetPathMissionResponsed
    );

    return () => {
      window.ipcRenderer.removeAllListeners(
        `${iwdResponse.mission.getPathMission}_${id}`
      );
    };
  }, []);

  /**
   * Trigger when path is changed
   */
  React.useEffect(() => {
    if (path.length > 0) {
      const nextPath = path[0];

      if (nextPath) {
        // Update next position for drift marker
        setMarkerPos({
          ...markerPos,
          nextPos: nextPath,
        });
      }
    }
  }, [path]);

  /**
   * Trigger when status is changed
   */
  React.useEffect(() => {
    if (
      !currentSelectElement ||
      currentSelectElement.type !== SelectableElement.VEHICLE ||
      (currentSelectElement?.value as CurrentVehicle).id !== id
    ) {
      return;
    }

    // Update status of current select vehicle
    const payload: CurrentSelectElement = {
      type: SelectableElement.VEHICLE,
      value: { id, position, status } as CurrentVehicle,
    };

    dispatch(iwdActions.changeCurrentSelectElement(JSON.stringify(payload)));
  }, [status]);

  return (
    <>
      <DriftMarker
        key={id}
        basePosition={position}
        prevPosition={markerPos.prevPos}
        position={markerPos.nextPos}
        duration={time}
        icon={planeIcon}
        eventHandlers={{
          click: handleClick,

          moveend: handleMoveEnd,

          move: (e: any) => {
            updatePath(e.latlng);
            updatePos(e.latlng);
          },

          contextmenu: handleContextmenu,
        }}
      />

      {status !== IwdVehicleStatus.idle && (
        <>
          {/* Line to next target */}
          <Polyline positions={[originPath, ...path]} />

          {/* Dot line from prev position to current position */}
          <Polyline color="red" positions={[...historyPath, originPath]} />

          {/* Target path to final destination */}
          <Polyline
            color="lime"
            positions={[originPath, path[path.length - 1]]}
          />
        </>
      )}
    </>
  );
}

export default React.memo(VehicleMarker);
