import { Fluid } from 'components/common/base/BaseContainer';
import { SelectChangeEvent, Stack, Switch, Typography, styled } from '@mui/material';
import { ThemedMenuItem, ThemedSelect } from 'components/cbdiEdit/themedComponents/ThemedComponents';
import React from 'react';
import { request, response } from 'projectEvents/cbdiEdit/editEvents';
import { request as cmRequest, response as cmResponse } from 'projectEvents/common/cmEvents';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { useDispatch, useSelector } from 'react-redux';
import { RootState } from 'projectRedux/Store';
import { ConnectStatus } from 'misc/constant/common/cmConstants';

const Root = styled(Fluid)(({ theme }) => ({
  backgroundColor: theme.editor.settingPage.bgColor,
  display: 'flex',
  flexDirection: 'column',
  padding: 50,
  color: theme.editor.settingPage.textColor,
}));

const SettingRow = styled('div')({
  display: 'flex',
  alignItems: 'center',
  fontSize: 20,
  gap: 100,
  '& > :first-of-type': {
    width: 50,
  },
  '& > :nth-of-type(2)': {
    width: 150,
  },
});

/* ------------------------------ Settings ------------------------------ */
export default function Settings() {
  /* ----------------------------- Redux ----------------------------- */
  const theme = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.theme);
  const xaiLiveDebug = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.xaiLiveDebug);
  const dispatch = useDispatch();
  const { setTheme, setXaiLiveDebug } = cbdiEditActions;

  const [isWsConnected, setIsWsConnected] = React.useState(false);

  /* -------------------------------- Callbacks ------------------------------- */
  const handleSelectTheme = (selectTheme: 'dark' | 'light') => {
    window.ipcRenderer.invoke(request.theme.setTheme, selectTheme);
  };

  const handleSwitchXaiLivePlayback = React.useCallback(() => {
    dispatch(setXaiLiveDebug(!xaiLiveDebug));
  }, [xaiLiveDebug]);

  const onWsStatus = (evt: Electron.IpcRendererEvent, status: ConnectStatus) => {
    if (status === ConnectStatus.disconnected) {
      setIsWsConnected(false);
    } else {
      setIsWsConnected(true);
    }
  };

  /* ----------------------------- useEffect hooks ---------------------------- */
  React.useEffect(() => {
    window.ipcRenderer.on(response.theme.themeChanged, (_e, newTheme) => {
      dispatch(setTheme(newTheme));
    });
    return () => {
      window.ipcRenderer.removeAllListeners(response.theme.themeChanged);
    };
  }, []);

  React.useEffect(() => {
    window.ipcRenderer.send(cmRequest.playback.switchXaiLivePlayback, xaiLiveDebug);
  }, [xaiLiveDebug]);

  React.useEffect(() => {
    window.ipcRenderer.send(cmRequest.websocket.status);

    const removeStatusListener = window.ipcRenderer.setupIpcListener(cmResponse.websocket.status, onWsStatus);

    return () => {
      removeStatusListener();
    };
  }, []);
  /* ------------------------------- Components ------------------------------- */
  return (
    <Root>
      <Stack direction="column" spacing={1}>
        <SettingRow>
          <div>Theme</div>
          <ThemedSelect
            onChange={(event: SelectChangeEvent<unknown>) => {
              handleSelectTheme(event.target.value as 'dark' | 'light');
            }}
            hasborder="true"
            value={theme}
          >
            <ThemedMenuItem value="dark">Dark Theme</ThemedMenuItem>
            <ThemedMenuItem value="light">Light Theme</ThemedMenuItem>
          </ThemedSelect>
        </SettingRow>

        {/* XAI Live Playback Debug */}
        <Stack direction="row" spacing={2} sx={{ alignItems: 'center' }}>
          <Typography
            sx={{
              fontSize: 20,
            }}
          >
            XAI Live Playback
          </Typography>
          <Switch
            sx={{
              '& .MuiSwitch-track': {
                backgroundColor: theme === 'dark' ? 'white' : '#000',
              },
            }}
            disabled={isWsConnected}
            checked={xaiLiveDebug}
            onChange={handleSwitchXaiLivePlayback}
          />
        </Stack>
      </Stack>
    </Root>
  );
}
