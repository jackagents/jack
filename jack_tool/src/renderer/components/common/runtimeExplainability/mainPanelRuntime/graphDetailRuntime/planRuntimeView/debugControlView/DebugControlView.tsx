import { useEffect, useState } from 'react';
import IconButton from '@mui/material/IconButton';
import { styled } from '@mui/material';
import PlayArrowIcon from '@mui/icons-material/PlayArrow';
import PauseIcon from '@mui/icons-material/Pause';
import RedoIcon from '@mui/icons-material/Redo';
import LinkIcon from '@mui/icons-material/Link';
import LinkOffIcon from '@mui/icons-material/LinkOff';
import { request } from 'misc/events/common/cmEvents';
import { useExplainabilityContext } from 'components/common/runtimeExplainability/context/explainabilityContext';

/* --------------------------------- Styles --------------------------------- */
const Root = styled('div')({
  display: 'flex',
  gap: 5,
});

function DebugControlView() {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { debugMode, project, connectStatus, setDebugMode } = useExplainabilityContext();
  /* ----------------------------- useState hooks ----------------------------- */
  const [autoPlay, setAutoPlay] = useState(false);
  /* -------------------------------- Callbacks ------------------------------- */
  const handleToggleDebugMode = () => {
    if (debugMode) {
      setDebugMode(false);
      setAutoPlay(false);
      window.ipcRenderer.send(request.playback.debug.exit);
    } else {
      setDebugMode(true);
      window.ipcRenderer.send(request.playback.debug.enter);
    }
  };

  const handleToggleAutoPlay = () => {
    if (autoPlay) {
      window.ipcRenderer.send(request.playback.debug.autoPlayPause);
    } else {
      window.ipcRenderer.send(request.playback.debug.autoPlay);
    }
    setAutoPlay(!autoPlay);
  };

  const handleNext = () => {
    window.ipcRenderer.send(request.playback.debug.stepNext);
  };
  /* ----------------------------- useEffect hooks ---------------------------- */
  useEffect(() => {
    setAutoPlay(false);
  }, [connectStatus, project]);

  return (
    <Root>
      <IconButton color={debugMode ? 'error' : 'success'} onClick={handleToggleDebugMode} title={debugMode ? 'exit trace' : 'start trace'}>
        {debugMode ? <LinkOffIcon /> : <LinkIcon />}
      </IconButton>
      <IconButton disabled={!debugMode || autoPlay} color="info" onClick={handleNext} title="next task">
        <RedoIcon />
      </IconButton>
      <IconButton
        disabled={!debugMode}
        color={autoPlay ? 'warning' : 'success'}
        onClick={handleToggleAutoPlay}
        title={autoPlay ? 'pause' : 'auto play'}
      >
        {autoPlay ? <PauseIcon /> : <PlayArrowIcon />}
      </IconButton>
    </Root>
  );
}

export default DebugControlView;
