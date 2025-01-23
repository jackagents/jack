import LOGGER from 'misc/addons/logger/LoggerSingleton';
import {
  ActionBeginBody,
  ActionStatus,
  ActionUpdateBody,
  AgentJoinTeamBody,
  BDILogActionBody,
  BDILogBody,
  BDILogConditionBody,
  BDILogGoalBody,
  BDILogGoalIntentionResult,
  BDILogIntentionBody,
  BDILogLevel,
  BDILogSleepBody,
  BDILogType,
  BusAddress,
  ControlBody,
  ControlCommand,
  DelegationBody,
  DelegationStatus,
  DeregisterBody,
  DropBody,
  DropMode,
  Event,
  EventType,
  Field,
  MessageBody,
  MessageData,
  NodeType,
  PerceptBody,
  PursueBody,
  RegisterBody,
} from 'misc/types/cbdi/cbdi-types-non-flatbuffer';

type BodyValidator = (type: EventType, body: object) => void;

const isRegisterBody = (type: EventType, body: object): body is RegisterBody => {
  if (type === EventType.REGISTER) {
    return true;
  }

  return false;
};

const isActionBeginBody = (type: EventType, body: object): body is ActionBeginBody => {
  if (type === EventType.ACTION_BEGIN) {
    return true;
  }

  return false;
};

const isActionUpdateBody = (type: EventType, body: object): body is ActionUpdateBody => {
  if (type === EventType.ACTION_UPDATE) {
    return true;
  }

  return false;
};

const isAgentJoinTeamBody = (type: EventType, body: object): body is AgentJoinTeamBody => {
  if (type === EventType.AGENT_JOIN_TEAM) {
    return true;
  }

  return false;
};

const isAgentLeaveTeamBody = (type: EventType, body: object): body is AgentJoinTeamBody => {
  if (type === EventType.AGENT_LEAVE_TEAM) {
    return true;
  }

  return false;
};

const isBdiLogBody = (type: EventType, body: object): body is BDILogBody => {
  if (type === EventType.BDI_LOG) {
    return true;
  }

  return false;
};

const isControlBody = (type: EventType, body: object): body is ControlBody => {
  if (type === EventType.CONTROL) {
    return true;
  }

  return false;
};

const isDelegationBody = (type: EventType, body: object): body is DelegationBody => {
  if (type === EventType.DELEGATION) {
    return true;
  }

  return false;
};

const isDeregisterBody = (type: EventType, body: object): body is DeregisterBody => {
  if (type === EventType.DEREGISTER) {
    return true;
  }

  return false;
};

const isDropBody = (type: EventType, body: object): body is DropBody => {
  if (type === EventType.DROP) {
    return true;
  }

  return false;
};

const isMessageBody = (type: EventType, body: object): body is MessageBody => {
  if (type === EventType.MESSAGE) {
    return true;
  }

  return false;
};

const isPerceptBody = (type: EventType, body: object): body is PerceptBody => {
  if (type === EventType.PERCEPT) {
    return true;
  }

  return false;
};

const isPursueBody = (type: EventType, body: object): body is PursueBody => {
  if (type === EventType.PURSUE) {
    return true;
  }

  return false;
};

const isBdiLogGoalBody = (logType: BDILogType, body: object): body is BDILogGoalBody => {
  if (
    logType === BDILogType.GOAL_STARTED ||
    logType === BDILogType.GOAL_FINISHED ||
    logType === BDILogType.SUB_GOAL_STARTED ||
    logType === BDILogType.SUB_GOAL_FINISHED
  ) {
    return true;
  }

  return false;
};

const isBdiLogIntentionBody = (logType: BDILogType, body: object): body is BDILogIntentionBody => {
  if (logType === BDILogType.INTENTION_STARTED || logType === BDILogType.INTENTION_FINISHED) {
    return true;
  }

  return false;
};

const isBdiLogActionBody = (logType: BDILogType, body: object): body is BDILogActionBody => {
  if (logType === BDILogType.ACTION_STARTED || logType === BDILogType.ACTION_FINISHED) {
    return true;
  }

  return false;
};

