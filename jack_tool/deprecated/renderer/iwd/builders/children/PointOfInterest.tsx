import L, { LeafletMouseEvent } from 'leaflet';
import React from 'react';
import { Marker } from 'react-leaflet';
import { useDispatch } from 'react-redux';
import {
  PoIType,
  SelectableElement,
  PointOfInterestType,
} from 'types/iwd/iwdTypes';
import { request } from 'projectEvents/common/cmEvents';
import { iwdActions } from 'projectRedux/reducers/iwd/iwdClientReducer';

interface Props {
  poi: PointOfInterestType;
}

function PoI(props: Props) {
  const dispatch = useDispatch();

  /**
   * Callback on click point of interest
   */
  const handleClick = React.useCallback((poi: PointOfInterestType) => {
    // Change current select element
    const payload = {
      type: SelectableElement.POI,
      value: poi,
    };

    dispatch(iwdActions.changeCurrentSelectElement(JSON.stringify(payload)));
  }, []);

  /**
   * Callback on right click point of interest
   */
  const handleContextmenu = React.useCallback((event: LeafletMouseEvent) => {
    event.originalEvent.preventDefault();

    window.ipcRenderer.invoke(
      request.project.contextmenu,
      SelectableElement.POI,
      JSON.stringify({
        containerPoint: {
          x: event.containerPoint.x,
          y: event.containerPoint.y,
        },
        latlngPoint: event.latlng,
      })
    );
  }, []);

  const divIcon = React.useMemo(() => {
    let color = 'white';

    if (props.poi.type === PoIType.UNKNOWN) {
      color = 'white';
    } else if (props.poi.type === PoIType.HOSTILE) {
      color = 'red';
    } else {
      color = '#4cbb17';
    }

    return L.divIcon({
      html: `
        <svg viewBox="0 0 500 820" version="1.1"
            xmlns="http://www.w3.org/2000/svg" xml:space="preserve" style="fill-rule: evenodd; clip-rule: evenodd; stroke-linecap: round;">
            <g transform="matrix(19.5417,0,0,19.5417,-7889.1,-9807.44)">
                <path fill=${color} d="M416.544,503.612C409.971,503.612 404.5,509.303 404.5,515.478C404.5,518.256 406.064,521.786 407.194,524.224L416.5,542.096L425.762,524.224C426.892,521.786 428.5,518.433 428.5,515.478C428.5,509.303 423.117,503.612 416.544,503.612ZM416.544,510.767C419.128,510.784 421.223,512.889 421.223,515.477C421.223,518.065 419.128,520.14 416.544,520.156C413.96,520.139 411.865,518.066 411.865,515.477C411.865,512.889 413.96,510.784 416.544,510.767Z" stroke-width="1.1px" stroke="black"/>
            </g>
        </svg>
        `,
      className: 'poi-div-icon',
      iconSize: [24, 40],
      iconAnchor: [12, 40],
    });
  }, [props.poi.type]);

  return (
    <>
      <Marker
        eventHandlers={{
          click: React.useCallback(() => {
            handleClick(props.poi);
          }, [props.poi]),
          contextmenu: handleContextmenu,
        }}
        position={props.poi.position}
        icon={divIcon}
      />
    </>
  );
}

export default React.memo(PoI);
