import React, { SyntheticEvent } from 'react';
import { IconButton, styled } from '@mui/material';
import { FileOpen } from '@mui/icons-material';
import { request, response } from 'misc/events/common/cmEvents';
import { ConnectStatus } from 'misc/constant/common/cmConstants';
import { secondsToMinutes } from 'misc/utils/common/rendererUtils';
import { useExplainabilityContext } from '../../context/explainabilityContext';
import ProgressBar from '../ProgressBar';

/* --------------------------------- Styles --------------------------------- */

const Root = styled('div')({
  flexGrow: 1,
  display: 'flex',
  alignItems: 'center',
  gap: 50,
});

const ModelLoadContainer = styled('div')({
  display: 'flex',
  alignItems: 'center',
  gap: 5,
  width: 300,
});

const DetailContainer = styled('div')({
  display: 'flex',
  alignItems: 'center',
  borderRadius: 5,
});

const PlaybackControl = () => {
  /* ---------------------------- useContext hooks ---------------------------- */
  const {
    project,
    debugMode,
    setConnectStatus,
    setAgentTreeGraphResetFlag,
    resetExplainabilityViewSelection,
  } = useExplainabilityContext();
  /* ----------------------------- useState hooks ----------------------------- */
  const [isPlaying, setIsPlaying] = React.useState(false);
  const [maxProgress, setMaxProgress] = React.useState(0);
  const [progress, setProgress] = React.useState(0);
  const [playbackFilePath, setPlaybackFilePath] = React.useState<string>();
  const [playbackFileName, setPlaybackFileName] = React.useState<string>();

  /* -------------------------------- Callbacks ------------------------------- */
  const resetPlayControlStates = () => {
    setIsPlaying(false);
    setMaxProgress(0);
    setProgress(0);
    setPlaybackFilePath(undefined);
    setPlaybackFileName(undefined);
  };

  const handleClickLoadPlayBackModel = () => {
    window.ipcRenderer.send(request.playback.openFile);
  };

  const handleFinishOpenFile = (
    _event: Electron.IpcRendererEvent,
    filePath: string,
    fileName: string,
    length: number
  ) => {
    // if length is undefined, means it fails to load playback file
    if (
      filePath !== undefined &&
      fileName !== undefined &&
      length !== undefined
    ) {
      resetExplainabilityViewSelection();
      setAgentTreeGraphResetFlag((prev) => {
        return !prev;
      });
      setIsPlaying(false);
      setProgress(0);
      setMaxProgress(length - 1);
      setPlaybackFilePath(filePath);
      setPlaybackFileName(fileName);
      setConnectStatus(ConnectStatus.connected);
    }
  };

  const handlePlaybackTick = (
    _event: Electron.IpcRendererEvent,
    index: number
  ) => {
    setProgress(index);
  };

  const togglePlay = () => {
    if (isPlaying === false) {
      window.ipcRenderer.send(request.playback.play);
    } else {
      window.ipcRenderer.send(request.playback.pause);
    }
    setIsPlaying((prevPlaying) => !prevPlaying);
  };

  const handleReset = () => {
    setProgress(0);
    window.ipcRenderer.send(request.playback.stop);
    resetExplainabilityViewSelection();
    setAgentTreeGraphResetFlag((prev) => {
      return !prev;
    });
    setIsPlaying(false);
  };

  const handleProgressChange = (event: Event, newValue: number | number[]) => {
    if (typeof newValue === 'number') {
      setProgress(newValue);
    }
  };

  const handleProgressChangeCommited = (
    event: Event | SyntheticEvent<Element, Event>,
    newValue: number | number[]
  ) => {
    if (typeof newValue === 'number') {
      window.ipcRenderer.send(request.playback.goToFrame, newValue);
    }
  };
  /* ---------------------------- useCallback hooks --------------------------- */
  const handlePlaybackGoToFrameSuccess = React.useCallback(() => {
    window.ipcRenderer.invoke(request.cbdi.discoverModels);
    if (isPlaying) {
      window.ipcRenderer.send(request.playback.play);
    }
  }, [isPlaying]);

  /* ------------------------------ useMemo hooks ----------------------------- */
  const playControlDisabled = React.useMemo(() => {
    if (project !== null && playbackFileName !== undefined && !debugMode) {
      return false;
    }
    return true;
  }, [playbackFileName, project, debugMode]);
  /* ----------------------------- useEffect hooks ---------------------------- */
  React.useEffect(() => {
    const removePlaybackOpenFileListener = window.ipcRenderer.setupIpcListener(
      response.playback.openFile,
      handleFinishOpenFile
    );

    const removePlaybackTickListener = window.ipcRenderer.setupIpcListener(
      response.playback.tick,
      handlePlaybackTick
    );

    return () => {
      removePlaybackOpenFileListener();
      removePlaybackTickListener();
    };
  }, []);

  React.useEffect(() => {
    const removePlaybackGoToFrameSuccessListener =
      window.ipcRenderer.setupIpcListener(
        response.playback.goToFrame,
        handlePlaybackGoToFrameSuccess
      );
    return () => {
      removePlaybackGoToFrameSuccessListener();
    };
  }, [isPlaying]);

  React.useEffect(() => {
    resetPlayControlStates();
  }, [project]);

  /**
   * when switch to debug mode, pause play back
   */
  React.useEffect(() => {
    if (debugMode) {
      window.ipcRenderer.send(request.playback.pause);
      setIsPlaying(false);
    }
  }, [debugMode]);

  return (
    <Root>
      <ModelLoadContainer>
        <div>Playback:</div>
        <div
          title={playbackFilePath}
          style={{
            flex: 1,
            overflow: 'hidden',
            whiteSpace: 'nowrap',
            textOverflow: 'ellipsis',
          }}
        >
          {playbackFileName || 'N/A'}
        </div>
        <IconButton
          color="inherit"
          title="load cbdi playback model"
          component="label"
          onClick={handleClickLoadPlayBackModel}
        >
          <FileOpen />
        </IconButton>
      </ModelLoadContainer>
      <DetailContainer style={{ flexGrow: 1 }}>
        <ProgressBar
          isPlaying={isPlaying}
          progress={progress}
          maxProgress={maxProgress}
          playControlDisabled={playControlDisabled}
          togglePlay={togglePlay}
          handleReset={handleReset}
          handleProgressChange={handleProgressChange}
          handleProgressChangeCommited={handleProgressChangeCommited}
        />
      </DetailContainer>
      <div>{`${secondsToMinutes(progress)}/${secondsToMinutes(
        maxProgress
      )}`}</div>
    </Root>
  );
};

export default PlaybackControl;
