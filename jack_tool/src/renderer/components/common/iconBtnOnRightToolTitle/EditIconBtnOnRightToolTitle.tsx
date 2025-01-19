import React from 'react';
import SettingsOutlinedIcon from '@mui/icons-material/SettingsOutlined';
import IconBtnOnRightToolTitle from './IconBtnOnRightToolTitle';

interface Props {
  disabled?: boolean;
  onClick?: React.MouseEventHandler<HTMLButtonElement>;
}

export default function EditIconBtnOnRightToolTitle({ onClick, disabled = false }: Props) {
  return (
    <IconBtnOnRightToolTitle title="Configure" onClick={onClick} disabled={disabled}>
      <SettingsOutlinedIcon sx={{ fontSize: '1.5rem' }} />
    </IconBtnOnRightToolTitle>
  );
}