const isBdiLogConditionBody = (logType: BDILogType, body: object): body is BDILogConditionBody => {
  if (logType === BDILogType.CONDITION) {
    return true;
  }

  return false;
};

const isBdiLogSleepBody = (logType: BDILogType, body: object): body is BDILogSleepBody => {
  if (logType === BDILogType.SLEEP_STARTED || logType === BDILogType.SLEEP_FINISHED) {
    return true;
  }

  return false;
};

/* ------------------------------- Validators ------------------------------- */
/**
 * Validate BusAddress type
 * @param param0
 */
const validateAddress = ({ id, name, type }: BusAddress) => {
  if (id === undefined) {
    throw new Error('BusAddress::id is undefined');
  } else if (typeof id !== 'string') {
    throw new Error('BusAddress::id is not string type');
  }

  if (name === undefined) {
    throw new Error('BusAddress::name is undefined');
  } else if (typeof name !== 'string') {
    throw new Error('BusAddress::name is not string type');
  }

  if (type === undefined) {
    throw new Error('BusAddress::type is undefined');
  } else if (typeof type !== 'string') {
    throw new Error('BusAddress::type is not string type');
  }
};

const validateFieldType = (field: Field) => {
  if (field === undefined) {
    throw new Error('Field is undefined');
  }
};

const validateMessageDataType = (messageData: MessageData) => {
  if (messageData !== null) {
    Object.values(messageData).forEach((field) => {
      validateFieldType(field);
    });
  }
};
const validateRegisterBody: BodyValidator = (type, body) => {
  if (isRegisterBody(type, body)) {
    const { address, proxy, start, team, templateType } = body;

    if (address === undefined) {
      throw new Error('Register::address is undefined');
    }
    // Validate address
    else {
      validateAddress(address);
    }

    if (proxy === undefined) {
      throw new Error('Register::proxy is undefined');
    } else if (typeof proxy !== 'boolean') {
      throw new Error('Register::proxy is not boolean type');
    }

    if (start === undefined) {
      throw new Error('Register::start is undefined');
    } else if (typeof start !== 'boolean') {
      throw new Error('Register::start is not boolean type');
    }

    if (team === undefined) {
      throw new Error('Register::team is undefined');
    }
    // Validate team address
    else {
      validateAddress(team);
    }

    if (templateType === undefined) {
      throw new Error('templateType is undefined');
    } else if (typeof templateType !== 'string') {
      throw new Error('templateType is not string type');
    }
  }
};

const validateDeregisterBody: BodyValidator = (type, body) => {
  if (isDeregisterBody(type, body)) {
    const { id, nodeType } = body;

    if (id === undefined) {
      throw new Error('Deregister::address is undefined');
    } else if (typeof id !== 'string') {
      throw new Error('Deregister::id is not string type');
    }

    if (nodeType === undefined) {
      throw new Error('Deregister::proxy is undefined');
    } else if (!(nodeType in NodeType)) {
      throw new Error('Deregister::nodeType is not in NodeType range');
    }
  }
};

