import { removeBeforeFirstDotAndDot } from 'misc/utils/common/commonUtils';
import { BDILog, BDILogAction, BDILogCondition, BDILogIntention, BDILogSleep, BDILogType } from 'types/cbdi/cbdi-types-non-flatbuffer';

/**
 * get bdi log title
 * @param bidLog BDILog
 * @returns bdi log title
 */
export function getBdiLogTitle(bidLog: BDILog) {
  let titleConcept;

  switch (bidLog.logType) {
    case BDILogType.GOAL_STARTED:
    case BDILogType.GOAL_FINISHED:
    case BDILogType.SUB_GOAL_STARTED:
    case BDILogType.SUB_GOAL_FINISHED:
      titleConcept = bidLog.goal || 'N/A';
      break;
    case BDILogType.INTENTION_STARTED:
    case BDILogType.INTENTION_FINISHED:
      titleConcept = (bidLog as BDILogIntention).plan || 'N/A';
      break;
    case BDILogType.ACTION_STARTED:
    case BDILogType.ACTION_FINISHED:
      titleConcept = (bidLog as BDILogAction).action || 'N/A';
      break;
    case BDILogType.SLEEP_STARTED:
    case BDILogType.SLEEP_FINISHED:
      titleConcept = `sleep ${(bidLog as BDILogSleep).sleepMs}`;
      break;
    case BDILogType.CONDITION:
      titleConcept = (bidLog as BDILogCondition).condition || 'N/A';
      break;
    default:
      break;
  }
  // remove the module prefix from task header
  return removeBeforeFirstDotAndDot(titleConcept);
}
