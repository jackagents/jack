import { Allotment } from 'allotment';
import React from 'react';
import { request, response } from 'misc/events/common/cmEvents';
import { decompressData } from 'misc/utils/common/dataCompression/dataCompression';
import { ConnectStatus } from 'misc/constant/common/cmConstants';
import { useExplainabilityContext } from '../context/explainabilityContext';
import PlanRuntimeView from './graphDetailRuntime/planRuntimeView/PlanRuntimeView';
import RuntimeLogView from './runtimeLogView/RuntimeLogView';
import { Node, Edge } from './graphDetailRuntime/agentGraph/convertNodesEdgesToTree';
import { AgentDirList } from './AgentDirList/AgentDirList';

export default function MainPanelRuntime() {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { hidden, project, connectStatus, agentTreeGraphResetFlag, setExploredTreeNodePaths, setCollapsedTreeNodePaths } = useExplainabilityContext();
  /* ----------------------------- useState hooks ----------------------------- */
  const [nodes, setNodes] = React.useState<Node[]>([]);

  const [isPlanViewVisible, setIsPlanViewVisible] = React.useState(false);

  /* -------------------------------- Callbacks ------------------------------- */
  const onNodeModel = async (_e: any, data: Uint8Array) => {
    if (!project) {
      return;
    }
    // Decompress and parse data
    const parsedGraphData: {
      nodes: Node[];
      edges: Edge[];
    } = JSON.parse(decompressData(data));
    setNodes(parsedGraphData.nodes || []);
  };

  /* ----------------------------- useEffect hooks ---------------------------- */
  React.useEffect(() => {
    setNodes([]);
    setExploredTreeNodePaths([]);
    setCollapsedTreeNodePaths([]);
    window.ipcRenderer.invoke(request.cbdi.discoverModels);

    const nodeModelCleanup = window.ipcRenderer.setupIpcListener(response.cbdi.nodeModel, onNodeModel);

    return () => {
      nodeModelCleanup();
    };
  }, [project]);

  React.useEffect(() => {
    setNodes([]);
    setExploredTreeNodePaths([]);
    setCollapsedTreeNodePaths([]);
  }, [agentTreeGraphResetFlag]);

  React.useEffect(() => {
    if (connectStatus === ConnectStatus.disconnected) {
      setNodes([]);
    } else {
      window.ipcRenderer.invoke(request.cbdi.discoverModels);
    }
  }, [connectStatus]);

  return (
    <div style={{ display: 'flex', height: '100%', overflow: 'auto' }}>
      <AgentDirList nodes={nodes} />
      <Allotment defaultSizes={[100, 100]}>
        <Allotment.Pane minSize={600}>
          <RuntimeLogView />
        </Allotment.Pane>
        <Allotment.Pane minSize={400} preferredSize="50%" snap visible={isPlanViewVisible}>
          {hidden ? null : (
            <PlanRuntimeView
              setIsPlanViewVisible={(isVisible: boolean) => {
                setIsPlanViewVisible(isVisible);
              }}
            />
          )}
        </Allotment.Pane>
      </Allotment>
    </div>
  );
}
