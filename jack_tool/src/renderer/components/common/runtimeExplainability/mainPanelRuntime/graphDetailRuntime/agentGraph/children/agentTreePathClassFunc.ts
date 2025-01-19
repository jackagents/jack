import { CustomNodeData } from 'components/common/runtimeExplainability/context/explainabilityContext';
import { Orientation, TreeLinkDatum } from 'react-d3-tree';

// This is a custom path class function that assigns a class name based on the target attribute
export function agentTreePathClassFunc(link: TreeLinkDatum, _orientation: Orientation) {
  const { source, target } = link;
  let sourceData: CustomNodeData | undefined;
  let targetData: CustomNodeData | undefined;
  if (source.data.attributes && source.data.attributes.nodeData) {
    sourceData = JSON.parse(source.data.attributes!.nodeData as string) as CustomNodeData;
  }
  if (target.data.attributes && target.data.attributes.nodeData) {
    targetData = JSON.parse(target.data.attributes!.nodeData as string) as CustomNodeData;
  }

  if (sourceData?.delegationStatus === 'Pending' || targetData?.delegationStatus === 'Pending') {
    return 'green-link';
  }

  if (source.data.name === 'virtualRoot') {
    return 'hidden-link';
  }

  return 'default-link';
}
