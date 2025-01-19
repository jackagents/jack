import React, { startTransition } from 'react';
import { List, styled } from '@mui/material';
import { BDILogIntentionOverviewsModel, BDILogIntentionsModel } from 'types/cbdi/cbdi-models-non-flatbuffer';
import { request, response } from 'misc/events/common/cmEvents';
import { useExplainabilityContext } from 'components/common/runtimeExplainability/context/explainabilityContext';
import IntentionGroupView from './children/IntentionGroupView';

interface Props {
  agentIntentionDic: BDILogIntentionOverviewsModel;
}

const Container = styled('ul')({
  display: 'flex',
  flexDirection: 'column',
  border: '2px solid white',
  fontSize: 14,
  height: '100%',
});

const SCRIBE_INTENTION_LIMIT = 10;
const INTENTIONTASKSUB_DURATION = 3000;

function IntentionTracker({ agentIntentionDic }: Props) {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { inspectNodeData } = useExplainabilityContext();
  /* ------------------------------ useRef hooks ------------------------------ */
  const scribeTimerIntentionIdListRef = React.useRef<string[]>([]);
  const intentionTaskSubscribeTimerRef = React.useRef<NodeJS.Timer>();
  const ref = React.useRef<HTMLUListElement | null>(null);
  /* ----------------------------- useState hooks ----------------------------- */
  const [listActive, setListActive] = React.useState(Object.fromEntries(Object.entries(agentIntentionDic).map((x) => [x[0], false])));
  const [intentionsModel, setIntentionsModel] = React.useState<BDILogIntentionsModel>({});

  const handleAddTimerWithIntentionId = (intentionId: string) => {
    if (!scribeTimerIntentionIdListRef.current.includes(intentionId)) {
      const newScribeTimerIntentionIdList = [...scribeTimerIntentionIdListRef.current, intentionId];

      if (newScribeTimerIntentionIdList.length > SCRIBE_INTENTION_LIMIT) {
        const removedIntentionId = newScribeTimerIntentionIdList.shift() as string;
        setListActive({ ...listActive, [removedIntentionId]: false });
      }
      scribeTimerIntentionIdListRef.current = newScribeTimerIntentionIdList;
    }
  };

  const handleRemoveTimerWithIntentionId = (intentionId: string) => {
    if (scribeTimerIntentionIdListRef.current.includes(intentionId)) {
      const index = scribeTimerIntentionIdListRef.current.findIndex((el) => el === intentionId);

      scribeTimerIntentionIdListRef.current.splice(index, 1);
    }
  };

  const onIntentionTasks = (_e: Electron.IpcRendererEvent, data: string) => {
    const newIntentionsModel = JSON.parse(data) as BDILogIntentionsModel | null;

    if (newIntentionsModel) {
      setIntentionsModel(newIntentionsModel);
    }
  };

  const intentionIds = React.useMemo(() => Object.keys(agentIntentionDic), [agentIntentionDic]);

  const handleClickLog = React.useCallback(
    (id: string) => {
      startTransition(() => {
        if (listActive[id] === true) {
          handleRemoveTimerWithIntentionId(id);
        } else {
          handleAddTimerWithIntentionId(id);
        }
        setListActive({ ...listActive, [id]: !listActive[id] });
      });
    },
    [listActive, intentionIds],
  );
  /* ----------------------------- useEffect hooks ---------------------------- */
  React.useEffect(() => {
    const removeIntentionTasksByIdsListener = window.ipcRenderer.setupIpcListener(response.bdilog.intentionTasksByIds, onIntentionTasks);

    return () => {
      removeIntentionTasksByIdsListener();

      clearInterval(intentionTaskSubscribeTimerRef.current);

      scribeTimerIntentionIdListRef.current = [];
    };
  }, []);

  React.useEffect(() => {
    clearInterval(intentionTaskSubscribeTimerRef.current);

    if (scribeTimerIntentionIdListRef.current.length > 0) {
      window.ipcRenderer.invoke(request.bdilog.intentionTasksByIds, scribeTimerIntentionIdListRef.current, inspectNodeData?.agentBusAddress?.id);

      intentionTaskSubscribeTimerRef.current = setInterval(() => {
        window.ipcRenderer.invoke(request.bdilog.intentionTasksByIds, scribeTimerIntentionIdListRef.current, inspectNodeData?.agentBusAddress?.id);
      }, INTENTIONTASKSUB_DURATION);
    }
  }, [JSON.stringify(scribeTimerIntentionIdListRef.current)]);
  return (
    <div>
      <Container id="scroller-intention-tracker" ref={ref}>
        <List style={{ padding: 0, margin: 0 }}>
          {intentionIds.map((intentionId) => (
            <IntentionGroupView
              key={intentionId}
              intentionObj={agentIntentionDic[intentionId]}
              tasks={intentionsModel[intentionId]?.tasks}
              goalId={agentIntentionDic[intentionId].goalId}
              intentionId={intentionId}
              handleClickLog={handleClickLog}
              active={listActive[intentionId]}
            />
          ))}
        </List>
      </Container>
    </div>
  );
}

export default React.memo(IntentionTracker);
