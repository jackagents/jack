import { Allotment } from 'allotment';
import ExplorerProject from 'components/cbdiEdit/explorerProject/ExplorerProject';
import MainPanelProject from 'components/cbdiEdit/mainPanelProject/MainPanelProject';
import WelcomePage from 'components/cbdiEdit/welcomePage/WelcomePage';
import { useDispatch, useSelector } from 'react-redux';
import { RootState } from 'projectRedux/Store';
import React from 'react';
import { CBDI_REDO_TYPE, CBDI_UNDO_TYPE } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { useTheme } from '@mui/material';
import CbdiEditReactflowContextProvider from 'components/cbdiEdit/CbdiEditReactflowContext/CbdiEditReactflowContext';

export default function Project() {
  /* ----------------------------- useTheme hooks ----------------------------- */
  const theme = useTheme();
  /* ----------------------------- Redux ----------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);

  const saved = useSelector((state: RootState) => state.cbdiEdit.cbdiEditSavedProject.saved);

  const pastLength = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.past.length);

  const dispatch = useDispatch();

  const handleKeyPress = React.useCallback(
    (e: KeyboardEvent) => {
      if (e.ctrlKey === true) {
        switch (e.key.toLowerCase()) {
          case 'z':
            if (pastLength > 1) {
              dispatch({ type: CBDI_UNDO_TYPE });
            }
            break;
          case 'y':
            dispatch({ type: CBDI_REDO_TYPE });
            break;
          default:
            break;
        }
      }
    },
    [pastLength],
  );

  // change allotment separator border color base on theme
  React.useEffect(() => {
    document.documentElement.style.setProperty('--separator-border', theme.editor.detailView.disableColor);
  }, [theme.editor.detailView.disableColor]);

  React.useEffect(() => {
    document.addEventListener('keydown', handleKeyPress);
    return () => {
      document.removeEventListener('keydown', handleKeyPress);
    };
  }, [handleKeyPress]);

  if (!current || !saved) {
    return <WelcomePage />;
  }

  return (
    <CbdiEditReactflowContextProvider>
      <Allotment defaultSizes={[20, 80]}>
        <Allotment.Pane preferredSize="20%" minSize={300} maxSize={500}>
          <ExplorerProject />
        </Allotment.Pane>
        <Allotment.Pane>
          <MainPanelProject key="main-panel-project" />
        </Allotment.Pane>
      </Allotment>
    </CbdiEditReactflowContextProvider>
  );
}
