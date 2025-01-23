/// This file is manually created for purpose of processing websocket nonflatbuffer bson messages

/// This will need to be updated manually everytime the `protocol.h` in cbdi core is changed

export enum NodeType {
  GENERIC = 'GENERIC',
  NODE = 'NODE',
  SERVICE = 'SERVICE',
  AGENT = 'AGENT',
  TEAM = 'TEAM',
  COUNT = 'COUNT',
}

export enum EventType {
  NONE = 'NONE',
  /// CBDI
  // TIMER = 'TIMER',
  CONTROL = 'CONTROL',
  PERCEPT = 'PERCEPT',
  PURSUE = 'PURSUE',
  DROP = 'DROP',
  // ACTION = 'ACTION',
  DELEGATION = 'DELEGATION',
  MESSAGE = 'MESSAGE',
  // ACTIONCOMPLETE = 'ACTIONCOMPLETE',
  // SCHEDULE = 'SCHEDULE',
  // AUCTION = 'AUCTION',
  // SHAREBELIEFSET = 'SHAREBELIEFSET',
  // TACTIC = 'TACTIC',
  // HEARTBEAT = 'HEARTBEAT',
  REGISTER = 'REGISTER',
  DEREGISTER = 'DEREGISTER',
  AGENT_JOIN_TEAM = 'AGENT_JOIN_TEAM',
  AGENT_LEAVE_TEAM = 'AGENT_LEAVE_TEAM',
  ACTION_BEGIN = 'ACTION_BEGIN',
  ACTION_UPDATE = 'ACTION_UPDATE',
  BDI_LOG = 'BDI_LOG',
  COUNT = 'COUNT',
}

export enum ControlCommand {
  START = 'START',
  PAUSE = 'PAUSE',
  STOP = 'STOP',
  COUNT = 'COUNT',
}

export enum DropMode {
  NORMAL = 'NORMAL',
  FORCE = 'FORCE',
}

export enum DelegationStatus {
  PENDING = 'PENDING',
  FAILED = 'FAILED',
  SUCCESS = 'SUCCESS',
}

export enum ActionStatus {
  FAILED = 'FAILED',
  SUCCESS = 'SUCCESS',
}

export enum BDILogLevel {
  NORMAL = 'NORMAL',
  IMPORTANT = 'IMPORTANT',
  CRITICAL = 'CRITICAL',
}

export enum BDILogType {
  GOAL_STARTED = 'GOAL_STARTED',
  GOAL_FINISHED = 'GOAL_FINISHED',
  SUB_GOAL_STARTED = 'SUB_GOAL_STARTED',
  SUB_GOAL_FINISHED = 'SUB_GOAL_FINISHED',
  INTENTION_STARTED = 'INTENTION_STARTED',
  INTENTION_FINISHED = 'INTENTION_FINISHED',
  ACTION_STARTED = 'ACTION_STARTED',
  ACTION_FINISHED = 'ACTION_FINISHED',
  SLEEP_STARTED = 'SLEEP_STARTED',
  SLEEP_FINISHED = 'SLEEP_FINISHED',
  CONDITION = 'CONDITION',
}

export enum BDILogGoalIntentionResult {
  FAILED = 'FAILED',
  SUCCESS = 'SUCCESS',
  DROPPED = 'DROPPED',
}

export type Timestamp = number;

export type V2 = number;

export type MessageData = Record<string, Field> | null;

export type Field = any;

export type AgentJoinTeamBody = TeamAgentBody;

export type AgentLeaveTeamBody = TeamAgentBody;

export interface BusAddress {
  type: NodeType;
  id: string;
  name: string;
}

export interface PerceptBody {
  beliefSet: string;
  field: Field;
}

export type MessageBody = {
  data: MessageData;
};

export interface PursueBody {
  goal: string;
  persistent: boolean;
  message: MessageData;
}

export interface ControlBody {
  command: ControlCommand;
}

