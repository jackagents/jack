import React from 'react';
import { request, response } from 'projectEvents/common/cmEvents';
import { useExplainabilityContext } from 'components/common/runtimeExplainability/context/explainabilityContext';
import JSONViewer, {
  LOADINGTIMEOUT_DURATION,
} from 'components/common/jsonViewer/JSONViewer';
import pako from 'pako';
import { CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';

/* -------------------------------- Properties -------------------------------- */
let agentDetailLoadingTimeout: NodeJS.Timeout;

/* --------------------------- AgentDetailJsonView -------------------------- */
export default function AgentDetailJsonView() {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { inspectNodeData, inspectAgentModel, setInspectAgentModel } =
    useExplainabilityContext();
  /* ----------------------------- useState hooks ----------------------------- */
  const [isLoadingAgentDetail, setIsLoadingAgentDetail] =
    React.useState<boolean>(false);

  const [isTimeout, setIsTimeout] = React.useState<boolean>(false);

  const [pending, startTransition] = React.useTransition();

  /* -------------------------------- Callbacks ------------------------------- */
  const onAgentModel = (_e: any, data: Uint8Array) => {
    clearTimeout(agentDetailLoadingTimeout);
    setIsTimeout(false);
    setIsLoadingAgentDetail(false);
    const decompressedData = pako.inflate(data, { to: 'string' });
    const parsedData = JSON.parse(decompressedData);

    // Update with node data
    startTransition(() => {
      setInspectAgentModel({ ...parsedData });
    });
  };

  const refreshAgentDetail = React.useCallback(() => {
    if (inspectNodeData && inspectNodeData.agentBusAddress) {
      setIsLoadingAgentDetail(true);

      window.ipcRenderer.invoke(
        request.cbdi.agentModel,
        inspectNodeData.agentBusAddress.id
      );
    }
  }, [inspectNodeData]);

  /* ----------------------------- useEffect hooks ---------------------------- */
  React.useEffect(() => {
    const removeAgentModelListener = window.ipcRenderer.setupIpcListener(
      response.cbdi.agentModel,
      onAgentModel
    );

    return () => {
      removeAgentModelListener();

      // clear timeout of request agent model
      clearTimeout(agentDetailLoadingTimeout);
    };
  }, []);

  React.useEffect(() => {
    // if inspectNodeData is team or agent
    if (
      inspectNodeData &&
      (inspectNodeData.type === CBDIEditorRootConceptType.TeamConceptType ||
        inspectNodeData.type === CBDIEditorRootConceptType.AgentConceptType)
    ) {
      setIsLoadingAgentDetail(true);

      agentDetailLoadingTimeout = setTimeout(() => {
        setIsLoadingAgentDetail(false);
        setIsTimeout(true);
      }, LOADINGTIMEOUT_DURATION);

      // Grab full model
      window.ipcRenderer.invoke(
        request.cbdi.agentModel,
        inspectNodeData.agentBusAddress!.id
      );
    }

    return () => {
      // clear timeout of request agent model
      clearTimeout(agentDetailLoadingTimeout);

      // set timeout to be false
      setIsTimeout(false);

      // set current agent data to be undefined
      setInspectAgentModel(undefined);
    };
  }, [inspectNodeData]);

  /* ------------------------------- Components ------------------------------- */
  return (
    <JSONViewer
      title="Agent Data"
      dataName={inspectNodeData?.agentBusAddress?.name || 'undefined'}
      isLoading={isLoadingAgentDetail}
      timeout={isTimeout}
      nodeData={inspectAgentModel}
      timeoutText={`Cannot find agent data for ${inspectNodeData?.agentBusAddress?.name}`}
      onClickRefresh={refreshAgentDetail}
    />
  );
}
