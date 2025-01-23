import React from 'react';
import './agentGraph.css';
import { Fluid } from 'components/common/base/BaseContainer';
import { response, request } from 'projectEvents/common/cmEvents';
import { styled } from '@mui/material';
import { ConnectStatus } from 'constant/common/cmConstants';
import { decompressData } from 'misc/utils/common/dataCompression/dataCompression';
import Tree, { Point } from 'react-d3-tree';
import { useExplainabilityContext } from '../../../context/explainabilityContext';
import { Node, Edge, convertNodesEdgesToTree } from './convertNodesEdgesToTree';
import { useCenteredTree } from './helper';
import { agentTreePathClassFunc } from './children/agentTreePathClassFunc';
import AgentTreeCustomNode, { EXPLAINABILITY_NODE_HEIGHT, EXPLAINABILITY_NODE_WIDTH } from './children/AgentTreeCustomNode';
import { AgentList } from './children/AgentList';

/* --------------------------------- Styles --------------------------------- */
const GraphContainer = styled(Fluid)({
  '&&&': {
    height: '100%',
    display: 'flex',
    flexDirection: 'column',
    position: 'relative',
  },
});

function AgentGraph() {
  /* ---------------------------- useContext hooks ---------------------------- */
  const {
    hidden,
    project,
    connectStatus,
    agentTreeGraphResetFlag,
    exploredTreeNodePaths,
    collapsedTreeNodePaths,
    setExploredTreeNodePaths,
    setCollapsedTreeNodePaths,
  } = useExplainabilityContext();

  /* ----------------------------- useState hooks ----------------------------- */
  const [nodes, setNodes] = React.useState<Node[]>([]);
  const [edges, setEdges] = React.useState<Edge[]>([]);

  /* ------------------------------ useMemo hooks ----------------------------- */
  const treeData = React.useMemo(() => {
    const mtreeData = convertNodesEdgesToTree(nodes, edges, exploredTreeNodePaths, collapsedTreeNodePaths, 3);

    return mtreeData;
  }, [nodes, edges, exploredTreeNodePaths.length, collapsedTreeNodePaths.length]);

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
    setEdges(parsedGraphData.edges || []);
  };

  /* ----------------------------- useEffect hooks ---------------------------- */
  React.useEffect(() => {
    setNodes([]);
    setEdges([]);
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
    setEdges([]);
    setExploredTreeNodePaths([]);
    setCollapsedTreeNodePaths([]);
  }, [agentTreeGraphResetFlag]);

  React.useEffect(() => {
    if (connectStatus === ConnectStatus.disconnected) {
      setNodes([]);
      setEdges([]);
    } else {
      window.ipcRenderer.invoke(request.cbdi.discoverModels);
    }
  }, [connectStatus]);

  const [translate, containerRef] = useCenteredTree();

  return (
    <GraphContainer>
      {hidden ? null : (
        <div style={{ width: '100%', height: '100%' }} ref={containerRef as any}>
          <Tree
            zoom={0.4}
            data={treeData}
            orientation="vertical"
            translate={translate as Point | undefined}
            pathFunc="step"
            nodeSize={{
              x: EXPLAINABILITY_NODE_WIDTH,
              y: EXPLAINABILITY_NODE_HEIGHT * 2.5,
            }}
            separation={{
              siblings: 1.6,
              nonSiblings: 2,
            }}
            pathClassFunc={agentTreePathClassFunc}
            renderCustomNodeElement={(rd3tProps) => <AgentTreeCustomNode rd3tProps={rd3tProps} />}
          />
        </div>
      )}
      <AgentList nodes={nodes} />
    </GraphContainer>
  );
}

export default AgentGraph;
