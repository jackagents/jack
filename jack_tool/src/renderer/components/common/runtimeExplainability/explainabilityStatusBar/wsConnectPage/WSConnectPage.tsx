import React from 'react';
import { styled } from '@mui/material';
import Switch from '@mui/material/Switch';
import { request, response } from 'projectEvents/common/cmEvents';
import TextField from '@mui/material/TextField';
import { CBDIAgent } from 'types/cbdi/cbdi-models-non-flatbuffer';
import { IpcRendererEvent } from 'electron';
import { ConnectStatus } from 'constant/common/cmConstants';
import IconButton from '@mui/material/IconButton';
import { ExpandLess, ExpandMore } from '@mui/icons-material';
import { useExplainabilityContext } from '../../context/explainabilityContext';

/* --------------------------------- Styles --------------------------------- */

const ConnectPageContainer = styled('div')({
  position: 'relative',
  display: 'flex',
  justifyContent: 'space-between',
  alignItems: 'center',
});

export const MuiSwitchLarge = styled(Switch)({
  '&&&': {
    width: 50,
    height: 25,
    padding: 0,
    backgroundColor: 'transparent',
    '& .MuiSwitch-switchBase': {
      margin: 3,
      padding: 0,
      transform: 'translateX(1px)',
      '&.Mui-checked': {
        transform: 'translateX(24px)',
      },
    },
    '& .MuiSwitch-thumb': {
      width: 19,
      height: 19,
      backgroundColor: 'white',
    },
    '& .MuiSwitch-track': {
      borderRadius: 19,
    },
  },
});

const DetailContainer = styled('div')({
  position: 'absolute',
  width: 300,
  top: 50,
  left: 0,
  zIndex: 9999,
  backgroundColor: '#323233',
  borderRadius: '0 0 5% 5%',
  color: 'white',
});

const ConfigContainer = styled('div')({
  width: '100%',
  display: 'flex',
  flexDirection: 'column',
  alignItems: 'center',
  padding: 20,
  gap: 20,
});

const PropertyContainer = styled('div')({
  width: '100%',
  heigth: 50,
  display: 'flex',
  alignItems: 'center',
  justifyContent: 'space-between',
  '&>div:first-of-type': {
    width: 50,
    fontWeight: 500,
    fontSize: 18,
  },
});

const CustomTextField = styled(TextField)({
  '& .MuiInputBase-input': {
    color: '#fff', // Text color
  },
  '& .MuiInput-underline:before': {
    borderBottomColor: '#fff8', // Semi-transparent underline
  },
  '& .MuiInput-underline:hover:before': {
    borderBottomColor: '#fff', // Solid underline on hover
  },
  '& .MuiInput-underline:after': {
    borderBottomColor: '#fff', // Solid underline on focus
  },
});

