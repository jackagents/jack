import { Allotment } from 'allotment';
import { useExplainabilityContext } from '../../context/explainabilityContext';
import RuntimeLogView from '../runtimeLogView/RuntimeLogView';
import AgentGraph from './agentGraph/AgentGraph';

export default function GraphDetailRuntime() {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { inspectNodeData } = useExplainabilityContext();

  /* ------------------------------- Components ------------------------------- */

  return (
    <Allotment vertical defaultSizes={[700, 300]}>
      <Allotment.Pane preferredSize="70%">
        <AgentGraph />
      </Allotment.Pane>
      <Allotment.Pane visible={inspectNodeData?.agentBusAddress !== undefined} minSize={200} maxSize={1000}>
        {inspectNodeData?.agentBusAddress !== undefined ? <RuntimeLogView /> : null}
      </Allotment.Pane>
    </Allotment>
  );
}
