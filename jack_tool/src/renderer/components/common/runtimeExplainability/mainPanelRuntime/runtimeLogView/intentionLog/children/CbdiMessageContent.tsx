import {
  BDILog,
  BDILogAction,
  BDILogCondition,
  BDILogGoal,
  BDILogGoalIntentionResult,
  BDILogIntention,
  BDILogSleep,
  BDILogType,
} from 'misc/types/cbdi/cbdi-types-non-flatbuffer';
import { styled } from '@mui/material';
import { getKeyByValue, removeBeforeFirstDotAndDot } from 'misc/utils/common/commonUtils';

const ContentContainer = styled('div')({
  width: '90%',
  lineHeight: '150%',
});

const ConceptContainer = styled('span')({
  borderRadius: '5%',
  backgroundColor: '#e3e6e8',
  fontWeight: 500,
  padding: '0 5px',
  '&:hover': {
    backgroundColor: '#6dbbfc',
  },
});

function getConceptContainer(conceptName: string | number | undefined) {
  // remove the module prefix from concept name
  return <ConceptContainer>{typeof conceptName === 'number' ? conceptName : removeBeforeFirstDotAndDot(conceptName)}</ConceptContainer>;
}

export default function CbdiMessageContent({ bdiLog }: { bdiLog: BDILog }) {
  let message = <ContentContainer>N/A</ContentContainer>;
  const { senderNode, sender, recipient, timestampUs, eventId, type, ...bdiLogBody } = bdiLog;
  switch (bdiLogBody.logType) {
    case BDILogType.GOAL_STARTED:
      message = <ContentContainer>Goal {getConceptContainer(bdiLogBody.goal)} started</ContentContainer>;
      break;

    case BDILogType.GOAL_FINISHED:
      message = (
        <ContentContainer>
          Goal {getConceptContainer(bdiLogBody.goal)} finished, and the goal{' '}
          {getKeyByValue(BDILogGoalIntentionResult, (bdiLogBody as BDILogIntention).result)?.replace('BDILogGoalIntentionResult_', '')}
          {/* {payload.goalFinished?.result ? 'succeeded' : 'failed'} */}
        </ContentContainer>
      );
      break;

    case BDILogType.SUB_GOAL_STARTED:
      message = <ContentContainer>Subgoal {getConceptContainer(bdiLogBody.goal)} started</ContentContainer>;

      break;

    case BDILogType.SUB_GOAL_FINISHED:
      message = (
        <ContentContainer>
          Subgoal {getConceptContainer(bdiLogBody.goal)} finished, and the subgoal{' '}
          {getKeyByValue(BDILogGoalIntentionResult, (bdiLogBody as BDILogGoal).result)?.replace('BDILogGoalIntentionResult_', '')}
          {/* {payload.subGoalFinished.success ? 'succeeded' : 'failed'} */}
        </ContentContainer>
      );
      break;

    case BDILogType.INTENTION_STARTED:
      message = <ContentContainer>Plan {getConceptContainer((bdiLogBody as BDILogIntention).plan)} started</ContentContainer>;

      break;

    case BDILogType.INTENTION_FINISHED:
      message = (
        <ContentContainer>
          Plan {getConceptContainer((bdiLogBody as BDILogIntention).plan)} finished, and the intention{' '}
          {/* {getConceptContainer(
            BDILogIntentionResults[payload.intentionFinished.result]
          )} */}
          {getKeyByValue(BDILogGoalIntentionResult, (bdiLogBody as BDILogIntention).result)?.replace('BDILogGoalIntentionResult_', '')}
        </ContentContainer>
      );
      break;

    case BDILogType.ACTION_STARTED:
      message = (
        <ContentContainer>
          Action {getConceptContainer((bdiLogBody as BDILogAction).action)} in plan {getConceptContainer((bdiLogBody as BDILogAction).plan)} to handle
          goal {getConceptContainer((bdiLogBody as BDILogAction).goal)} started
        </ContentContainer>
      );

      break;

    case BDILogType.ACTION_FINISHED:
      message = (
        <ContentContainer>
          Action {getConceptContainer((bdiLogBody as BDILogAction).action)} in plan {getConceptContainer((bdiLogBody as BDILogAction).plan)} to handle
          goal {getConceptContainer((bdiLogBody as BDILogAction).goal)} finished, and the action{' '}
          {(bdiLogBody as BDILogAction).success ? 'succeeded' : 'failed'}
        </ContentContainer>
      );
      break;

    case BDILogType.SLEEP_STARTED:
      message = (
        <ContentContainer>
          Sleep {getConceptContainer(Math.floor((bdiLogBody as BDILogSleep).sleepMs / 1000))} seconds in plan{' '}
          {getConceptContainer((bdiLogBody as BDILogSleep).plan)} to handle goal {getConceptContainer(bdiLogBody.goal)} started
        </ContentContainer>
      );

      break;

    case BDILogType.SLEEP_FINISHED:
      message = (
        <ContentContainer>
          Sleep {getConceptContainer(Math.floor((bdiLogBody as BDILogSleep).sleepMs / 1000))} seconds in plan{' '}
          {getConceptContainer((bdiLogBody as BDILogSleep).plan)} to handle goal {getConceptContainer(bdiLogBody.goal)} finished
        </ContentContainer>
      );

      break;

    case BDILogType.CONDITION:
      message = (
        <ContentContainer>
          Condition {getConceptContainer((bdiLogBody as BDILogCondition).condition)} in plan{' '}
          {getConceptContainer((bdiLogBody as BDILogCondition).plan)} to handle goal {getConceptContainer(bdiLogBody.goal)}{' '}
          {(bdiLogBody as BDILogCondition).success ? 'succeeded' : 'failed'}
        </ContentContainer>
      );

      break;

    default:
      break;
  }

  return message;
}
