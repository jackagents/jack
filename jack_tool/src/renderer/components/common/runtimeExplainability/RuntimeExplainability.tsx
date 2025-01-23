/* eslint-disable react/jsx-no-constructed-context-values */
import MainPanelRuntime from 'components/common/runtimeExplainability/mainPanelRuntime/MainPanelRuntime';
import { BDILOGINSPECTMODE } from 'constant/cbdi/cbdiConstants';
import { ConnectMode, ConnectStatus } from 'constant/common/cmConstants';
import React, { useState } from 'react';
import { CBDIAgent, CBDIService } from 'types/cbdi/cbdi-models-non-flatbuffer';
import { CBDIEditorProject } from 'types/cbdiEdit/cbdiEditModel';
import { request } from 'misc/events/common/cmEvents';
import { CbdiEditReactflowViewportDataDic } from 'components/cbdiEdit/CbdiEditReactflowContext/CbdiEditReactflowContextType';
import { NODE_FONT_SIZE } from 'components/cbdiEdit/reactFlowPlanEditor/reactFlowPlanEditorConstant';
import { CustomNodeData, ExplainabilityContext, InspectAgentGoal } from './context/explainabilityContext';
import ExplainabilityStatusBar from './explainabilityStatusBar/ExplainabilityStatusBar';

/* -------------------------- RuntimeExplainability ------------------------- */
/* -------------------------------- Interface ------------------------------- */
interface RuntimeExplainabilityProps {
  hidden?: boolean;
  outProject?: CBDIEditorProject | null;
  isLivePlayback?: boolean;
}
function RuntimeExplainability(props: RuntimeExplainabilityProps) {
  const { hidden, outProject, isLivePlayback } = props;

  /* ----------------------------- useState hooks ----------------------------- */
  const [connectStatus, setConnectStatus] = useState<ConnectStatus>(ConnectStatus.disconnected);
  const [connectMode, setConnectMode] = useState<ConnectMode>(ConnectMode.playback);
  const [debugMode, setDebugMode] = useState<boolean>(false);
  const [agentTreeGraphResetFlag, setAgentTreeGraphResetFlag] = useState<boolean>(false);
  const [project, setProject] = useState<CBDIEditorProject | null>(null);
  const [projectFilePath, setProjectFilePath] = useState<string>('');
  const [bdiLogInspectMode, setBdiLogInspectMode] = useState<BDILOGINSPECTMODE>(BDILOGINSPECTMODE.SUMMARY);
  const [inspectNodeData, setInspectNodeData] = useState<CustomNodeData>();
  const [inspectAgentModel, setInspectAgentModel] = useState<CBDIAgent>();
  const [inspectAgentGoal, setInspectAgentGoal] = useState<InspectAgentGoal>();
  const [allNodes, setAllNodes] = useState<(CBDIAgent | CBDIService)[]>([]);
  const [exploredTreeNodePaths, setExploredTreeNodePaths] = useState<string[]>([]);
  const [collapsedTreeNodePaths, setCollapsedTreeNodePaths] = useState<string[]>([]);
  const [viewportDataDic, setViewportDataDic] = React.useState<CbdiEditReactflowViewportDataDic>({});
  const [planGraphTaskScale, setPlanGraphTaskScale] = React.useState<number>(1);

  /* -------------------------------- Callbacks ------------------------------- */
  const resetExplainabilityViewSelection = () => {
    setInspectNodeData(undefined);
    setInspectAgentGoal(undefined);
    setExploredTreeNodePaths([]);
    setExploredTreeNodePaths([]);
    setViewportDataDic({});
    setDebugMode(false);
    // exit playback debug mode
    window.ipcRenderer.send(request.playback.debug.exit);
  };

  /* ----------------------------- useEffect hooks ---------------------------- */
  React.useEffect(() => {
    if (outProject !== undefined) {
      // reset explainability view
      resetExplainabilityViewSelection();
      setAgentTreeGraphResetFlag((prev) => !prev);
      // reset playback file
      window.ipcRenderer.invoke(request.playback.resetFile);
      // set cbdi model file for explainability view
      setProject(outProject);
      window.ipcRenderer.invoke(request.cbdi.setEditModel, outProject);
      // exit playback debug mode
      window.ipcRenderer.send(request.playback.debug.exit);
    }
  }, [outProject]);

  // Stop playback manager when change mode
  React.useEffect(() => {
    window.ipcRenderer.invoke(request.playback.stop);
  }, [connectMode]);
  /* ------------------------------- Components ------------------------------- */
  return (
    <ExplainabilityContext.Provider
      value={{
        viewportDataDic,
        setViewportDataDic,
        hidden: hidden === true,
        isLivePlayback: isLivePlayback === true,
        agentTreeGraphResetFlag,
        connectStatus,
        connectMode,
        project,
        projectFilePath,
        bdiLogInspectMode,
        inspectNodeData,
        inspectAgentModel,
        inspectAgentGoal,
        allNodes,
        exploredTreeNodePaths,
        collapsedTreeNodePaths,
        debugMode,
        planGraphTaskScale,
        setAgentTreeGraphResetFlag,
        setConnectStatus,
        setConnectMode,
        setProject,
        setProjectFilePath,
        setBdiLogInspectMode,
        setInspectNodeData,
        setInspectAgentModel,
        setInspectAgentGoal,
        setAllNodes,
        setExploredTreeNodePaths,
        setCollapsedTreeNodePaths,
        resetExplainabilityViewSelection,
        setDebugMode,
        setPlanGraphTaskScale,
      }}
    >
      <ExplainabilityStatusBar />
      <MainPanelRuntime />
    </ExplainabilityContext.Provider>
  );
}

export default React.memo(RuntimeExplainability);
