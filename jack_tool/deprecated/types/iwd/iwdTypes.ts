import { LatLng } from 'leaflet';
import {
  BusAddress,
  Delegation,
  // PursueEvent,
  Register,
} from 'types/cbdi/auto-generated-types';
// import {
//   BusAddress,
//   Delegation,
//   PursueEvent,
//   Register,
// } from 'types/cbdi/cbdiTypes';
import { GeoPosition, IwdVehicleStatus } from './iwdVehicleModel';

export enum SelectableElement {
  NONE = 'none',
  AGENT = 'agent',
  POI = 'poi',
  ZONE = 'zone',
  VEHICLE = 'vehicle',
}

export enum PoIType {
  UNKNOWN = 'unknown',
  HOSTILE = 'hostile',
  FRIENDLY = 'friendly',
}

export enum TabIndex {
  poi = 'poi',
  agent = 'agent',
  vehicle = 'vehicle',
  intruder = 'intruder',
  sensor = 'sensor',
}

export interface IContextMenuItem {
  label: string;
  callback: () => void;
}

export interface CustomMenu {
  searchMenu: boolean;
  creatorMenu: boolean;
  settingMenu: boolean;
  simMapMenu: boolean;
  explainability: boolean;
  scenarioEditor: boolean;
}

// Template Prefab
export interface Prefab {
  name: string;
  children: any[];
  components: string[];
}

export interface SavedEntity {
  UUID: string;
  components: string[];
  type: string;
  position: [number, number];
  componentData: any;
}

export interface SavedZone {
  UUID: string;
  bounds: [number, number][][];
  type: number;
  color: string;
  lastChanged: string;
}

export interface SavedData {
  zone: SavedZone[];
  entities: SavedEntity[];
}

export interface LoadResult {
  savedData: SavedData;
  nonGeoSpaEntities: SavedEntity[];
}

export interface OutputResult {
  metric: number;
  no_of_TO: number;
  mcost: number;
  killedTO: number;
  activeTO: number;
  activeDA: number;
  killratio: number;
  mlaunched: number;
  mlaunch: number;
  llaunch: number;
  survivability: number;
  armcost: number;
}

export type BeliefArray = [string, string | number | any];

export type BeliefSetsArray = [string, BeliefArray[]];

export type BeliefMap = Map<string, string | number | any>;

export type BeliefSets = BeliefSetsArray[] | BeliefMap;

// export type Agent = Register &
//   PursueEvent & {
//     timestamp: number;
//     members: BusAddress[];
//     auctions: Delegation[];
//     delagations: Delegation[];
//     beliefSets: BeliefSets;
//   };
export type Agent = Register & {
  timestamp: number;
  members: BusAddress[];
  auctions: Delegation[];
  delagations: Delegation[];
  beliefSets: BeliefSets;
};

export interface PointOfInterestType {
  id: string;
  position: LatLng;
  type: PoIType;
  name?: string;
  dateCreated: string;
}

export interface CurrentAgent {
  id: string;
}

export type Zone = {
  id: string;
  type: number;
  polygons: LatLng[][];
  lastChanged: string;
};

export interface CurrentVehicle extends CurrentAgent {
  position: GeoPosition;
  status: IwdVehicleStatus;
}

export type CurrentZone = SavedZone;

export type CurrentSelectElement = {
  type: SelectableElement;
  value:
    | CurrentAgent
    | PointOfInterestType
    | CurrentZone
    | CurrentVehicle
    | undefined;
} | null;

export type FilterCheckBox = {
  hostile: boolean;
  friendly: boolean;
  unknown: boolean;
};

export enum BuildMode {
  NONE = 'none',
  ZONE = 'zone',
  CLOSE = 'close',
  LOAD = 'load',
  VEHICLE = 'vehicle',
  REBASE_VEHICLE = 'rebase-vehicle',
  PATH_MISSION = 'path-mission',
  INTRUDER_MISSION = 'intruder-mission',
  SCOUT_MISSION = 'scout-mission',
}

export enum DrivingMode {
  AUTO = 'auto',
  MANUAL = 'manual',
  SAFETY_LOCK = 'safety-lock',
}

export enum IwdIconButtonId {
  none = 'none',
  activate = 'activate',
  auto = 'auto',
  manual = 'manual',
  safety = 'safety',
  base = 'base',
  path = 'path',
  intruder = 'intruder',
  scout = 'scout',
}

export interface KonvaAgent {
  x: number;
  y: number;
  id: string;
  name: string;
  rotation: number;
  historyPath: number[];
  patrolX?: number;
  patrolY?: number;
}
