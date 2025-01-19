import {
  Container,
  CustomFormControlLabel,
  CustomFormGroup,
  CustomSwitch,
} from 'components/common/base/BaseContainer';
import React from 'react';
import { Stack, Divider, styled } from '@mui/material';
import ConnectionStatus from 'components/common/connectionStatus/ConnectionStatus';
import { request, response } from 'projectEvents/common/cmEvents';
import { IpcRendererEvent } from 'electron';
import { ConnectStatus } from 'constant/common/cmConstants';

const ToolbarDivider = styled(Divider)(({ theme }) => ({
  height: '70%',
  backgroundColor: theme.custom?.dividerBgColor,
}));

const time = 10000;
let timer: NodeJS.Timeout | null = null;
export default function ClientStatusControl() {
  const [connect, setConnect] = React.useState(ConnectStatus.disconnected);
  const connectionText = React.useMemo(() => {
    switch (connect) {
      case ConnectStatus.connecting:
        return 'Connecting';

      case ConnectStatus.disconnected:
        return 'Disconnect';

      case ConnectStatus.connected:
        return 'Connected';

      default:
        return '';
    }
  }, [connect]);
  /**
   * start timer function
   */
  const startTimer = React.useCallback(() => {
    return setTimeout(() => {
      if (connect === ConnectStatus.disconnected) {
        window.ipcRenderer.invoke(request.websocket.disconnect);
        setConnect(ConnectStatus.disconnected);
      }
    }, time);
  }, [connect]);

  /**
   * Callback on changing connect status
   */
  const handleChange = React.useCallback(() => {
    if (connect === ConnectStatus.connected) {
      window.ipcRenderer.invoke(request.websocket.disconnect);
    } else if (connect === ConnectStatus.disconnected) {
      window.ipcRenderer.invoke(request.websocket.connect);
      setConnect(ConnectStatus.connecting);
      timer = startTimer();
    }
  }, [connect, startTimer]);

  const onWsConnected = () => {
    setConnect(ConnectStatus.connected);

    if (timer) {
      clearTimeout(timer);
    }
  };

  const onWsDisconnected = () => {
    setConnect(ConnectStatus.disconnected);
  };

  const onWsStatus = (_e: IpcRendererEvent, status: ConnectStatus) => {
    setConnect(status);
  };

  React.useEffect(() => {
    window.ipcRenderer.send(request.websocket.status);

    window.ipcRenderer.on(response.websocket.status, onWsStatus);
    window.ipcRenderer.on(response.websocket.connected, onWsConnected);
    window.ipcRenderer.on(response.websocket.disconnected, onWsDisconnected);

    return () => {
      window.ipcRenderer.removeListener(response.websocket.status, onWsStatus);
      window.ipcRenderer.removeListener(
        response.websocket.connected,
        onWsConnected
      );
      window.ipcRenderer.removeListener(
        response.websocket.disconnected,
        onWsDisconnected
      );
    };
  }, []);

  return (
    <Stack direction="row">
      <Container container>
        <CustomFormGroup>
          <CustomFormControlLabel
            control={
              <CustomSwitch
                onChange={handleChange}
                checked={connect !== ConnectStatus.disconnected}
              />
            }
            label={connectionText}
          />
        </CustomFormGroup>
      </Container>

      <Container container>
        <ToolbarDivider orientation="vertical" />
      </Container>

      <ConnectionStatus connected={connect === ConnectStatus.connected} />
    </Stack>
  );
}
