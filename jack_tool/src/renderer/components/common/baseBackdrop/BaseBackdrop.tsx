import * as React from 'react';
import Backdrop from '@mui/material/Backdrop';
import CircularProgress from '@mui/material/CircularProgress';
import { Button, styled } from '@mui/material';
import { request } from 'projectEvents/common/cmEvents';
import { LOADING_PAGE_CLOSE_BUTTON_NAME } from 'misc/constant/cbdiEdit/cbdiEditConstant';

interface Props {
  loading: boolean;
  closable: boolean;
  percentage?: number;
  closeBtnName?: string;
}

const CloseButton = styled(Button)(({ theme }) => ({
  backgroundColor: '#333',
  color: 'white',
  borderRadius: 5,
  '&:hover': {
    backgroundColor: '#555',
  },
}));

export function BaseBackdrop({ loading, percentage, closable, closeBtnName = 'Close' }: Props) {
  const [open, setOpen] = React.useState(false);

  const handleClickClose = () => {
    if (closeBtnName === LOADING_PAGE_CLOSE_BUTTON_NAME.DISCONNECT_WS) {
      // Close ws
      window.ipcRenderer.send(request.websocket.disconnect);
    }

    setOpen(false);
  };

  React.useEffect(() => {
    setOpen(loading);
  }, [loading]);

  return (
    <div key="base-backdrop-root" id="base-backdrop-root">
      <Backdrop
        sx={{
          color: '#fff',
          zIndex: (theme) => theme.zIndex.drawer + 1,
          display: 'flex',
          flexDirection: 'column',
          alignItems: 'center',
        }}
        open={open}
      >
        <CircularProgress color="inherit" />
        <br />
        {percentage && <span>{percentage}%</span>}

        {closable && <CloseButton onClick={handleClickClose}>{closeBtnName}</CloseButton>}
      </Backdrop>
    </div>
  );
}
