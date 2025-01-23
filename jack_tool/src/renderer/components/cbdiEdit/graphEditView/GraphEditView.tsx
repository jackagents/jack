import React from 'react';
import { Fluid } from 'components/common/base/BaseContainer';
import { IconButton, styled } from '@mui/material';
import Placeholder from 'components/common/placeholder/Placeholder';
import { getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { Allotment } from 'allotment';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { useDispatch, useSelector } from 'react-redux';
import { ArrowBack, ArrowForward } from '@mui/icons-material';
import { isModuleConceptOverview } from 'misc/utils/common/commonUtils';
import { RootState } from 'projectRedux/Store';
import { CBDIEditorProject } from 'misc/types/cbdiEdit/cbdiEditModel';
import { ModuleConcept, Mod, CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import Detail from '../detail/Detail';
import Konvas from '../konvas/Konvas';

/* --------------------------------- Styles --------------------------------- */
const TabRow = styled('div')({
  width: '100%',
  height: 30,
  backgroundColor: '#252525',
});

const Tab = styled('div')({
  display: 'inline-block',
  textAlign: 'center',
  height: 28,
  minWidth: 100,
  fontSize: 12,
  fontWeight: 'lighter',
  lineHeight: '30px',
  backgroundColor: '#202020',
  color: '#a6a6a6',
  padding: '0 1em',
});

const Content = styled('div')({
  position: 'absolute',
  inset: '30px 0 0 0',
});
/* ------------------------------ GraphEditView ----------------------------- */

function GraphEditView() {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const selectedTreeNodeConceptStackActiveIndex = useSelector(
    (state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.selectedTreeNodeConceptStackActiveIndex,
  );
  const selectedTreeNodeConcept = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.selectedTreeNodeConcept);
  const selectedGraphConcept = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.graph.selectedNodeConcept);
  const selectedTreeNodeConceptStack = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.selectedTreeNodeConceptStack);
  const { saved } = useSelector((state: RootState) => state.cbdiEdit.cbdiEditSavedProject);
  const dispatch = useDispatch();
  const { popSelectedTreeNodeConceptStack, pushSelectedTreeNodeConceptStack, setScrollToSelectedTreeNodeFlag } = cbdiEditActions;

  /* ------------------------------ useMemo hooks ----------------------------- */
  const backActive = React.useMemo(() => selectedTreeNodeConceptStackActiveIndex > 0, [selectedTreeNodeConceptStackActiveIndex]);
  const forwardActive = React.useMemo(
    () => selectedTreeNodeConceptStackActiveIndex < selectedTreeNodeConceptStack.length - 1,
    [selectedTreeNodeConceptStack, selectedTreeNodeConceptStackActiveIndex],
  );

  const minDetailWidth = React.useMemo(() => {
    const selectedGraphObj = getObjectByModuleConcept(current, selectedGraphConcept);
    if (!selectedGraphObj) {
      return 300;
    }
    switch (selectedGraphObj._objectType) {
      case CBDIEditorRootConceptType.TeamConceptType:
      case CBDIEditorRootConceptType.AgentConceptType:
      case CBDIEditorRootConceptType.RoleConceptType:
      case CBDIEditorRootConceptType.PlanConceptType:
      case CBDIEditorRootConceptType.ResourceConceptType:
      case CBDIEditorRootConceptType.EntityConceptType:
      case CBDIEditorRootConceptType.EventConceptType:
      case CBDIEditorRootConceptType.EnumConceptType:
        return 450;

      case CBDIEditorRootConceptType.ServiceConceptType:
      case CBDIEditorRootConceptType.MessageConceptType:
      case CBDIEditorRootConceptType.GoalConceptType:
      case CBDIEditorRootConceptType.ActionConceptType:
        return 450;

      default:
        return 400;
    }
  }, [selectedGraphConcept]);

  /* -------------------------------- Functions ------------------------------- */
  const getCbdiObjectNameByReferConcept = (mproject: CBDIEditorProject, referConcept: ModuleConcept) => {
    const obj = getObjectByModuleConcept(mproject, referConcept);
    if (obj) {
      return obj.name;
    }
    return '';
  };
  if (!current || !saved || !selectedTreeNodeConcept) {
    return (
      <Fluid style={{ backgroundColor: '#202020' }}>
        <Placeholder label="Welcome" />
      </Fluid>
    );
  }

  const object = getObjectByModuleConcept(current, selectedTreeNodeConcept);

  /* ------------------------------- Components ------------------------------- */
  if (!object && !isModuleConceptOverview(selectedTreeNodeConcept)) {
    return (
      <Fluid style={{ backgroundColor: '#202020' }}>
        <Placeholder label="Welcome" />
      </Fluid>
    );
  }

  const color = (() => {
    if (object) {
      switch (object._mod) {
        case Mod.Addition: {
          return '#4fd165';
        }
        case Mod.Deletion: {
          return '#d40b0b';
        }
        case Mod.Update: {
          return '#e09e58';
        }
        default: {
          return 'white';
        }
      }
    }
    return 'white';
  })();

  return (
    <Fluid>
      <TabRow style={{ gap: 30 }}>
        <IconButton
          disabled={!backActive}
          title="back"
          sx={{
            p: '5px',
            color: 'white',
            ':disabled': { color: 'gray', opacity: 0.5 },
          }}
          onClick={() => {
            dispatch(popSelectedTreeNodeConceptStack());
            dispatch(setScrollToSelectedTreeNodeFlag());
          }}
        >
          <ArrowBack />
        </IconButton>
        <IconButton
          disabled={!forwardActive}
          title="forward"
          sx={{
            p: '5px',
            color: 'white',
            ':disabled': { color: 'gray', opacity: 0.5 },
          }}
          onClick={() => {
            dispatch(pushSelectedTreeNodeConceptStack());
            dispatch(setScrollToSelectedTreeNodeFlag());
          }}
        >
          <ArrowForward />
        </IconButton>
        <Tab
          style={{
            color,
            fontStyle: color !== 'white' ? 'italic' : 'normal',
          }}
        >
          {object
            ? getCbdiObjectNameByReferConcept(current, selectedTreeNodeConcept) + (object._mod === Mod.Deletion ? ' (missing)' : '')
            : 'overview'}
        </Tab>
      </TabRow>
      <Content>
        <Allotment defaultSizes={[80, 20]}>
          <Allotment.Pane preferredSize="80%">
            <Konvas />
          </Allotment.Pane>
          <Allotment.Pane minSize={minDetailWidth} maxSize={600}>
            <Detail />
          </Allotment.Pane>
        </Allotment>
      </Content>
    </Fluid>
  );
}

export default React.memo(GraphEditView);
