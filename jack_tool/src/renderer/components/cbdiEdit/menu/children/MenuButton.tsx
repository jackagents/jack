import React from 'react';
import { styled, Menu, Button } from '@mui/material';
import MenuItem from './MenuItem';
import MenuAction from './MenuAction';

const TopMenuButton = styled(Button)({
  height: '100%',
  padding: '0 0.5em',
});

/* ************************************************************************************************
 * MenuButton
 * *********************************************************************************************** */
interface Props {
  label: string | JSX.Element;
  submenu: MenuAction[];
  style?: React.CSSProperties;
}

interface States {
  anchorEl: HTMLElement | null;
}

export default class MenuButton extends React.Component<Props, States> {
  /* ********************************************************************************************
   * Properties
   * ******************************************************************************************* */
  // eslint-disable-next-line react/static-property-placement
  static defaultProps = {
    // eslint-disable-next-line react/default-props-match-prop-types
    label: 'Menu',
  };

  /* ********************************************************************************************
   * Constructor
   * ******************************************************************************************* */
  constructor(props: Props) {
    super(props);

    this.state = {
      anchorEl: null,
    };
  }

  /* ********************************************************************************************
   * Callbacks
   * ******************************************************************************************* */
  onMenuClick = (event: React.MouseEvent<HTMLButtonElement>) => {
    this.setState({ anchorEl: event.currentTarget });
  };

  onClose = () => {
    this.setState({ anchorEl: null });
  };

  /* ********************************************************************************************
   * Functions
   * ******************************************************************************************* */
  render() {
    return (
      <div>
        <TopMenuButton
          variant="text"
          color="secondary"
          aria-controls="simple-menu"
          aria-haspopup="true"
          onClick={this.onMenuClick}
          style={this.props.style ? this.props.style : undefined}
        >
          {this.props.label}
        </TopMenuButton>
        <Menu
          id="simple-menu"
          anchorOrigin={{
            vertical: 'bottom',
            horizontal: 'left',
          }}
          transformOrigin={{
            vertical: 'top',
            horizontal: 'left',
          }}
          keepMounted
          anchorEl={this.state.anchorEl}
          open={Boolean(this.state.anchorEl)}
          onClose={this.onClose}
        >
          {this.props.submenu.map((action: MenuAction, key: number) => {
            return (
              <MenuItem
                key={key as number}
                label={action.label!}
                onClick={action.onClick!}
                onClose={this.onClose}
              />
            );
          })}
        </Menu>
      </div>
    );
  }
}
