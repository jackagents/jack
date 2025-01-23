import { GoalInfoItem } from 'misc/types/cbdi/cbdi-models-non-flatbuffer';
import { MessageData } from 'misc/types/cbdi/cbdi-types-non-flatbuffer';

export const getGoalContextMsgArr = (goalContextMsg: MessageData[]): Record<string, any>[] => {
  const goalContextMsgArr: Record<string, any>[] = [];
  if (goalContextMsg) {
    goalContextMsg.forEach((el) => {
      if (el && Object.keys(el).length !== 0) {
        Object.keys(el).forEach((fieldKey) => {
          goalContextMsgArr.push({
            [fieldKey]: JSON.stringify(el[fieldKey]),
          });
        });
      }
    });
  }
  return goalContextMsgArr;
};

export type AgentGoalGroupDic = {
  [goalTemplateName: string]: {
    active: GoalInfoItem[];
    complete: GoalInfoItem[];
  };
};
