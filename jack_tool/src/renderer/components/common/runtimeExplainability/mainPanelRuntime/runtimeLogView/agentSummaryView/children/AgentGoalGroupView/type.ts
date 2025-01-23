import { GoalInfoItem } from 'misc/types/cbdi/cbdi-models-non-flatbuffer';

export type PlanGroupDic = {
  [planTemplateName: string]: {
    active: GoalInfoItem[];
    complete: GoalInfoItem[];
  };
};
