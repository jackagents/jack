import { LatLng } from 'leaflet';
import { iwdActions } from 'projectRedux/reducers/iwd/iwdClientReducer';
import React from 'react';
import { useDispatch } from 'react-redux';
import { PointOfInterestType } from 'types/iwd/iwdTypes';
import { v4 } from 'uuid';
import ConfigGeoSpatialContent from 'components/common/configGeoSpatialContent/ConfigGeoSpatialContent';
import { useActiveElement } from 'hooks/common/useActiveElement';
import { CONSTANT_STRING } from 'constant/common/cmConstants';
import ConfigPoITypeContent from './ConfigPoITypeContent';

interface Props {
  poi: PointOfInterestType;
}

enum Focus {
  none = 'none',
  lat = 'lat',
  lng = 'lng',
}

function ConfigPoIContent({ poi }: Props) {
  const dispatch = useDispatch();
  const [focus, setFocus] = React.useState<Focus>(Focus.none);

  const focusedElement = useActiveElement();

  React.useEffect(() => {
    if (!focusedElement) {
      console.log('focusedElement is null');
      return;
    }

    const element = focusedElement as HTMLInputElement;
    //
    if (element.name === 'lat') {
      setFocus(Focus.lat);
    }
    //
    else if (element.name === 'lng') {
      setFocus(Focus.lng);
    }
    //
    else if (
      element.name !== CONSTANT_STRING.BTN_SAVE &&
      element.name !== CONSTANT_STRING.BTN_CANCEL
    ) {
      setFocus(Focus.none);
    }
  }, [focusedElement]);

  const handleSaveGeoPos = React.useCallback(
    (newPos: LatLng) => {
      // Save value
      const newPoi: PointOfInterestType = {
        ...poi,
        position: newPos,
      };

      dispatch(iwdActions.updatePointOfInterest(JSON.stringify(newPoi)));
    },
    [poi]
  );

  return (
    <div>
      <ConfigPoITypeContent key={v4()} poi={poi} />
      <ConfigGeoSpatialContent
        key={v4()}
        focus={focus}
        onSave={handleSaveGeoPos}
        position={poi.position}
      />
    </div>
  );
}

export default React.memo(ConfigPoIContent);
