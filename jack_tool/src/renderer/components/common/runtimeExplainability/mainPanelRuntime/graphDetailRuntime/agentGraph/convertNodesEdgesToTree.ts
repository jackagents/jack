import { CustomNodeData } from 'components/common/runtimeExplainability/context/explainabilityContext';
import { CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { copy } from 'misc/utils/cbdiEdit/Helpers';
import { RawNodeDatum } from 'react-d3-tree';

export type Node = {
  id: string;
  data: CustomNodeData;
};

export type Edge = {
  source: string;
  target: string;
};

// A function that takes an array of nodes as input and returns a new array of nodes without duplicates
const removeDuplicates = (nodes: Node[]): Node[] => {
  // Create an empty set to store the seen node ids
  const seen = new Set();
  // Create an empty array to store the unique nodes
  const unique: Node[] = [];
  // Loop through the nodes array
  nodes.forEach((node) => {
    // If the node id is not in the seen set, add it to the set and the unique array
    if (!seen.has(node.id)) {
      seen.add(node.id);
      unique.push(node);
    }
  });

  // Return the unique array
  return unique;
};

/**
 * Recursive sort grandchildren of node by name.
 * Grandparents are roles, parents are goals, and grandchildren are teams/agents
 * @param node
 * @returns
 */
const sortGrandChildren = (node: RawNodeDatum) => {
  // Clone the input node to avoid modifying the original data
  const clonedNode = copy(node) as RawNodeDatum;

  // Check if the node has children
  if (clonedNode.children === undefined) {
    return clonedNode;
  }

  for (let i = 0; i < clonedNode.children.length; i++) {
    const child1 = clonedNode.children[i];

    // Check if child1 has attributes
    if (child1.attributes && typeof child1.attributes.nodeData === 'string') {
      const data1 = JSON.parse(child1.attributes.nodeData) as CustomNodeData;

      // Check if data1 is of type "role_t" and has children
      if (data1.type === CBDIEditorRootConceptType.RoleConceptType && child1.children && child1.children.length > 0) {
        for (let j = 0; j < child1.children.length; j++) {
          const child2 = child1.children[j];

          // Check if child2 has attributes
          if (child2.attributes && typeof child2.attributes.nodeData === 'string') {
            const data2: CustomNodeData = JSON.parse(child2.attributes.nodeData);

            // Check if data2 is of type "goal_t", has a single child, and that child has agentBusAddress (agent or team)
            if (
              data2.type === CBDIEditorRootConceptType.GoalConceptType &&
              child2.children &&
              child2.children.length === 1 &&
              child2.children[0].attributes &&
              typeof child2.children[0].attributes.nodeData === 'string' &&
              (JSON.parse(child2.children[0].attributes.nodeData) as CustomNodeData).agentBusAddress
            ) {
              // Sort child1's children by name
              child1.children.sort((a, b) => {
                if (
                  a.children === undefined ||
                  b.children === undefined ||
                  a.children.length !== 1 ||
                  b.children.length !== 1 ||
                  a.children[0].attributes === undefined ||
                  b.children[0].attributes === undefined
                ) {
                  return 0;
                }
                if (typeof a.children[0].attributes.nodeData === 'string' && typeof b.children[0].attributes.nodeData === 'string') {
                  const aData = JSON.parse(a.children[0].attributes.nodeData) as CustomNodeData;

                  const bData = JSON.parse(b.children[0].attributes.nodeData) as CustomNodeData;

                  if (aData.agentBusAddress && bData.agentBusAddress) {
                    if (aData.agentBusAddress.name > bData.agentBusAddress.name) {
                      return 1;
                    }

                    if (aData.agentBusAddress.name < bData.agentBusAddress.name) {
                      return -1;
                    }
                  }
                }

                return 0;
              });
            }
          }
        }
      }
    }

    // Recursive sort
    clonedNode.children[i] = sortGrandChildren(child1);
  }

  return clonedNode;
};

// A function that takes an array of nodes, an array of edges, and a rank as input
// and returns an array of RawNodeDatum objects representing the tree structure
export function convertNodesEdgesToTree(
  nodes: Node[],
  edges: Edge[],
  exploredTreeNodePaths: string[],
  collapsedTreeNodePaths: string[],
  defaultDisplayRank = Infinity,
): RawNodeDatum {
  // A helper function that finds the children of a given node
  const findChildren = (node: Node): Node[] => {
    const children: Node[] = [];
    // Filter the edges that have the node as the source
    const outgoingEdges = edges.filter((edge) => edge.source === node.id);
    // Map the edges to their target nodes
    outgoingEdges.forEach((edge) => {
      const childNode = nodes.find((el) => el.id === edge.target);
      if (childNode) {
        children.push(childNode);
      }
    });
    // Return the unique children array
    return removeDuplicates(children);
  };

  // A helper function that converts a node and its children to a RawNodeDatum object
  const nodeToObject = (node: Node, level = 0, path = ''): RawNodeDatum => {
    // Find the children of the node
    const children = findChildren(node);
    // Create an object with the name property as the node id and the attributes property as the node data
    // Add the nodePath attribute to the object, which is the path concatenated with the node id

    const nodePath = path + node.id;

    const obj: RawNodeDatum = {
      name: node.id,
      attributes: {
        hasChildren: children.length > 0,
        nodePath,
        nodeData: JSON.stringify(node.data),
      },
    };
    // If the node has any children, and the level is less than the rank or the node path is in the expanded node paths, add them as an array of RawNodeDatum objects
    if (
      children.length > 0 &&
      obj.attributes &&
      typeof obj.attributes.nodePath === 'string' &&
      !collapsedTreeNodePaths.includes(obj.attributes!.nodePath) &&
      (level < defaultDisplayRank ||
        exploredTreeNodePaths.includes(obj.attributes!.nodePath) ||
        (node.data.type !== CBDIEditorRootConceptType.AgentConceptType && node.data.type !== CBDIEditorRootConceptType.TeamConceptType))
    ) {
      obj.children = children
        .map((child) => nodeToObject(child, level + 1, `${obj.attributes!.nodePath}/`))
        // Sort by label (for roles)
        .sort((a, b) => {
          if (a.attributes === undefined || b.attributes === undefined) {
            return 0;
          }
          if (typeof a.attributes.nodeData === 'string' && typeof b.attributes.nodeData === 'string') {
            const aData = JSON.parse(a.attributes.nodeData);
            const bData = JSON.parse(b.attributes.nodeData);
            return aData.label > bData.label ? 1 : -1;
          }
          return 0;
        });
    }
    // Return the object
    return obj;
  };

  // Find all the root nodes, which are the nodes that have no incoming edges
  const rootNodes = nodes.filter((node) => !edges.some((edge) => edge.target === node.id));
  // Convert each root node and its children to a RawNodeDatum object
  const rootObjs = rootNodes
    .map((node) => nodeToObject(node))
    // Sort grandchildren from same role node
    .map((r) => sortGrandChildren(r));

  // Create a virtual root object with a name of 'root' and the root objects as its children
  const virtualRootObj: RawNodeDatum = {
    name: 'virtualRoot',
    children: rootObjs,
  };
  // Return an array with the virtual root object as the only element

  return virtualRootObj;
}
