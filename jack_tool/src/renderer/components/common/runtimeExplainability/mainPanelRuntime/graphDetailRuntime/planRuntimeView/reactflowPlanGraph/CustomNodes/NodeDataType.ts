import { TaskStatus } from 'misc/constant/common/cmConstants';
import { BDILogLevel } from 'misc/types/cbdi/cbdi-types-non-flatbuffer';
import { CBDIEditorJSONPlanNode } from 'misc/types/cbdiEdit/cbdiEditTypes';

export type ActiveNodeData = {
  taskStatus?: TaskStatus;
  bdiLogLevel?: BDILogLevel;
  remark?: string;
  subGoalId?: string;
  isCurrentTask?: boolean;
} & CBDIEditorJSONPlanNode;
