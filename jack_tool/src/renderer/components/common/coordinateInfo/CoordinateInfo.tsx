import { Divider, Typography } from '@mui/material';
import { styled, SxProps, Theme } from '@mui/material/styles';
import L, { LatLng, LeafletEvent } from 'leaflet';
import React from 'react';
import { useMapEvents } from 'react-leaflet';
import { Z_INDEX } from 'constant/common/cmConstants';
import Topography from './Topography';

const Info = styled('div')({
  position: 'absolute',
  bottom: 10,
  left: 10,
  width: 'auto',
  height: 'auto',
  minWidth: '13vw',
  minHeight: '2vh',
  backgroundColor: 'rgba(0,0,0,0.5)',
  padding: '1em',
  zIndex: Z_INDEX.CONTROL,
});

const StyledDivider = styled(Divider)({
  marginTop: 1,
});

const StyledText = styled(Typography)(({ theme }) => ({
  margin: '5px',
  fontSize: theme.custom?.text.size.small,
  color: theme.custom?.text.color,
}));

interface Props {
  sx?: SxProps<Theme>;
}
function CoordinateInfo(props: Props) {
  const [hidden, setHidden] = React.useState(false);
  const [coordinate, setCoordinate] = React.useState<LatLng>();
  const divRef = React.useRef<HTMLDivElement>(null);

  /**
   * Leaflet map
   */
  const map = useMapEvents({
    moveend: (event: LeafletEvent) => {
      setCoordinate(map.getCenter());
    },
  });

  React.useEffect(() => {
    if (map) {
      map.on({
        overlayadd: (e) => {
          if (e.name === 'Coordinate Info') {
            setHidden(false);
          }
        },
        overlayremove: (e) => {
          if (e.name === 'Coordinate Info') {
            setHidden(true);
          }
        },
      });
    }

    return () => {
      if (map) {
        map.off('overlayadd');
        map.off('overlayremove');
      }
    };
  }, [map]);

  React.useEffect(() => {
    if (divRef.current) {
      L.DomEvent.disableClickPropagation(divRef.current);
      L.DomEvent.disableScrollPropagation(divRef.current);
    }

    setCoordinate(map.getCenter());
  }, []);

  return (
    <Info
      onMouseEnter={() => {
        map.dragging.disable();
      }}
      onMouseLeave={() => {
        map.dragging.enable();
      }}
      ref={divRef}
      className="coordinate-info"
      sx={props.sx}
      hidden={hidden}
    >
      <StyledText>Center: </StyledText>
      <StyledText>
        Latitude: {coordinate ? coordinate.lat : 'undefined'}
      </StyledText>
      <StyledText>
        Longitude: {coordinate ? coordinate.lng : 'undefined'}
      </StyledText>
      <StyledDivider />
      <StyledText>Topography: </StyledText>
      <Topography latlng={coordinate} />
    </Info>
  );
}

CoordinateInfo.defaultProps = {
  sx: null,
};

export default React.memo(CoordinateInfo);
