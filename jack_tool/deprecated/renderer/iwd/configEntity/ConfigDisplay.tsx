import { styled } from '@mui/material';
import React from 'react';
import { useSelector } from 'react-redux';
import {
  CurrentVehicle,
  SelectableElement,
  CurrentAgent,
  PointOfInterestType,
  SavedZone,
} from 'types/iwd/iwdTypes';
import { RootState } from 'projectRedux/Store';
import ConfigEntityContent from './Agent/ConfigEntityContent';
import ConfigPoIContent from './PoI/ConfigPoIContent';
import ConfigZone from './Zone/ConfigZone';
import ConfigVehicleContent from './Vehicle/ConfigVehicleContent';

const ConfigDisplayDiv = styled(`div`)({});

function ConfigDisplay() {
  const currentSelectElement = useSelector(
    (state: RootState) => state.iwd.currentSelectElement
  );

  const getContent = () => {
    if (!currentSelectElement) return null;

    if (currentSelectElement.type === SelectableElement.AGENT) {
      return (
        <ConfigEntityContent
          currentAgent={currentSelectElement.value as CurrentAgent}
        />
      );
    }

    if (currentSelectElement.type === SelectableElement.POI) {
      return (
        <ConfigPoIContent
          poi={currentSelectElement.value as PointOfInterestType}
        />
      );
    }

    if (currentSelectElement.type === SelectableElement.ZONE) {
      return (
        <ConfigZone currentZone={currentSelectElement.value as SavedZone} />
      );
    }

    if (currentSelectElement.type === SelectableElement.VEHICLE) {
      return (
        <ConfigVehicleContent
          vehicle={currentSelectElement.value as CurrentVehicle}
        />
      );
    }

    return null;
  };

  return <ConfigDisplayDiv>{getContent()}</ConfigDisplayDiv>;
}

export default React.memo(ConfigDisplay);
