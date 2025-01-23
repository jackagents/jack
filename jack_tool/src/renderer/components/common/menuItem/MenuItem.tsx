import React from 'react';
import { MenuItem as MuiMenuItem } from '@mui/material';

/* ************************************************************************************************
 * MenuItem
 * *********************************************************************************************** */
interface Props {
  label: string;
  onClick: () => void;
  onClose: () => void;
  disabled?: boolean;
}

export default class MenuItem extends React.Component<Props> {
  /* ********************************************************************************************
   * Callbacks
   * ******************************************************************************************* */
  onClick = () => {
    this.props.onClick();
    this.props.onClose();
  };

  /* ********************************************************************************************
   * Functions
   * ******************************************************************************************* */
  render() {
    return (
      <MuiMenuItem
        disabled={this.props.disabled || false}
        onClick={this.onClick}
      >
        {this.props.label}
      </MuiMenuItem>
    );
  }
}
