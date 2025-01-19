import React from 'react';
import CancelOutlinedIcon from '@mui/icons-material/CancelOutlined';
import IconBtnOnRightToolTitle from './IconBtnOnRightToolTitle';

interface Props {
  onClick?: React.MouseEventHandler<HTMLButtonElement>;
}

export default function ExitIconBtnOnRightToolTitle({ onClick }: Props) {
  return (
    <IconBtnOnRightToolTitle title="Close" onClick={onClick}>
      <CancelOutlinedIcon sx={{ fontSize: '1.5rem' }} />
    </IconBtnOnRightToolTitle>
  );
}
