import React, { SyntheticEvent } from 'react';
import { styled } from '@mui/material';
import { request, response } from 'misc/events/common/cmEvents';
import { ConnectStatus } from 'misc/constant/common/cmConstants';
import { useExplainabilityContext } from '../../context/explainabilityContext';
import ProgressBar from '../ProgressBar';
import WSConnectPage from '../wsConnectPage/WSConnectPage';

/* --------------------------------- Styles --------------------------------- */

const Root = styled('div')({
  flexGrow: 1,
  display: 'flex',
  alignItems: 'center',
  gap: 50,
});

const DetailContainer = styled('div')({
  display: 'flex',
  alignItems: 'center',
  borderRadius: 5,
});

const LiveControl = () => {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { project, connectStatus, debugMode, isLivePlayback } =
    useExplainabilityContext();

  /* ----------------------------- useState hooks ----------------------------- */
  const [isLive, setIsLive] = React.useState(true);
  const [isPlaying, setIsPlaying] = React.useState(false);
  const [isHoldingProgress, setIsHoldingProgress] = React.useState(false);
  const [maxProgress, setMaxProgress] = React.useState(0);
  const [progress, setProgress] = React.useState(0);
  const [fixedProgressPercentage, setFixedProgressPercentage] = React.useState<
    number | undefined
  >(undefined);
  // Live websocket server is stale/pause
  const [isLiveStale, setIsLiveStale] = React.useState(false);

  /* -------------------------------- Callbacks ------------------------------- */
  const resetPlayControlStates = () => {
    setIsPlaying(false);
    setIsLive(true);
    setMaxProgress(0);
    setProgress(0);
  };

  const togglePlay = () => {
    if (isPlaying === false) {
      window.ipcRenderer.send(request.playback.play);
    } else {
      if (isLive) {
        setIsLive(false);
      }
      window.ipcRenderer.send(request.playback.pause);
    }
    setIsPlaying((prevPlaying) => !prevPlaying);
  };

  const handleResetToLive = () => {
    setIsLive(true);
    setProgress(maxProgress);
    setFixedProgressPercentage(undefined);
    window.ipcRenderer.send(request.playback.goLive);
  };

  const handleProgressChange = React.useCallback(
    (event: Event, newValue: number | number[]) => {
      if (typeof newValue === 'number') {
        setIsLive(false);
        setProgress(newValue);
        setIsHoldingProgress(true);
        if (!isLiveStale) {
          setFixedProgressPercentage(newValue / maxProgress);
        }
      }
    },
    [isLiveStale, maxProgress]
  );

  const handleProgressChangeCommited = (
    event: Event | SyntheticEvent<Element, Event>,
    newValue: number | number[]
  ) => {
    if (typeof newValue === 'number') {
      setIsHoldingProgress(false);
      window.ipcRenderer.send(request.playback.goToFrame, newValue);
    }
  };

  const handleGoToLiveSuccess = () => {
    setIsPlaying(true);
    window.ipcRenderer.invoke(request.cbdi.discoverModels);
  };
  /* ------------------------------- useCallback hooks ------------------------------ */

  const handlePlaybackGoToFrameSuccess = React.useCallback(() => {
    window.ipcRenderer.invoke(request.cbdi.discoverModels);
    if (isPlaying) {
      window.ipcRenderer.send(request.playback.play);
    }
    if (isLive) {
      setIsLive(false);
      setIsPlaying(true);
    }
  }, [isPlaying, isLive]);

  const handleFeedLiveProgress = React.useCallback(
    (_event: Electron.IpcRendererEvent, length: number) => {
      // if length is undefined, means it fails to load playback file
      if (length !== undefined && !isHoldingProgress) {
        setMaxProgress(length - 1);
        if (isLive) {
          // setProgress(length - 1);
          setIsPlaying(true);
        }
      }

      setIsLiveStale(false);
    },
    [isLive, isHoldingProgress]
  );

  const handlePlaybackTick = React.useCallback(
    (_event: Electron.IpcRendererEvent, index: number) => {
      if (!isHoldingProgress) {
        setProgress(index);
      }
    },
    [isHoldingProgress]
  );

  const handleStale = React.useCallback(() => {
    setIsLiveStale(true);
    setFixedProgressPercentage(undefined);
  }, []);

  const handlePause = () => {
    setIsPlaying(false);
  };

  /* ------------------------------ useMemo hooks ----------------------------- */
  const playControlDisabled = React.useMemo(() => {
    if (!isLivePlayback) {
      return true;
    }
    if (debugMode) {
      return true;
    }
    if (
      project !== null &&
      connectStatus === ConnectStatus.connected &&
      maxProgress > 0
    ) {
      return false;
    }

    return true;
  }, [connectStatus, project, maxProgress, isLivePlayback, debugMode]);
  /* ----------------------------- useEffect hooks ---------------------------- */

  React.useEffect(() => {
    const removeGoToLiveSuccessListener = window.ipcRenderer.setupIpcListener(
      response.playback.goLive,
      handleGoToLiveSuccess
    );

    const liveModeStaleCleanup = window.ipcRenderer.setupIpcListener(
      response.playback.stale,
      handleStale
    );

    const commandFrontendPauseCleanup = window.ipcRenderer.setupIpcListener(
      response.playback.commandFrontendPause,
      handlePause
    );

    return () => {
      removeGoToLiveSuccessListener();
      liveModeStaleCleanup();
      commandFrontendPauseCleanup();
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
  }, [isLive, isPlaying]);

  React.useEffect(() => {
    const removePlaybackTickListener = window.ipcRenderer.setupIpcListener(
      response.playback.tick,
      handlePlaybackTick
    );

    const removeFeedLiveProgressListener = window.ipcRenderer.setupIpcListener(
      response.playback.feedLiveProgress,
      handleFeedLiveProgress
    );

    return () => {
      removeFeedLiveProgressListener();
      removePlaybackTickListener();
    };
  }, [isLive, isHoldingProgress]);

  React.useEffect(() => {
    resetPlayControlStates();
  }, [project]);

  React.useEffect(() => {
    if (connectStatus === ConnectStatus.disconnected) {
      resetPlayControlStates();
    }
  }, [connectStatus]);

  /**
   * when progress catch up maxProgress
   * if it is not live,
   * and it is playing or it is not in debugMode
   * reset to live mode
   */
  React.useEffect(() => {
    if (isPlaying && !isLive && progress === maxProgress) {
      handleResetToLive();
    }
  }, [progress, maxProgress, isPlaying, isLive]);

  React.useEffect(() => {
    if (!isLive && !isLiveStale) {
      setFixedProgressPercentage(progress / maxProgress);
    }
  }, [isLiveStale]);

  React.useEffect(() => {
    // make isLive false when change to debugMode
    setIsLive((prev) => {
      if (prev && debugMode) {
        return false;
      }
      return prev;
    });
    if (debugMode) {
      // pause play when change to debugMode
      setIsPlaying(false);
    }
  }, [debugMode]);

  return (
    <Root>
      <WSConnectPage />
      <DetailContainer style={{ flexGrow: 1 }}>
        <ProgressBar
          isPlaying={isPlaying}
          isLive={isLive}
          progress={progress}
          maxProgress={maxProgress}
          fixedProgressPercentage={fixedProgressPercentage}
          playControlDisabled={playControlDisabled}
          togglePlay={togglePlay}
          handleResetToLive={handleResetToLive}
          handleProgressChange={handleProgressChange}
          handleProgressChangeCommited={handleProgressChangeCommited}
        />
      </DetailContainer>
      <div>{isLive ? 'Live' : undefined}</div>
    </Root>
  );
};

export default LiveControl;
