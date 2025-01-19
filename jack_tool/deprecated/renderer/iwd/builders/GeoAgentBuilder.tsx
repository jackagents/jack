import React from 'react';
import { useSelector } from 'react-redux';
import { BELIEF_SET_KEYS } from 'constant/common/cmConstants';
import { Agent } from 'types/iwd/iwdTypes';
import { RootState } from 'projectRedux/Store';
import GeoAgentLayer from './children/GeoAgentLayer';

function GeoAgentBuilder() {
  const [geoAgents, setGeoAgents] = React.useState<Agent[]>([]);

  const { agents } = useSelector((state: RootState) => state.iwd);

  const updateGeoAgents = () => {
    // when there is new geo agent (agent with position beliefset), update geoAgents array
    const tempGeoAgents: Agent[] = [];
    for (let i = 0; i < agents.length; i += 1) {
      const agent = agents[i];
      if (agent.beliefSets) {
        const beliefSets = new Map(agent.beliefSets);
        // Get all geo agents in agents
        if (beliefSets.has(BELIEF_SET_KEYS.POSITION)) {
          tempGeoAgents.push(agent);
          // TODO: Check if this agent is new
        }
      }
    }

    // Set geo agent array
    setGeoAgents([...tempGeoAgents]);
  };

  React.useEffect(() => {
    // Currently run at most 1 per second (limited from backend)
    updateGeoAgents();
  }, [agents]);

  return (
    <div className="geo-agent-builder">
      <GeoAgentLayer geoAgents={geoAgents} />
    </div>
  );
}

export default React.memo(GeoAgentBuilder);
