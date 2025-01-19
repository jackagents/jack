import { ReactElement } from 'react';
import {
  CBDIEditorAgentGoal,
  CBDIEditorConceptSchema,
  CBDIEditorEventMessageSchema,
  CBDIEditorMessageFieldSchema,
  CBDIEditorMessageWidgetSchema,
  CBDIEditorModuleConceptWithId,
  CBDIEditorObject,
  CBDIEditorJSONPlanEdge,
  CBDIEditorJSONPlanNode,
  CBDIEditorPlanNodeType,
  CBDIEditorRootConceptAddition,
  CBDIEditorServiceTopicSchema,
  CBDIEditorSharedMessage,
  EnumField,
  ModuleConcept,
  PlanOrderType,
  IPlanEdge,
  IPlanNode,
  PlanEditorNodeType,
  TDropCondition,
  TPreCondition,
  TSatisfied,
  Vec2Type,
} from './cbdiEditTypes';

export interface IModelError {
  keys: string[];
  machineKeys: string[];
  error_type: string;
  error_string: string;
}

export interface IModelWarning {
  title: string;
  detail?: ReactElement;
  fixHandler?: () => void;
}

/**
 * Root level cbdi editor `message`.
 */
export type CBDIEditorRootMessageSchema = CBDIEditorConceptSchema & {
  component: boolean;
  /**
   * Messages accept a list of widgets that the editor uses to make available a custom UI widget to edit the field.
   */
  editor: CBDIEditorMessageWidgetSchema[];
  fields: CBDIEditorMessageFieldSchema[];
};

/**
 * Root level cbdi editor `enum`
 */
export type CBDIEditorRootEnumSchema = CBDIEditorConceptSchema & {
  fields: EnumField[];
};

/**
 * Root level cbdi editor `event`
 */
export type CBDIEditorRootEventSchema = CBDIEditorConceptSchema & {
  message: CBDIEditorEventMessageSchema;
  /**
   * Events can additionally be configured to require an entity.
   * When this flag is set it indicates to the scenario editor that when this event is added
   * it must enter ’entity picking mode’ to select the source entity that the event will originate from.
   */
  requires_entity: boolean;
};

/**
 * Root level cbdi editor `resource`
 */
export type CBDIEditorRootResourceSchema = CBDIEditorConceptSchema & {
  type: string;
  min: number;
  max: number;
};

/**
 * Root level cbdi editor `action`
 */
export type CBDIEditorRootActionSchema = CBDIEditorConceptSchema & {
  request: ModuleConcept;
  reply: ModuleConcept;
  /**
   * The feedback message is assigned on each action which indicates the message type
   * that will be returned as a progress update during execution of actions
   */
  feedback: ModuleConcept;
};

/**
 * Root level cbdi editor `goal`
 */
export type CBDIEditorRootGoalSchema = CBDIEditorConceptSchema &
  TPreCondition &
  TDropCondition &
  TSatisfied & {
    heuristic: boolean;
    query_messages: ModuleConcept[];
    resources: ModuleConcept[];
    message: ModuleConcept;
  };

/**
 * Root level cbdi editor `service`
 */
export type CBDIEditorRootServiceSchema = CBDIEditorConceptSchema & {
  action_handlers: ModuleConcept[];
  /**
   * Topics are the messages that a service is capable of publishing to agents that are linked to it
   */
  topics: CBDIEditorServiceTopicSchema[];
};

/**
 * Root level cbdi editor `entity`
 */
export type CBDIEditorRootEntitySchema = CBDIEditorConceptSchema & {
  /**
   * List of children entities
   */
  children: ModuleConcept[];

  /**
   * Services that entity is using, services and messages are splited in version 0.6 because Daniel Chen wants to make it easier to maintain.
   */
  services: ModuleConcept[];

  /**
   * Actually components
   */
  messages: ModuleConcept[];

  /**
   * Doyle Thai mentioned about cbdi hack need for this flag ???
   */
  agent: boolean;
};

/**
 * Root level cbdi editor `plan`
 */
export type CBDIEditorRootPlanSchema = CBDIEditorConceptSchema &
  TPreCondition &
  TDropCondition & {
    query_messages: ModuleConcept[];
    effects: boolean;
    handles: ModuleConcept;
    tasks: CBDIEditorJSONPlanNode[];
    edges: CBDIEditorJSONPlanEdge[]; // Use the same type since its only basic type
  };

