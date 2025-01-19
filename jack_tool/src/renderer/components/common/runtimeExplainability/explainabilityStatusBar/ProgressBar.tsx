import React, { SyntheticEvent } from 'react';
import Slider from '@mui/material/Slider';
import PlayArrowIcon from '@mui/icons-material/PlayArrow';
import PauseIcon from '@mui/icons-material/Pause';
import ReplayIcon from '@mui/icons-material/Replay';
import LiveIcon from '@mui/icons-material/Adjust';
import { IconButton, styled } from '@mui/material';
import { secondsToMinutes } from 'misc/utils/common/rendererUtils';
import { ConnectMode } from 'misc/constant/common/cmConstants';
import { useExplainabilityContext } from '../context/explainabilityContext';

const ProgressBarContainer = styled('div')({
  flexGrow: 1,
  display: 'flex',
  alignItems: 'center',
  gap: 8,
});

interface Props {
  isPlaying: boolean;
  isLive?: boolean;
  progress: number;
  maxProgress: number;
  playControlDisabled: boolean;
  fixedProgressPercentage?: number;
  togglePlay: () => void;
  handleReset?: () => void;
  handleResetToLive?: () => void;
  handleProgressChange: (event: Event, newValue: number | number[]) => void;
  handleProgressChangeCommited: (
    event: Event | SyntheticEvent<Element, Event>,
    newValue: number | number[]
  ) => void;
}

const ProgressBar = ({
  isPlaying,
  isLive,
  progress,
  maxProgress,
  playControlDisabled,
  fixedProgressPercentage,
  togglePlay,
  handleReset,
  handleResetToLive,
  handleProgressChange,
  handleProgressChangeCommited,
}: Props) => {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { connectMode } = useExplainabilityContext();

  /* ------------------------------ useMemo hooks ----------------------------- */
  const displayProgress = React.useMemo(() => {
    if (connectMode === ConnectMode.live && isLive) {
      return maxProgress;
    }
    if (
      connectMode === ConnectMode.playback ||
      fixedProgressPercentage === undefined
    ) {
      return progress;
    }
    return fixedProgressPercentage * maxProgress;
  }, [progress, connectMode, fixedProgressPercentage, maxProgress, isLive]);
  return (
    <ProgressBarContainer>
      <IconButton
        color="primary"
        disabled={playControlDisabled}
        onClick={togglePlay}
        title={isPlaying ? 'pause' : 'play'}
      >
        {isPlaying ? <PauseIcon /> : <PlayArrowIcon />}
      </IconButton>
      <Slider
        disabled={playControlDisabled}
        max={maxProgress}
        value={displayProgress}
        onChange={handleProgressChange}
        onChangeCommitted={handleProgressChangeCommited}
        valueLabelDisplay="on"
        valueLabelFormat={secondsToMinutes(progress)}
        sx={{
          color: 'black',
          width: '100%',
          position: 'relative',
          '& .MuiSlider-thumb': {
            cursor: 'grab',
            '&:focus, &:hover, &.Mui-active, &.Mui-focusVisible': {
              boxShadow: 'inherit',
            },
            '&:before': {
              display: 'none',
            },
          },
          '& .MuiSlider-valueLabel': {
            color: 'black',
            fontSize: 12,
            fontWeight: 'normal',
            position: 'absolute',
            top: 44,
            backgroundColor: 'unset',
            '&:before': {
              display: 'none',
            },
            '& *': {
              background: 'transparent',
            },
          },
        }}
      />
      {connectMode === ConnectMode.live ? (
        <IconButton
          disabled={playControlDisabled || isLive}
          onClick={handleResetToLive}
          style={{
            color: isLive && !playControlDisabled ? 'red' : 'gray',
          }}
        >
          <LiveIcon />
        </IconButton>
      ) : null}
      {connectMode === ConnectMode.playback ? (
        <IconButton
          color="secondary"
          disabled={playControlDisabled}
          onClick={handleReset}
          title="reset"
        >
          <ReplayIcon />
        </IconButton>
      ) : null}
    </ProgressBarContainer>
  );
};

export default ProgressBar;
