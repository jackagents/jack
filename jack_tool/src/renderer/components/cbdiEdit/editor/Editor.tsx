import React from 'react';
import { styled } from '@mui/material';
import { Col, Fluid, Row } from 'components/common/base/BaseContainer';
import { MODE_PROJECT, MODE_RUNTIME, MODE_SETTINGS } from 'constant/cbdiEdit/cbdiEditConstant';
import { RootState } from 'projectRedux/Store';
import { useSelector } from 'react-redux';
import SideMenu from './children/SideMenu';
import Settings from './children/Settings';
import StatusBar from '../statusbar/StatusBar';
import RuntimeExplainability from '../../common/runtimeExplainability/RuntimeExplainability';
import Project from './children/Project';
/* --------------------------------- Styles --------------------------------- */
const BodyRow = styled(Row)({
  top: 0,
  bottom: 25,
});

const SideMenuCol = styled(Col)({
  width: 50,
  left: 0,
  zIndex: 0,
});

const SplitViewCol = styled(Col)({
  left: 50,
  right: 0,
  minWidth: 800,
});

const StatusBarRow = styled(Row)({
  height: 25,
  bottom: 0,
});

// Use styled component to set color to black to fix bug text color was changed
// to white in scenario editor
const ScenarioEditorContainer = styled(Fluid)({
  color: 'black',
});

/* --------------------------------- Editor --------------------------------- */
function Editor() {
  /* ---------------------------------- Redux --------------------------------- */
  const mode = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.explorer.mode);

  const xaiLiveDebug = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.xaiLiveDebug);

  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);

  const viewport = React.useMemo(() => {
    switch (mode) {
      case MODE_SETTINGS:
        return (
          <Fluid id="fluid-settings">
            <Settings />
          </Fluid>
        );
      case MODE_PROJECT:
        return (
          <Fluid id="fluid-project">
            <Project key="cbdi-editor-project" />
          </Fluid>
        );
      default:
        return null;
    }
  }, [mode]);

  /* ------------------------------- Components ------------------------------- */
  return (
    <Fluid>
      <BodyRow>
        <SideMenuCol>
          <SideMenu modelLoaded={!!current} />
        </SideMenuCol>

        <SplitViewCol>
          {viewport}

          {/* We want the explainability tab to keep running (without being unloaded) */}
          <Fluid
            style={{
              position: 'relative',
              display: 'flex',
              flexDirection: 'column',
            }}
            hidden={mode !== MODE_RUNTIME}
            id="fluid-runtime"
          >
            <RuntimeExplainability hidden={mode !== MODE_RUNTIME} outProject={current} isLivePlayback={xaiLiveDebug} />
          </Fluid>
        </SplitViewCol>
      </BodyRow>
      <StatusBarRow>
        <StatusBar />
      </StatusBarRow>
    </Fluid>
  );
}

export default React.memo(Editor);
