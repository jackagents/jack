import React from 'react';
import { Circle, Marker, Polygon, Polyline, useMapEvents } from 'react-leaflet';
import { useDispatch, useSelector } from 'react-redux';
import { v4 } from 'uuid';
import { ZONE_COLOR_TYPE, ZONE_TYPE } from 'constant/common/cmConstants';
import { BuildMode, SavedData, SavedZone, Zone } from 'types/iwd/iwdTypes';
import { response } from 'projectEvents/common/cmEvents';
import { iwdActions } from 'projectRedux/reducers/iwd/iwdClientReducer';
import { RootState } from 'projectRedux/Store';
import GeoZone from './children/GeoZone';

let polygon: [number, number][] = [];
let multiPolygon: [number, number][][] = [];
let editPolygonIndex = -1;

const blueOptions = { color: 'blue' };
const redOptions = { fillColor: 'red' };

function ZoneBuilder() {
  // drawble: control add marker and polygon
  const [drawable, setDrawable] = React.useState<boolean>(false);
  // area: temporary polygon to render when build zone
  const [area, setArea] = React.useState<[number, number][][]>([]);
  // savedArea: saved zone to render after finalised
  const [savedArea, setSavedArea] = React.useState<SavedZone[]>([]);
  // markers: represent clicked point to create polygon
  const [markers, setMarkers] = React.useState<[number, number][]>([]);
  // circleRef: reference to the Circle component at the starting marker
  const circleRef = React.useRef<any>();
  // dispatch redux
  const dispatch = useDispatch();

  // use selector to get redux state
  const {
    currentBuildMode,
    savedData,
    saveMode,
    undoZoneBuilder,
    dragMode,
    currentZoneType,
  } = useSelector((state: RootState) => state.iwd);

  // zone
  const zone = useSelector((state: RootState) => state.iwd.savedData.zone);

  /**
   * Reset zone
   */
  const resetZone = () => {
    setDrawable(false);
    setMarkers([]);
    setArea([]);
    polygon = [];
    multiPolygon = [];
    editPolygonIndex = -1;
    // Turn off drag mode in zone builder
    dispatch(iwdActions.toggleDragMode(false));
    dispatch(iwdActions.setZoneType(ZONE_TYPE.NONE));
  };

  /**
   * Remove dragable marker in dragmode
   */
  const removeDragableMarker = () => {
    const newMarkers = [...markers];
    newMarkers.pop();
    setMarkers(newMarkers);
  };

  React.useEffect(() => {
    if (drawable) map.getContainer().style.cursor = 'pointer';
    else map.getContainer().style.cursor = '';
  }, [drawable]);

  // Old tewa code
  // React.useEffect(() => {
  //   if (currentBuildMode === BUILD_MODE.LOADED) {
  //     // On Load
  //     setSavedArea(zone);
  //   }
  // }, [savedData]);

  React.useEffect(() => {
    // Old tewa code
    // On loaded
    // if (
    //   currentBuildMode === BUILD_MODE.LOADED ||
    //   currentBuildMode === BUILD_MODE.CLOSE
    // ) {
    //   dispatch(actions.changeBuildMode(BUILD_MODE.NONE));
    // }

    // After saved
    if (saveMode !== BuildMode.NONE) {
      dispatch(iwdActions.setSaveMode(BuildMode.NONE));
    }
  }, [savedArea]);

  /**
   * Undo last marker action
   */
  const undo = React.useCallback(() => {
    if (drawable && polygon.length > 0) {
      if (polygon.length === 1 && multiPolygon.length > 0) {
        // CASE: JUST CREATED A POLYGON
        // const a = multiPolygon.splice(multiPolygon.length - 1, 1);
        const temp = multiPolygon.pop() as [number, number][];
        polygon = [...temp];
        setArea([...multiPolygon]);

        if (dragMode) {
          const pointer = markers.pop() as [number, number];
          setMarkers([...polygon, pointer]);
        } else {
          setMarkers([...polygon]);
        }
      } else {
        // CASE: JUST ADDED A MARKER

        polygon.pop();
        const newMarkers = [...markers];

        if (dragMode) {
          const pointer = newMarkers.pop() as [number, number];
          newMarkers.pop();
          setMarkers([...newMarkers, pointer]);
        } else {
          newMarkers.pop();
          setMarkers([...newMarkers]);
        }
      }
    }
  }, [dragMode, drawable, markers]);

  /**
   * Callback on create zone
   */
  const onCreateZone = React.useCallback(
    (_event: Electron.IpcRendererEvent, newZone: string) => {
      const z = JSON.parse(newZone) as Zone;
      if (z) {
        setDrawable(true);
        dispatch(iwdActions.setZoneType(z.type));
        dispatch(iwdActions.changeBuildMode(BuildMode.ZONE));
      }
    },
    []
  );

  /**
   * Callback on trigger edit mode
   */
  const triggerEditSavedArea = React.useCallback(
    (z: SavedZone) => {
      if (savedArea.length > 0) {
        // Trigger Editable
        dispatch(iwdActions.changeBuildMode(BuildMode.ZONE));
        dispatch(iwdActions.setZoneType(z.type));
        dispatch(iwdActions.toggleUndoZoneBuilder(true));

        // Get the polygon on top that contains point and make it editable
        setArea([...z.bounds]);
        setMarkers([z.bounds[z.bounds.length - 1][0]]);
        polygon = [...[z.bounds[z.bounds.length - 1][0]]];
        multiPolygon = [...z.bounds];

        const i = savedArea.findIndex((x) => x.UUID === z.UUID);

        // Remove the polygon from saved area
        const newSaved = [...savedArea];
        newSaved.splice(i, 1);
        editPolygonIndex = i;
        setSavedArea([...newSaved]);
      }
    },
    [savedArea]
  );

  /**
   * Callback on undo zone
   */
  const onUndoZone = React.useCallback(
    (_event: Electron.IpcRendererEvent, z: string) => {
      if (
        currentBuildMode !== BuildMode.ZONE &&
        currentBuildMode !== BuildMode.NONE
      ) {
        return;
      }

      // If selected a zone and not drawing
      if (!drawable) triggerEditSavedArea(JSON.parse(z) as SavedZone);
      // If is drawing
      else undo();
    },
    [currentBuildMode, drawable, triggerEditSavedArea, undo]
  );

  // Initialization
  React.useEffect(() => {
    window.ipcRenderer.on(response.project.createZone, onCreateZone);
    window.ipcRenderer.on(response.project.undoZone, onUndoZone);

    return () => {
      window.ipcRenderer.removeAllListeners(response.project.createZone);
      window.ipcRenderer.removeAllListeners(response.project.undoZone);
    };
  }, [onCreateZone, onUndoZone]);

  React.useEffect(() => {
    // On chose zone type
    if (currentBuildMode === BuildMode.ZONE) {
      setDrawable(true);
    }
    // Old tewa code
    // // On Load
    // else if (currentBuildMode === BuildMode.LOAD) {
    //   resetZone();
    //   dispatch(actions.updateLoadingProgress());
    // }
    // On close
    else if (currentBuildMode === BuildMode.CLOSE) {
      resetZone();
      setSavedArea([]);
    }
    // On Cancel
    else if (saveMode === BuildMode.NONE) {
      if (!undoZoneBuilder && area.length === 0) resetZone();
      else {
        // Case 1: Created a polygon but not saved in store yet
        // should reset zone, do nothing else
        if (zone.length <= 0) {
          console.log('case 1');
        }
        // Case 2: Created a polygon, saved, reedit and cancel
        // should return the previous polygon
        else {
          console.log('case 2');
          cancel();
        }

        resetZone();
        dispatch(iwdActions.toggleDragMode(false));
      }
    }
    // On Save
    else if (saveMode === BuildMode.ZONE) {
      if (dragMode) removeDragableMarker();

      // CASE: Save if new multipolygon created
      // CASE: Edited polygon => save
      if (
        (multiPolygon.length > 0 && !undoZoneBuilder) ||
        (undoZoneBuilder && polygon.length === 1 && multiPolygon.length > 0)
      ) {
        const newSavedArea: SavedZone = {
          UUID: v4(),
          bounds: [...area],
          type: currentZoneType,
          color: ZONE_COLOR_TYPE[currentZoneType],
          lastChanged: window.mainUtils.getCurrentDateStr(),
        };

        const newSavedData: SavedData = {
          ...savedData,
          zone: [...savedArea, newSavedArea],
        };

        dispatch(iwdActions.updateSavedData(newSavedData));

        // Saved area to map
        setSavedArea([...savedArea, newSavedArea]);
      }

      // CASE: No new multipolygon was created => do nothing in store remove jsx
      // CASE: Undoing and delete all editing polygon or undoing => update store
      else if (undoZoneBuilder) {
        cancel();
      }

      resetZone();
    }
  }, [saveMode, currentBuildMode]);

  React.useEffect(() => {
    // On undo zone builder
    if (undoZoneBuilder) {
      undo();
      // props.toggleUndoZoneBuilder(false);
    }
  }, [undoZoneBuilder]);

  React.useEffect(() => {
    // On click drag mode
    if (!dragMode) removeDragableMarker();
  }, [dragMode]);

  /**
   * Generate new radius for Circle component according to zoom
   * @returns radius
   */
  const genNewRad = () => {
    const zoom = map.getZoom();
    const { lat } = map.getCenter();
    const metersPerPixel =
      (156543.03392 * Math.cos((lat * Math.PI) / 180)) / 2 ** zoom;
    const desiredRadiusInPixels = 10;
    const radius = metersPerPixel * desiredRadiusInPixels;
    return radius;
  };

  /**
   * Leaflet map
   */
  const map = useMapEvents({
    click(e) {
      // Left click
      if (drawable) {
        // Create polygon after click on the starting position
        if (
          circleRef.current &&
          circleRef.current.getBounds().contains(e.latlng) &&
          polygon.length > 0
        ) {
          // Return if only polygon has only 2 different points
          if (polygon.length === 2) return;

          // Finish the 1st zone
          multiPolygon.push(polygon);

          // Add the 1st zone into area to render
          setArea([...multiPolygon]);

          // Reset polygon and keep the first marker as start point
          polygon = [Object.values(markers[0]) as [number, number]];

          // Reset markers to only keep the origin
          setMarkers([markers[0]]);

          dispatch(iwdActions.toggleUndoZoneBuilder(false));
          return;
        }

        // Add new point to polygon array
        if (polygon) {
          const newPoint: [number, number] = Object.values(e.latlng) as [
            number,
            number
          ];
          polygon.push(newPoint);
          setMarkers([...markers, newPoint]);
        }
      }
    },

    mousemove(e) {
      // Mouse move
      // Dragmode
      if (drawable && dragMode) {
        const newMarkers = [...markers];
        if (newMarkers.length > polygon.length) {
          newMarkers[markers.length - 1] = Object.values(e.latlng) as [
            number,
            number
          ];
        } else {
          newMarkers.push(Object.values(e.latlng) as [number, number]);
        }

        setMarkers([...newMarkers]);
      }
    },

    zoomend() {
      // Recalculate circle
      const radius = genNewRad();

      if (circleRef.current) {
        circleRef.current.setRadius(radius);
      }
    },
  });

  /**
   * Draw circle at points of polygon
   * @returns React.Component
   */
  const DrawCircle = () => {
    if (markers.length > 0) {
      return (
        <Circle
          ref={circleRef}
          center={markers[0]}
          pathOptions={redOptions}
          radius={genNewRad()}
        />
      );
    }
    return null;
  };

  /**
   * Draw marker at the last added point in polygon
   * @returns React.Component[]
   */
  const Markers = () => {
    return markers.map((position, index) => {
      const id = `Marker_${index}`;
      return <Marker key={id} position={position} />;
    });
  };

  /**
   * Render polygons
   * @returns React.Component[]
   */
  const SavedPolygon = () => {
    return savedArea.map((z) => {
      return z && <GeoZone key={z.UUID} zone={z} />;
    });
  };

  /**
   * On cancel editing polygon
   */
  const cancel = () => {
    const saved: SavedZone[] = [...savedArea];

    if (polygon.length > 0 || saveMode === BuildMode.NONE) {
      // CASE: Undoing but not created a multipolygon => return previous polygon to store
      const z = savedData.zone[editPolygonIndex];
      saved.splice(editPolygonIndex, 0, z);
    }
    setSavedArea([...saved]);
    dispatch(iwdActions.toggleUndoZoneBuilder(false));
  };

  return (
    <div className="zone-builder">
      {Markers()}
      {DrawCircle()}
      <Polyline pathOptions={blueOptions} positions={markers} />
      {/* current drawing polygon */}
      <Polygon pathOptions={blueOptions} positions={area} />
      {SavedPolygon()}
    </div>
  );
}

export default React.memo(ZoneBuilder);