/* ------------------------------- Properties ------------------------------- */
let timer: NodeJS.Timeout;
const TIMEOUT_DURATION = 10000;
export default function WSConnectPage() {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { connectStatus, project, setConnectStatus, setInspectNodeData, setInspectAgentGoal } = useExplainabilityContext();
  /* ----------------------------- useState hooks ----------------------------- */
  const [openDetail, setOpenDetail] = React.useState(false);
  const [ipAddress, setIpAddress] = React.useState('127.0.0.1');
  const [port, setPort] = React.useState(9001);
  const [connectedNode, setConnectedNode] = React.useState<CBDIAgent | undefined>(undefined);

  /* -------------------------------- Callbacks ------------------------------- */

  /**
   * Callback on changing connect status
   */
  const handleChange = React.useCallback(
    (mipAddress: string, mport: number) => {
      if (connectStatus === ConnectStatus.connected) {
        window.ipcRenderer.invoke(request.websocket.disconnect);
      } else {
        // New ipaddress since 20230922 (bson websocket)
        const newAddress = `ws://${mipAddress}:${mport}/bus`;
        // Old address
        // const newAddress = `ws://${mipAddress}:${mport}`;
        window.ipcRenderer.invoke(request.websocket.connect, newAddress);
        setConnectStatus(ConnectStatus.connected);
        if (timer) {
          clearTimeout(timer);
        }
        timer = setTimeout(() => {
          setConnectStatus(ConnectStatus.disconnected);
          console.log('send disconnected message');

          window.ipcRenderer.invoke(request.websocket.disconnect);
        }, TIMEOUT_DURATION);
      }
    },
    [connectStatus],
  );

  const onWsConnected = () => {
    setConnectStatus(ConnectStatus.connected);
    clearTimeout(timer);
  };

  const onWsDisconnected = () => {
    setConnectStatus(ConnectStatus.disconnected);
    clearTimeout(timer);
  };

  const onNodeInfo = (_e: any, data: string) => {
    const obj = JSON.parse(data) as CBDIAgent;
    setConnectedNode(obj);
  };

  const onWsStatus = (_e: IpcRendererEvent, status: ConnectStatus) => {
    setConnectStatus(status);
  };
  /* ----------------------------- useEffect hooks ---------------------------- */
  React.useEffect(() => {
    window.ipcRenderer.send(request.websocket.status);

    const removeStatusListener = window.ipcRenderer.setupIpcListener(response.websocket.status, onWsStatus);
    const removeConnectedListener = window.ipcRenderer.setupIpcListener(response.websocket.connected, onWsConnected);
    const removeDisconnectedListener = window.ipcRenderer.setupIpcListener(response.websocket.disconnected, onWsDisconnected);
    const removeNodeInfoListener = window.ipcRenderer.setupIpcListener(response.cbdi.nodeInfo, onNodeInfo);

    return () => {
      removeStatusListener();
      removeConnectedListener();
      removeDisconnectedListener();
      removeNodeInfoListener();
    };
  }, []);

  React.useEffect(() => {
    if (connectStatus === ConnectStatus.disconnected) {
      setConnectedNode(undefined);
      setInspectNodeData(undefined);
      setInspectAgentGoal(undefined);
    } else {
      window.ipcRenderer.invoke(request.cbdi.nodeInfo);
    }
  }, [connectStatus]);

  return (
    <ConnectPageContainer>
      <MuiSwitchLarge
        disabled={project === null && connectStatus === ConnectStatus.disconnected}
        checked={connectStatus === ConnectStatus.connected}
        onChange={() => {
          handleChange(ipAddress, port);
        }}
      />
      {connectStatus === ConnectStatus.connected ? 'Connected' : 'Disconnected'}
      <IconButton
        onClick={() => {
          setOpenDetail(!openDetail);
        }}
      >
        {openDetail ? <ExpandLess color="inherit" /> : <ExpandMore color="inherit" />}
      </IconButton>
      <DetailContainer hidden={!openDetail}>
        {connectStatus === ConnectStatus.connected ? (
          <ConfigContainer>
            <PropertyContainer>
              <div>IP:</div>
              <div>{ipAddress}</div>
            </PropertyContainer>
            <PropertyContainer>
              <div>Port:</div>
              <div>{port}</div>
            </PropertyContainer>
            <PropertyContainer>
              <div>Node:</div>
              <div>{connectedNode?.address.name}</div>
            </PropertyContainer>
          </ConfigContainer>
        ) : (
          <ConfigContainer>
            <PropertyContainer>
              <div>IP:</div>
              <CustomTextField
                variant="standard"
                required
                id="ws-editor-ip"
                type="url"
                inputProps={{ maxLength: 15 }}
                sx={{ width: 150 }}
                value={ipAddress}
                onChange={(e) => {
                  setIpAddress(e.target.value);
                }}
              />
            </PropertyContainer>
            <PropertyContainer>
              <div>Port:</div>
              <CustomTextField
                variant="standard"
                required
                id="ws-editor-port"
                type="number"
                sx={{ width: 150 }}
                InputProps={{
                  inputProps: { min: 0, max: 65535 },
                }}
                value={port}
                onChange={(e) => {
                  setPort(+e.target.value);
                }}
              />
            </PropertyContainer>
          </ConfigContainer>
        )}
      </DetailContainer>
    </ConnectPageContainer>
  );
}
