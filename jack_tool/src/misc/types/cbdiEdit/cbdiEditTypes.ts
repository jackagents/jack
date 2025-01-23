import { DEFAULT_CONTROL_POINT_DISTANCE } from 'components/cbdiEdit/reactFlowPlanEditor/reactFlowPlanEditorConstant';

/**
 * Cbdi editor object mod
 */
export enum Mod {
  None,
  Addition,
  Deletion,
  Update,
}

/* ---------------------------- Root concept type --------------------------- */

/**
 * CBDI editor root level concept type
 */
export enum CBDIEditorRootConceptType {
  TeamConceptType = 'team',
  AgentConceptType = 'agent',
  RoleConceptType = 'role',
  ActionConceptType = 'action',
  GoalConceptType = 'goal',
  PlanConceptType = 'plan',
  TacticConceptType = 'tactic',
  ResourceConceptType = 'resource',
  MessageConceptType = 'message',
  ServiceConceptType = 'service',
  EntityConceptType = 'entity',
  EventConceptType = 'event',
  EnumConceptType = 'enum',
}

export type CBDIEditorModuleConceptWithId = {
  id: string;
  moduleConcept: ModuleConcept;
};

export type CBDIEditorRootConceptAddition = {
  _mod: Mod;
  _objectType: CBDIEditorRootConceptType;
};

/**
 * Root level cbdi editor concept basic structure
 */
export type CBDIEditorConceptSchema = {
  uuid: string;
  module: string;
  name: string;
  note: string;
};

export type CBDIEditorObject = CBDIEditorConceptSchema & CBDIEditorRootConceptAddition;

/* --------------------------- message sub types -------------------------- */
export type IMapping = {
  from: string;
  to: string;
};

export type CBDIEditorCustomMessage = {
  Custom: ModuleConcept;
};

export type CBDIEditorEnumMessage = {
  Enum: ModuleConcept;
};

export type CBDIEditorTNonBasicMessageSchema = CBDIEditorCustomMessage | CBDIEditorEnumMessage;

/**
 * CBDI editor message's field.
 */
export type CBDIEditorMessageFieldSchema = {
  name: string;
  note: string;
  type: CBDIEditorTBasicMessageSchema | CBDIEditorTNonBasicMessageSchema;
  default: string;
  is_array: boolean;
  /**
   * The scenario editor will respect the hidden value assigned on an event
   * to avoid rendering the field in the event editor.
   */
  hidden: boolean;
};

/**
 * Message widget attributes
 */
export type CBDIEditorMessageWidgetAttributesSchema = {
  [field: string]: any;
};

/**
 * Message widget
 */
export type CBDIEditorMessageWidgetSchema = {
  widget: string;
  attributes: CBDIEditorMessageWidgetAttributesSchema;
};

/* ----------------------------- event sub types ---------------------------- */
/**
 * message default schema for event
 */
export type CBDIEditorEventMessageDefaultSchema = {
  name: string;
  note: string;
  value: string;
};

/**
 * message schema for event
 */
export type CBDIEditorEventMessageSchema = ModuleConcept & {
  /**
   * Events can be configured with default values to allow per-event configuration of the associated message for the event.
   * This separates interface definitions (messages) from event definition (configuration of messages)
   */
  defaults: CBDIEditorEventMessageDefaultSchema[];
};

/* ---------------------------- service sub types --------------------------- */
/**
 * Service topic schema
 */
export type CBDIEditorServiceTopicSchema = {
  name: string;
  message: ModuleConcept;
};
/* ------------------------------- Query types ------------------------------ */
export type TQuery = {
  custom: boolean;
  query: string;
};

export type TPreCondition = {
  precondition: TQuery;
};

export type TDropCondition = {
  dropcondition: TQuery;
};

export type TSatisfied = {
  satisfied: TQuery;
};
/* ----------------------------- role sub types ----------------------------- */
export type CBDIEditorSharedMessage = ModuleConcept & {
  read: boolean;
  write: boolean;
};

/* ----------------------------- plan sub types ---------------------------- */
export enum CBDIEditorPlanNodeType {
  ActionPlanNodeType = 'action',
  GoalPlanNodeType = 'goal',
  ConditionPlanNodeType = 'condition',
  SleepPlanNodeType = 'sleep',
  StartPlanNodeType = 'start',
  EndPlanNodeType = 'end',
}

// edge condition enum
export enum CBDIEditorPlanEditorEdgeCondition {
  True = 'true',
  False = 'false',
}

