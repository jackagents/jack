import { ZONE_TYPE } from 'constant/common/cmConstants';
import { LeafletMouseEvent } from 'leaflet';
import { request } from 'projectEvents/common/cmEvents';
import { iwdActions } from 'projectRedux/reducers/iwd/iwdClientReducer';
import React from 'react';
import { Polygon, Tooltip } from 'react-leaflet';
import { useDispatch } from 'react-redux';
import {
  CurrentSelectElement,
  SavedZone,
  SelectableElement,
} from 'types/iwd/iwdTypes';

interface Props {
  zone: SavedZone;
}

function GeoZone(props: Props) {
  const dispatch = useDispatch();

  /**
   * Callback on click
   */
  const handleClick = React.useCallback((z: SavedZone) => {
    const payload: CurrentSelectElement = {
      type: SelectableElement.ZONE,
      value: z,
    };

    dispatch(iwdActions.changeCurrentSelectElement(JSON.stringify(payload)));
  }, []);

  /**
   * Callback on right click
   */
  const handleContextmenu = React.useCallback(
    (event: LeafletMouseEvent, z: SavedZone) => {
      event.originalEvent.preventDefault();

      window.ipcRenderer.invoke(
        request.project.contextmenu,
        SelectableElement.ZONE,
        JSON.stringify({
          containerPoint: {
            x: event.containerPoint.x,
            y: event.containerPoint.y,
          },
          latlngPoint: event.latlng,
        }),
        JSON.stringify(z)
      );
    },
    []
  );

  return (
    <Polygon
      pathOptions={{ color: props.zone.color }}
      positions={props.zone.bounds}
      eventHandlers={{
        click: () => {
          handleClick(props.zone);
        },
        contextmenu: (e: LeafletMouseEvent) => {
          handleContextmenu(e, props.zone);
        },
      }}
    >
      <Tooltip>{Object.keys(ZONE_TYPE)[props.zone.type]}</Tooltip>
    </Polygon>
  );
}

export default React.memo(GeoZone);
