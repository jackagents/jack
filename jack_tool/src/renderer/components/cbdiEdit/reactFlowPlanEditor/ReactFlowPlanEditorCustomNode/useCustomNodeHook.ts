import { ReactFlowState, useStore } from 'reactflow';

const selector = (state: ReactFlowState) => ({
  connectionNodeId: state.connectionNodeId,
  connectionStartHandle: state.connectionStartHandle,
  edges: state.edges,
});

export default function useCustomNodeHook(nodeId: string) {
  const { connectionNodeId, edges, connectionStartHandle } = useStore(selector);
  const isConnecting = !!connectionNodeId;
  let isTarget = isConnecting;
  if (connectionNodeId) {
    if (edges.find((edge) => edge.source === connectionNodeId && edge.sourceHandle === connectionStartHandle?.handleId && edge.target === nodeId)) {
      isTarget = false;
    }
  }

  return isTarget;
}
