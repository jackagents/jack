import { DivIcon, divIcon, LatLng } from 'leaflet';
import React from 'react';
import { Marker } from 'react-leaflet';
import { useSelector } from 'react-redux';
import {
  SelectableElement,
  CurrentAgent,
  PointOfInterestType,
  CurrentVehicle,
} from 'types/iwd/iwdTypes';
import { getPositionBelief } from 'misc/utils/common/rendererUtils';
import { RootState } from 'projectRedux/Store';
import { IwdVehicleStatus } from 'types/iwd/iwdVehicleModel';

const cirIcon = divIcon({
  className: 'pulsating-circle',
  iconSize: [50, 50],
  iconAnchor: [25, 25],
});

const agentCirIcon = divIcon({
  className: 'agent-circle pulsating-circle',
  iconSize: [50, 50],
  iconAnchor: [25, 25],
});

interface CircleProp {
  select: boolean;
  position: LatLng | undefined;
  icon: DivIcon | undefined;
}

const initProps: CircleProp = {
  select: false,
  position: undefined,
  icon: undefined,
};

export default function SelectedCircle() {
  const [circleState, setCircleState] = React.useState<CircleProp>({
    ...initProps,
  });

  const { agents, currentSelectElement } = useSelector(
    (state: RootState) => state.iwd
  );

  React.useEffect(() => {
    if (
      currentSelectElement &&
      currentSelectElement.type === SelectableElement.AGENT
    ) {
      const curr = currentSelectElement.value as CurrentAgent;
      const agent = agents.find((x) => x.address.id === curr.id);

      setCircleState({
        select: true,
        position: getPositionBelief(agent?.beliefSets),
        icon: agentCirIcon,
      });
      return;
    }

    // Point of interest
    if (
      currentSelectElement &&
      currentSelectElement.type === SelectableElement.POI
    ) {
      const curr = currentSelectElement.value as PointOfInterestType;

      setCircleState({
        select: true,
        position: curr.position,
        icon: cirIcon,
      });
      return;
    }

    // Vehicle
    if (
      currentSelectElement &&
      currentSelectElement.type === SelectableElement.VEHICLE
    ) {
      const curr = currentSelectElement.value as CurrentVehicle;

      // Only show effect when idle
      if (curr.status === IwdVehicleStatus.idle) {
        setCircleState({
          select: true,
          position: new LatLng(curr.position.lat, curr.position.lng),
          icon: cirIcon,
        });

        return;
      }
    }

    // None
    setCircleState({
      select: false,
      position: undefined,
      icon: undefined,
    });
  }, [currentSelectElement]);

  return circleState.position ? (
    <Marker
      pane="shadowPane"
      position={circleState.position}
      icon={circleState.icon}
    />
  ) : null;
}
