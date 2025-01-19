/* eslint-disable react/prefer-stateless-function */
/* eslint-disable no-multi-str */
import React from 'react';
import { styled } from '@mui/material';
import { Flex } from '../base/BaseContainer';

const Root = styled(Flex)({
  justifyContent: 'center',
  alignItems: 'center',
  color: 'whitesmoke',
  border: 'thin dashed whitesmoke',
  background:
    'linear-gradient(135deg, transparent 25%, #00000050 25%, #00000050 50%, transparent 50%, \
         transparent 75%, #00000050 75%, #00000050 100%)',
  backgroundSize: '14.14px 14.14px',
});

/* ************************************************************************************************
 * Placeholder
 * *********************************************************************************************** */
interface Props {
  label: string;
}

export default class Placeholder extends React.Component<Props, {}> {
  /* ********************************************************************************************
   * Functions
   * ******************************************************************************************* */
  render() {
    return <Root>{this.props.label}</Root>;
  }
}
