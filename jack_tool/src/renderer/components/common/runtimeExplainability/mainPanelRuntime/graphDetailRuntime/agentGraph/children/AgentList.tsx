import React from 'react';
import { styled } from '@mui/material';
import { areObjectsEqual } from 'misc/utils/common/commonUtils';
import ExpandableContainer from 'components/common/expandableContainer/ExpandableContainer';
import { CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { CustomNodeData, useExplainabilityContext } from '../../../../context/explainabilityContext';
import { Node } from '../convertNodesEdgesToTree';

/* --------------------------------- Styles --------------------------------- */

const ListContainer = styled('div')({
  display: 'flex',
  flexDirection: 'column',
  gap: 20,
  overflow: 'auto',
});

const ListItem = styled('div')({
  padding: 10,
  cursor: 'pointer',
  '&.selected': {
    backgroundColor: '#808080',
  },
});

export function AgentList({ nodes }: { nodes: Node[] }) {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { inspectNodeData, setInspectNodeData } = useExplainabilityContext();

  /* ------------------------------ useMemo hooks ----------------------------- */
  const agentTeamNodes = React.useMemo(
    () =>
      nodes
        .filter((el) => {
          if (el.data.type === CBDIEditorRootConceptType.AgentConceptType || el.data.type === CBDIEditorRootConceptType.TeamConceptType) {
            return true;
          }
          return false;
        })
        .sort((a: Node, b: Node) => a.data.agentBusAddress!.name.localeCompare(b.data.agentBusAddress!.name)),
    [nodes],
  );

  /* -------------------------------- Callbacks ------------------------------- */

  const handleListItemClick = (data: CustomNodeData) => {
    if (areObjectsEqual(data, inspectNodeData)) {
      setInspectNodeData(undefined);
    } else {
      setInspectNodeData(data);
    }
  };

  return (
    <ExpandableContainer
      customStyles={{
        top: 100,
        left: 0,
        width: 300,
        height: 500,
        gap: 10,
        paddingTop: 10,
      }}
      buttonTitle="Agent List"
    >
      <div style={{ textAlign: 'center' }}>Agent List</div>
      <ListContainer>
        {agentTeamNodes.map((el, index) => (
          <ListItem
            key={index as number}
            className={areObjectsEqual(el.data, inspectNodeData) ? 'selected' : undefined}
            onClick={() => {
              handleListItemClick(el.data);
            }}
          >
            {el.data.agentBusAddress?.name}
          </ListItem>
        ))}
      </ListContainer>
    </ExpandableContainer>
  );
}
