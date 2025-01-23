import React from 'react';
import { Menu, MenuItem } from '@mui/material';
import { ZONE_TYPE } from 'constant/common/cmConstants';

interface Props {
  anchorEl: HTMLElement | null;
  open: boolean;
  onClose: () => void;
  onMenuListClick: (
    event: React.MouseEvent<HTMLElement>,
    index: number
  ) => void;
}

export default function MapList({
  anchorEl,
  open,
  onClose,
  onMenuListClick,
}: Props) {
  return (
    <Menu
      id="basic-menu"
      anchorEl={anchorEl}
      open={open}
      onClose={onClose}
      MenuListProps={{
        'aria-labelledby': 'basic-button',
      }}
    >
      {Object.keys(ZONE_TYPE).flatMap((type, index) => {
        if (type.toLowerCase() === 'none') return [];

        return (
          <MenuItem
            key={`id_${type}`}
            onClick={(event) => {
              onMenuListClick(event, index);
            }}
          >
            {type}
          </MenuItem>
        );
      })}
    </Menu>
  );
}
