import './iwdMap.css';
import { styled } from '@mui/material/styles';
import {
  MapContainer,
  TileLayer,
  LayersControl,
  LayerGroup,
  useMapEvents,
} from 'react-leaflet';
import { LatLng, LeafletMouseEvent, Map } from 'leaflet';
import React from 'react';
import { useDispatch, useSelector } from 'react-redux';
import { Col, Fluid, Row } from 'components/common/base/BaseContainer';
import {
  DEFAULT_MAP_CENTER,
  MODE,
  Z_INDEX,
} from 'misc/constant/common/cmConstants';
import { RootState } from 'projectRedux/Store';
import { BuildMode, SelectableElement } from 'types/iwd/iwdTypes';
import { iwdActions } from 'projectRedux/reducers/iwd/iwdClientReducer';
import CreatorMenu from 'components/iwd/creatorMenu/CreatorMenu';
import SearchMenu from 'components/iwd/searchMenu/SearchMenu';
import { BaseBackdrop } from 'components/common/baseBackdrop/BaseBackdrop';
import ContextMenuLayer from 'components/iwd/contextMenu/ContextmenuLayer';
import ConfigEntityMenu from 'components/iwd/configEntity/ConfigEntityMenu';
import SaveModal from 'components/iwd/modal/SaveModal';
import ZoneBuilder from 'components/iwd/builders/ZoneBuilder';
import PolylineMeasure from 'components/common/polylineMeasure/PolylineMeasure';
import Crosshair from 'components/common/crosshair/Crosshair';
import CoordinateInfo from 'components/common/coordinateInfo/CoordinateInfo';
import GeoAgentBuilder from 'components/iwd/builders/GeoAgentBuilder';
import PointOfInterestBuilder from 'components/iwd/builders/PointOfInterestBuilder';
import SelectedEffectBuilder from 'components/iwd/builders/SelectedEffectBuilder';
import VehicleBuilder from 'components/iwd/builders/VehicleBuilder';
import { request } from 'projectEvents/common/cmEvents';
import {
  DEFAULT_MAP_COORDINATE,
  DEFAULT_ZOOM,
} from 'constant/common/cmElectronStoreKey';
import PMTilesLayer from 'components/common/pmtilesLayer/PMTilesLayer';
import PathMissionBuilder from '../builders/PathMissionBuilder';

// UI style
const Root = styled(Fluid)({
  overflow: 'hidden',
});

const MapRow = styled(Row)({
  top: 0,
  bottom: 0,
});

const CreatorMenuCol = styled(Col)({
  width: '17vw',
  // minWidth: '250px',
  zIndex: Z_INDEX.MENU,
  left: 0,
});

const ConfigEntityCol = styled(Col)({
  width: '20vw',
  right: 0,
  top: 0,
});

const SearchMenuCol = styled(Col)({
  left: 0,
  width: '17vw',
  zIndex: Z_INDEX.MENU,
  // minWidth: '250px',
});

const ContextMenuLayerDiv = styled(Row)({
  top: 0,
  bottom: 0,
});

// function resize the leaflet map when change client mode
function ResizeMapComponent() {
  const { explorer, currentSelectElement, currentBuildMode } = useSelector(
    (state: RootState) => state.iwd
  );

  const { closeConfigDisplay } = iwdActions;

  const dispatch = useDispatch();

  const map = useMapEvents({
    click: (event: LeafletMouseEvent) => {
      // Close config entity on click on map
      event.originalEvent.preventDefault();

      // Should only close when build mode is none
      if (
        currentBuildMode === BuildMode.NONE &&
        currentSelectElement &&
        currentSelectElement.type !== SelectableElement.NONE
      ) {
        dispatch(closeConfigDisplay());
      }
    },
  });

  React.useEffect(() => {
    map.invalidateSize();
  }, [explorer.mode]);

  return null;
}

interface Props {
  useLocalMap?: boolean;
}

