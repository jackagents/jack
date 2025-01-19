import { ButtonGroup, IconButton, styled } from '@mui/material';
import React from 'react';

import CancelIcon from '@mui/icons-material/Cancel';
import CheckCircleIcon from '@mui/icons-material/CheckCircle';
import { CONSTANT_STRING } from 'constant/common/cmConstants';

interface Props {
  focus: boolean;
  onClickSave: () => void;
  onClickCancel: () => void;
}

const BtnGroup = styled(ButtonGroup)({
  alignItems: 'center',
  width: '30%',
});

const BtnIcon = styled(IconButton)({
  width: '50%',
});

const StyledCancelIcon = styled(CancelIcon)(({ theme }) => ({
  fontSize: theme.custom?.text.size.large,
}));

const StyledCheckCircleIcon = styled(CheckCircleIcon)(({ theme }) => ({
  fontSize: theme.custom?.text.size.large,
}));

function ButtonGroupAdornment(props: Props) {
  return props.focus ? (
    <BtnGroup>
      <BtnIcon
        onClick={props.onClickCancel}
        name={CONSTANT_STRING.BTN_CANCEL}
        color="error"
      >
        <StyledCancelIcon />
      </BtnIcon>
      <BtnIcon
        onClick={props.onClickSave}
        name={CONSTANT_STRING.BTN_SAVE}
        color="success"
      >
        <StyledCheckCircleIcon />
      </BtnIcon>
    </BtnGroup>
  ) : null;
}

export default React.memo(ButtonGroupAdornment);
