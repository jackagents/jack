import { stratify, tree } from 'd3-hierarchy';
import { Edge, Node } from 'reactflow';

const dummyNode = {
  id: 'dummy_node',
  label: 'Dummy Node',
  type: 'dummy_node_type',
};

const g = tree();

interface Options {
  width: number;
  height: number;
}

export function getD3HierarchyLayout(
  nodes: Node[],
  edges: Edge[],
  options: Options
) {
  if (nodes.length === 0) return { nodes, edges };

  // Find nodes with no incoming edges (root nodes)
  const incomingEdgesCount = new Map<string, number>();
  edges.forEach((edge) => {
    incomingEdgesCount.set(
      edge.target,
      (incomingEdgesCount.get(edge.target) || 0) + 1
    );
  });
  const rootNodes = nodes.filter((node) => !incomingEdgesCount.has(node.id));

  if (rootNodes.length > 1) {
    const dummyEdges = rootNodes.map((rootNode) => ({
      id: `dummy_edge_${rootNode.id}`,
      source: dummyNode.id,
      target: rootNode.id,
      type: 'dummy_edge_type',
    }));

    // Combine the dummy node and edges with the original nodes and edges
    const allNodes = [...nodes, dummyNode];
    const allEdges = [...edges, ...dummyEdges];

    // Proceed with the layout
    const hierarchy = stratify()
      .id((node: any) => node.id)
      .parentId(
        (node: any) =>
          allEdges.find((edge: any) => edge.target === node.id)?.source
      );

    try {
      const root = hierarchy(allNodes);
      const layout = g.nodeSize([options.width, options.height])(root);

      return {
        nodes: layout.descendants().map((node: any) => ({
          ...node.data,
          position: { x: node.x, y: node.y },
        })),
        edges: allEdges,
      };
    } catch (error) {
      throw new Error(
        'Error creating the hierarchical layout. Please check your data and ensure there is only one root node.'
      );
    }
  } else {
    const hierarchy = stratify()
      .id((node: any) => node.id)
      .parentId(
        (node: any) =>
          edges.find((edge: any) => edge.target === node.id)?.source
      );

    // Catch potential multiple roots issue
    try {
      const root = hierarchy(nodes);
      const layout = g.nodeSize([options.width, options.height])(root);

      return {
        nodes: layout.descendants().map((node: any) => ({
          ...node.data,
          position: { x: node.x, y: node.y },
        })),
        edges,
      };
    } catch (error) {
      throw new Error(
        'Error creating the hierarchical layout. Please check your data and ensure there is only one root node.'
      );
    }
  }
}