// UI component CustomizeMap
function GeoMap({ useLocalMap = false }: Props) {
  // Map reference
  const map = React.useRef<Map>(null);
  const rootRef = React.useRef<HTMLDivElement>(null);

  const { currentBuildMode, menu, currentSelectElement } = useSelector(
    (state: RootState) => {
      return state.iwd;
    }
  );

  const isLoading = useSelector(
    (state: RootState) => state.iwd.prompt.isLoading
  );

  const mode = useSelector((state: RootState) => state.iwd.explorer.mode);

  const [satellite, setSatellite] = React.useState('');
  const [street, setStreet] = React.useState('');
  const satelliteLayer: any = React.useRef(null);
  const streetViewLayer: any = React.useRef(null);
  const defaultMapPos: [number, number] = React.useMemo(() => {
    return window.electronStore.get(DEFAULT_MAP_COORDINATE, DEFAULT_MAP_CENTER);
  }, []);
  const defaultZoom: number = React.useMemo(() => {
    return window.electronStore.get(DEFAULT_ZOOM, 22);
  }, []);

  const saveInfoBeforeClose = React.useCallback(() => {
    if (!map.current) return;

    const { lat, lng } = map.current.getCenter();
    const zoom = map.current.getZoom();

    // Save current map coordinate
    window.electronStore.set(DEFAULT_MAP_COORDINATE, [lat, lng]);

    // Save current map zoom level
    window.electronStore.set(DEFAULT_ZOOM, zoom);
  }, [map]);

  React.useEffect(() => {
    setSatellite(
      window.electronStore.get(
        'satelliteMapAPI',
        'https://api.mapbox.com/styles/v1/danielchen93/ckxvbly5v2ml216msrvh19pu3/tiles/256/{z}/{x}/{y}@2x?access_token=pk.eyJ1IjoiZGFuaWVsY2hlbjkzIiwiYSI6ImNreHZhcjRrZDZnMGMyb29jd3NxcnpwdXAifQ.HKXvH8zIiY73mKMmh04odg'
      )
    );

    setStreet(
      window.electronStore.get(
        'streetViewMapAPI',
        'https://api.mapbox.com/styles/v1/danielchen93/ckxvbchyclj7r15t9egr2vplm/tiles/256/{z}/{x}/{y}@2x?access_token=pk.eyJ1IjoiZGFuaWVsY2hlbjkzIiwiYSI6ImNreHZhcjRrZDZnMGMyb29jd3NxcnpwdXAifQ.HKXvH8zIiY73mKMmh04odg'
      )
    );

    const resizeObserver = new ResizeObserver(() => {
      map.current?.invalidateSize();
    });

    if (rootRef.current) {
      resizeObserver.observe(rootRef.current);
    }

    return () => {
      if (rootRef.current) {
        resizeObserver.unobserve(rootRef.current);
      }
    };
  }, []);

  React.useEffect(() => {
    if (satelliteLayer.current) {
      satelliteLayer.current.setUrl(satellite);
    }
  }, [satellite]);

  React.useEffect(() => {
    if (streetViewLayer.current) {
      streetViewLayer.current.setUrl(street);
    }
  }, [street]);

  React.useEffect(() => {
    window.ipcRenderer.on(request.project.saveInfo, saveInfoBeforeClose);

    return () => {
      window.ipcRenderer.removeListener(
        request.project.saveInfo,
        saveInfoBeforeClose
      );
    };
  }, [saveInfoBeforeClose]);

  React.useEffect(() => {
    // This is for offseting leaflet top left control
    if (!map.current) return;

    const topLeftControlContainer = map.current
      .getContainer()
      .getElementsByClassName('leaflet-control-container')[0]
      .getElementsByClassName('leaflet-top leaflet-left')[0] as HTMLElement;

    if (topLeftControlContainer && mode !== MODE.MODE_NONE) {
      topLeftControlContainer.style.left = '17vw';
    } else {
      topLeftControlContainer.style.left = '0';
    }
  }, [menu]);

  return (
    <Root ref={rootRef}>
      <MapRow>
        <MapContainer
          ref={map}
          center={new LatLng(defaultMapPos[0], defaultMapPos[1])}
          zoom={defaultZoom}
          minZoom={4}
          maxZoom={25}
          worldCopyJump
        >
          {/* <ZoomControl position="topright" /> */}
          <ResizeMapComponent />
          <LayersControl sortLayers position="topright">
            {/* Local map needs tiles-server */}
            {useLocalMap && (
              <LayersControl.BaseLayer name="Offline map">
                <LayerGroup>
                  <PMTilesLayer />
                </LayerGroup>
              </LayersControl.BaseLayer>
            )}

            <LayersControl.BaseLayer checked name="Satellite Street">
              <TileLayer
                maxNativeZoom={22}
                ref={satelliteLayer}
                maxZoom={25}
                url={satellite}
              />
            </LayersControl.BaseLayer>
            <LayersControl.BaseLayer name="Street">
              <TileLayer
                maxNativeZoom={22}
                ref={streetViewLayer}
                maxZoom={25}
                url={street}
              />
            </LayersControl.BaseLayer>

            {/* Zone builder and display layer */}
            <LayersControl.Overlay checked name="Zone">
              <LayerGroup>
                <ZoneBuilder />
              </LayerGroup>
            </LayersControl.Overlay>

            {/* Geo agent builder and display layer */}
            <LayersControl.Overlay checked name="Agents">
              <LayerGroup>
                <GeoAgentBuilder />
              </LayerGroup>
            </LayersControl.Overlay>

            {/* PoI builder and display layer */}
            <LayersControl.Overlay checked name="Point Of Interest">
              <LayerGroup>
                <PointOfInterestBuilder />
              </LayerGroup>
            </LayersControl.Overlay>

            {/* Vehicle builder and display layer */}
            <LayersControl.Overlay checked name="Vehicle">
              <LayerGroup>
                <VehicleBuilder />
              </LayerGroup>
            </LayersControl.Overlay>

            {/* Path mission builder and display layer */}
            {/* <LayersControl.Overlay checked name="PathMission"> */}
            <LayerGroup>
              <PathMissionBuilder />
            </LayerGroup>
            {/* </LayersControl.Overlay> */}

            {/* Selected effect */}
            <SelectedEffectBuilder />

            {/* Coordinate info */}
            <LayersControl.Overlay checked name="Coordinate Info">
              <LayerGroup>
                <CoordinateInfo
                  sx={{
                    left:
                      menu.creatorMenu || menu.searchMenu ? '17.5vw' : '.5vw',
                  }}
                />
              </LayerGroup>
            </LayersControl.Overlay>

            {/* Crosshair in center of map */}
            <LayerGroup>
              <Crosshair />
            </LayerGroup>

            {/* Distance tool */}
            <LayerGroup>
              <PolylineMeasure />
            </LayerGroup>
          </LayersControl>

          {/* Save modal to confirm create zone/ rebase vehicle */}
          {(currentBuildMode === BuildMode.ZONE ||
            currentBuildMode === BuildMode.REBASE_VEHICLE ||
            currentBuildMode === BuildMode.PATH_MISSION ||
            currentBuildMode === BuildMode.SCOUT_MISSION) && <SaveModal />}

          {/* Config entity menu - on the right hand */}
          {currentSelectElement &&
            currentSelectElement.type !== SelectableElement.NONE && (
              <ConfigEntityCol>
                <ConfigEntityMenu />
              </ConfigEntityCol>
            )}

          {/* Control mouse click on map and contextmenu on map */}
          <ContextMenuLayerDiv className="mouse-control">
            <ContextMenuLayer />
          </ContextMenuLayerDiv>
        </MapContainer>
      </MapRow>

      {isLoading && <BaseBackdrop loading={isLoading} />}

      {/* Creator menu for zone - First button on the left hand */}
      {menu.creatorMenu && (
        <CreatorMenuCol>
          <CreatorMenu />
        </CreatorMenuCol>
      )}

      {/* Search menu and list of agents - Second menu on left hand */}
      {menu.searchMenu && (
        <SearchMenuCol>
          <SearchMenu />
        </SearchMenuCol>
      )}
    </Root>
  );
}

export default React.memo(GeoMap);