/**
 * Root level cbdi editor `tactic`
 */
export type CBDIEditorRootTacticSchema = CBDIEditorConceptSchema & {
  use_plan_list: boolean;
  plan_list: ModuleConcept[];
  plan_order: PlanOrderType;
  plan_loop: number;
  goal: ModuleConcept;
};

/**
 * Root level cbdi editor `role`
 */
export type CBDIEditorRootRoleSchema = CBDIEditorConceptSchema & {
  goals: ModuleConcept[];
  messages: CBDIEditorSharedMessage[];
};

/**
 * Root level cbdi editor `agent`
 */
export type CBDIEditorRootAgentSchema = CBDIEditorConceptSchema & {
  action_handlers: ModuleConcept[];
  beliefs: ModuleConcept[];
  plans: ModuleConcept[];
  resources: ModuleConcept[];
  roles: ModuleConcept[];
  message_handlers: ModuleConcept[];
  services: ModuleConcept[];
  goals: CBDIEditorAgentGoal[];
};

/**
 * Root level cbdi editor `team`
 */
export type CBDIEditorRootTeamSchema = CBDIEditorRootAgentSchema;

/**
 * Saved Out Project Model
 */
export type CBDIEditorModuleModel = {
  project: ModuleProjectInfo;
  enums: CBDIEditorRootEnumSchema[];
  teams: CBDIEditorRootTeamSchema[];
  agents: CBDIEditorRootAgentSchema[];
  tactics: CBDIEditorRootTacticSchema[];
  roles: CBDIEditorRootRoleSchema[];
  actions: CBDIEditorRootActionSchema[];
  goals: CBDIEditorRootGoalSchema[];
  plans: CBDIEditorRootPlanSchema[];
  resources: CBDIEditorRootResourceSchema[];
  messages: CBDIEditorRootMessageSchema[];
  services: CBDIEditorRootServiceSchema[];
  entities: CBDIEditorRootEntitySchema[];
  events: CBDIEditorRootEventSchema[];
};

/* --------------------------- CBDI Project models -------------------------- */

export const InitialStartNode: IPlanNode = {
  nodeId: CBDIEditorPlanNodeType.StartPlanNodeType,
  type: PlanEditorNodeType.Circle,
  nodeData: {
    id: CBDIEditorPlanNodeType.StartPlanNodeType,
    note: '',
    type: CBDIEditorPlanNodeType.StartPlanNodeType,
  },
  metaData: {
    position: {
      x: 0,
      y: 0,
    },
    timestamp: 0,
  },
};

export const InitialEndNode: IPlanNode = {
  nodeId: CBDIEditorPlanNodeType.EndPlanNodeType,
  type: PlanEditorNodeType.Circle,
  nodeData: {
    id: CBDIEditorPlanNodeType.EndPlanNodeType,
    note: '',
    type: CBDIEditorPlanNodeType.EndPlanNodeType,
  },
  metaData: {
    position: {
      x: 0,
      y: 400,
    },
    timestamp: 0,
  },
};

/**
 * Cbdi editor root object dic
 */
export type CBDIEditorRootObjectDic = {
  [uuid: string]: CBDIEditorObject;
};

/* ---------------------------------- plan ---------------------------------- */
/**
 * override plan schema with meta data
 */
export type CBDIEditorOverridePlanSchema = Omit<CBDIEditorRootPlanSchema, 'tasks' | 'edges'> & {
  tasks: IPlanNode[];
  edges: IPlanEdge[];
};

export type CBDIEditorProjectPlan = CBDIEditorRootConceptAddition & CBDIEditorOverridePlanSchema;
/* --------------------------------- message -------------------------------- */
export type CBDIEditorOverrideMessageFieldSchema = Omit<CBDIEditorMessageFieldSchema, 'default'> & {
  id: string;
  default: string | number | Vec2Type | boolean;
  isDefaultValid: boolean;
};

