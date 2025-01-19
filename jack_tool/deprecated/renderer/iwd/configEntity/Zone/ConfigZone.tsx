import { styled, Typography } from '@mui/material';
import calc from 'misc/utils/common/rendererUtils';
import { SavedZone } from 'types/iwd/iwdTypes';
import { CustomStack } from 'components/common/base/BaseContainer';

interface Props {
  currentZone: SavedZone;
}

const LabelText = styled(Typography)({
  color: 'white',
});

export default function ConfigZone(props: Props) {
  /**
   * Do area calculation
   * @returns
   */
  const calculateArea = () => {
    let area = 0;
    for (let i = 0; i < props.currentZone.bounds.length; i += 1) {
      const polygon = props.currentZone.bounds[i];
      area += calc(polygon).area;
    }

    return area;
  };

  /**
   * Convert square meter into square kilometer
   * @param squareMeter square meter
   * @returns
   */
  const toSquareKm = (squareMeter: number) => {
    return squareMeter / 1000000;
  };

  return (
    <div className="config-zone">
      <CustomStack>
        <LabelText>ID: {props.currentZone.UUID}</LabelText>
        <LabelText>Type: {props.currentZone.type}</LabelText>
        {props.currentZone.bounds.map((x, i) => {
          return (
            <div key={props.currentZone.UUID}>
              <LabelText>Polygon {i}</LabelText>
              {x.map((y, i2) => {
                return (
                  <div key={`${y[0]}.${y[1]}`}>
                    <LabelText>Point {i2}</LabelText>
                    <LabelText>lat: {y[0]}</LabelText>
                    <LabelText>lng: {y[1]}</LabelText>
                  </div>
                );
              })}
            </div>
          );
        })}
        <LabelText>
          Last changed:{' '}
          {new Date(parseInt(props.currentZone.lastChanged, 10)).toLocaleString(
            'en-AU'
          )}
        </LabelText>
        <LabelText>
          Total area: {toSquareKm(calculateArea()).toLocaleString('en-AU')} km2
        </LabelText>
      </CustomStack>
    </div>
  );
}
