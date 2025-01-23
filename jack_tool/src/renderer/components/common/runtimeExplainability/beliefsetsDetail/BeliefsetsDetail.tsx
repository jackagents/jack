import { Fluid } from 'components/common/base/BaseContainer';
import { styled } from '@mui/material';
import { CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import AgentDetailJsonView from './agentDetailJsonView/AgentDetailJsonView';
import { useExplainabilityContext } from '../context/explainabilityContext';
import DelegationDetailJsonView from './delegationDetailJsonView/DelegationDetailJsonView';

/* --------------------------------- Styles --------------------------------- */
const Root = styled(Fluid)({
  display: 'flex',
  flexDirection: 'column',
});

const AgentDetailContainer = styled('div')({
  flexGrow: 1,
  overflow: 'auto',
});

const PlaceholderContainer = styled('div')({
  display: 'flex',
  justifyContent: 'center',
  alignItems: 'center',
  width: '100%',
  height: '100%',
  borderRadius: '4px',
  color: 'gray',
});

/* ---------------------------- BeliefsetsDetail ---------------------------- */
export default function BeliefsetsDetail() {
  const { inspectNodeData } = useExplainabilityContext();

  return (
    <Root>
      <AgentDetailContainer>
        {(inspectNodeData?.type ===
          CBDIEditorRootConceptType.AgentConceptType ||
          inspectNodeData?.type ===
            CBDIEditorRootConceptType.TeamConceptType) && (
          <AgentDetailJsonView />
        )}

        {inspectNodeData?.type ===
          CBDIEditorRootConceptType.GoalConceptType && (
          <DelegationDetailJsonView />
        )}

        {!inspectNodeData && (
          <PlaceholderContainer>No Agent/Goal selected</PlaceholderContainer>
        )}
      </AgentDetailContainer>
    </Root>
  );
}