export type CBDIEditorOverrideMessageSchema = Omit<CBDIEditorRootMessageSchema, 'fields'> & {
  fields: CBDIEditorOverrideMessageFieldSchema[];
};
export type CBDIEditorProjectMessage = CBDIEditorRootConceptAddition & CBDIEditorOverrideMessageSchema;
/* ---------------------------------- enum ---------------------------------- */
export type CBDIEditorProjectEnum = CBDIEditorRootConceptAddition & CBDIEditorRootEnumSchema;
/* ---------------------------------- event --------------------------------- */
export type CBDIEditorProjectEvent = CBDIEditorRootConceptAddition & CBDIEditorRootEventSchema;
/* -------------------------------- resource -------------------------------- */
export type CBDIEditorProjectResource = CBDIEditorRootConceptAddition & CBDIEditorRootResourceSchema;
/* --------------------------------- action --------------------------------- */
export type CBDIEditorProjectAction = CBDIEditorRootConceptAddition & CBDIEditorRootActionSchema;
/* ---------------------------------- goal ---------------------------------- */
export type CBDIEditorProjectGoal = CBDIEditorRootConceptAddition & CBDIEditorRootGoalSchema;
/* --------------------------------- service -------------------------------- */
export type CBDIEditorProjectService = CBDIEditorRootConceptAddition & CBDIEditorRootServiceSchema;
/* --------------------------------- entity --------------------------------- */
export type CBDIEditorProjectEntity = CBDIEditorRootConceptAddition & CBDIEditorRootEntitySchema;
/* --------------------------------- tactic --------------------------------- */
export type CBDIEditorOverrideTacticSchema = Omit<CBDIEditorRootTacticSchema, 'plan_list'> & {
  plan_list: CBDIEditorModuleConceptWithId[];
};
export type CBDIEditorProjectTactic = CBDIEditorRootConceptAddition & CBDIEditorOverrideTacticSchema;
/* ---------------------------------- role ---------------------------------- */
export type CBDIEditorProjectRole = CBDIEditorRootConceptAddition & CBDIEditorRootRoleSchema;
/* ---------------------------------- agent --------------------------------- */
export type CBDIEditorProjectAgent = CBDIEditorRootConceptAddition & CBDIEditorRootAgentSchema;
/* ---------------------------------- team ---------------------------------- */
export type CBDIEditorProjectTeam = CBDIEditorRootConceptAddition & CBDIEditorRootTeamSchema;

export type ModuleSearchPath = { path: string; recursive: boolean };

/**
 * module path obj model
 */
export type ModulePathObjModel = { name: string; path: string };

/**
 * cbdi editor project model
 */
export type CBDIEditorProjectModel = {
  name: string;
  jack_model_version: string;
  modules: ModulePathObjModel[];
};

export type ModuleProjectInfo = {
  name: string;
  namespaces: string[];
  major_version?: string;
  minor_version?: string;
  patch_version?: string;
  generator?: string;
  modules: { name: string; filePath: string }[];
  search_paths: ModuleSearchPath[];
};

export type ModuleProjectInfoDic = { [moduleName: string]: ModuleProjectInfo };

/**
 * module path obj model
 */
export type ModulePathObj = { name: string; path: string | undefined; valid: boolean };

/**
 * project model after loading to editor
 */
export type CBDIEditorProject = {
  name: string;
  namespaces: string[];
  jack_model_version: string;
  modulePaths: ModulePathObj[];
  moduleProjectInfoDic: ModuleProjectInfoDic;
  cbdiObjects: CBDIEditorRootObjectDic;
  enums: ModuleConcept[];
  teams: ModuleConcept[];
  agents: ModuleConcept[];
  tactics: ModuleConcept[];
  roles: ModuleConcept[];
  goals: ModuleConcept[];
  plans: ModuleConcept[];
  resources: ModuleConcept[];
  actions: ModuleConcept[];
  messages: ModuleConcept[];
  services: ModuleConcept[];
  entities: ModuleConcept[];
  events: ModuleConcept[];
};

/**
 * node metaData model for plan manual layout
 */
export type FlatNodeMetaDataModel = {
  NodeId: string;
  PositionX: number;
  PositionY: number;
  Timestamp: number;
};

/**
 * edge metaData model for plan manual layout
 */
export type FlatEdgeMetaDataModel = {
  SourceId: string;
  TargetId: string;
  ControlPoints: { x: number; y: number }[];
  Timestamp: number;
};

export type FlatMetaData = {
  node: FlatNodeMetaDataModel[];
  edge: FlatEdgeMetaDataModel[];
};

/**
 * project model with plan position meta data
 * before loading to editor
 */
export type CBDIEditorModuleWithMetaDataModel = {
  moduleModel: CBDIEditorModuleModel;
  flatMetaData?: FlatMetaData;
};
