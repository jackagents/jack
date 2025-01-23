import JSONViewer from 'components/common/jsonViewer/JSONViewer';
import React from 'react';
import { Delegation } from 'misc/types/cbdi/cbdi-types-non-flatbuffer';
import { CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { useExplainabilityContext } from '../../context/explainabilityContext';

export default function DelegationDetailJsonView() {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { inspectNodeData } = useExplainabilityContext();

  const [nodeData, setNodeData] = React.useState<Delegation>();

  React.useEffect(() => {
    if (
      inspectNodeData &&
      inspectNodeData.type === CBDIEditorRootConceptType.GoalConceptType &&
      inspectNodeData.delegation
    ) {
      setNodeData(inspectNodeData.delegation);
    }

    return () => {
      setNodeData(undefined);
    };
  }, [inspectNodeData]);

  return (
    <JSONViewer
      title="Delegation Data"
      dataName={inspectNodeData?.delegation?.goal || 'undefined'}
      nodeData={nodeData}
      timeoutText={`Cannot find delegation data for ${
        inspectNodeData?.delegation?.goal || 'undefined'
      }`}
    />
  );
}
