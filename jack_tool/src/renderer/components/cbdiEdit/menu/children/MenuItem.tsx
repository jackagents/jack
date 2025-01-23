import React from 'react';
import { Typography, MenuItem as SelectMenuItem } from '@mui/material';

/* ************************************************************************************************
 * MenuItem
 * *********************************************************************************************** */

interface Props {
  label: string;
  onClick: () => void;
  onClose: () => void;
}

export default class MenuItem extends React.Component<Props, {}> {
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
    let shortcutText = '';
    switch (this.props.label) {
      case 'New':
        shortcutText = 'Ctrl + N';
        break;
      case 'Open':
        shortcutText = 'Ctrl + O';
        break;
      case 'Save':
        shortcutText = 'Ctrl + S';
        break;
      default:
        break;
    }
    return (
      <SelectMenuItem style={{}} onClick={this.onClick}>
        {/* {this.props.label} */}
        <div style={{ flex: '1 1 auto' }}>
          <span>{this.props.label}</span>
        </div>
        <Typography variant="body2">{shortcutText}</Typography>
      </SelectMenuItem>
    );
  }
}
