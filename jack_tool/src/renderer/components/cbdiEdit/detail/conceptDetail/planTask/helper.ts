import { CBDIEditorOverrideMessageFieldSchema } from 'misc/types/cbdiEdit/cbdiEditModel';
import { IPlanNode, IPlanEdge, CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';

export interface NodeWithDistance {
  task: IPlanNode;
  distance: number;
}

export interface NodeWithDistanceDic {
  [taskId: string]: NodeWithDistance;
}

export interface NodesWithFieldsDistance {
  taskId: string;
  messageName: string;
  fields: CBDIEditorOverrideMessageFieldSchema[];
  distance: number;
  type: CBDIEditorRootConceptType.ActionConceptType | CBDIEditorRootConceptType.GoalConceptType;
}

export const findBranchesAndCreateNodeDictionary = (
  nodes: IPlanNode[],
  edges: IPlanEdge[],
  startNodeId: string,
  endNodeId: string,
): NodeWithDistanceDic => {
  const adjacencyList: { [key: string]: string[] } = {};
  edges.forEach((edge) => {
    const { source, target } = edge;
    if (!adjacencyList[source]) {
      adjacencyList[source] = [];
    }
    adjacencyList[source].push(target);
  });

  const visited = new Set<string>();
  const branches: IPlanNode[][] = [];

  const dfs = (nodeId: string, branch: IPlanNode[]) => {
    visited.add(nodeId);
    const node = nodes.find((el) => el.nodeId === nodeId);

    if (node) {
      branch.push(node);
    }

    if (nodeId === endNodeId) {
      branches.push([...branch]); // Copy the branch when reaching the end node
    } else if (adjacencyList[nodeId]) {
      adjacencyList[nodeId].forEach((neighbor) => {
        if (!visited.has(neighbor)) {
          dfs(neighbor, branch);
        }
      });
    }

    branch.pop(); // Backtrack by removing the last node
    visited.delete(nodeId);
  };

  const initialBranch: IPlanNode[] = [];
  dfs(startNodeId, initialBranch);

  // Create the dictionary
  const result: NodeWithDistanceDic = {};

  branches.forEach((branch) => {
    let distance = 0;

    branch.forEach((node) => {
      const { nodeId } = node;

      if (!result[nodeId] || distance > result[nodeId].distance) {
        result[nodeId] = { task: node, distance };
      }

      distance++;
    });
  });

  // Adjust distances from the `startNode`
  Object.keys(result).forEach((nodeId) => {
    result[nodeId].distance = result[endNodeId].distance - result[nodeId].distance;
  });
  delete result[endNodeId];

  return result;
};
