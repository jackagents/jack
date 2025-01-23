import { IpcRendererEvent } from 'electron';
import { LatLng } from 'leaflet';
import React from 'react';
import { useDispatch, useSelector } from 'react-redux';
import { v4 } from 'uuid';
import * as Topography from 'addons/.leaflet-topography';
import { DEFAULT_POINT_OF_INTEREST as DEFAULT_POINT_OF_INTEREST_TYPE } from 'constant/common/cmConstants';
import { PointOfInterestType } from 'types/iwd/iwdTypes';
import { response } from 'projectEvents/common/cmEvents';
import { iwdActions } from 'projectRedux/reducers/iwd/iwdClientReducer';
import { RootState } from 'projectRedux/Store';
import PoI from './children/PointOfInterest';

function PointOfInterestBuilder() {
  const [markers, setMarkers] = React.useState<PointOfInterestType[]>([]);
  const { pointsOfInterest } = useSelector((state: RootState) => state.iwd);

  const dispatch = useDispatch();

  const handleCreate = React.useCallback(
    async (_event: IpcRendererEvent, lat: number, lng: number) => {
      // Turn on loading
      dispatch(iwdActions.toggleLoading(true));

      // Get Topography through mapbox api and module calculation
      const results = await Topography.getTopography(new LatLng(lat, lng));

      if (results.latlng.lat !== lat || results.latlng.lng !== lng) {
        return;
      }

      // Create new point of interest object
      const poi: PointOfInterestType = {
        id: v4(),
        type: DEFAULT_POINT_OF_INTEREST_TYPE,
        position: new LatLng(lat, lng, results.elevation),
        dateCreated: Date.parse(new Date().toString()).toString(),
      };

      // Update markers for rendering
      setMarkers([...markers, poi]);

      // Add new marker
      dispatch(iwdActions.addNewPointOfInterest(JSON.stringify(poi)));

      // Turn off loading
      dispatch(iwdActions.toggleLoading(false));
    },
    []
  );

  React.useEffect(() => {
    window.ipcRenderer.on(response.project.createPointOfInterest, handleCreate);

    return () => {
      window.ipcRenderer.removeAllListeners(
        response.project.createPointOfInterest
      );
    };
  }, [markers]);

  React.useEffect(() => {
    // On add / remove point of interest
    if (pointsOfInterest.length !== markers.length) {
      setMarkers([...pointsOfInterest]);
      return;
    }

    // On update point of interest
    markers.forEach((m) => {
      pointsOfInterest.forEach((p) => {
        if (m.id === p.id && (m.type !== p.type || m.position !== p.position)) {
          setMarkers([...pointsOfInterest]);
        }
      });
    });
  }, [pointsOfInterest]);

  return (
    <div className="point-of-interest-builder">
      {markers.map((marker: PointOfInterestType) => {
        return <PoI key={marker.id} poi={marker} />;
      })}
    </div>
  );
}

export default React.memo(PointOfInterestBuilder);
