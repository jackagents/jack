// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

/// Local
#include <jack/event-protocol/protocol.h>
#include <jack/corelib.h>

namespace aos::jack::protocol
{
std::string BusAddress::toString() const
{
    std::string result = JACK_FMT("BusAddress{{type={}, id={}, name={}}}",
                                  nodeTypeString(type),
                                  compactString(id),
                                  name);
    return result;
}

std::string Event::toString() const
{
    std::string result = JACK_FMT("Event{{timestampUs={}, type={}, senderNode={}, sender={}, recipient={}, eventId={}}}",
                                  timestampUs,
                                  eventTypeString(type),
                                  makeBusAddressString(senderNode, true /*compact*/),
                                  makeBusAddressString(sender, true /*compact*/),
                                  makeBusAddressString(recipient, true /*compact*/),
                                  compactString(eventId.toString()));
    return result;
}

std::string Control::toString() const
{
    std::string result = JACK_FMT("Command{{{}, command={}}}",
                                  static_cast<const Event*>(this)->toString(),
                                  commandTypeString(command));
    return result;
}

std::string Percept::toString() const
{
    std::string result = JACK_FMT("Percept{{{}, beliefSet={}, field={}}}",
                                  static_cast<const Event*>(this)->toString(),
                                  beliefSet,
                                  field.toString());
    return result;
}

std::string Message::toString() const
{
    std::string result = JACK_FMT("Message{{{}, data={}}}",
                                  static_cast<const Event*>(this)->toString(),
                                  data->toString());
    return result;
}

std::string Pursue::toString() const
{
    std::string result = JACK_FMT("Pursue{{{}, goal={}, persistent={}, message={}}}",
                                  static_cast<const Event*>(this)->toString(),
                                  goal,
                                  persistent,
                                  message->toString());
    return result;
}

void to_json(nlohmann::json& nlohmann_json_j, const Message& nlohmann_json_t)
{
    // call the base
    nlohmann::to_json(nlohmann_json_j, static_cast<const Event &>(nlohmann_json_t));

    auto& msgBlob = nlohmann_json_j["message"];
    if (nlohmann_json_t.data) {
        nlohmann_json_t.data->serialise(msgBlob);
    } else {
        nlohmann_json_j["message"] = {};
    }
}

void to_json(nlohmann::json& nlohmann_json_j, const Pursue& nlohmann_json_t)
{
    // call the base
    nlohmann::to_json(nlohmann_json_j, static_cast<const Event &>(nlohmann_json_t));

    // serialise the fields
    nlohmann_json_j["goal"] = nlohmann_json_t.goal;
    nlohmann_json_j["persistent"] = nlohmann_json_t.persistent;

    auto& msgBlob = nlohmann_json_j["message"];

    if (nlohmann_json_t.message) {
        nlohmann_json_t.message->serialise(msgBlob);
        
    } else {
        nlohmann_json_j["message"] = {};
    }

    /// Reflection version outputs the message type name as well
    /*
    auto& msgBlob = nlohmann_json_j["message"];

    // patch for fields
    auto& fieldsBlob = msgBlob["fields"];

    if (nlohmann_json_t.message) {
        nlohmann_json_t.message->serialise(fieldsBlob);
        msgBlob["name"] = nlohmann_json_t.message->schema();
        
    } else {
        nlohmann_json_j["message"] = {};
        msgBlob["name"] = "";
    }*/
}

void to_json(nlohmann::json& nlohmann_json_j, const Delegation& nlohmann_json_t)
{
    // call the base
    nlohmann::to_json(nlohmann_json_j, static_cast<const Event &>(nlohmann_json_t));

    // serialise the fields
    nlohmann_json_j["status"] = nlohmann_json_t.status;
    nlohmann_json_j["goal"] = nlohmann_json_t.goal;
    nlohmann_json_j["goalId"] = nlohmann_json_t.goalId;

    auto& msg = nlohmann_json_j["message"];
    if (nlohmann_json_t.message) {
        nlohmann_json_t.message->serialise(msg);
    } else {
        nlohmann_json_j["message"] = {};
    }

    nlohmann_json_j["analyse"] = nlohmann_json_t.analyse;
    nlohmann_json_j["score"] = nlohmann_json_t.score;
    nlohmann_json_j["team"] = nlohmann_json_t.team;
    nlohmann_json_j["teamId"] = nlohmann_json_t.teamId;
}

void to_json(nlohmann::json& nlohmann_json_j, const ActionUpdate& nlohmann_json_t)
{
    // call the base
    nlohmann::to_json(nlohmann_json_j, static_cast<const Event &>(nlohmann_json_t));

    // serialise the fields
    nlohmann_json_j["name"] = nlohmann_json_t.name;
    nlohmann_json_j["taskId"] = nlohmann_json_t.taskId;
    nlohmann_json_j["goal"] = nlohmann_json_t.goal;
    nlohmann_json_j["goalId"] = nlohmann_json_t.goalId;
    nlohmann_json_j["intentionId"] = nlohmann_json_t.intentionId;
    nlohmann_json_j["plan"] = nlohmann_json_t.plan;
    nlohmann_json_j["status"] = nlohmann_json_t.status;

    auto& msg = nlohmann_json_j["reply"];
    if (nlohmann_json_t.reply) {
        nlohmann_json_t.reply->serialise(msg);
    } else {
        nlohmann_json_j["reply"] = {};
    }
}

void to_json(nlohmann::json& nlohmann_json_j, const ActionBegin& nlohmann_json_t)
{
    // call the base
    nlohmann::to_json(nlohmann_json_j, static_cast<const Event &>(nlohmann_json_t));

    // serialise the fields
    nlohmann_json_j["name"] = nlohmann_json_t.name;
    nlohmann_json_j["taskId"] = nlohmann_json_t.taskId;
    nlohmann_json_j["goal"] = nlohmann_json_t.goal;
    nlohmann_json_j["goalId"] = nlohmann_json_t.goalId;
    nlohmann_json_j["intentionId"] = nlohmann_json_t.intentionId;
    nlohmann_json_j["plan"] = nlohmann_json_t.plan;

    auto& msg = nlohmann_json_j["message"];
    if (nlohmann_json_t.message) {
        nlohmann_json_t.message->serialise(msg);
    } else {
        nlohmann_json_j["message"] = {};
    }

    nlohmann_json_j["resourceLocks"] = nlohmann_json_t.resourceLocks;
}

void from_json(const nlohmann::json& nlohmann_json_j, Pursue& nlohmann_json_t)
{
    nlohmann::from_json(nlohmann_json_j, static_cast<Event&>(nlohmann_json_t));

    nlohmann_json_j.at("goal").get_to(nlohmann_json_t.goal);
    nlohmann_json_j.at("persistent").get_to(nlohmann_json_t.persistent);

    /// @todo finish
    // find the message type
    // create a new message
    // serialise into the new message
    //nlohmann_json_j.at("message").get_to(nlohmann_json_t.message);
    assert(0);
}

void from_json(const nlohmann::json& nlohmann_json_j, Delegation& nlohmann_json_t)
{
    nlohmann::from_json(nlohmann_json_j, static_cast<Event&>(nlohmann_json_t));
    
    /// @todo finish
    assert(0);
}

std::string Drop::toString() const
{
    std::string result = JACK_FMT("Drop{{{}, goal={}, goalId={}, mode={}}}",
                                  static_cast<const Event*>(this)->toString(),
                                  goal,
                                  compactString(goalId),
                                  dropModeString(mode));
    return result;
}

std::string Delegation::toString() const
{
    std::string_view statusString = delegationStatusString(status);
    std::string result = JACK_FMT("Delegation{{{}, status={}, goal={}, goalId={}, analyse={}, score={}, message={}}}",
                                  static_cast<const Event*>(this)->toString(),
                                  statusString,
                                  goal,
                                  compactString(goalId),
                                  analyse,
                                  score,
                                  message->toString());
    return result;
}

/// Protocol
std::string Register::toString() const
{
    std::string result = JACK_FMT("Register{{{}, address={}, template={}}}",
                                  static_cast<const Event*>(this)->toString(),
                                  makeBusAddressString(address, true /*compact*/),
                                  templateType);
    return result;
}

std::string AgentJoinTeam::toString() const
{
    std::string result = JACK_FMT("AgentJoinTeam{{{}, agent={}, team={}}}",
                                  static_cast<const Event*>(this)->toString(),
                                  makeBusAddressString(agent, true /*compact*/),
                                  makeBusAddressString(team, true /*compact*/));
    return result;
}

std::string ActionBegin::toString() const
{
    ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
    StringBuilder builder          = StringBuilder(scratch.arena);
    builder.append(FMT_STRING("ActionBegin{{{}, name={}, taskId={}, message={}, resourcesLocks={{"),
                   static_cast<const Event*>(this)->toString(),
                   name,
                   compactString(taskId),
                   message->toString());

    for (size_t resourceIndex = 0; resourceIndex < resourceLocks.size(); resourceIndex++) {
       std::string_view resource = resourceLocks[resourceIndex];
       if (resourceIndex) {
           builder.appendRef(", ");
       }
       builder.appendRef(resource);
    }

    builder.append(FMT_STRING("}}, goal={}, goalId={}, intentionId={}, plan={}}}"),
                   goal,
                   compactString(goalId),
                   compactString(intentionId),
                   plan);

    std::string result = builder.toString();
    return result;
}

std::string ActionUpdate::toString() const
{
    std::string result = JACK_FMT("ActionUpdate{{{}, name={}, taskId={}, status={}, goal={}, goalId={}, plan={}, reply={}}}",
                                  static_cast<const Event*>(this)->toString(),
                                  name,
                                  compactString(taskId),
                                  actionStatusString(status),
                                  goal,
                                  compactString(goalId),
                                  plan,
                                  reply->toString());
    return result;
}

std::string BDILog::toString() const
{
    ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
    StringBuilder builder          = StringBuilder(scratch.arena);
    builder.append(FMT_STRING("BDILog{{{}, level={}, logType={}"),
                   static_cast<const Event*>(this)->toString(),
                   bdiLogLevelString(level),
                   bdiLogTypeString(logType));

    switch (logType) {
        case BDILogType_GOAL_STARTED:
        case BDILogType_GOAL_FINISHED:
        case BDILogType_SUB_GOAL_STARTED:
        case BDILogType_SUB_GOAL_FINISHED:
            builder.append(FMT_STRING("goal={}"), goal.toString());
            break;

        case BDILogType_INTENTION_STARTED:
        case BDILogType_INTENTION_FINISHED:
            builder.append(FMT_STRING("intention={}"), intention.toString());
            break;

        case BDILogType_ACTION_STARTED:
        case BDILogType_ACTION_FINISHED:
            builder.append(FMT_STRING("action={}"), action.toString());
            break;

        case BDILogType_SLEEP_STARTED:
        case BDILogType_SLEEP_FINISHED:
            builder.append(FMT_STRING("sleep={}"), sleep.toString());
            break;

        case BDILogType_CONDITION:
            builder.append(FMT_STRING("condition={}"), condition.toString());
            break;
    }

    builder.appendRef("}");
    std::string result = builder.toString();
    return result;
}

std::string BDILog::Goal::toString() const
{
    std::string result = JACK_FMT("BDILog::Goal{{goal={}, goalId={}, intentionId={}, taskId={}, dropReason={}, result={}}}",
                                  goal,
                                  compactString(goalId),
                                  compactString(intentionId),
                                  compactString(taskId),
                                  dropReason,
                                  result);
    return result;
}

std::string BDILog::Intention::toString() const
{
    std::string result = JACK_FMT("BDILog::Intention{{goal={}, goalId={}, intentionId={}, plan={}, result={}}}",
                                  goal,
                                  compactString(goalId),
                                  compactString(intentionId),
                                  plan,
                                  result);
    return result;
}

std::string BDILog::Action::toString() const
{
    std::string result = JACK_FMT("BDILog::Intention{{goal={}, goalId={}, intentionId={}, plan={}, taskId={}, action={}, reasoning={}, success={}}}",
                                  goal,
                                  compactString(goalId),
                                  compactString(intentionId),
                                  plan,
                                  compactString(taskId),
                                  action,
                                  reasoning,
                                  success);
    return result;
}

std::string BDILog::Sleep::toString() const
{
    std::string result = JACK_FMT("BDILog::Sleep{{goal={}, goalId={}, intentionId={}, plan={}, taskId={}, sleepMs={}}}",
                                  goal,
                                  compactString(goalId),
                                  compactString(intentionId),
                                  plan,
                                  compactString(taskId),
                                  sleepMs);
    return result;
}

std::string BDILog::Condition::toString() const
{
    std::string result = JACK_FMT("BDILog::Condition{{goal={}, goalId={}, intentionId={}, plan={}, taskId={}, condition={}, success={}}}",
                                  goal,
                                  compactString(goalId),
                                  compactString(intentionId),
                                  plan,
                                  compactString(taskId),
                                  condition,
                                  success);
    return result;
}

// The to_json() and from_json() functions below are specialisations
// utilised by the nlohmann JSON serialisation mechanism.
// These functions must be in the same namespace as the class
// they operate on and are declared friends.
// These particular BDILog functions are complicated by the
// desire to use a single DDS topic for BDILog as well the
// desire to use a DDS union type deliniate each of the
// log types, goal, intention, action, sleep, condition.
void to_json(nlohmann::json& j, const BDILog& e)
{
    to_json(j, static_cast<const Event&>(e));
    j["level"] = e.level;
    j["logType"] = e.logType;
    switch(e.logType)
    {
    case BDILogType_GOAL_STARTED:
    case BDILogType_GOAL_FINISHED:
    case BDILogType_SUB_GOAL_STARTED:
    case BDILogType_SUB_GOAL_FINISHED:
        j["goal"] = e.goal.goal;
        j["goalId"] = e.goal.goalId;
        j["intentionId"] = e.goal.intentionId;
        j["taskId"] = e.goal.taskId;
        j["dropReason"] = e.goal.dropReason;
        j["result"] = e.goal.result;
        break;

    case BDILogType_INTENTION_STARTED:
    case BDILogType_INTENTION_FINISHED:
        j["goal"] = e.intention.goal;
        j["goalId"] = e.intention.goalId;
        j["intentionId"] = e.intention.intentionId;
        j["plan"] = e.intention.plan;
        j["result"] = e.intention.result;
        break;

    case BDILogType_ACTION_STARTED:
    case BDILogType_ACTION_FINISHED:
        j["goal"] = e.action.goal;
        j["goalId"] = e.action.goalId;
        j["intentionId"] = e.action.intentionId;
        j["plan"] = e.action.plan;
        j["taskId"] = e.action.taskId;
        j["action"] = e.action.action;
        j["reasoning"] = e.action.reasoning;
        j["success"] = e.action.success;
        break;

   case BDILogType_SLEEP_STARTED:
   case BDILogType_SLEEP_FINISHED:
        j["goal"] = e.sleep.goal;
        j["goalId"] = e.sleep.goalId;
        j["intentionId"] = e.sleep.intentionId;
        j["plan"] = e.sleep.plan;
        j["taskId"] = e.sleep.taskId;
        j["sleepMs"] = e.sleep.sleepMs;
        break;

   case BDILogType_CONDITION:
        j["goal"] = e.condition.goal;
        j["goalId"] = e.condition.goalId;
        j["intentionId"] = e.condition.intentionId;
        j["plan"] = e.condition.plan;
        j["taskId"] = e.condition.taskId;
        j["condition"] = e.condition.condition;
        j["success"] = e.condition.success;
        break;
    }
}

void from_json(const nlohmann::json& j, BDILog& e)
{
    from_json(j, static_cast<Event&>(e));
    j.at("level").get_to(e.level);
    j.at("logType").get_to(e.logType);
    switch(e.logType)
    {
    case BDILogType_GOAL_STARTED:
    case BDILogType_GOAL_FINISHED:
    case BDILogType_SUB_GOAL_STARTED:
    case BDILogType_SUB_GOAL_FINISHED:
        j.at("goal").get_to(e.goal.goal);
        j.at("goalId").get_to(e.goal.goalId);
        j.at("intentionId").get_to(e.goal.intentionId);
        j.at("taskId").get_to(e.goal.taskId);
        j.at("dropReason").get_to(e.goal.dropReason);
        j.at("result").get_to(e.goal.result);
        break;

    case BDILogType_INTENTION_STARTED:
    case BDILogType_INTENTION_FINISHED:
        j.at("goal").get_to(e.intention.goal);
        j.at("goalId").get_to(e.intention.goalId);
        j.at("intentionId").get_to(e.intention.intentionId);
        j.at("plan").get_to(e.intention.plan);
        j.at("result").get_to(e.intention.result);
        break;

    case BDILogType_ACTION_STARTED:
    case BDILogType_ACTION_FINISHED:
        j.at("goal").get_to(e.action.goal);
        j.at("goalId").get_to(e.action.goalId);
        j.at("intentionId").get_to(e.action.intentionId);
        j.at("taskId").get_to(e.action.taskId);
        j.at("plan").get_to(e.action.plan);
        j.at("taskId").get_to(e.action.taskId);
        j.at("action").get_to(e.action.action);
        j.at("reasoning").get_to(e.action.reasoning);
        j.at("success").get_to(e.action.success);
        break;

    case BDILogType_SLEEP_STARTED:
    case BDILogType_SLEEP_FINISHED:
        j.at("goal").get_to(e.sleep.goal);
        j.at("goalId").get_to(e.sleep.goalId);
        j.at("intentionId").get_to(e.sleep.intentionId);
        j.at("taskId").get_to(e.sleep.taskId);
        j.at("plan").get_to(e.sleep.plan);
        j.at("taskId").get_to(e.sleep.taskId);
        j.at("sleepMs").get_to(e.sleep.sleepMs);
        break;

    case BDILogType_CONDITION:
        j.at("goal").get_to(e.condition.goal);
        j.at("goalId").get_to(e.condition.goalId);
        j.at("intentionId").get_to(e.condition.intentionId);
        j.at("taskId").get_to(e.condition.taskId);
        j.at("plan").get_to(e.condition.plan);
        j.at("taskId").get_to(e.condition.taskId);
        j.at("condition").get_to(e.condition.condition);
        j.at("success").get_to(e.condition.success);
        break;
    }
}

std::string_view format_as(NodeType value)
{
    std::string_view result = nodeTypeString(value);
    return result;
}

std::string_view format_as(EventType value)
{
    std::string_view result = eventTypeString(value);
    return result;
}

std::string_view format_as(ControlCommand value)
{
    std::string_view result = commandTypeString(value);
    return result;
}

std::string_view format_as(AnyType value)
{
    std::string_view result = anyTypeString(value);
    return result;
}

std::string_view format_as(DropMode value)
{
    std::string_view result = dropModeString(value);
    return result;
}

std::string_view format_as(DelegationStatus value)
{
    std::string_view result = delegationStatusString(value);
    return result;
}

std::string_view format_as(ActionStatus value)
{
    std::string_view result = actionStatusString(value);
    return result;
}

std::string_view format_as(BDILogLevel value)
{
    std::string_view result = bdiLogLevelString(value);
    return result;
}

std::string_view format_as(BDILogGoalIntentionResult value)
{
    std::string_view result = bdiLogGoalIntentionResultString(value);
    return result;
}

std::string format_as(const BusAddress& value)
{
    std::string result = value.toString();
    return result;
}

std::string format_as(const Event& value)
{
    std::string result = value.toString();
    return result;
}

std::string format_as(const Control& value)
{
    std::string result = value.toString();
    return result;
}

std::string format_as(const Percept& value)
{
    std::string result = value.toString();
    return result;
}

std::string format_as(const Pursue& value)
{
    std::string result = value.toString();
    return result;
}

std::string format_as(const Drop& value)
{
    std::string result = value.toString();
    return result;
}

std::string format_as(const Delegation& value)
{
    std::string result = value.toString();
    return result;
}

std::string format_as(const Register& value)
{
    std::string result = value.toString();
    return result;
}

std::string format_as(const AgentJoinTeam& value)
{
    std::string result = value.toString();
    return result;
}

std::string format_as(const ActionBegin& value)
{
    std::string result = value.toString();
    return result;
}

std::string format_as(const ActionUpdate& value)
{
    std::string result = value.toString();
    return result;
}

std::string format_as(const BDILog& value)
{
    std::string result = value.toString();
    return result;
}

std::string format_as(const BDILog::Goal& value)
{
    std::string result = value.toString();
    return result;
}

std::string format_as(const BDILog::Intention& value)
{
    std::string result = value.toString();
    return result;
}

std::string format_as(const BDILog::Action& value)
{
    std::string result = value.toString();
    return result;
}

std::string format_as(const BDILog::Sleep& value)
{
    std::string result = value.toString();
    return result;
}

std::string format_as(const BDILog::Condition& value)
{
    std::string result = value.toString();
    return result;
}

/******************************************************************************
 * Enum Strings
 ******************************************************************************/
#define SWITCH_CASE_ENUM_STRING(prefix, enum_value) prefix##_##enum_value: return #enum_value
std::string_view nodeTypeString(NodeType type)
{
    switch (type) {
        case SWITCH_CASE_ENUM_STRING(NodeType, GENERIC);
        case SWITCH_CASE_ENUM_STRING(NodeType, NODE);
        case SWITCH_CASE_ENUM_STRING(NodeType, SERVICE);
        case SWITCH_CASE_ENUM_STRING(NodeType, AGENT);
        case SWITCH_CASE_ENUM_STRING(NodeType, TEAM);
        case SWITCH_CASE_ENUM_STRING(NodeType, COUNT);
    }
    return "NODE_TYPE_BAD_ENUM_VALUE";
}

std::string_view eventTypeString(EventType type)
{
    switch (type) {
        case SWITCH_CASE_ENUM_STRING(EventType, NONE);
        /// JACK
        case SWITCH_CASE_ENUM_STRING(EventType, CONTROL);
        case SWITCH_CASE_ENUM_STRING(EventType, PERCEPT);
        case SWITCH_CASE_ENUM_STRING(EventType, MESSAGE);
        case SWITCH_CASE_ENUM_STRING(EventType, PURSUE);
        case SWITCH_CASE_ENUM_STRING(EventType, DROP);
        case SWITCH_CASE_ENUM_STRING(EventType, DELEGATION);
        /// Protocol
        case SWITCH_CASE_ENUM_STRING(EventType, REGISTER);
        case SWITCH_CASE_ENUM_STRING(EventType, DEREGISTER);
        case SWITCH_CASE_ENUM_STRING(EventType, AGENT_JOIN_TEAM);
        case SWITCH_CASE_ENUM_STRING(EventType, AGENT_LEAVE_TEAM);
        case SWITCH_CASE_ENUM_STRING(EventType, ACTION_BEGIN);
        case SWITCH_CASE_ENUM_STRING(EventType, ACTION_UPDATE);
        case SWITCH_CASE_ENUM_STRING(EventType, BDI_LOG);
        case SWITCH_CASE_ENUM_STRING(EventType, COUNT);
    }
    return "EVENT_TYPE_BAD_ENUM_VALUE";
}

std::string_view commandTypeString(ControlCommand command)
{
    switch (command) {
        case SWITCH_CASE_ENUM_STRING(ControlCommand, START);
        case SWITCH_CASE_ENUM_STRING(ControlCommand, PAUSE);
        case SWITCH_CASE_ENUM_STRING(ControlCommand, STOP);
        case SWITCH_CASE_ENUM_STRING(ControlCommand, COUNT);
    }
    return "COMMAND_TYPE_BAD_ENUM_VALUE";
}

std::string_view anyTypeString(AnyType type)
{
    switch (type) {
        case SWITCH_CASE_ENUM_STRING(AnyType, I8);
        case SWITCH_CASE_ENUM_STRING(AnyType, I16);
        case SWITCH_CASE_ENUM_STRING(AnyType, I32);
        case SWITCH_CASE_ENUM_STRING(AnyType, I64);
        case SWITCH_CASE_ENUM_STRING(AnyType, U8);
        case SWITCH_CASE_ENUM_STRING(AnyType, U16);
        case SWITCH_CASE_ENUM_STRING(AnyType, U32);
        case SWITCH_CASE_ENUM_STRING(AnyType, U64);
        case SWITCH_CASE_ENUM_STRING(AnyType, F32);
        case SWITCH_CASE_ENUM_STRING(AnyType, F64);
        case SWITCH_CASE_ENUM_STRING(AnyType, Bool);
        case SWITCH_CASE_ENUM_STRING(AnyType, V2);
        case SWITCH_CASE_ENUM_STRING(AnyType, String);
        case SWITCH_CASE_ENUM_STRING(AnyType, Message);
        case SWITCH_CASE_ENUM_STRING(AnyType, Count);
    }
    return "ANY_TYPE_BAD_ENUM_VALUE";
}

std::string_view anyTypePropertyString(AnyType type, bool array)
{
    switch (type) {
        case AnyType_I8:      return array ? "I8[]"      : "I8";
        case AnyType_I16:     return array ? "I16[]"     : "I16";
        case AnyType_I32:     return array ? "I32[]"     : "I32";
        case AnyType_I64:     return array ? "I64[]"     : "I64";
        case AnyType_U8:      return array ? "U8[]"      : "U8";
        case AnyType_U16:     return array ? "U16[]"     : "U16";
        case AnyType_U32:     return array ? "U32[]"     : "U32";
        case AnyType_U64:     return array ? "U64[]"     : "U64";
        case AnyType_F32:     return array ? "F32[]"     : "F32";
        case AnyType_F64:     return array ? "F64[]"     : "F64";
        case AnyType_Bool:    return array ? "Bool[]"    : "Bool";
        case AnyType_V2:      return array ? "Vec2[]"    : "Vec2";
        case AnyType_String:  return array ? "String[]"  : "String";
        case AnyType_Message: return array ? "Message[]" : "Message";
        case AnyType_Count:   break;
    }
    return "ANY_TYPE_BAD_ENUM_VALUE";
}

std::string_view dropModeString(DropMode drop)
{
    switch (drop) {
        case SWITCH_CASE_ENUM_STRING(DropMode, NORMAL);
        case SWITCH_CASE_ENUM_STRING(DropMode, FORCE);
    }
    return "DROP_MODE_BAD_ENUM_VALUE";
}

std::string_view delegationStatusString(DelegationStatus value)
{
    switch(value) {
        case SWITCH_CASE_ENUM_STRING(DelegationStatus, PENDING);
        case SWITCH_CASE_ENUM_STRING(DelegationStatus, FAILED);
        case SWITCH_CASE_ENUM_STRING(DelegationStatus, SUCCESS);
    }
    return "DELEGATION_STATUS_BAD_ENUM_VALUE";
}

std::string_view actionStatusString(ActionStatus type)
{
    switch (type) {
        case SWITCH_CASE_ENUM_STRING(ActionStatus, SUCCESS);
        case SWITCH_CASE_ENUM_STRING(ActionStatus, FAILED);
    }
    return "ACTION_STATUS_BAD_ENUM_VALUE";
}

std::string_view bdiLogLevelString(BDILogLevel value)
{
    switch (value) {
        case SWITCH_CASE_ENUM_STRING(BDILogLevel, NORMAL);
        case SWITCH_CASE_ENUM_STRING(BDILogLevel, IMPORTANT);
        case SWITCH_CASE_ENUM_STRING(BDILogLevel, CRITICAL);
    }
    return "BDI_LOG_LEVEL_BAD_ENUM_VALUE";
}

std::string_view bdiLogTypeString(BDILogType value)
{
    switch (value) {
        case SWITCH_CASE_ENUM_STRING(BDILogType, GOAL_STARTED);
        case SWITCH_CASE_ENUM_STRING(BDILogType, GOAL_FINISHED);
        case SWITCH_CASE_ENUM_STRING(BDILogType, SUB_GOAL_STARTED);
        case SWITCH_CASE_ENUM_STRING(BDILogType, SUB_GOAL_FINISHED);
        case SWITCH_CASE_ENUM_STRING(BDILogType, INTENTION_STARTED);
        case SWITCH_CASE_ENUM_STRING(BDILogType, INTENTION_FINISHED);
        case SWITCH_CASE_ENUM_STRING(BDILogType, ACTION_STARTED);
        case SWITCH_CASE_ENUM_STRING(BDILogType, ACTION_FINISHED);
        case SWITCH_CASE_ENUM_STRING(BDILogType, SLEEP_STARTED);
        case SWITCH_CASE_ENUM_STRING(BDILogType, SLEEP_FINISHED);
        case SWITCH_CASE_ENUM_STRING(BDILogType, CONDITION);
    }
    return "BDI_LOG_TYPE_BAD_ENUM_VALUE";
}

std::string_view bdiLogGoalIntentionResultString(BDILogGoalIntentionResult value)
{
    switch (value) {
        case SWITCH_CASE_ENUM_STRING(BDILogGoalIntentionResult, FAILED);
        case SWITCH_CASE_ENUM_STRING(BDILogGoalIntentionResult, SUCCESS);
        case SWITCH_CASE_ENUM_STRING(BDILogGoalIntentionResult, DROPPED);
    }
    return "BDI_LOG_GOAL_INTENTION_RESULT_BAD_ENUM_VALUE";
}

BDILogGoalIntentionResult finishStateToBDILogGoalIntentionResult(FinishState finishState)
{
    protocol::BDILogGoalIntentionResult result = {};
    switch (finishState) {
        case FinishState::NOT_YET: JACK_INVALID_CODE_PATH; result = protocol::BDILogGoalIntentionResult_FAILED; break;
        case FinishState::DROPPED: result = protocol::BDILogGoalIntentionResult_DROPPED; break;
        case FinishState::FAILED:  result = protocol::BDILogGoalIntentionResult_FAILED; break;
        case FinishState::SUCCESS: result = protocol::BDILogGoalIntentionResult_SUCCESS; break;
    }
    return result;
}

#undef SWITCH_CASE_ENUM_STRING

/******************************************************************************
 * Events
 ******************************************************************************/
/// \todo This is copied from util.h because I we may seperate the event protocol from core.
static std::string compactString(std::string_view input,
                                 unsigned         head = 3,
                                 unsigned         tail = 3,
                                 unsigned         partitionSize = 2,
                                 char             partitionChar = '.')
{
    std::string result;
    std::size_t minLength = head + tail + partitionSize;
    if (input.size() > minLength) {
       result.append(input.data(), head);
       result.append(partitionSize, partitionChar);
       result.append(input.data() + input.size() - tail, tail);
    } else {
       result = input;
    }

    return result;
}

/// \note Denotes the order of fields in the '/' delimited bus address string.
/// Changing the order of this enum will change the order that the bus address
/// is formatted into a string. i.e.  <protocol>/<node_type>/<name>/<id>
///
/// "jack/node/IWatchDogPlanner/000000000"
/// "jack/node/IWatchDogPlanner/000000000"
/// "jack/team/Coordinator/000000000"
/// "jack/agent/UAVScout/000000000"
/// "jack/service/UAVScoutDroneBattery/000000000"
///
enum BusAddressStringField
{
    BusAddressStringField_PROTOCOL,
    BusAddressStringField_NODE_TYPE,
    BusAddressStringField_NAME,
    BusAddressStringField_ID,
    BusAddressStringField_COUNT,
};

std::string makeBusAddressStringFrom(std::string_view uuid, NodeType type, std::string_view name, bool compact)
{
    static_assert(BusAddressStringField_COUNT == 4, "Encoding a bus address string to a bus address is written to expect 4 fields");
    std::string result;
    if (uuid.empty() || type < 0 || type >= NodeType_COUNT || name.empty()) {
        return result;
    }

    result.reserve(32);
    for (int fieldIndex = 0; fieldIndex < BusAddressStringField_COUNT; fieldIndex++) {
        if (fieldIndex) {
            result.append("/");
        }

        switch (fieldIndex) {
            case BusAddressStringField_PROTOCOL: {
                result.append("jack");
        } break;

            case BusAddressStringField_NODE_TYPE: {
                std::string_view typeString = nodeTypeString(type);
                result.append(typeString);
            } break;

            case BusAddressStringField_NAME: {
                result.append(name);
            } break;

            case BusAddressStringField_ID: {
                result.append(compact ? compactString(uuid) : uuid);
            } break;
        }
    }
    return result;
}

std::string makeBusAddressString(const BusAddress &address, bool compact)
{
    std::string result = makeBusAddressStringFrom(address.id, address.type, address.name, compact);
    return result;
}

BusAddress makeBusAddressFromCString(const char* address, size_t addressSize)
{
    BusAddress result = {};
    if (!address || addressSize <= 0) {
        JACK_PROTOCOL_ASSERT(!result.valid());
        return result;
    }

    /// Verify the number of forward slashes in the address
    const char *addressEnd = address + addressSize;
    {
        int       fwdSlashCount          = 0;
        for (const char *ch = address; ch != addressEnd; ch++) {
            if (ch[0] == '/') {
                fwdSlashCount++;
            }

            if (fwdSlashCount >= (BusAddressStringField_COUNT - 1)) {
                break;
            }
        }

        if (fwdSlashCount != (BusAddressStringField_COUNT - 1)) {
            /// Invalid number of fields, either too many or too little.
            /// Potentially malicious input.
            return result;
        }
    }

    /// Break the address into each field between two forward slashes, i.e.
    /// pull the string enclosed by two forward slashes '/<string>/' into
    /// a range.
    struct Range {
        const char *begin;
        const char *end;
    };

    static_assert(BusAddressStringField_COUNT == 4, "Converting a bus address string to a bus address is written to expect 4 fields");
    Range fields[BusAddressStringField_COUNT] = {};
    int   fieldsCount               = 0;

    for (Range field = {address, address}; field.end != addressEnd; ) {
        field.end++;
        if (field.end == addressEnd || field.end[0] == '/') {
            fields[fieldsCount++] = field;
            field.begin = std::min(addressEnd, field.end + 1);
            field.end   = field.begin;
        }
    }
    JACK_PROTOCOL_ASSERT(fieldsCount == BusAddressStringField_COUNT);

    /// Extract each field into the bus address
    for (int fieldIndex = 0; fieldIndex < BusAddressStringField_COUNT; fieldIndex++) {
        Range field = fields[fieldIndex];
        if (fieldIndex == BusAddressStringField_NODE_TYPE) {
            if (!nodeTypeFromCString(field.begin, field.end - field.begin, result.type)) {
                JACK_PROTOCOL_ASSERT(!result.valid());
                break;
            }
        } else if (fieldIndex == BusAddressStringField_ID) {
            result.id = std::string(field.begin, field.end - field.begin);
        } else if (fieldIndex == BusAddressStringField_NAME) {
            result.name = std::string(field.begin, field.end - field.begin);
        }
    }

    /// Invalid result, zero clear the bus address
    if (!result.valid()) {
        result = {};
        JACK_PROTOCOL_ASSERT(!result.valid());
    }

    return result;
}

BusAddress makeBusAddressFromString(std::string_view address)
{
    BusAddress result = makeBusAddressFromCString(address.data(), address.size());
    return result;
}

bool nodeTypeFromCString(const char *src, size_t srcSize, NodeType &type)
{
    for (size_t typeInt = 0; typeInt < NodeType_COUNT; typeInt++) {
        auto             typeEnum   = static_cast<NodeType>(typeInt);
        std::string_view typeString = nodeTypeString(typeEnum);
        if (std::string_view(src, srcSize) == typeString) {
            type = typeEnum;
            return true;
        }
    }

    return false;
}

bool nodeTypeFromString(std::string_view src, NodeType &type)
{
    bool result = nodeTypeFromCString(src.data(), src.size(), type);
    return result;
}
} /// namespace aos::jack::protocol