const validateActionBeginBody: BodyValidator = (type, body) => {
  if (isActionBeginBody(type, body)) {
    const { goal, goalId, intentionId, name, message, plan, resourceLocks, taskId } = body;

    if (goal === undefined) {
      throw new Error('ActionBegin::goal is undefined');
    } else if (typeof goal !== 'string') {
      throw new Error('ActionBegin::goal is not string type');
    }

    if (goalId === undefined) {
      throw new Error('ActionBegin::goalId is undefined');
    } else if (typeof goalId !== 'string') {
      throw new Error('ActionBegin::goalId is not string type');
    }

    if (intentionId === undefined) {
      throw new Error('ActionBegin::intentionId is undefined');
    } else if (typeof intentionId !== 'string') {
      throw new Error('ActionBegin::intentionId is not string type');
    }

    if (name === undefined) {
      throw new Error('ActionBegin::name is undefined');
    } else if (typeof name !== 'string') {
      throw new Error('ActionBegin::name is not string type');
    }

    if (message === undefined) {
      throw new Error('ActionBegin::message is undefined');
    } else if (!Array.isArray(message)) {
      throw new Error('ActionBegin::parameters is not array');
    } else {
      validateMessageDataType(message);
    }

    if (plan === undefined) {
      throw new Error('ActionBegin::plan is undefined');
    } else if (typeof plan !== 'string') {
      throw new Error('ActionBegin::plan is not string type');
    }

    if (resourceLocks === undefined) {
      throw new Error('ActionBegin::resourceLocks is undefined');
    } else if (resourceLocks.some((item) => typeof item !== 'string') || !Array.isArray(resourceLocks)) {
      throw new Error('ActionBegin::resourceLocks is not string array type');
    }

    if (taskId === undefined) {
      throw new Error('ActionBegin::taskId is undefined');
    } else if (typeof taskId !== 'string') {
      throw new Error('ActionBegin::taskId is not string type');
    }
  }
};

const validateActionUpdateBody: BodyValidator = (type, body) => {
  if (isActionUpdateBody(type, body)) {
    const { goal, goalId, intentionId, name, plan, taskId, reply, status } = body;

    if (goal === undefined) {
      throw new Error('ActionUpdate::goal is undefined');
    } else if (typeof goal !== 'string') {
      throw new Error('ActionUpdate::goal is not string type');
    }

    if (goalId === undefined) {
      throw new Error('ActionUpdate::goalId is undefined');
    } else if (typeof goalId !== 'string') {
      throw new Error('ActionUpdate::goalId is not string type');
    }

    if (intentionId === undefined) {
      throw new Error('ActionUpdate::intentionId is undefined');
    } else if (typeof intentionId !== 'string') {
      throw new Error('ActionUpdate::intentionId is not string type');
    }

    if (name === undefined) {
      throw new Error('ActionUpdate::name is undefined');
    } else if (typeof name !== 'string') {
      throw new Error('ActionUpdate::name is not string type');
    }

    if (reply === undefined) {
      throw new Error('ActionUpdate::reply is undefined');
    } else {
      validateMessageDataType(reply);
    }

    if (plan === undefined) {
      throw new Error('ActionUpdate::plan is undefined');
    } else if (typeof plan !== 'string') {
      throw new Error('ActionUpdate::plan is not string type');
    }

    if (status === undefined) {
      throw new Error('ActionUpdate::status is undefined');
    } else if (!(status in ActionStatus)) {
      throw new Error('ActionUpdate::status is not in range of ActionSTatus');
    }

    if (taskId === undefined) {
      throw new Error('ActionUpdate::taskId is undefined');
    } else if (typeof taskId !== 'string') {
      throw new Error('ActionUpdate::taskId is not string type');
    }
  }
};

const validateAgentJoinTeamBody: BodyValidator = (type, body) => {
  if (isAgentJoinTeamBody(type, body)) {
    const { agent, team } = body;

    if (agent === undefined) {
      throw new Error('AgentJoinTeam::agent is undefined');
    } else {
      validateAddress(agent);
    }

    if (team === undefined) {
      throw new Error('AgentJoinTeam::team is undefined');
    } else {
      validateAddress(team);
    }
  }
};

const validateAgentLeaveTeamBody: BodyValidator = (type, body) => {
  if (isAgentLeaveTeamBody(type, body)) {
    const { agent, team } = body;

    if (agent === undefined) {
      throw new Error('AgentLeaveTeam::agent is undefined');
    } else {
      validateAddress(agent);
    }

    if (team === undefined) {
      throw new Error('AgentLeaveTeam::team is undefined');
    } else {
      validateAddress(team);
    }
  }
};

const validateControlBody: BodyValidator = (type, body) => {
  if (isControlBody(type, body)) {
    const { command } = body;

    if (command === undefined) {
      throw new Error('ControlCommand::command is undefined');
    } else if (!(command in ControlCommand)) {
      throw new Error('ControlCommand::command is not in range of ControlCommand');
    }
  }
};

