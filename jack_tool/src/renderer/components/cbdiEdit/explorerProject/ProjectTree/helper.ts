import { TreeData } from '../helper';

// Function to find a nodeId by its expandedIds and nodeIds
export const findToggledNodeId = (expanded: string[], nodeIds: string[]) => {
  if (nodeIds.length > expanded.length) {
    return nodeIds.find((id) => !expanded.includes(id));
  }
  if (nodeIds.length < expanded.length) {
    return expanded.find((id) => !nodeIds.includes(id));
  }
  return undefined;
};

// Function to find a node by its id
export const findNodeById = (node: TreeData, targetId: string): TreeData | null => {
  if (node.nodeId === targetId) {
    return node; // Return the node if the id matches
  }
  if (node.children) {
    for (let i = 0; i < node.children.length; i++) {
      const result = findNodeById(node.children[i], targetId);
      if (result) {
        return result; // Return the found node
      }
    }
  }

  return null; // Return null if no node is found
};
