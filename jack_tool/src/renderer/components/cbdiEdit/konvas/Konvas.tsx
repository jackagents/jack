/* eslint-disable no-bitwise */
import React from 'react';
import { styled } from '@mui/material';
import { Fluid } from 'components/common/base/BaseContainer';
import { getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { useSelector } from 'react-redux';
import { RootState } from 'projectRedux/Store';
import { CBDIEditorObject, CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import GraphWidgets from './children/GraphWidgets';
import CbdiEditTreeGraph from '../cbdiEditTreeGraph/CbdiEditTreeGraph';
import ReactFlowPlanEditor from '../reactFlowPlanEditor/ReactFlowPlanEditor';

/* --------------------------------- Styles --------------------------------- */
const Root = styled(Fluid)({
  overflow: 'hidden',
  color: 'white',
});

/* --------------------------------- Konvas --------------------------------- */

function Konvas() {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const selectedTreeNodeConcept = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.selectedTreeNodeConcept);

  /* -------------------------------- useMemo hooks ------------------------------- */

  const object = React.useMemo(() => {
    const mobject: CBDIEditorObject | undefined = getObjectByModuleConcept(current, selectedTreeNodeConcept);

    return mobject;
  }, [selectedTreeNodeConcept, current]);

  return (
    <Root>
      {object?._objectType === CBDIEditorRootConceptType.PlanConceptType ? (
        <ReactFlowPlanEditor key={selectedTreeNodeConcept?.uuid} />
      ) : (
        <CbdiEditTreeGraph key={selectedTreeNodeConcept?.uuid} />
      )}

      <GraphWidgets />
    </Root>
  );
}

export default React.memo(Konvas);
