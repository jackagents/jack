// import { PoIType } from '../../../../deprecated/types/iwd/iwdTypes';

export const MODE = {
  MODE_NONE: 'none',
  MODE_SCENARIO_CREATION: 'ScenarioCreation',
  MODE_OUTPUT_DISPLAY: 'outputDisplay',
  MODE_SETTINGS: 'settings',
  MODE_SEARCH: 'search',
  MODE_ADD: 'add',
};

export const ZONE_TYPE = {
  NONE: 0,
  NO_GO: 1,
  // LAND: 2,
  // SEA: 3,
  GEO_FENCE: 2,
};

export const ZONE_COLOR = {
  NONE: 'white',
  NO_GO: 'black',
  LAND: 'green',
  SEA: 'yellow',
};

export const ZONE_COLOR_TYPE = Object.values(ZONE_COLOR);

// export const BUILD_MODE = {
//   NONE: 0,
//   ZONE: 1,
//   ENTITY: 2,
//   LOAD: 3,
//   // Don't add value 4
//   LOADING: 5,
//   LOADED: 6,
//   CLOSE: 7,
//   CONFIG_ENTITY: 8,
//   SELECT_DA: 9,
//   CONFIGURING: 10,
//   MOVE_ENTITY: 11,
//   MOVE_END: 12,
//   PLACE_ENTITY: 13,
//   BOUNDARY: 14,
//   CSV: 15,
//   RUN_MATLAB: 16,
// };

export const DEFAULT_MAP_CENTER: [number, number] = [-37.7487967579053, 145.10940412400754];

export const SCALED_VALUE = [0.25, 0.5, 1, 2, 4, 8];
export const CREATOR_BUTTON_ID = {
  SAVE: 0,
  MAP: 1,
  INTRUDER: 2,
  POI: 3,
  VEHICLE: 4,
  SENSOR: 5,
  ENABLE: 6,
  DISABLE: 7,
  SPEED: 8,
  BOUNDARY: 9,
};

export const DEFAULT_SLIDER_VALUE_INDEX = 2;
export const CREATOR_MENU = {
  CONFIG: 'Config',
  ASSETS: 'Assets',
  MAP: 'Map',
  BOUNDARY: 'Boundary',
  VEHICLE_CONTROL: 'Vehicle Control',
  SIMULATION: 'Simulation',
  NONE: 'None',
  AGENTS: 'Agents',
  SEARCH: 'Filter',
  CHOOSE: 'Choose entities',
  NON_GEO_SPA: 'Non placed',
  FLIGHT: 'Unknown', // This is missing in client@1.5.0 - using unknown string for now
};

export const BUILDER_EVENT_DATA_TYPE = {
  SimFrameMaker: 'SimFrameMaker',
  SimCreateEntity: 'SimCreateEntity',
  SimCreateZone: 'SimCreateZone',
  SimUpdateComponent: 'SimUpdateComponent',
};

export const MENUBAR_EVENT_TYPE = {
  NONE: 0,
  SAVE: 1,
  SAVING: 2,
  SAVED: 3,
  LOAD: 4,
  LOADING: 5,
  LOADED: 6,
  CLOSE: 7,
  CLOSING: 8,
  CLOSED: 9,
};

export const Z_INDEX = {
  MENU: 400,
  CONTROL: 800,
  MODAL: 1000,
  MAP_OVERLAY: 5000,
};

// export const DEFAULT_POINT_OF_INTEREST = PoIType.UNKNOWN;

export const CONSTANT_STRING = {
  BTN_SAVE: 'btnSave',
  BTN_CANCEL: 'btnCancel',
  LAT: 'lat',
  LNG: 'lng',
  ALT: 'alt',
  POINTS_OF_INTEREST: 'Points of Interest',
  AGENTS: 'Agents',
  FILTER: 'Filter',
  SORT_LABEL: {
    DATE_CREATED: 'Date Created',
    NAME: 'Name',
    TYPE: 'Type',
    STATUS: 'Status',
  },
  NAME: 'name',
  ID: 'Id',
  TYPE: 'Type',
  STATUS: 'Status',
};

export const BELIEF_SET_KEYS = {
  POSITION: 'positionbeliefs',
};

export const NOTI_MESS = {
  CHOOSE_ZONE: 'Please choose zone type',
};

export const MAXIMUM_LOG_MSGS = 200;

export const MAXIMUM_INTENTIONS = 100;

export const SERVER_LOG_HISTORY_THRESHOLD = 1000;

export enum EApplicationName {
  iba = 'iba',
  tewa = 'tewa',
  iwd = 'iwd',
  editor = 'editor',
}

export enum ConnectStatus {
  connecting,
  connected,
  disconnected,
}

export enum ConnectMode {
  playback,
  live,
}

/**
 * task status color enum for intention log
 */
export enum TaskStatusColorEnum {
  STARTED = '#264653',
  FAILED = '#e76f51',
  DROPPED = '#e9c46a',
  SUCCESS = '#2a9d8f',
}

export enum TaskStatus {
  STARTED = 'started',
  FAILED = 'failed',
  DROPPED = 'dropped',
  SUCCESS = 'success',
}
