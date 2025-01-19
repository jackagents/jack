import { Grid, Stack, styled } from '@mui/material';
import { ConfigLabelTitle } from 'components/common/base/BaseContainer';
import React from 'react';
import IntruderMissionButton from 'components/iwd/iconButtons/intruderMissionButton/IntruderMissionButton';
import PathMissionButton from 'components/iwd/iconButtons/pathMissionButton/PathMissionButton';
import ScoutMissionButton from 'components/iwd/iconButtons/scoutMissionButton/ScoutMissionButton';
import { IconButtonGroup } from 'components/iwd/iwdStyledComponents/IwdStyledComponents';
import CurrentMissionButton from 'components/iwd/iconButtons/currentMissionButton/CurrentMissionButton';
import { IwdIconButtonId } from 'types/iwd/iwdTypes';

const StyledStack = styled(Stack)({
  margin: '0 10px 0 10px',
  height: '2vw',
});

interface Props {
  current: IwdIconButtonId;
  onClick: (event: React.MouseEvent<HTMLButtonElement>) => void;
}

function MissionConfig({ current, onClick }: Props) {
  return (
    <Grid>
      <ConfigLabelTitle>Mission Config</ConfigLabelTitle>
      <StyledStack direction="row">
        <IconButtonGroup>
          <CurrentMissionButton runningMission={false} onClick={onClick} />
          <IntruderMissionButton
            activated={current === IwdIconButtonId.intruder}
            onClick={onClick}
          />
          <PathMissionButton
            activated={current === IwdIconButtonId.path}
            onClick={onClick}
          />
          <ScoutMissionButton
            activated={current === IwdIconButtonId.scout}
            onClick={onClick}
          />
        </IconButtonGroup>
      </StyledStack>
    </Grid>
  );
}

export default React.memo(MissionConfig);