const validateDelegationBody: BodyValidator = (type, body) => {
  if (isDelegationBody(type, body)) {
    const { analyse, goal, goalId, message, score, status, team, teamId } = body;

    if (goal === undefined) {
      throw new Error('Delegation::goal is undefined');
    } else if (typeof goal !== 'string') {
      throw new Error('Delegation::goal is not string type');
    }

    if (goalId === undefined) {
      throw new Error('Delegation::goalId is undefined');
    } else if (typeof goalId !== 'string') {
      throw new Error('Delegation::goalId is not string type');
    }

    if (analyse === undefined) {
      throw new Error('Delegation::analyse is undefined');
    } else if (typeof analyse !== 'boolean') {
      throw new Error('Delegation::analyse is not boolean type');
    }

    if (score === undefined) {
      throw new Error('Delegation::score is undefined');
    } else if (typeof score !== 'number') {
      throw new Error('Delegation::score is not number type');
    }

    if (message === undefined) {
      throw new Error('Delegation::message is undefined');
    } else {
      validateMessageDataType(message);
    }

    if (team === undefined) {
      throw new Error('Delegation::team is undefined');
    } else if (typeof team !== 'string') {
      throw new Error('Delegation::team is not string type');
    }

    if (status === undefined) {
      throw new Error('Delegation::status is undefined');
    } else if (!(status in DelegationStatus)) {
      throw new Error('Delegation::status is not in range of DelegationStatus');
    }

    if (teamId === undefined) {
      throw new Error('Delegation::taskId is undefined');
    } else if (typeof teamId !== 'string') {
      throw new Error('Delegation::taskId is not string type');
    }
  }
};

const validateDropBody: BodyValidator = (type, body) => {
  if (isDropBody(type, body)) {
    const { goal, goalId, mode } = body;

    if (goal === undefined) {
      throw new Error('Drop::goal is undefined');
    } else if (typeof goal !== 'string') {
      throw new Error('Drop::goal is not string type');
    }

    if (goalId === undefined) {
      throw new Error('Drop::goalId is undefined');
    } else if (typeof goalId !== 'string') {
      throw new Error('Drop::goalId is not string type');
    }

    if (mode === undefined) {
      throw new Error('Drop::mode is undefined');
    } else if (!(mode in DropMode)) {
      throw new Error('Drop::mode is not in DropMode range');
    }
  }
};

const validateMessageBody: BodyValidator = (type, body) => {
  if (isMessageBody(type, body)) {
    const { data } = body;

    if (data === undefined) {
      throw new Error('Message::data is undefined');
    } else {
      validateMessageDataType(data);
    }
  }
};

const validatePerceptBody: BodyValidator = (type, body) => {
  if (isPerceptBody(type, body)) {
    const { beliefSet, field } = body;

    if (beliefSet === undefined) {
      throw new Error('Percept::beliefSet is undefined');
    } else if (typeof beliefSet !== 'string') {
      throw new Error('Percept::beliefSet is not string type');
    }

    if (field === undefined) {
      throw new Error('Percept::fields is undefined');
    } else {
      validateFieldType(field);
    }
  }
};

const validatePursueBody: BodyValidator = (type, body) => {
  if (isPursueBody(type, body)) {
    const { goal, message, persistent } = body;

    if (goal === undefined) {
      throw new Error('Pursue::goal is undefined');
    } else if (typeof goal !== 'string') {
      throw new Error('Pursue::goal is not string type');
    }

    if (persistent === undefined) {
      throw new Error('Pursue::persistent is undefined');
    } else if (typeof persistent !== 'boolean') {
      throw new Error('Pursue::persistent is not boolean type');
    }

    if (message === undefined) {
      throw new Error('Pursue::message is undefined');
    } else {
      validateMessageDataType(message);
    }
  }
};

