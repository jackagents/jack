import { Belief } from 'misc/types/cbdi/cbdi-models-non-flatbuffer';
import { styled } from '@mui/material';
import React from 'react';
import { JsonToTable } from 'react-json-to-table';
import { convertNestedArrayToObjects } from 'misc/utils/common/commonUtils';

const Root = styled('div')({
  '&&& .json-to-table tr:nth-of-type(even)': {
    backgroundColor: 'transparent',
  },
});

function AgentBeliefSetsView({ beliefSets }: { beliefSets: Belief | undefined }) {
  const processedBeliefSetsJson = React.useMemo(() => {
    if (beliefSets !== undefined || beliefSets !== null) {
      return convertNestedArrayToObjects(beliefSets);
    }
    return {};
  }, [beliefSets]);

  if (beliefSets === undefined) return null;
  return (
    <Root>
      <JsonToTable json={processedBeliefSetsJson} />
    </Root>
  );
}

export default AgentBeliefSetsView;
