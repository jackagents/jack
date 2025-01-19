import { NOTI_MESS } from 'constant/common/cmConstants';

export const MODE = {
  MODE_NONE: 'none',
  MODE_SCENARIO_CREATION: 'ScenarioCreation',
  MODE_OUTPUT_DISPLAY: 'outputDisplay',
  MODE_SETTINGS: 'settings',
  MODE_SEARCH: 'search',
  MODE_ADD: 'add',
  MODE_SIM_MAP: 'sim-map',
  MODE_RUNTIME_EXPLAINABILITY: 'runtime-explainability',
  MODE_SCENARIO_EDITOR: 'scenario_editor',
};

export const IWD_NOTI_MESS = {
  ...NOTI_MESS,
  PLACE_VEHICLE: 'Please place vehicle on map',
};
