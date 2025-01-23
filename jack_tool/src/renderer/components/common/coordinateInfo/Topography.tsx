import React from 'react';
import { LatLng } from 'leaflet';
import { styled, Typography } from '@mui/material';
import * as Topography from 'addons/.leaflet-topography';

const options = {
  token:
    'pk.eyJ1IjoiZGFuaWVsY2hlbjkzIiwiYSI6ImNreHZhcjRrZDZnMGMyb29jd3NxcnpwdXAifQ.HKXvH8zIiY73mKMmh04odg',
};

const style = { margin: '5px', fontSize: '.7vw' };

interface Topo {
  elevation: any;
  slope: number;
  aspect: number;
  resolution: number;
  latlng: LatLng;
}

interface Props {
  latlng: LatLng | undefined;
}

const StyledText = styled(Typography)(({ theme }) => ({
  margin: '5px',
  fontSize: theme.custom?.text.size.small,
  color: theme.custom?.text.color,
}));

export default function TopographyInfo(props: Props) {
  const [topo, setTopo] = React.useState<Topo>();

  /**
   * Get topography using API
   * @param latlng L.latlng
   */
  const requestTopo = async (latlng: LatLng) => {
    Topography.getTopography(latlng, options)
      .then((results) => {
        if (props.latlng === results.latlng) {
          setTopo({ ...results });
        }

        return latlng;
      })
      .catch((e) => console.log(e));
  };

  React.useEffect(() => {
    if (!props.latlng) return;

    requestTopo(props.latlng);
  }, [props.latlng]);

  return (
    <>
      <StyledText>Elevation: {topo && topo.elevation}</StyledText>
      <StyledText>Slope: {topo && topo.slope}</StyledText>
      <StyledText>Aspect: {topo && topo.aspect}</StyledText>
      <StyledText>Resolution: {topo && topo.resolution}</StyledText>
    </>
  );
}
