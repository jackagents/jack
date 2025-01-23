import { ProcessedIntentionTask } from 'types/cbdi/cbdi-models-non-flatbuffer';
import { bdiStatusIcon } from 'misc/icons/cbdi/cbdiIcons';
import { styled } from '@mui/material';
import { formatMicrosecondOffset } from 'misc/utils/common/rendererUtils';
import { BDILogLevel } from 'types/cbdi/cbdi-types-non-flatbuffer';
import { CBDIEditorPlanNodeType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { getTaskInfo } from './Helper';

const TaskContainer = styled('div')(({ severitycolor }: { severitycolor: string }) => ({
  display: 'flex',
  justifyContent: 'space-between',
  backgroundColor: severitycolor,
  margin: '5px 0',
  padding: 5,
  border: '3px solid transparent',
  '&:hover': {
    border: '3px solid white',
  },
}));

interface Props {
  task: ProcessedIntentionTask;
}

export default function IntentionTaskView(props: Props) {
  const { reasoning, timestamp, taskStatus, level, taskType } = props.task;
  const { severityColor, taskIcon, taskContent } = getTaskInfo(props.task);

  return (
    <TaskContainer severitycolor={severityColor}>
      <div style={{ flexBasis: '40%' }}>
        <div style={{ display: 'flex', alignItems: 'center' }}>
          <img width={15} height={15} src={taskIcon} alt="" />
          <div style={{ paddingLeft: 5 }}>{taskContent}</div>
        </div>
        <div>{formatMicrosecondOffset(timestamp)}</div>
      </div>

      <div>
        <div hidden={level !== BDILogLevel.CRITICAL}>Critical Decision</div>
        <div hidden={taskType !== CBDIEditorPlanNodeType.ActionPlanNodeType || reasoning === undefined}>{reasoning}</div>
      </div>
      <div
        style={{
          flexBasis: '30%',
          paddingRight: 10,
          display: 'flex',
          flexDirection: 'column',
          justifyContent: 'center',
          alignItems: 'flex-end',
        }}
      >
        <img width={20} height={20} src={bdiStatusIcon[taskStatus]} alt="" />
        <div>{taskStatus}</div>
      </div>
    </TaskContainer>
  );
}
