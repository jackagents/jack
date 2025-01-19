import React from 'react';
import SaveOutlinedIcon from '@mui/icons-material/SaveOutlined';
import IconBtnOnRightToolTitle from './IconBtnOnRightToolTitle';

interface Props {
  onClick?: React.MouseEventHandler<HTMLButtonElement>;
}

export default function SaveIconBtnOnRightToolTitle({ onClick }: Props) {
  return (
    <IconBtnOnRightToolTitle title="Save" onClick={onClick}>
      <SaveOutlinedIcon sx={{ fontSize: '1.5rem' }} />
    </IconBtnOnRightToolTitle>
  );
}
