import { TaskStatus } from 'misc/constant/common/cmConstants';
import {
  BDILogActionBody,
  BDILogConditionBody,
  BDILogGoalIntentionResult,
  BDILogLevel,
  BDILogSleepBody,
  BDILogType,
  BusAddress,
  DelegationBody,
  Event,
  PursueBody,
  RegisterBody,
  Timestamp,
  MessageData,
} from 'misc/types/cbdi/cbdi-types-non-flatbuffer';

export type Belief = {
  [key: string]: any;
};

/**
 * Agent instance
 */
export type CBDIAgent = {
  // Register
  proxy?: boolean;
  address: BusAddress;
  templateType?: string;
  start?: boolean;
  team?: BusAddress;
  teams?: BusAddress[];
  // Goals
  goalList?: GoalList;
  // Delegations
  auctions?: DelegationBody[];
  delegations?: DelegationBody[];
  // Pursue
  pursues?: Record<string, PursueBody[]>;
  // Belief sets
  beliefSets?: Belief;
  lastUpdated: Timestamp;
  [key: string]: any;
};

export interface CBDIService extends RegisterBody {
  lastUpdated: Timestamp;
  [key: string]: any;
}

export interface GoalList {
  [goalId: string]: { subGoalList: GoalList; intention: CBDIIntention };
}

/**
 * CBDI intention model
 */
export interface CBDIIntention {
  goal: string;
  goalId: string;
  plan: string;
  tasks: { taskEvent: CBDIPlanNode; currentTask: string };
  dropped: boolean;
}

export type CBDIPlanNode = (BDILogActionBody | BDILogSleepBody | BDILogConditionBody)[];

export interface CBDITeam extends CBDIAgent {
  members?: BusAddress[];
}

export interface TCBDILogsDict {
  [agentId: string]: Event[];
}

export interface TCBDIIntentionsDict {
  [agentId: string]: BDILogIntentionsModel;
}

export interface GoalInfoItem {
  agentId: string;
  goalId: string;
  goalTemplateName: string;
  bdiLogLevel: BDILogLevel;
  intentionId?: string;
  planTemplateName?: string;
  lastestTask?: BDILogIntentionTask;
  intentionResult?: BDILogGoalIntentionResult;
  goalResult?: BDILogGoalIntentionResult;
  goalContextMsg?: MessageData[];
}

export interface AgentSummaryGoalInfo {
  agentBeliefsets: Belief | undefined;
  currentIntentions: GoalInfoItem[];
  recentImportantIntentions: GoalInfoItem[];
}

/**
 * Intention Overview model
 */
export interface BDILogIntentionOverviewModel {
  goalId: string;
  startingTimestamp: Timestamp;
  planTemplateName: string | undefined;
  goalTemplateName: string | undefined;
  intentionStatus: TaskStatus;
  level: BDILogLevel;
}

/**
 * Intention Overviews model
 */
export interface BDILogIntentionOverviewsModel {
  [intentionId: string]: BDILogIntentionOverviewModel;
}

/**
 * Intention Overview dictionary
 */
export interface BDILogAgentIntentionOverviews {
  [agentId: string]: BDILogIntentionOverviewsModel;
}

/**
 * Intention model
 */
export interface BDILogIntentionModel {
  startingTimestamp: Timestamp;
  tasks: BDILogIntentionTask[];
  level: BDILogLevel;
  goalId: string;
  intentionId: string;
  planTemplateName?: string;
}

/**
 * Intentions model
 */
export interface BDILogIntentionsModel {
  [intentionId: string]: BDILogIntentionModel;
}

export interface ReasoningItem {
  [taskId: string]: {
    // taskTemplateName: string | undefined;
    reasoningText: string;
    level: BDILogLevel;
  };
}

export interface ReasoningDic {
  [intentionId: string]: ReasoningItem;
}

/**
 *
 */
export interface BDILogGoalIntentionItemModel {
  goalId: string;
  goalTemplateName: string;
  goalFinished: boolean;
  level: BDILogLevel;
  reasoningDic: ReasoningDic;
  latestIntentionWithHighestLevel?: BDILogIntentionModel;
  allIntentions: BDILogIntentionModel[];
  goalResult?: BDILogGoalIntentionResult;
}

/**
 * Group Intention by goal id
 */
export interface BDILogGoalIntentionDicModel {
  [goalId: string]: BDILogGoalIntentionItemModel;
}

/**
 * Intention dictionary
 */
export interface BDILogAgentIntentions {
  [agentId: string]: BDILogIntentionsModel;
}

/**
 * Goals/desires dictionary
 */
export interface BDILogAgentGoals {
  [agentId: string]: BDILogGoalIntentionDicModel;
}

export interface TCBDIIntentionForNotification {
  goalId: string;
  reasoningDic: ReasoningDic;
  intentionData?: BDILogIntentionModel;
  goalContextMsg?: MessageData[];
  planTemplateName?: string;
  goalTemplateName: string;
  agentAddress?: BusAddress;
}

export interface BDILogIntentionTask {
  logType: BDILogType;
  goal: string;
  goalId: string;
  timestamp: Timestamp;

  /**
   * Goal does not have plan
   */
  plan?: string;

  /**
   * bdi log level
   */
  level: BDILogLevel;

  /**
   * For intention log event
   */
  intentionResult?: BDILogGoalIntentionResult;

  /**
   * For action log event
   */
  action?: string;

  /**
   * action reason
   */
  reasoning?: string;

  /**
   * goal drop reason
   */
  dropReason?: string;

  /**
   * For action log event
   */
  actionSuccess?: boolean;

  /**
   * For sleep log event
   */
  sleepMs?: Timestamp;

  /**
   * For condition log event
   */
  condition?: string;
  conditionSuccess?: boolean;

  /**
   * For goal log event
   */
  goalResult?: BDILogGoalIntentionResult;
  /**
   * For tasks other than intention started and intention finished
   */
  taskId: string;
}

export interface SeverityFilterData {
  value: BDILogLevel;
  label: string;
}

export interface BdiLogTypeFilterData {
  value: string;
  label: string;
}

export interface ProcessedIntentionTask extends BDILogIntentionTask {
  taskType: string;
  taskStatus: string;
}
