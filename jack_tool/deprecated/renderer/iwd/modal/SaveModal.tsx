import { Button, styled } from '@mui/material';
import { useSnackbar } from 'notistack';
import React from 'react';
import { BuildMode } from 'types/iwd/iwdTypes';
import { useDispatch } from 'react-redux';

import {
  iwdClientSlice,
  IwdClientState,
} from 'projectRedux/reducers/iwd/iwdClientReducer';
import {
  useSliceActions,
  useSliceSelector,
} from 'projectRedux/sliceProvider/SliceProvider';
import Modal from 'components/common/modal/Modal';

const StyledBtn = styled(Button)(({ theme }) => ({
  color: theme.custom?.text.color,
}));

function SaveModal() {
  const { enqueueSnackbar } = useSnackbar();

  const { currentBuildMode, dragMode } = useSliceSelector() as IwdClientState;

  const { toggleDragMode, setSaveMode, changeBuildMode } =
    useSliceActions() as typeof iwdClientSlice.actions;

  const dispatch = useDispatch();

  const handleClick = React.useCallback(
    ({ target }: React.MouseEvent<HTMLButtonElement>) => {
      switch ((target as HTMLButtonElement).id) {
        case 'drag-mode': {
          dispatch(toggleDragMode(!dragMode));
          break;
        }

        case 'save': {
          dispatch(setSaveMode(currentBuildMode));
          dispatch(changeBuildMode(BuildMode.NONE));
          enqueueSnackbar('Saved', { variant: 'success' });
          break;
        }

        case 'cancel': {
          enqueueSnackbar('Canceled', { variant: 'info' });
          dispatch(changeBuildMode(BuildMode.NONE));
          break;
        }
        default:
          break;
      }
    },
    [dragMode, currentBuildMode]
  );

  const elements = React.useMemo(() => {
    if (currentBuildMode === BuildMode.ZONE) {
      return (
        <StyledBtn key="dragmode" id="drag-mode" onClick={handleClick}>
          {dragMode ? 'Drag Mode: On' : 'Drag Mode: Off'}
        </StyledBtn>
      );
    }

    return undefined;
  }, [currentBuildMode, dragMode, handleClick]);

  return <Modal onClick={handleClick} elements={elements} />;
}

export default React.memo(SaveModal);