const validateBdiLogGoalBody = (logType: BDILogType, body: object) => {
  if (isBdiLogGoalBody(logType, body)) {
    const { dropReason, goal, goalId, intentionId, result, taskId } = body;

    if (intentionId === undefined) {
      throw new Error('BdiLogGoal::intentionId is undefined');
    } else if (typeof intentionId !== 'string') {
      throw new Error('BdiLogGoal::intentionId is not string type');
    }

    if (goal === undefined) {
      throw new Error('BdiLogGoal::goal is undefined');
    } else if (typeof goal !== 'string') {
      throw new Error('BdiLogGoal::goal is not string type');
    }

    if (goalId === undefined) {
      throw new Error('BdiLogGoal::goalId is undefined');
    } else if (typeof goalId !== 'string') {
      throw new Error('BdiLogGoal::goalId is not string type');
    }

    if (dropReason === undefined) {
      throw new Error('BdiLogGoal::dropReason is undefined');
    } else if (typeof dropReason !== 'string') {
      throw new Error('BdiLogGoal::dropReason is not string type');
    }

    if (taskId === undefined) {
      throw new Error('BdiLogGoal::taskId is undefined');
    } else if (typeof taskId !== 'string') {
      throw new Error('BdiLogGoal::taskId is not string type');
    }

    if (result === undefined) {
      throw new Error('BdiLogGoal::result is undefined');
    } else if (!(result in BDILogGoalIntentionResult)) {
      throw new Error('BdiLogGoal::result is not in BDILogGoalIntentionResult range');
    }
  }
};

const validateBdiLogIntentionBody = (logType: BDILogType, body: object) => {
  if (isBdiLogIntentionBody(logType, body)) {
    const { goal, goalId, intentionId, plan, result } = body;

    if (intentionId === undefined) {
      throw new Error('BdiLogIntention::intentionId is undefined');
    } else if (typeof intentionId !== 'string') {
      throw new Error('BdiLogIntention::intentionId is not string type');
    }

    if (goal === undefined) {
      throw new Error('BdiLogIntention::goal is undefined');
    } else if (typeof goal !== 'string') {
      throw new Error('BdiLogIntention::goal is not string type');
    }

    if (goalId === undefined) {
      throw new Error('BdiLogIntention::goalId is undefined');
    } else if (typeof goalId !== 'string') {
      throw new Error('BdiLogIntention::goalId is not string type');
    }

    if (plan === undefined) {
      throw new Error('BdiLogIntention::plan is undefined');
    } else if (typeof plan !== 'string') {
      throw new Error('BdiLogIntention::plan is not string type');
    }

    if (result === undefined) {
      throw new Error('BdiLogIntention::result is undefined');
    } else if (!(result in BDILogGoalIntentionResult)) {
      throw new Error('BdiLogIntention::result is not in BDILogGoalIntentionResult range');
    }
  }
};

const validateBdiLogActionBody = (logType: BDILogType, body: object) => {
  if (isBdiLogActionBody(logType, body)) {
    const { action, goal, goalId, intentionId, plan, reasoning, success, taskId } = body;

    if (intentionId === undefined) {
      throw new Error('BdiLogAction::intentionId is undefined');
    } else if (typeof intentionId !== 'string') {
      throw new Error('BdiLogAction::intentionId is not string type');
    }

    if (goal === undefined) {
      throw new Error('BdiLogAction::goal is undefined');
    } else if (typeof goal !== 'string') {
      throw new Error('BdiLogAction::goal is not string type');
    }

    if (goalId === undefined) {
      throw new Error('BdiLogAction::goalId is undefined');
    } else if (typeof goalId !== 'string') {
      throw new Error('BdiLogAction::goalId is not string type');
    }

    if (action === undefined) {
      throw new Error('BdiLogAction::action is undefined');
    } else if (typeof action !== 'string') {
      throw new Error('BdiLogAction::action is not string type');
    }

    if (taskId === undefined) {
      throw new Error('BdiLogAction::taskId is undefined');
    } else if (typeof taskId !== 'string') {
      throw new Error('BdiLogAction::taskId is not string type');
    }

    if (plan === undefined) {
      throw new Error('BdiLogAction::plan is undefined');
    } else if (typeof plan !== 'string') {
      throw new Error('BdiLogAction::plan is not string type');
    }

    if (reasoning === undefined) {
      throw new Error('BdiLogAction::reasoning is undefined');
    } else if (typeof reasoning !== 'string') {
      throw new Error('BdiLogAction::reasoning is not string type');
    }

    if (success === undefined) {
      throw new Error('BdiLogAction::success is undefined');
    } else if (typeof success !== 'boolean') {
      throw new Error('BdiLogAction::success is not boolean type');
    }
  }
};

