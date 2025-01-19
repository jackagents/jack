import { Grid, Stack, styled } from '@mui/material';
import * as RegEx from 'misc/regex/regex';
import { LatLng } from 'leaflet';
import React from 'react';
import { CONSTANT_STRING } from 'constant/common/cmConstants';
// import ButtonGroupAdornment from 'components/iwd/configEntity/PoI/ButtonGroupAdornment';
import {
  ConfigLabelTitle,
  CustomTextfield,
} from 'components/common/base/BaseContainer';

const GeoSpaStack = styled(Stack)({
  margin: '.5vw',
});

enum Focus {
  none = 'none',
  lat = 'lat',
  lng = 'lng',
}

interface Props {
  position: LatLng;
  focus: string;
  onSave?: (newPosition: LatLng) => void;
  label?: string;
}

interface PoIContentProps {
  position: {
    lat: string;
    lng: string;
    alt: string;
  };
}

function ConfigGeoSpatialContent({ position, onSave, focus, label }: Props) {
  // const [focus, setFocus] = React.useState<Focus>(Focus.none);

  const [content, setContent] = React.useState<PoIContentProps>({
    position: {
      lat: position.lat.toString(),
      lng: position.lng.toString(),
      alt: position.alt?.toString() || '',
    },
  });

  React.useEffect(() => {
    setContent({
      position: {
        lat: position.lat.toString(),
        lng: position.lng.toString(),
        alt: position.alt?.toString() || '',
      },
    });
  }, [position]);

  // const focusedElement = useActiveElement();

  // React.useEffect(() => {
  //   if (!focusedElement) {
  //     console.log('focusedElement is null');
  //     return;
  //   }

  //   const element = focusedElement as HTMLInputElement;

  //   if (element.name === CONSTANT_STRING.LAT) {
  //     setFocus(Focus.lat);
  //   } else if (element.name === CONSTANT_STRING.LNG) {
  //     setFocus(Focus.lng);
  //   } else if (
  //     element.name !== CONSTANT_STRING.BTN_SAVE &&
  //     element.name !== CONSTANT_STRING.BTN_CANCEL
  //   ) {
  //     setFocus(Focus.none);
  //   }
  // }, [focusedElement]);

  /**
   * Callback for HTML Input element changed events
   */
  const handleChange = React.useCallback(
    (event: React.ChangeEvent<HTMLInputElement>) => {
      switch (event.target.name) {
        case CONSTANT_STRING.LAT:
          if (!RegEx.LatFloatStringReg.test(event.target.value)) {
            return;
          }

          setContent({
            ...content,
            position: { ...content.position, lat: event.target.value },
          });

          break;
        case CONSTANT_STRING.LNG:
          if (!RegEx.LngFloatStringReg.test(event.target.value)) {
            return;
          }

          setContent({
            ...content,
            position: { ...content.position, lng: event.target.value },
          });

          break;
        case CONSTANT_STRING.ALT:
          // setValue({ ...value, alt: event.target.value });
          setContent({
            ...content,
            position: { ...content.position, alt: event.target.value },
          });

          break;
        default:
          break;
      }
    },
    [content]
  );

  /**
   * Callback when click button save
   */
  const handleClickBtnSave = React.useCallback(() => {
    if (!onSave) return;

    onSave(
      new LatLng(
        parseFloat(content.position.lat),
        parseFloat(content.position.lng),
        parseFloat(content.position.alt)
      )
    );
  }, [content]);

  /**
   * Callback when click button cancel
   */
  const handleClickBtnCancel = React.useCallback(() => {
    // Clear value
    switch (focus) {
      case 'lat':
        setContent({
          ...content,
          position: {
            ...content.position,
            lat: position.lat.toString(),
          },
        });
        break;
      case 'lng':
        setContent({
          ...content,
          position: {
            ...content.position,
            lng: position.lng.toString(),
          },
        });
        break;
      default:
        break;
    }
  }, [focus, content]);

  return (
    <Grid>
      <ConfigLabelTitle>{label || 'Geospatial Position'}</ConfigLabelTitle>
      <GeoSpaStack direction="column" spacing="1vw">
        <CustomTextfield
          label="Lat"
          name={CONSTANT_STRING.LAT}
          value={content.position.lat || ''}
          onChange={handleChange}
          // InputProps={{
          //   endAdornment: (
          //     <ButtonGroupAdornment
          //       onClickCancel={handleClickBtnCancel}
          //       onClickSave={handleClickBtnSave}
          //       focus={focus === 'lat'}
          //     />
          //   ),
          // }}
        />

        <CustomTextfield
          label="Lng"
          name={CONSTANT_STRING.LNG}
          value={content.position.lng || ''}
          onChange={handleChange}
          // InputProps={{
          //   endAdornment: (
          //     <ButtonGroupAdornment
          //       onClickCancel={handleClickBtnCancel}
          //       onClickSave={handleClickBtnSave}
          //       focus={focus === 'lng'}
          //     />
          //   ),
          // }}
        />

        <CustomTextfield
          label="Alt"
          disabled
          name={CONSTANT_STRING.ALT}
          value={content.position.alt || ''}
          onChange={handleChange}
        />
      </GeoSpaStack>
    </Grid>
  );
}

export default React.memo(ConfigGeoSpatialContent);
