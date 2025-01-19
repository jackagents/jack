import { styled } from '@mui/material';
import { useExplainabilityContext } from 'components/common/runtimeExplainability/context/explainabilityContext';
import { request, response } from 'projectEvents/common/cmEvents';
import React from 'react';
import ReactLoading from 'react-loading';
import { TCBDIIntentionForNotification } from 'types/cbdi/cbdi-models-non-flatbuffer';
import { Fluid } from 'components/common/base/BaseContainer';
import CloseIcon from 'assets/common/icons/close-w-10.png';
import { ConnectMode } from 'misc/constant/common/cmConstants';
import { removeBeforeFirstDotAndDot } from 'misc/utils/common/commonUtils';
import ReactFlowPlanGraph from './reactflowPlanGraph/ReactFlowPlanGraph';
import DebugControlView from './debugControlView/DebugControlView';

/* --------------------------------- Styles --------------------------------- */
const Root = styled(Fluid)({
  display: 'flex',
  flexDirection: 'column',
});

const TitleContainer = styled('div')({
  color: '#ffffff',
  fontSize: 14,
  backgroundColor: '#424242',
  flex: 0,
  height: 48,
  zIndex: 99,
  minHeight: 48,
  display: 'flex',
  justifyContent: 'space-between',
  alignItems: 'center',
  paddingLeft: 20,
  '& > :first-of-type': {
    flex: '1 1 auto',
  },
  '& > :last-of-type': {
    flex: '0 0 auto',
  },
});

const TitleCloseIcon = styled('img')({
  width: 20,
  height: 20,
  marginLeft: 8,
  cursor: 'pointer',
  padding: 4,
  borderRadius: '50%',
  '&:hover': {
    backgroundColor: 'gray',
  },
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
  zIndex: 9,
});

const ContentContainer = styled('div')({
  flexGrow: 1,
  maxHeight: 'calc(100% - 48px)',
  overflowY: 'auto',
});

/* -------------------------------- Properties -------------------------------- */
let intentionDataSubTimer: NodeJS.Timer;
const LOADINGTIMEOUT_DURATION = 10000;
let intentionDataLoadingTimeout: NodeJS.Timeout;
/* -------------------------------- Interface ------------------------------- */
interface Props {
  setIsPlanViewVisible: (isVisible: boolean) => void;
}
/* ----------------------------- PlanRuntimeView ---------------------------- */
export default function PlanRuntimeView({ setIsPlanViewVisible }: Props) {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { inspectAgentGoal, connectMode, isLivePlayback, setInspectAgentGoal } = useExplainabilityContext();

  /* ----------------------------- useState hooks ----------------------------- */
  const [isLoading, setIsLoading] = React.useState<boolean>(false);
  const [isTimeout, setIsTimeout] = React.useState<boolean>(false);
  const [currentIntentionData, setCurrentIntentionData] = React.useState<TCBDIIntentionForNotification>();

  /* -------------------------------- Functions ------------------------------- */
  const endLoadingTimer = () => {
    setIsLoading(false);
    setIsTimeout(false);
    clearTimeout(intentionDataLoadingTimeout);
    setCurrentIntentionData(undefined);
  };

  const startLoadingTimer = () => {
    setIsLoading(true);

    // Grab data first time
    window.ipcRenderer.invoke(request.cbdi.getIntentionByAgentGoalId, inspectAgentGoal);

    // Start timer for timeout
    intentionDataLoadingTimeout = setTimeout(() => {
      setIsLoading(false);
      setIsTimeout(true);
    }, LOADINGTIMEOUT_DURATION);
  };

  /* -------------------------------- Callbacks ------------------------------- */
  const onResponseGetIntentionWithContextById = (_e: Electron.IpcRendererEvent, data: string) => {
    setCurrentIntentionData((prev) => {
      if (data && data !== JSON.stringify(prev)) {
        const mIntention = JSON.parse(data) as TCBDIIntentionForNotification | null;
        if (mIntention !== null) {
          console.log('intention plan data', mIntention);
          clearTimeout(intentionDataLoadingTimeout);
          setIsTimeout(false);
          setIsLoading(false);

          return mIntention;
        }
      }
      return prev;
    });
  };

  const onInspectingIntentionUnavailable = () => {
    setInspectAgentGoal(undefined);
  };

  /* ------------------------------ useMemo hooks ----------------------------- */
  const showingDebugControl = React.useMemo(() => {
    if (connectMode === ConnectMode.playback) {
      return true;
    }
    if (connectMode === ConnectMode.live && isLivePlayback) {
      return true;
    }
    return false;
  }, [connectMode, isLivePlayback]);

  /* ----------------------------- useEffect hooks ---------------------------- */
  React.useEffect(() => {
    const removeOnResponseGetIntentionForNotificationById = window.ipcRenderer.setupIpcListener(
      response.cbdi.getIntentionForNotificationById,
      onResponseGetIntentionWithContextById,
    );

    const cleanupOnInspectingIntentionUnavailable = window.ipcRenderer.setupIpcListener(
      response.playback.inspectingIntentionUnavailable,
      onInspectingIntentionUnavailable,
    );

    return () => {
      removeOnResponseGetIntentionForNotificationById();
      cleanupOnInspectingIntentionUnavailable();

      clearInterval(intentionDataSubTimer);
      clearTimeout(intentionDataLoadingTimeout);
    };
  }, []);

  React.useEffect(() => {
    endLoadingTimer();

    // This will either tell backend to drop inspector or change the inspecting intention id
    window.ipcRenderer.send(request.cbdi.inspectPlanGoalIdChanged, inspectAgentGoal);

    if (inspectAgentGoal) {
      startLoadingTimer();

      setIsPlanViewVisible(true);
    } else {
      setIsPlanViewVisible(false);
    }
  }, [inspectAgentGoal]);

  return (
    <Root key="plan-runtime-view-root" id="plan-runtime-view-root">
      <TitleContainer>
        <div style={{ display: 'flex', minWidth: 0 }}>
          <div
            style={{
              whiteSpace: 'nowrap',
              overflow: 'hidden',
              textOverflow: 'ellipsis',
            }}
          >
            {currentIntentionData?.agentAddress?.name}::
            {removeBeforeFirstDotAndDot(currentIntentionData?.goalTemplateName)}
          </div>
          <TitleCloseIcon
            src={CloseIcon}
            alt="Close"
            onClick={(e) => {
              e.stopPropagation();
              setInspectAgentGoal(undefined);
            }}
          />
        </div>
        {showingDebugControl && <DebugControlView />}
      </TitleContainer>

      <ContentContainer id="plan-runtime-view-content-container">
        {/* Timeout display */}
        {isTimeout && <LoadingRoot>{`Cannot find plan for goal id: ${inspectAgentGoal}`}</LoadingRoot>}

        {isLoading && (
          <LoadingRoot>
            <ReactLoading type="spinningBubbles" color="gray" width="20%" height="20%" />
          </LoadingRoot>
        )}

        {currentIntentionData && <ReactFlowPlanGraph intentionData={currentIntentionData} />}
      </ContentContainer>
    </Root>
  );
}
