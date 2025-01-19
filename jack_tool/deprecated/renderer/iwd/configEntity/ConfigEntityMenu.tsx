import React from 'react';
import { useDispatch } from 'react-redux';
import { iwdActions } from 'projectRedux/reducers/iwd/iwdClientReducer';
import CommonConfigEntityMenu from 'components/common/configEntity/CommonConfigEntityMenu';
import ConfigDisplay from './ConfigDisplay';

function ConfigEntityMenu() {
  const dispatch = useDispatch();

  const handleClose = React.useCallback(() => {
    dispatch(iwdActions.closeConfigDisplay());
  }, []);

  return (
    <CommonConfigEntityMenu
      onClose={handleClose}
      configDisplay={<ConfigDisplay />}
    />
  );
}

export default React.memo(ConfigEntityMenu);
