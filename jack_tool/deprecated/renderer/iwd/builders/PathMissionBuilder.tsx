import { LatLng, LeafletMouseEvent } from 'leaflet';
import { waypointDivIcon } from 'misc/icons/common/cmIcons';
import { GeoPosition } from 'misc/types/iwd/iwdVehicleModel';
import { iwdRequest } from 'projectEvents/iwd/iwdEvents';
import {
  iwdActions,
  IwdClientState,
} from 'projectRedux/reducers/iwd/iwdClientReducer';
import { useSliceSelector } from 'projectRedux/sliceProvider/SliceProvider';
import React from 'react';
import { Marker, Polyline, useMapEvents } from 'react-leaflet';
import { useDispatch } from 'react-redux';
import { BuildMode, CurrentVehicle } from 'types/iwd/iwdTypes';

function PathMissionBuilder() {
  const { currentBuildMode, saveMode, currentSelectElement } =
    useSliceSelector() as IwdClientState;

  const { setSaveMode } = iwdActions;

  const dispatch = useDispatch();

  const drawable = React.useMemo(() => {
    if (currentBuildMode === BuildMode.PATH_MISSION) return true;
    return false;
  }, [currentBuildMode]);

  const [paths, setPaths] = React.useState<LatLng[]>([]);

  const uniqWaypoints = React.useMemo(() => {
    return [
      ...paths
        // get list of lat,lng/values
        .reduce(
          (map, { lat, lng }) =>
            map.set(`${lat}_${lng}`, [
              ...(map.get(`${lat}_${lng}`) || []),
              { lat, lng } as GeoPosition,
            ]),
          new Map()
        ),
    ];
  }, [paths]);

  const handleClickMap = React.useCallback(
    (e: LeafletMouseEvent) => {
      if (drawable) {
        // Add to path mission
        setPaths([...paths, e.latlng]);
      }
    },
    [drawable, paths]
  );

  const handleClickWaypoint = React.useCallback(
    (e: LeafletMouseEvent) => {
      if (drawable) {
        // add path with same lat lng
        setPaths([...paths, e.latlng]);
      }
    },
    [drawable, paths]
  );

  const map = useMapEvents({
    click: handleClickMap,
  });

  React.useEffect(() => {
    if (currentBuildMode === BuildMode.PATH_MISSION) {
      setPaths([]);
    }
  }, [currentBuildMode]);

  React.useEffect(() => {
    // When save path mission
    if (saveMode === BuildMode.PATH_MISSION) {
      const data: GeoPosition[] = paths.map((x) => {
        return { lat: x.lat, lng: x.lng };
      });

      // Send paths to backend to create mission
      window.ipcRenderer.send(
        iwdRequest.mission.newPathMission,
        (currentSelectElement?.value as CurrentVehicle).id,
        JSON.stringify(data)
      );

      // Remove path mission builder rendering
      setPaths([]);

      // Return to none build mode
      dispatch(setSaveMode(BuildMode.NONE));
    }
  }, [saveMode]);

  return (
    <div className="path-mission-builder">
      {/* Path mission line */}
      <Polyline positions={paths} />

      <>
        {/* Path mission waypoints */}
        {uniqWaypoints.map(([key, value]) => {
          const [latlng] = value;
          return (
            <Marker
              icon={waypointDivIcon}
              key={key}
              position={latlng}
              eventHandlers={{
                click: handleClickWaypoint,
              }}
            />
          );
        })}
      </>
    </div>
  );
}

export default React.memo(PathMissionBuilder);
