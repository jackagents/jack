import { Stack } from '@mui/material';
import React from 'react';
import { useSelector } from 'react-redux';
import L from 'leaflet';
import { Agent, CurrentAgent } from 'types/iwd/iwdTypes';
import { RootState } from 'projectRedux/Store';

interface Props {
  currentAgent: CurrentAgent;
}

function ConfigEntityContent(props: Props) {
  // current agent
  const [agent, setAgent] = React.useState<Agent | null>(null);
  // Reference to div
  const divRef = React.useRef<any>(null);
  // agents list
  const { agents } = useSelector((state: RootState) => state.iwd);

  React.useEffect(() => {
    L.DomEvent.disableClickPropagation(divRef.current);
    L.DomEvent.disableScrollPropagation(divRef.current);
  }, []);

  React.useEffect(() => {
    // Get the current entity that user want to config
    const { id } = props.currentAgent;

    // Get save entity from non geo spatial/prototypes
    const result = agents.find((x) => x.address.id === id);

    // If result, set current agent
    if (result) {
      setAgent({ ...result });
    }
  }, [props.currentAgent]);

  /**
   * Return agent's information as JSON string
   * @returns
   */
  const options = () => {
    if (agent) {
      return (
        <div style={{ color: 'white' }}>
          <pre style={{ overflow: 'scroll' }}>
            {JSON.stringify(agent, null, 3)}
          </pre>
        </div>
      );
    }
    return null;
  };

  return (
    <div ref={divRef}>
      <Stack>{options()}</Stack>
    </div>
  );
}

export default React.memo(ConfigEntityContent);