export interface DropBody {
  goal: string;
  goalId: string;
  mode: DropMode;
}

export interface DelegationBody {
  status: DelegationStatus;
  goal: string;
  goalId: string;
  message: MessageData;
  analyse: boolean;
  score: number;
  team: string;
  teamId: string;
}

// export interface HeartbeatBody {
//   time: Timestamp;
// }

export interface RegisterBody {
  proxy: boolean;
  address: BusAddress;
  templateType: string;
  start: boolean;
  team: BusAddress;
}

export interface DeregisterBody {
  id: string;
  nodeType: NodeType;
}

export interface TeamAgentBody {
  team: BusAddress;
  agent: BusAddress;
}

export interface ActionBeginBody {
  name: string;
  taskId: string;
  goal: string;
  goalId: string;
  intentionId: string;
  plan: string;
  message: MessageData;
  resourceLocks: string[];
}

export interface ActionUpdateBody {
  name: string;
  taskId: string;
  goal: string;
  goalId: string;
  intentionId: string;
  plan: string;
  status: ActionStatus;
  reply: MessageData;
}

export interface BDILogGoalBody {
  goal: string;
  goalId: string;
  intentionId: string;
  taskId: string;
  dropReason: string;
  result: BDILogGoalIntentionResult;
}

export interface BDILogIntentionBody {
  goal: string;
  goalId: string;
  intentionId: string;
  plan: string;
  result: BDILogGoalIntentionResult;
}

export interface BDILogActionBody {
  goal: string;
  goalId: string;
  intentionId: string;
  plan: string;
  taskId: string;
  action: string;
  reasoning: string;
  success: boolean;
}

export interface BDILogSleepBody {
  goal: string;
  goalId: string;
  intentionId: string;
  plan: string;
  taskId: string;
  sleepMs: Timestamp;
}

export interface BDILogConditionBody {
  goal: string;
  goalId: string;
  intentionId: string;
  plan: string;
  taskId: string;
  condition: string;
  success: boolean;
}

export interface BDILogCommon {
  level: BDILogLevel;
  logType: BDILogType;
}

export interface BasicEvent {
  senderNode: BusAddress;
  sender: BusAddress;
  recipient: BusAddress;
  timestampUs: Timestamp;
  eventId: string;
  type: EventType;
}

export type BDILogGoal = BDILogCommon & BDILogGoalBody;
export type BDILogIntention = BDILogCommon & BDILogIntentionBody;
export type BDILogAction = BDILogCommon & BDILogActionBody;
export type BDILogSleep = BDILogCommon & BDILogSleepBody;
export type BDILogCondition = BDILogCommon & BDILogConditionBody;
export type BDILogBody = BDILogGoal | BDILogIntention | BDILogAction | BDILogSleep | BDILogCondition;

export type Register = BasicEvent & RegisterBody;
export type Control = BasicEvent & ControlBody;
export type Percept = BasicEvent & PerceptBody;
export type Pursue = BasicEvent & PursueBody;
export type Drop = BasicEvent & DropBody;
export type Delegation = BasicEvent & DelegationBody;
export type Deregister = BasicEvent & DeregisterBody;
// export type Heartbeat = BasicEvent & HeartbeatBody;
export type AgentJoinTeam = BasicEvent & AgentJoinTeamBody;
export type AgentLeaveTeam = BasicEvent & AgentLeaveTeamBody;
export type ActionBegin = BasicEvent & ActionBeginBody;
export type ActionUpdate = BasicEvent & ActionUpdateBody;
export type Message = BasicEvent & MessageBody;
export type BDILog = BasicEvent & BDILogBody;

export type Event =
  | Register
  | Control
  | Percept
  | Pursue
  | Drop
  | Delegation
  | Deregister
  // | Heartbeat
  | AgentJoinTeam
  | AgentLeaveTeam
  | ActionBegin
  | ActionUpdate
  | Message
  | BDILog;