export type CBDIEditorJSONPlanNode = {
  id: string;
  note: string;
  type: CBDIEditorPlanNodeType;
  action?: ModuleConcept;
  goal?: ModuleConcept;
  async?: boolean;
  duration?: number;
  mappings?: IMapping[];
  conditiontext?: string;
  updateTimestamp?: number;
};

export type CBDIEditorJSONPlanEdge = {
  condition: CBDIEditorPlanEditorEdgeCondition;
  sourceid: string;
  targetid: string;
};

/* ----------------------- reactflow planEditor types ----------------------- */
// node type enum
export enum PlanEditorNodeType {
  Rectangle = 'rectangle',
  Circle = 'circle',
}

// reactflow node meta data
export interface PlanNodeMetaData {
  position: { x: number; y: number };
  timestamp: number;
}

export type EdgeBezierControlPoint = {
  x: number;
  y: number;
  active?: number;
};

export const defaultEdgeControlPoints = [
  { x: 0, y: DEFAULT_CONTROL_POINT_DISTANCE },
  { x: 0, y: -DEFAULT_CONTROL_POINT_DISTANCE },
];

// edge data
export type PlanEdgeData = {
  condition: CBDIEditorPlanEditorEdgeCondition;
  controlPoints: EdgeBezierControlPoint[];
  timestamp: number;
};

export type PlanNodeData = CBDIEditorJSONPlanNode & {
  updateTimestamp: number;
};

// project plan node
export interface IPlanNode {
  nodeId: string;
  type: PlanEditorNodeType;
  nodeData: CBDIEditorJSONPlanNode;
  metaData?: PlanNodeMetaData;
  selected?: boolean;
}

// project plan edge
export interface IPlanEdge {
  source: string;
  target: string;
  edgeData: PlanEdgeData;
  selected?: boolean;
}
/* ----------------------------- Enum sub types ----------------------------- */
/**
 * Enum field type
 */
export type EnumField = {
  name: string;
  value: number;
  note: string;
};

/* ----------------------------- Goal sub types ----------------------------- */
/**
 * agent goal
 */
export type CBDIEditorAgentGoal = ModuleConcept & {
  startup_goal: boolean;
  startup_tactic: ModuleConcept;
};

/* -------------------------------- Ohter category type ------------------------------- */

export enum CBDIEditorOtherCategoryType {
  EnumOptionType = 'enumOption',
  RootType = 'root',
  ModuleType = 'module',
  OverviewType = 'overview',
  FolderType = 'folder',
  MessageFieldType = 'messageField',
  ReactFlowPlan = 'reactFlowPlan',
}
/* --------------------------- Message field sub types -------------------------- */
/**
 * Message field basic type
 */
export enum CBDIEditorTBasicMessageSchema {
  I8Type = 'I8',
  I16Type = 'I16',
  I32Type = 'I32',
  I64Type = 'I64',
  U8Type = 'U8',
  U16Type = 'U16',
  U32Type = 'U32',
  U64Type = 'U64',
  F32Type = 'F32',
  F64Type = 'F64',
  BoolType = 'Bool',
  Vec2Type = 'Vec2',
  StringType = 'String',
}

export type Vec2Type = {
  x: number;
  y: number;
};

/**
 * Module concept for reference another concept
 */
export type ModuleConcept = {
  /**
   * Uuid of the root messages
   */
  uuid: string;
  /**
   * Name of the root message
   */
  name: string;
  /**
   * Module of the root message
   */
  module: string;
};
/**
 * empty module concept
 */
export const EmptyModuleConcept = {
  uuid: '',
  name: '',
  module: '',
};
/* ---------------------------- tactic sub types ---------------------------- */
export enum PlanOrderType {
  ExcludePlanAfterAttempt = 'ExcludePlanAfterAttempt',
  Strict = 'Strict',
  ChooseBestPlan = 'ChooseBestPlan',
}

export interface OptionData {
  value: string;
  moduleConcept?: ModuleConcept;
  label: string;
  isDisabled?: boolean;
  objectType: string;
}

export const ProjectConceptListArray = [
  'teams',
  'agents',
  'roles',
  'plans',
  'tactics',
  'goals',
  'actions',
  'messages',
  'resources',
  'services',
  'enums',
  'entities',
  'events',
] as const;

export type ProjectConceptListType = (typeof ProjectConceptListArray)[number];

export type ConceptFieldType =
  | 'teams'
  | 'agents'
  | 'roles'
  | 'plans'
  | 'tactics'
  | 'goals'
  | 'actions'
  | 'messages'
  | 'resources'
  | 'services'
  | 'query_messages'
  | 'action_handlers'
  | 'beliefs'
  | 'children';
