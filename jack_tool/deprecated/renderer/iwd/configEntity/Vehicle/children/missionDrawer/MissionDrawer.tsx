import { Drawer } from '@mui/material';
import React from 'react';
import { IwdIconButtonId } from 'types/iwd/iwdTypes';
import PathMissionConfig from 'components/iwd/configEntity/Vehicle/children/pathMissionConfig/PathMissionConfig';

interface Props {
  drawer: boolean;
  current: IwdIconButtonId;
  onClose: () => void;
}

function MissionDrawer({ drawer, onClose, current }: Props) {
  return (
    <Drawer anchor="right" open={drawer} onClose={onClose}>
      {current === IwdIconButtonId.path && <PathMissionConfig />}
    </Drawer>
  );
}

export default React.memo(MissionDrawer);
