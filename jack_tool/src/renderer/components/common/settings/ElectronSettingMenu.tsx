import React, { PropsWithChildren } from 'react';
import Typography from '@mui/material/Typography';
import { List, styled } from '@mui/material';
import { Fluid } from 'components/common/base/BaseContainer';
import { request } from 'misc/events/common/cmEvents';
import SettingItem from './SettingItem';

const StyledFluid = styled(Fluid)({
  marginTop: 50,
  marginLeft: 50,
});

const StyledList = styled(List)({
  marginTop: 50,
});

const StyledText = styled(Typography)(({ theme }) => ({
  color: theme.custom?.setting.textColor,
}));

type Props = PropsWithChildren;

function ElectronSettingMenu({ children }: Props) {
  // Map api
  const [satelliteMapAPI, setSatelliteMapAPI] = React.useState('');
  const [streetMapAPI, setStreetMapAPI] = React.useState('');

  React.useEffect(() => {
    // get api from electron store
    let satelliteAPI = window.electronStore.get('satelliteMapAPI') as string;
    let streetViewAPI = window.electronStore.get('streetViewMapAPI') as string;

    // if nothing in store, use default api
    if (!satelliteAPI) {
      satelliteAPI =
        'https://api.mapbox.com/styles/v1/danielchen93/ckxvbly5v2ml216msrvh19pu3/tiles/256/{z}/{x}/{y}@2x?access_token=pk.eyJ1IjoiZGFuaWVsY2hlbjkzIiwiYSI6ImNreHZhcjRrZDZnMGMyb29jd3NxcnpwdXAifQ.HKXvH8zIiY73mKMmh04odg';
      window.electronStore.set('satelliteMapAPI', satelliteAPI);
    }

    if (!streetViewAPI) {
      streetViewAPI =
        'https://api.mapbox.com/styles/v1/danielchen93/ckxvbchyclj7r15t9egr2vplm/tiles/256/{z}/{x}/{y}@2x?access_token=pk.eyJ1IjoiZGFuaWVsY2hlbjkzIiwiYSI6ImNreHZhcjRrZDZnMGMyb29jd3NxcnpwdXAifQ.HKXvH8zIiY73mKMmh04odg';
      window.electronStore.set('streetViewMapAPI', streetViewAPI);
    }

    setSatelliteMapAPI(satelliteAPI);
    setStreetMapAPI(streetViewAPI);
  }, []);

  return (
    <StyledFluid>
      <StyledText variant="h4">Setting</StyledText>
      <StyledList>
        <SettingItem
          label="Sattellite API"
          defaultValue={satelliteMapAPI}
          request={request.project.satellite}
        />
        <SettingItem
          sx={{ marginTop: '4em' }}
          label="Streetview API"
          defaultValue={streetMapAPI}
          request={request.project.streetView}
        />
      </StyledList>

      {children}
    </StyledFluid>
  );
}

export default React.memo(ElectronSettingMenu);
