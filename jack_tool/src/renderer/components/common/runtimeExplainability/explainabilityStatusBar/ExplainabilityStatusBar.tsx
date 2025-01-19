import React from 'react';
import { Button, styled } from '@mui/material';
import { ConnectMode, ConnectStatus } from 'misc/constant/common/cmConstants';
import { request } from 'misc/events/common/cmEvents';
import Select from 'react-select';
import { useExplainabilityContext } from '../context/explainabilityContext';
import PlaybackControl from './playbackControl/PlaybackControl';
import LiveControl from './liveControl/LiveControl';

/* --------------------------------- Styles --------------------------------- */

const StatusContainer = styled('div')(
  ({ isconnected }: { isconnected: number }) => ({
    height: 60,
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'space-between',
    backgroundColor: isconnected ? '#2a9d8f' : '#e76f51',
    padding: '5px 50px',
    gap: 60,
  })
);

const ModeContainer = styled('div')({
  display: 'flex',
  alignItems: 'center',
  padding: '5px 0',
  gap: 20,
});

const ResetButton = styled(Button)({
  minWidth: 'auto',
  backgroundColor: '#333',
  color: 'white',
  borderRadius: 5,
  '&:hover': {
    backgroundColor: '#555',
  },
});

/* -------------------------------- Contants -------------------------------- */
interface ModeOption {
  value: number;
  label: string;
}
const ModeOptions: ModeOption[] = [
  { value: ConnectMode.playback, label: 'Playback' },
  { value: ConnectMode.live, label: 'Live' },
];

const ExplainabilityStatusBar = () => {
  /* ---------------------------- useContext hooks ---------------------------- */
  const {
    connectStatus,
    connectMode,
    setConnectStatus,
    setConnectMode,
    setAgentTreeGraphResetFlag,
    resetExplainabilityViewSelection,
  } = useExplainabilityContext();

  /* -------------------------------- Callbacks ------------------------------- */

  const handleReset = () => {
    resetExplainabilityViewSelection();
    setAgentTreeGraphResetFlag((prev) => {
      return !prev;
    });
    window.ipcRenderer.invoke(request.cbdi.reset);
  };

  const handleChangeMode = (newValue: unknown) => {
    setConnectMode((newValue as ModeOption).value);
    // reset the view
    handleReset();
    // disconnect websock connection
    setConnectStatus(ConnectStatus.disconnected);
  };

  /* ------------------------------ useMemo hooks ----------------------------- */
  const value = React.useMemo(() => {
    return ModeOptions.find((el) => el.value === connectMode);
  }, [connectMode]);
  return (
    <StatusContainer
      key="status-container"
      id="status-container"
      isconnected={connectStatus === ConnectStatus.connected ? 1 : 0}
    >
      <div
        style={{ flexGrow: 1, display: 'flex', alignItems: 'center', gap: 50 }}
      >
        <ModeContainer>
          <div>Mode:</div>
          <Select
            isSearchable={false}
            value={value}
            options={ModeOptions}
            onChange={handleChangeMode}
            styles={{
              menuPortal: (base) => ({ ...base, zIndex: 9999 }),
              container: (base) => ({ ...base, fontSize: 14, width: 120 }),
            }}
          />
        </ModeContainer>
        {connectMode === ConnectMode.live ? (
          <LiveControl />
        ) : (
          <PlaybackControl />
        )}
      </div>
      {connectMode === ConnectMode.live ? (
        <ResetButton onClick={handleReset}>Reset</ResetButton>
      ) : null}
    </StatusContainer>
  );
};

export default ExplainabilityStatusBar;
