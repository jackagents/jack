import React, { startTransition } from 'react';
import Tabs from '@mui/material/Tabs';
import Tab from '@mui/material/Tab';
import { styled } from '@mui/material';
import { Fluid } from 'components/common/base/BaseContainer';
import { BDILOGINSPECTMODE } from 'constant/cbdi/cbdiConstants';
import { request, response } from 'projectEvents/common/cmEvents';
import { AgentSummaryGoalInfo, BDILogIntentionOverviewsModel } from 'types/cbdi/cbdi-models-non-flatbuffer';
import ReactLoading from 'react-loading';
import { Event } from 'types/cbdi/cbdi-types-non-flatbuffer';
import pako from 'pako';
import { DELAY_TIME } from 'misc/constant/cbdiEdit/cbdiEditConstant';
import { CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import IntentionTracker from './intentionTracker/IntentionTracker';
import { useExplainabilityContext } from '../../context/explainabilityContext';
import IntentionLogView from './intentionLog/IntentionLogView';
import AgentSummaryView from './agentSummaryView/AgentSummaryView';
import BeliefsetsDetail from '../../beliefsetsDetail/BeliefsetsDetail';
import AgentGraph from '../graphDetailRuntime/agentGraph/AgentGraph';

/* --------------------------------- Styles --------------------------------- */

const Root = styled(Fluid)({
  display: 'flex',
  flexDirection: 'column',
});

const TitleContainer = styled('div')({
  backgroundColor: '#424242',
  flex: '0 0 48px',
});

interface StyledTabProps {
  label: string;
  value: number;
}

const StyledTab = styled((props: StyledTabProps) => (
  // eslint-disable-next-line react/jsx-props-no-spreading
  <Tab disableRipple {...props} />
))({
  '&&&': {
    textTransform: 'none',
    marginRight: 1,
    color: '#999999',
    fontWeight: 500,
    fontFamily: 'sans-serif',
    '&.Mui-selected': {
      color: '#ffffff',
    },
    '&.Mui-focusVisible': {
      backgroundColor: '#e9e9e9',
      height: 2,
    },
  },
});

const ContentContainer = styled('div')({
  flexGrow: 1,
  position: 'relative',
  display: 'flex',
  flexDirection: 'column',
  overflow: 'hidden',
});

const LoadingRoot = styled('div')({
  position: 'absolute',
  width: '100%',
  height: '100%',
  top: 0,
  left: 0,
  display: 'flex',
  justifyContent: 'center',
  alignItems: 'center',
  zIndex: 99,
});

/* -------------------------------- Properties -------------------------------- */

let agentBdiLogsSubscribeTimer: NodeJS.Timer;
const AGENTBDILOGSSUB_DURATION = 3000;
const LOADINGTIMEOUT_DURATION = 10000;
let agentLogsLoadingTimeout: NodeJS.Timeout;
/* ----------------------------- RuntimeLogView ----------------------------- */
export default function RuntimeLogView() {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { inspectNodeData, bdiLogInspectMode, setBdiLogInspectMode } = useExplainabilityContext();
  /* ----------------------------- useState hooks ----------------------------- */
  const [agentSummaryIntentionInfo, setAgentSummaryIntentionInfo] = React.useState<AgentSummaryGoalInfo>();
  const [intentionOverviews, setIntentionOverviews] = React.useState<BDILogIntentionOverviewsModel>({});
  const [logs, setLogs] = React.useState<Event[]>([]);
  const [isLoadingBdiLogs, setIsLoadingBdiLogs] = React.useState<boolean>(false);
  const [isTimeout, setIsTimeout] = React.useState<boolean>(false);

  const defferedIntentionOverviews = React.useDeferredValue(intentionOverviews);

  /* -------------------------------- Callbacks ------------------------------- */
  const handleChange = (_event: React.SyntheticEvent, newValue: BDILOGINSPECTMODE) => {
    setBdiLogInspectMode(newValue);
  };

  const onAgentSummaryIntentionInfo = (_e: Electron.IpcRendererEvent, data: string) => {
    setAgentSummaryIntentionInfo((prev) => {
      if (data && data !== JSON.stringify(prev)) {
        const newAgentSummaryIntentionInfo = JSON.parse(data) as AgentSummaryGoalInfo;

        clearTimeout(agentLogsLoadingTimeout);
        setIsTimeout(false);
        setIsLoadingBdiLogs(false);
        console.log('agent summary', newAgentSummaryIntentionInfo);

        return newAgentSummaryIntentionInfo;
      }
      return prev;
    });
  };

  const onIntentionOverview = (_e: Electron.IpcRendererEvent, data: Uint8Array) => {
    // decompress and parse data
    const decompressedData = pako.inflate(data, { to: 'string' });
    const parsedData = JSON.parse(decompressedData);

    clearTimeout(agentLogsLoadingTimeout);
    setIsTimeout(false);
    setIsLoadingBdiLogs(false);

    // use transition rendering
    startTransition(() => {
      // update intentions
      setIntentionOverviews({ ...parsedData });
    });
  };

  const onLogs = (_e: Electron.IpcRendererEvent, data: string) => {
    setLogs((prev) => {
      if (data && data !== JSON.stringify(prev)) {
        const mlogs = (JSON.parse(data) || []) as Event[] | null;
        if (mlogs !== null && mlogs.length > 0) {
          clearTimeout(agentLogsLoadingTimeout);
          setIsTimeout(false);
          setIsLoadingBdiLogs(false);
          console.log('logs', mlogs);
          return mlogs;
        }
      }
      return prev;
    });
  };

  /* ----------------------------- useEffect hooks ---------------------------- */
  React.useEffect(() => {
    if (inspectNodeData?.type !== CBDIEditorRootConceptType.AgentConceptType && inspectNodeData?.type !== CBDIEditorRootConceptType.TeamConceptType) {
      setAgentSummaryIntentionInfo(undefined);
    }
  }, [intentionOverviews, inspectNodeData]);

  React.useEffect(() => {
    const removeAgentSummaryIntentionInfoListener = window.ipcRenderer.setupIpcListener(
      response.bdilog.agentSummaryIntentionInfo,
      onAgentSummaryIntentionInfo,
    );

    const removeLogsListener = window.ipcRenderer.setupIpcListener(response.bdilog.logs, onLogs);

    const removeIntentionOverviewsListener = window.ipcRenderer.setupIpcListener(response.bdilog.intentionOverviews, onIntentionOverview);

    return () => {
      removeAgentSummaryIntentionInfoListener();
      removeLogsListener();
      removeIntentionOverviewsListener();

      clearInterval(agentBdiLogsSubscribeTimer);
      clearTimeout(agentLogsLoadingTimeout);
    };
  }, []);

  React.useEffect(() => {
    clearInterval(agentBdiLogsSubscribeTimer);
    clearTimeout(agentLogsLoadingTimeout);

    setAgentSummaryIntentionInfo(undefined);
    setIntentionOverviews({});
    setLogs([]);
    setIsTimeout(false);

    if (inspectNodeData?.agentBusAddress) {
      setIsLoadingBdiLogs(true);
      agentLogsLoadingTimeout = setTimeout(() => {
        setIsLoadingBdiLogs(false);
        setIsTimeout(true);
      }, LOADINGTIMEOUT_DURATION);

      if (bdiLogInspectMode === BDILOGINSPECTMODE.SUMMARY) {
        // First time
        window.ipcRenderer.invoke(request.bdilog.agentSummaryIntentionInfo, inspectNodeData!.agentBusAddress!.id);

        // By interval
        agentBdiLogsSubscribeTimer = setInterval(() => {
          window.ipcRenderer.invoke(request.bdilog.agentSummaryIntentionInfo, inspectNodeData!.agentBusAddress!.id);
        }, AGENTBDILOGSSUB_DURATION);
      } else if (bdiLogInspectMode === BDILOGINSPECTMODE.INTENTION_TRACKER) {
        setTimeout(() => {
          // First time
          window.ipcRenderer.invoke(request.bdilog.intentionOverviews, inspectNodeData!.agentBusAddress!.id);

          // Interval
          agentBdiLogsSubscribeTimer = setInterval(() => {
            window.ipcRenderer.invoke(request.bdilog.intentionOverviews, inspectNodeData!.agentBusAddress!.id);
          }, AGENTBDILOGSSUB_DURATION);
        }, DELAY_TIME.INTENTIONS_VIEW);
      }
      // Logs tracker
      else if (bdiLogInspectMode === BDILOGINSPECTMODE.INTENTION_LOG) {
        // First time
        window.ipcRenderer.invoke(request.bdilog.logs, inspectNodeData!.agentBusAddress!.id);

        // Interval
        agentBdiLogsSubscribeTimer = setInterval(() => {
          window.ipcRenderer.invoke(request.bdilog.logs, inspectNodeData!.agentBusAddress!.id);
        }, AGENTBDILOGSSUB_DURATION);
      } else {
        setIsLoadingBdiLogs(false);
        setIsTimeout(false);
      }
    } else {
      setIsLoadingBdiLogs(false);
    }
  }, [inspectNodeData, bdiLogInspectMode]);

  /* ------------------------------- Components ------------------------------- */
  return (
    <Root id="runtime-log-view-root">
      <TitleContainer>
        <Tabs value={bdiLogInspectMode} onChange={handleChange}>
          <StyledTab value={BDILOGINSPECTMODE.SUMMARY} label="Agent Summary" />
          <StyledTab value={BDILOGINSPECTMODE.INTENTION_TRACKER} label="Intention Tracker" />
          <StyledTab value={BDILOGINSPECTMODE.INTENTION_LOG} label="Intention Log" />
          <StyledTab value={BDILOGINSPECTMODE.BELIEFSETS} label="Beliefsets" />
          <StyledTab value={BDILOGINSPECTMODE.AGENT_TREE_GRAPH} label="Agent Tree Graph" />
        </Tabs>
      </TitleContainer>
      <ContentContainer>
        <div style={{ overflow: 'auto', flexGrow: 1 }}>
          {(() => {
            if (bdiLogInspectMode === BDILOGINSPECTMODE.BELIEFSETS) {
              return <BeliefsetsDetail />;
            }

            if (bdiLogInspectMode === BDILOGINSPECTMODE.AGENT_TREE_GRAPH) {
              return <AgentGraph />;
            }

            if (isLoadingBdiLogs) {
              return (
                <LoadingRoot>
                  <ReactLoading type="spinningBubbles" color="gray" width="5%" height="5%" />
                </LoadingRoot>
              );
            }
            if (isTimeout) {
              return <LoadingRoot>Cannot find data for {inspectNodeData?.agentBusAddress?.name}</LoadingRoot>;
            }
            switch (bdiLogInspectMode) {
              case BDILOGINSPECTMODE.SUMMARY:
                return <AgentSummaryView agentSummaryIntentionInfo={agentSummaryIntentionInfo} />;

              case BDILOGINSPECTMODE.INTENTION_TRACKER:
                return <IntentionTracker agentIntentionDic={defferedIntentionOverviews} />;

              case BDILOGINSPECTMODE.INTENTION_LOG:
                return <IntentionLogView agentIntentionLogArr={logs} />;

              default:
                return null;
            }
          })()}
        </div>
      </ContentContainer>
    </Root>
  );
}
