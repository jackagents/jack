import { Box, Tabs, Tab, styled, Icon, Stack } from '@mui/material';
import React from 'react';
import LocationOnOutlinedIcon from '@mui/icons-material/LocationOnOutlined';
import DirectionsCarFilledOutlinedIcon from '@mui/icons-material/DirectionsCarFilledOutlined';
import SensorsOutlinedIcon from '@mui/icons-material/SensorsOutlined';
import PersonPinCircleOutlinedIcon from '@mui/icons-material/PersonPinCircleOutlined';
import AdbOutlinedIcon from '@mui/icons-material/AdbOutlined';
import imgs from 'misc/images';

const TabDisplayBox = styled(Box)(({ theme }) => ({
  width: '100%',
  '& .Mui-selected': { backgroundColor: theme.custom?.tab.selectedTab.bgColor },
}));

const TabIcon = styled(Tab)(({ theme }) => ({
  color: theme.custom?.text.color,
  backgroundColor: theme.custom?.backgroundColor,
}));

const StyledTab = styled(Tabs)(({}) => ({}));

interface Props {
  onTabChange: (value: string) => void;
}

export const AgentIcon = () => {
  return (
    <Icon>
      <img alt="agent-icon" src={imgs.activeAgent} height={25} width={25} />
    </Icon>
  );
};

export default function SearchDisplayTabs(props: Props) {
  const [value, setValue] = React.useState('agent');

  const handleChange = React.useCallback(
    (event: React.SyntheticEvent, newValue: string) => {
      setValue(newValue);
    },
    []
  );

  const handleChangeIWD = React.useCallback(
    (event: React.SyntheticEvent, newValue: string) => {
      setValue(newValue);
    },
    []
  );

  React.useEffect(() => {
    props.onTabChange(value);
  }, [value]);

  return (
    <TabDisplayBox>
      <Stack direction="column">
        <StyledTab
          variant="scrollable"
          value={value}
          onChange={handleChangeIWD}
        >
          <TabIcon
            value="vehicle"
            label="VEHICLE"
            icon={<DirectionsCarFilledOutlinedIcon />}
          />
          {/* <StyledDivider orientation="vertical" /> */}
          <TabIcon
            value="intruder"
            label="INTRUDER"
            icon={<AdbOutlinedIcon />}
          />
          {/* <StyledDivider orientation="vertical" /> */}
          <TabIcon
            value="sensor"
            label="SENSOR"
            icon={<SensorsOutlinedIcon />}
          />
          <TabIcon
            value="agent"
            label="AGENT"
            icon={<PersonPinCircleOutlinedIcon />}
          />
          {/* <StyledDivider orientation="vertical" /> */}
          <TabIcon
            value="poi"
            label="INTEREST"
            icon={<LocationOnOutlinedIcon />}
          />
        </StyledTab>

        {/* <StyledTab
          variant="fullWidth"
          value={value}
          onChange={handleChange}
        ></StyledTab> */}
      </Stack>
    </TabDisplayBox>
  );
}
