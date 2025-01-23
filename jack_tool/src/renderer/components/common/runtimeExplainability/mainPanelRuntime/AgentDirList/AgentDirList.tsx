import React from 'react';
import { styled } from '@mui/material';
import { areObjectsEqual } from 'misc/utils/common/commonUtils';
import { CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { CustomNodeData, useExplainabilityContext } from '../../context/explainabilityContext';
import { Node } from '../graphDetailRuntime/agentGraph/convertNodesEdgesToTree';

/* --------------------------------- Styles --------------------------------- */

const ListContainer = styled('div')({
  display: 'flex',
  flexDirection: 'column',
  height: '100%',
  width: 200,
  borderRight: '1px solid black',
});

const ListItem = styled('div')({
  padding: '10px 10px',
  cursor: 'pointer',
  '&.selected': {
    backgroundColor: '#808080',
  },
});

export function AgentDirList({ nodes }: { nodes: Node[] }) {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { inspectNodeData, setInspectNodeData, setInspectAgentGoal } = useExplainabilityContext();

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
    setInspectAgentGoal(undefined);

    if (areObjectsEqual(data, inspectNodeData)) {
      setInspectNodeData(undefined);
    } else {
      setInspectNodeData(data);
    }
  };

  return (
    <ListContainer>
      <div
        style={{
          flex: '0 0 48px',
          display: 'flex',
          justifyContent: 'center',
          alignItems: 'center',
          backgroundColor: '#424242',
          color: '#ffffff',
          fontSize: 14,
        }}
      >
        Agent List
      </div>
      <div style={{ flex: 1, display: 'flex', flexDirection: 'column', overflow: 'auto' }}>
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
      </div>
    </ListContainer>
  );
}
