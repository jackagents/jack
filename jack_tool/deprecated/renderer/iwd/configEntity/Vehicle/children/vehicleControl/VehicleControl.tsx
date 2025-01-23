import { Grid, Stack, styled } from '@mui/material';
import { ConfigLabelTitle } from 'components/common/base/BaseContainer';
import React from 'react';
import { IconButtonGroup } from 'components/iwd/iwdStyledComponents/IwdStyledComponents';
import AutoModeButton from 'components/iwd/iconButtons/autoModeButton/AutoModeButton';
import ManualModeButton from 'components/iwd/iconButtons/manualModeButton/ManualModeButton';
import SafetyLockModeButton from 'components/iwd/iconButtons/safetyLockModeButton/SafetyLockModeButton';
import { DrivingMode } from 'types/iwd/iwdTypes';
import ActivatedButton from 'components/iwd/iconButtons/activatedButton/ActivatedButton';
import ReturnToBaseButton from 'components/iwd/iconButtons/returnToBaseButton/ReturnToBaseButton';

const StyledStack = styled(Stack)({
  margin: '0 10px 0 10px',
  height: '2vw',
});

interface Props {
  activated: boolean;
  mode: DrivingMode;
  returnToBase: boolean;
  onClick: () => void;
}

function VehicleControl({ activated, mode, returnToBase, onClick }: Props) {
  return (
    <Grid>
      <ConfigLabelTitle>VehicleControl</ConfigLabelTitle>
      <StyledStack direction="row">
        <IconButtonGroup>
          <AutoModeButton mode={mode} onClick={onClick} />
          <ManualModeButton mode={mode} onClick={onClick} />
          <SafetyLockModeButton mode={mode} onClick={onClick} />
          <ActivatedButton activated={activated} onClick={onClick} />
        </IconButtonGroup>
        <ReturnToBaseButton onClick={onClick} returnToBase={returnToBase} />
      </StyledStack>
    </Grid>
  );
}

export default React.memo(VehicleControl);
