import React from 'react';
import { IntentionSeverityColorArr, bdiStatusIcon, nodeIcon } from 'misc/icons/cbdi/cbdiIcons';
import { BDILogLevel } from 'types/cbdi/cbdi-types-non-flatbuffer';
import { Skeleton, styled } from '@mui/material';
import { formatMicrosecondOffset } from 'misc/utils/common/rendererUtils';
import PlayArrowIcon from '@mui/icons-material/PlayArrow';
import Menu from '@mui/material/Menu';
import MenuItem from '@mui/material/MenuItem';
import { useExplainabilityContext } from 'components/common/runtimeExplainability/context/explainabilityContext';
import { BDILogIntentionOverviewModel, BDILogIntentionTask, ProcessedIntentionTask } from 'types/cbdi/cbdi-models-non-flatbuffer';
import { request } from 'misc/events/common/cmEvents';
import { CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { removeBeforeFirstDotAndDot } from 'misc/utils/common/commonUtils';
import IntentionTaskView from './IntentionTaskView';
import { processTasks } from '../processTaskWorker/processTaskWorker';

/* --------------------------------- Styles --------------------------------- */

const StyledMenu = styled(Menu)({
  '& .MuiList-root': {
    backgroundColor: 'white',
  },
});

const StyledMenuItem = styled(MenuItem)({
  root: {
    fontWeight: 'normal',
    fontSize: 13,
    color: '#a6a6a6',
    minWidth: 150,
    lineHeight: 1,
  },
});

const IntentionContainer = styled('li')({
  color: '#000000',
  border: '2px solid white',
  marginBottom: 10,
});

const TitleContainer = styled('div')(({ severitycolor }: { severitycolor: string }) => ({
  display: 'flex',
  justifyContent: 'space-between',
  backgroundColor: severitycolor,
  border: '3px solid transparent',
  '&:hover': {
    border: '3px solid white',
  },
}));

const TaskContainer = styled('div')({
  paddingLeft: 20,
});

/* --------------------------- IntentionGroupView --------------------------- */
interface Props {
  intentionObj: BDILogIntentionOverviewModel;
  goalId: string;
  intentionId: string;
  handleClickLog: (id: string) => void;
  active: boolean;
  tasks: BDILogIntentionTask[] | undefined;
}
/* -------------------------------- Properties -------------------------------- */

const LOADINGTIMEOUT_DURATION = 10000;
/* --------------------------- IntentionGroupView --------------------------- */
function IntentionGroupView({ intentionObj, active, handleClickLog, goalId, intentionId, tasks }: Props) {
  /* ------------------------------ useRef hooks ------------------------------ */
  const intentionTaskLoadingTimeoutRef = React.useRef<NodeJS.Timeout>();

  /* ---------------------------- useContext hooks ---------------------------- */
  const { project, setInspectAgentGoal, inspectNodeData } = useExplainabilityContext();
  /* ----------------------------- useState hooks ----------------------------- */
  const [processedTasks, setProcessedTasks] = React.useState<ProcessedIntentionTask[]>([]);
  const [isLoadingTasks, setIsLoadingTasks] = React.useState<boolean>(false);
  const [isTimeout, setIsTimeout] = React.useState<boolean>(false);
  const [contextMenu, setContextMenu] = React.useState<{
    mouseX: number;
    mouseY: number;
  } | null>(null);

  /* -------------------------------- Callbacks ------------------------------- */
  const handleContextMenu = (event: React.MouseEvent) => {
    event.preventDefault();
    setContextMenu(
      contextMenu === null
        ? {
            mouseX: event.clientX + 2,
            mouseY: event.clientY - 6,
          }
        : null,
    );
  };

  const handleClose = () => {
    setContextMenu(null);
  };

  // TODO
  // Pin inspect id
  const handleInspectPlan = (e: React.MouseEvent<HTMLLIElement, MouseEvent>) => {
    e.stopPropagation();
    setInspectAgentGoal({ agentId: inspectNodeData?.agentBusAddress?.id, goalId });
    handleClose();
  };

  /* ----------------------------- useEffect hooks ---------------------------- */

  React.useEffect(() => {
    if (active !== undefined) {
      clearTimeout(intentionTaskLoadingTimeoutRef.current);
      setProcessedTasks([]);
      if (active === true) {
        setIsLoadingTasks(true);
        window.ipcRenderer.invoke(request.bdilog.intentionTasksByIds, intentionId);

        intentionTaskLoadingTimeoutRef.current = setTimeout(() => {
          setIsLoadingTasks(false);
          setIsTimeout(true);
        }, LOADINGTIMEOUT_DURATION);
      }
    }
    return () => {
      clearTimeout(intentionTaskLoadingTimeoutRef.current);
    };
  }, [active]);

  React.useEffect(() => {
    if (tasks === undefined) {
      setProcessedTasks([]);
    } else {
      const newProcessedTasks = processTasks(tasks) as ProcessedIntentionTask[];
      if (newProcessedTasks) {
        clearTimeout(intentionTaskLoadingTimeoutRef.current);
        setIsTimeout(false);
        setIsLoadingTasks(false);
        setProcessedTasks(newProcessedTasks);
      }
    }
  }, [tasks]);

  /* ------------------------------- Components ------------------------------- */
  const { startingTimestamp, level, planTemplateName, intentionStatus, goalTemplateName } = intentionObj;

  const severityColor = IntentionSeverityColorArr[level || BDILogLevel.NORMAL];
  const scroller = document.getElementById(`scroller-intention-tracker`);

  return (
    <IntentionContainer>
      <div
        style={{
          height: 20,
          display: 'flex',
          justifyContent: 'center',
          alignItems: 'center',
          backgroundColor: '#786c6b',
          color: 'white',
        }}
      >
        {removeBeforeFirstDotAndDot(goalTemplateName)}
      </div>
      <TitleContainer
        onContextMenu={handleContextMenu}
        onClick={() => {
          handleClickLog(intentionId);
          if (scroller && scroller.scrollTop <= 0) {
            scroller.scrollTo(0, 1);
          }
        }}
        severitycolor={severityColor}
        className={active ? 'checked' : ''}
      >
        <div style={{ flexBasis: '40%', display: 'flex' }}>
          <PlayArrowIcon
            style={{
              color: active ? '#06b6ef' : '#815ba4',
              transform: active ? 'rotate(90deg)' : '',
            }}
          />
          <div
            style={{
              display: 'flex',
              flexDirection: 'column',
              justifyContent: 'center',
              alignItems: 'flex-start',
            }}
          >
            <img alt="node-icon" width={20} src={nodeIcon[CBDIEditorRootConceptType.PlanConceptType]} />
            <div> {formatMicrosecondOffset(startingTimestamp)}</div>
          </div>
        </div>
        <div
          style={{
            display: 'flex',
            flexDirection: 'column',
            justifyContent: 'center',
            alignItems: 'center',
            flexBasis: '20%',
          }}
        >
          <div hidden={intentionObj.level !== BDILogLevel.CRITICAL}>Critical Decision</div>
          <div style={{ display: 'flex', flexBasis: '40%' }}>
            <div>{removeBeforeFirstDotAndDot(planTemplateName)}</div>
          </div>
        </div>
        <div
          style={{
            display: 'flex',
            flexDirection: 'column',
            alignItems: 'flex-end',
            justifyContent: 'center',
            flexBasis: '40%',
          }}
        >
          <img alt="bdi-status-icon" width={20} height={20} src={bdiStatusIcon[intentionStatus]} />
          <div>{intentionStatus}</div>
        </div>
        <StyledMenu
          open={contextMenu !== null}
          onClose={handleClose}
          style={{ paddingTop: '0!important', paddingBottom: '0!important' }}
          anchorReference="anchorPosition"
          anchorPosition={contextMenu !== null ? { top: contextMenu.mouseY, left: contextMenu.mouseX } : undefined}
        >
          <StyledMenuItem
            style={{
              color: !project ? 'red' : 'black',
            }}
            disabled={!project}
            onClick={handleInspectPlan}
          >
            Inspect plan
            {!project ? ' disabled (no model file loaded)' : null}
          </StyledMenuItem>
        </StyledMenu>
      </TitleContainer>

      {/* Tasks */}

      {(() => {
        if (!active) {
          return null;
        }
        if (isLoadingTasks) {
          return (
            <div
              style={{
                display: 'flex',
                justifyContent: 'center',
                alignItems: 'center',
                padding: '10px 20px',
              }}
            >
              <Skeleton variant="rectangular" width="100%" height={40} />
            </div>
          );
        }
        if (isTimeout) {
          return (
            <div
              style={{
                display: 'flex',
                justifyContent: 'center',
                alignItems: 'center',
                padding: '10px 20px',
              }}
            >{`Cannot find tasks for intention id: ${intentionId}`}</div>
          );
        }
        if (processedTasks.length > 0) {
          return (
            <TaskContainer>
              {processedTasks.map((task, index) => (
                <IntentionTaskView key={index as number} task={task} />
              ))}
            </TaskContainer>
          );
        }
        return null;
      })()}
    </IntentionContainer>
  );
}

export default React.memo(IntentionGroupView);