const validateBdiLogConditionBody = (logType: BDILogType, body: object) => {
  if (isBdiLogConditionBody(logType, body)) {
    const { condition, goal, goalId, intentionId, plan, success, taskId } = body;

    if (intentionId === undefined) {
      throw new Error('BdiLogCondition::intentionId is undefined');
    } else if (typeof intentionId !== 'string') {
      throw new Error('BdiLogCondition::intentionId is not string type');
    }

    if (goal === undefined) {
      throw new Error('BdiLogCondition::goal is undefined');
    } else if (typeof goal !== 'string') {
      throw new Error('BdiLogCondition::goal is not string type');
    }

    if (goalId === undefined) {
      throw new Error('BdiLogCondition::goalId is undefined');
    } else if (typeof goalId !== 'string') {
      throw new Error('BdiLogCondition::goalId is not string type');
    }

    if (condition === undefined) {
      throw new Error('BdiLogCondition::condition is undefined');
    } else if (typeof condition !== 'string') {
      throw new Error('BdiLogCondition::condition is not string type');
    }

    if (taskId === undefined) {
      throw new Error('BdiLogCondition::taskId is undefined');
    } else if (typeof taskId !== 'string') {
      throw new Error('BdiLogCondition::taskId is not string type');
    }

    if (plan === undefined) {
      throw new Error('BdiLogCondition::plan is undefined');
    } else if (typeof plan !== 'string') {
      throw new Error('BdiLogCondition::plan is not string type');
    }

    if (success === undefined) {
      throw new Error('BdiLogCondition::success is undefined');
    } else if (typeof success !== 'boolean') {
      throw new Error('BdiLogCondition::success is not boolean type');
    }
  }
};

const validateBdiLogSleepBody = (logType: BDILogType, body: object) => {
  if (isBdiLogSleepBody(logType, body)) {
    const { goal, goalId, intentionId, plan, sleepMs, taskId } = body;

    if (intentionId === undefined) {
      throw new Error('BdiLogSleep::intentionId is undefined');
    } else if (typeof intentionId !== 'string') {
      throw new Error('BdiLogSleep::intentionId is not string type');
    }

    if (goal === undefined) {
      throw new Error('BdiLogSleep::goal is undefined');
    } else if (typeof goal !== 'string') {
      throw new Error('BdiLogSleep::goal is not string type');
    }

    if (goalId === undefined) {
      throw new Error('BdiLogSleep::goalId is undefined');
    } else if (typeof goalId !== 'string') {
      throw new Error('BdiLogSleep::goalId is not string type');
    }

    if (sleepMs === undefined) {
      throw new Error('BdiLogSleep::sleepMs is undefined');
    } else if (typeof sleepMs !== 'number') {
      throw new Error('BdiLogSleep::sleepMs is not number type');
    }

    if (taskId === undefined) {
      throw new Error('BdiLogSleep::taskId is undefined');
    } else if (typeof taskId !== 'string') {
      throw new Error('BdiLogSleep::taskId is not string type');
    }

    if (plan === undefined) {
      throw new Error('BdiLogSleep::plan is undefined');
    } else if (typeof plan !== 'string') {
      throw new Error('BdiLogSleep::plan is not string type');
    }
  }
};

