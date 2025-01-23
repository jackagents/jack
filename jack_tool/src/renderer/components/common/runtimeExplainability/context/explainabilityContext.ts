import { BDILOGINSPECTMODE } from 'constant/cbdi/cbdiConstants';
import { ConnectStatus, ConnectMode } from 'constant/common/cmConstants';
import { createContext, useContext } from 'react';
import { BusAddress, Delegation } from 'types/cbdi/cbdi-types-non-flatbuffer';
import { CBDIAgent, CBDIService } from 'types/cbdi/cbdi-models-non-flatbuffer';
import { CBDIEditorProject } from 'types/cbdiEdit/cbdiEditModel';
import { CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { CbdiEditReactflowViewportDataDic } from 'components/cbdiEdit/CbdiEditReactflowContext/CbdiEditReactflowContextType';

export interface CustomNodeData {
  type: CBDIEditorRootConceptType;
  label: string;
  children?: CustomNodeData[];
  agentBusAddress?: BusAddress;
  delegation?: Delegation;
  subGoalId?: string;
  opacity?: number;
  delegationStatus?: string;
  delegatingAgentAddress?: BusAddress;
}

export type InspectAgentGoal = {
  agentId: string | undefined;
  goalId: string | undefined;
};

interface ContextProps {
  hidden: boolean;
  isLivePlayback: boolean;
  agentTreeGraphResetFlag: boolean;
  connectStatus: ConnectStatus;
  connectMode: ConnectMode;
  bdiLogInspectMode: BDILOGINSPECTMODE;
  project: CBDIEditorProject | null;
  projectFilePath: string;
  inspectNodeData?: CustomNodeData;
  inspectAgentModel?: CBDIAgent;
  inspectAgentGoal?: InspectAgentGoal;
  allNodes: (CBDIAgent | CBDIService)[];
  exploredTreeNodePaths: string[];
  collapsedTreeNodePaths: string[];
  debugMode: boolean;
  viewportDataDic: CbdiEditReactflowViewportDataDic;
  planGraphTaskScale: number;
  setViewportDataDic: React.Dispatch<React.SetStateAction<CbdiEditReactflowViewportDataDic>>;
  setAgentTreeGraphResetFlag: React.Dispatch<React.SetStateAction<boolean>>;
  setConnectStatus: React.Dispatch<React.SetStateAction<ConnectStatus>>;
  setConnectMode: React.Dispatch<React.SetStateAction<ConnectMode>>;
  setProject: React.Dispatch<React.SetStateAction<CBDIEditorProject | null>>;
  setProjectFilePath: React.Dispatch<React.SetStateAction<string>>;
  setBdiLogInspectMode: React.Dispatch<React.SetStateAction<BDILOGINSPECTMODE>>;
  setInspectNodeData: React.Dispatch<React.SetStateAction<CustomNodeData | undefined>>;
  setInspectAgentModel: React.Dispatch<React.SetStateAction<CBDIAgent | undefined>>;
  setInspectAgentGoal: React.Dispatch<React.SetStateAction<InspectAgentGoal | undefined>>;
  setAllNodes: React.Dispatch<React.SetStateAction<(CBDIAgent | CBDIService)[]>>;
  setExploredTreeNodePaths: React.Dispatch<React.SetStateAction<string[]>>;
  setCollapsedTreeNodePaths: React.Dispatch<React.SetStateAction<string[]>>;
  resetExplainabilityViewSelection: () => void;
  setDebugMode: React.Dispatch<React.SetStateAction<boolean>>;
  setPlanGraphTaskScale: React.Dispatch<React.SetStateAction<number>>;
}
// Define a new context object
export const ExplainabilityContext = createContext<ContextProps>({
  hidden: false,
  isLivePlayback: false,
  agentTreeGraphResetFlag: false,
  connectStatus: ConnectStatus.disconnected,
  connectMode: ConnectMode.playback,
  bdiLogInspectMode: BDILOGINSPECTMODE.SUMMARY,
  project: null,
  projectFilePath: '',
  allNodes: [],
  exploredTreeNodePaths: [],
  collapsedTreeNodePaths: [],
  debugMode: false,
  viewportDataDic: {},
  planGraphTaskScale: 1,
  setViewportDataDic: () => {},
  setAgentTreeGraphResetFlag: () => {},
  setConnectStatus: () => {},
  setConnectMode: () => {},
  setProject: () => {},
  setProjectFilePath: () => {},
  setBdiLogInspectMode: () => {},
  setInspectNodeData: () => {},
  setInspectAgentModel: () => {},
  setInspectAgentGoal: () => {},
  setAllNodes: () => {},
  setExploredTreeNodePaths: () => {},
  setCollapsedTreeNodePaths: () => {},
  resetExplainabilityViewSelection: () => {},
  setDebugMode: () => {},
  setPlanGraphTaskScale: () => {},
});

// Create a custom hook to make it easier to use the context
export const useExplainabilityContext = () => useContext(ExplainabilityContext);
