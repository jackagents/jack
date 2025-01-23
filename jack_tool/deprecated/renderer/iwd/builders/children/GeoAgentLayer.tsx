import React from 'react';
import { BELIEF_SET_KEYS } from 'constant/common/cmConstants';
import { Agent, BeliefArray, BeliefMap } from 'types/iwd/iwdTypes';
import GeoAgent from './GeoAgent';

interface Props {
  geoAgents: Agent[];
}

function GeoAgentLayer(props: Props) {
  const [components, setComponents] = React.useState<JSX.Element[]>([]);

  React.useEffect(() => {
    const temp = [];
    for (let i = 0; i < props.geoAgents.length; i += 1) {
      const element = props.geoAgents[i];

      // Find if the geo agent has been rendered
      const updateElement = components.findIndex(
        (x) => element.address.id === x.props.agent.address.id
      );

      // Rendered if new
      if (updateElement < 0) {
        if (!element.beliefSets) return;

        // Create position beliefs map object
        const positionBeliefs: BeliefMap = new Map(
          new Map(element.beliefSets).get(
            BELIEF_SET_KEYS.POSITION
          ) as BeliefArray[]
        );

        if (!positionBeliefs) {
          return;
        }

        temp.push(
          <GeoAgent
            key={element.address.id}
            positionBeliefs={positionBeliefs}
            agent={element}
          />
        );
      }
      // TODO: update if there are changes
      // else {
      //   // const agent = components[updateElement].props.agent as Agent;
      // }
    }

    setComponents([...components, ...temp]);
  }, [props.geoAgents]);

  return <>{components}</>;
}

// Memorise the layer, so that it won't be rerendered unless props.geoAgents changed
export default React.memo(GeoAgentLayer);
