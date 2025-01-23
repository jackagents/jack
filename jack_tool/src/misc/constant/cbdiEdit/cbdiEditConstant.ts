import {
  CBDIEditorOtherCategoryType,
  CBDIEditorRootConceptType,
  CBDIEditorTBasicMessageSchema,
  PlanOrderType,
} from 'misc/types/cbdiEdit/cbdiEditTypes';

export const MODE_NONE = 'none';
export const MODE_PROJECT = 'project';
export const MODE_SEARCH = 'search';
export const MODE_RUNTIME = 'runtime';
export const MODE_SETTINGS = 'settings';

export const DEFAULT_PLAN_POLICY = {
  use_plan_list: false,
  plan_list: [],
  plan_order: PlanOrderType.ExcludePlanAfterAttempt,
  plan_loop: 1,
};

export const DELAY_TIME = {
  GRAPH: 0,
  AGENT_JSON_VIEW: 20,
  INTENTIONS_VIEW: 40,
};

export const EDITOR_OVERVIEW_MODULECONCEPT = {
  uuid: CBDIEditorOtherCategoryType.OverviewType,
  module: '',
  name: CBDIEditorOtherCategoryType.OverviewType,
};

export const LOADING_PAGE_CLOSE_BUTTON_NAME = {
  DISCONNECT_WS: 'Disconnect WS',
};

export const CONCEPTWIDTH = 80;
export const CONCEPTHEIGHT = 80;
export const POINTWIDTH = CONCEPTWIDTH / 2;
export const POINTHEIGHT = CONCEPTHEIGHT / 2;
export const PLANNODEFONTSIZE = CONCEPTWIDTH / 6;

export const MAXPLANEDITORSCALE = 1;
export const MINPLANEDITORSCALE = 0.3;

export const FIELDTYPES: (CBDIEditorTBasicMessageSchema | 'Custom' | 'Enum')[] = [...Object.values(CBDIEditorTBasicMessageSchema), 'Custom', 'Enum'];

export const treeRootCategory = Object.values(CBDIEditorRootConceptType);
