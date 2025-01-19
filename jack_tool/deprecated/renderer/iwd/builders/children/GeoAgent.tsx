import { LatLng, LeafletMouseEvent } from 'leaflet';
import React from 'react';
import { Marker } from 'react-leaflet';
import { useDispatch } from 'react-redux';
import { activeAgentIcon } from 'misc/icons/common/cmIcons';
import {
  Agent,
  BeliefMap,
  CurrentSelectElement,
  SelectableElement,
} from 'types/iwd/iwdTypes';
import { request } from 'projectEvents/common/cmEvents';
import { iwdActions } from 'projectRedux/reducers/iwd/iwdClientReducer';

interface Props {
  positionBeliefs: BeliefMap;
  agent: Agent;
}

// Hardcoded offset
const offSet = {
  lat: -30.4,
  lng: 135.4,
};

function GeoAgent(props: Props) {
  const [position, setPosition] = React.useState<LatLng>();
  const dispatch = useDispatch();

  React.useEffect(() => {
    const lat = props.positionBeliefs.get('positiony') / 100 + offSet.lat;
    const lng = props.positionBeliefs.get('positionx') / 100 + offSet.lng;

    setPosition(new LatLng(lat, lng));
  }, [props.positionBeliefs]);

  /**
   * Callback on click agent
   */
  const handleClick = React.useCallback(() => {
    const payload: CurrentSelectElement = {
      type: SelectableElement.AGENT,
      value: { id: props.agent.address.id },
    };

    dispatch(iwdActions.changeCurrentSelectElement(JSON.stringify(payload)));
    // dispatch(actions.changeCurrentAgent({ id: props.agent.address.id }));
  }, []);

  /**
   * Callback on right click agent
   */
  const handleContextmenu = React.useCallback((event: LeafletMouseEvent) => {
    event.originalEvent.preventDefault();
    window.ipcRenderer.invoke(
      request.project.contextmenu,
      SelectableElement.AGENT,
      JSON.stringify({
        containerPoint: {
          x: event.containerPoint.x,
          y: event.containerPoint.y,
        },
        latlngPoint: event.latlng,
      }),
      JSON.stringify(props.agent)
    );
  }, []);

  return position ? (
    <>
      <Marker
        eventHandlers={{
          click: handleClick,
          contextmenu: handleContextmenu,
        }}
        icon={activeAgentIcon}
        position={position}
      />
    </>
  ) : null;
}

export default React.memo(GeoAgent);
