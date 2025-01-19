import React from 'react';
import DeleteOutlineOutlinedIcon from '@mui/icons-material/DeleteOutlineOutlined';
import IconBtnOnRightToolTitle from './IconBtnOnRightToolTitle';

interface Props {
  onClick?: React.MouseEventHandler<HTMLButtonElement>;
}

export default function DeleteIconBtnOnRightToolTitle({ onClick }: Props) {
  return (
    <IconBtnOnRightToolTitle title="Delete" onClick={onClick}>
      <DeleteOutlineOutlinedIcon sx={{ fontSize: '1.5rem' }} />
    </IconBtnOnRightToolTitle>
  );
}