const validateSubBdiLogBody = (logType: BDILogType, body: object) => {
  switch (logType) {
    case BDILogType.GOAL_STARTED:
    case BDILogType.GOAL_FINISHED:
    case BDILogType.SUB_GOAL_STARTED:
    case BDILogType.SUB_GOAL_FINISHED:
      validateBdiLogGoalBody(logType, body);
      break;

    case BDILogType.INTENTION_STARTED:
    case BDILogType.INTENTION_FINISHED:
      validateBdiLogIntentionBody(logType, body);
      break;

    case BDILogType.ACTION_STARTED:
    case BDILogType.ACTION_FINISHED:
      validateBdiLogActionBody(logType, body);
      break;

    case BDILogType.CONDITION:
      validateBdiLogConditionBody(logType, body);
      break;

    case BDILogType.SLEEP_STARTED:
    case BDILogType.SLEEP_FINISHED:
      validateBdiLogSleepBody(logType, body);
      break;

    default:
      break;
  }
};

const validateBdiLogBody: BodyValidator = (type, body) => {
  if (isBdiLogBody(type, body)) {
    const { level, logType, ...restBdiLogBody } = body;

    if (level === undefined) {
      throw new Error('level is undefined');
    } else if (!(level in BDILogLevel)) {
      throw new Error('level is not in BDILogLevel range');
    }

    if (logType === undefined) {
      throw new Error('logType is undefined');
    } else if (!(logType in BDILogType)) {
      throw new Error('logType is not in BDILogType range');
    }

    validateSubBdiLogBody(logType, restBdiLogBody);
  }
};

/**
 * Validate C-BDI message
 * @param data
 */
const validateCbdiMessage = (data: Event) => {
  const { eventId, recipient, sender, senderNode, timestampUs, type, ...body } = data;

  if (eventId === undefined) {
    throw new Error('eventId is undefined');
  } else if (typeof eventId !== 'string') {
    throw new Error('eventId is not string type');
  }

  if (recipient === undefined) {
    throw new Error('recipient is undefined');
  } else {
    validateAddress(recipient);
  }

  if (sender === undefined) {
    throw new Error('sender is undefined');
  } else {
    validateAddress(sender);
  }

  if (senderNode === undefined) {
    throw new Error('senderNode is undefined');
  } else {
    validateAddress(senderNode);
  }

  if (timestampUs === undefined) {
    throw new Error('timestampUs is undefined');
  } else if (typeof timestampUs !== 'number') {
    throw new Error('timestampUs is not number type');
  }

  if (type === undefined) {
    throw new Error('type is undefined');
  } else if (!(type in EventType)) {
    LOGGER.error(type);
    throw new Error('type is not in range of EventType');
  }

  // Check body
  switch (type) {
    case EventType.REGISTER:
      validateRegisterBody(type, body);
      break;
    case EventType.ACTION_BEGIN:
      validateActionBeginBody(type, body);
      break;
    case EventType.ACTION_UPDATE:
      validateActionUpdateBody(type, body);
      break;
    case EventType.AGENT_JOIN_TEAM:
      validateAgentJoinTeamBody(type, body);
      break;
    case EventType.AGENT_LEAVE_TEAM:
      validateAgentLeaveTeamBody(type, body);
      break;
    case EventType.CONTROL:
      validateControlBody(type, body);
      break;
    case EventType.DELEGATION:
      validateDelegationBody(type, body);
      break;
    case EventType.DEREGISTER:
      validateDeregisterBody(type, body);
      break;
    case EventType.DROP:
      validateDropBody(type, body);
      break;
    case EventType.MESSAGE:
      validateMessageBody(type, body);
      break;
    case EventType.PERCEPT:
      validatePerceptBody(type, body);
      break;
    case EventType.PURSUE:
      validatePursueBody(type, body);
      break;
    case EventType.BDI_LOG:
      validateBdiLogBody(type, body);
      break;
    case EventType.COUNT:
      break;
    default:
      break;
  }
};

export default validateCbdiMessage;
