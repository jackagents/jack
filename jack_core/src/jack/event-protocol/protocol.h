#pragma once

/// \todo We used to have a protocol::Any for variants but I've removed that and
/// inline a jack::Field. The back and forth marshalling is expensive, time
/// consuming and bug prone. We initially were going to separate the
/// event-protocol as a library hence the JACK agnostic header but that has not
/// eventuated.
///
/// This is fine because we're solving exactly the problem we have, not a
/// problem we speculate we have. The jack fields have been integrated in a
/// first pass for explainability but lacks some features.
///
/// We don't support arrays of primitive types, but, we do support arrays of
/// messages being sent over the bus as that is currently the use case we're
/// tackling.
#include <jack/fieldregistry.h>
#include <jack/message.h>
#include <jack/engine/uuid.h>

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>
#include <vector>
#include <string_view>

#if !defined(JACK_PROTOCOL_ASSERT)
    #if defined(_MSC_VER) || defined(__MINGW32__)
        #define JACK_PROTOCOL_ASSERT(expr) if (!(expr)) { __debugbreak(); }
    #elif defined(__clang__) || defined(__GNUC__)
        #include <signal.h>
        #define JACK_PROTOCOL_ASSERT(expr) if (!(expr)) { raise(SIGTRAP); }
    #else
        #include <cassert>
        #define JACK_PROTOCOL_ASSERT(expr) assert((expr))
    #endif
#endif

namespace aos::jack::protocol
{
enum NodeType
{
    NodeType_GENERIC,
    NodeType_NODE,
    NodeType_SERVICE,
    NodeType_AGENT,
    NodeType_TEAM,
    NodeType_COUNT,
};

NLOHMANN_JSON_SERIALIZE_ENUM(NodeType, {
    {NodeType_GENERIC, "GENERIC"},
    {NodeType_NODE, "NODE"},
    {NodeType_SERVICE, "SERVICE"},
    {NodeType_AGENT, "AGENT"},
    {NodeType_TEAM, "TEAM"},
    {NodeType_COUNT, "COUNT"},
})

struct BusAddress
{
    BusAddress() = default;

    BusAddress(NodeType type, std::string_view id, std::string_view name) : type(type) , id(id) , name(name) { }

    NodeType    type = {};
    std::string id;   /// \todo 2x U64s or some smaller representation scheme
    std::string name;
    bool operator==(const BusAddress &other) const { return   id == other.id; }
    bool operator!=(const BusAddress &other) const { return !(id == other.id); }

    /// A bus address is valid when it is empty (i.e. zero initialised) or both
    /// name and ID must be set with a valid type.
    bool valid() const { return (type >= 0 && type < NodeType_COUNT) &&
                                ((id.empty() && name.empty()) || (id.size() && name.size())); }

    /// Check if the bus address has been set to a specific entity.
    bool isSet() const { return valid() && name.size() && id.size(); }

    std::string toString() const;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(BusAddress, type, id, name)
};

enum EventType
{
    EventType_NONE,
    /// JACK
    EventType_CONTROL,
    EventType_PERCEPT,
    EventType_PURSUE,
    EventType_DROP,
    EventType_DELEGATION,
    EventType_MESSAGE,
    /// Protocol
    EventType_REGISTER,
    EventType_DEREGISTER,
    EventType_AGENT_JOIN_TEAM,
    EventType_AGENT_LEAVE_TEAM,
    EventType_ACTION_BEGIN,
    EventType_ACTION_UPDATE,
    EventType_BDI_LOG,
    EventType_COUNT,
};


NLOHMANN_JSON_SERIALIZE_ENUM(EventType, {
    {EventType_NONE, "NONE"},
    /// JACK
    {EventType_CONTROL, "CONTROL"},
    {EventType_PERCEPT, "PERCEPT"},
    {EventType_PURSUE, "PURSUE"},
    {EventType_DROP, "DROP"},
    {EventType_DELEGATION, "DELEGATION"},
    {EventType_MESSAGE, "MESSAGE"},
    /// Protocol
    {EventType_REGISTER, "REGISTER"},
    {EventType_DEREGISTER, "DEREGISTER"},
    {EventType_AGENT_JOIN_TEAM, "AGENT_JOIN_TEAM"},
    {EventType_AGENT_LEAVE_TEAM, "AGENT_LEAVE_TEAM"},
    {EventType_ACTION_BEGIN, "ACTION_BEGIN"},
    {EventType_ACTION_UPDATE, "ACTION_UPDATE"},
    {EventType_BDI_LOG, "BDI_LOG"},
    {EventType_COUNT, "COUNT"},
})

struct Event
{
    Event() = default;

    Event(EventType type) : type(type) {}

    virtual ~Event() {}

    std::string toString() const;

    /// @return True if the event is recognised and was deserializable
    bool isOk() const { return type > EventType_NONE && type < EventType_COUNT; }

    uint64_t  timestampUs = 0;                  ///< Timestamp of the event in microseconds
    EventType type        = EventType_NONE;     ///< The type of the event
    UniqueId  eventId     = UniqueId::random(); ///< A unique identifier for the event

    /// \note Event source information
    BusAddress senderNode = {}; ///< The address of the node that sent the event.
    BusAddress sender     = {}; ///< The entity in the node that sent the event.
    /// (Optional) The target of the event. If the recipient is empty (i.e.
    /// zero-initialised) then the recipient is considered everyone on the bus
    /// listening for this message.
    BusAddress recipient  = {};

    friend void to_json(nlohmann::json& j, const Event& e) {
        j["timestampUs"] = e.timestampUs;
        j["type"]        = e.type;
        j["eventId"]     = e.eventId.toString();
        j["senderNode"]  = e.senderNode;
        j["sender"]      = e.sender;
        j["recipient"]   = e.recipient;
    }

    friend void from_json(const nlohmann::json& j, Event& e) {
        j.at("timestampUs").get_to(e.timestampUs);
        j.at("type").get_to(e.type);

        std::string eventId;
        j.at("eventId").get_to(eventId);
        e.eventId = UniqueId::initFromString(eventId);

        j.at("senderNode").get_to(e.senderNode);
        j.at("sender").get_to(e.sender);
        j.at("recipient").get_to(e.recipient);
    }
};


/******************************************************************************
 * JACK
 ******************************************************************************/
enum ControlCommand
{
    ControlCommand_START,
    ControlCommand_PAUSE,
    ControlCommand_STOP,
    ControlCommand_COUNT,
};

NLOHMANN_JSON_SERIALIZE_ENUM(ControlCommand, {
    {ControlCommand_START, "START"},
    {ControlCommand_PAUSE, "PAUSE"},
    {ControlCommand_STOP, "STOP"},
    {ControlCommand_COUNT, "COUNT"},
})

#define JACK_NLOHMANN_DEFINE_DERIVED_TYPE_INTRUSIVE(Type, BaseType, ...)  \
    friend void to_json(nlohmann::json& nlohmann_json_j, const Type& nlohmann_json_t) { nlohmann::to_json(nlohmann_json_j, static_cast<const BaseType &>(nlohmann_json_t)); NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, __VA_ARGS__)) } \
    friend void from_json(const nlohmann::json& nlohmann_json_j, Type& nlohmann_json_t) { nlohmann::from_json(nlohmann_json_j, static_cast<BaseType&>(nlohmann_json_t)); NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, __VA_ARGS__)) }

struct Control : Event
{

    std::string toString() const;

    Control() : Event(EventType_CONTROL) {}
    ControlCommand command = ControlCommand_START;
    JACK_NLOHMANN_DEFINE_DERIVED_TYPE_INTRUSIVE(Control, Event, command)
};

enum AnyType
{
    AnyType_U8,
    AnyType_U16,
    AnyType_U32,
    AnyType_U64,
    AnyType_I8,
    AnyType_I16,
    AnyType_I32,
    AnyType_I64,
    AnyType_F32,
    AnyType_F64,
    AnyType_Bool,
    AnyType_V2,
    AnyType_String,
    AnyType_Message,
    AnyType_Count,
};

NLOHMANN_JSON_SERIALIZE_ENUM(AnyType, {
    {AnyType_U8, "U8"},
    {AnyType_U16, "U16"},
    {AnyType_U32, "U32"},
    {AnyType_U64, "U64"},
    {AnyType_I8, "I8"},
    {AnyType_I16, "I16"},
    {AnyType_I32, "I32"},
    {AnyType_I64, "I64"},
    {AnyType_F32, "F32"},
    {AnyType_F64, "F64"},
    {AnyType_Bool, "Bool"},
    {AnyType_V2, "V2"},
    {AnyType_String, "String"},
    {AnyType_Message, "Message"},
    {AnyType_Count, "Count"},
})

struct Percept : Event
{
    Percept() : Event(EventType_PERCEPT) {}

    std::string toString() const;

    std::string      beliefSet;
    aos::jack::Field field;
    JACK_NLOHMANN_DEFINE_DERIVED_TYPE_INTRUSIVE(Percept, Event, beliefSet, field)
};

struct Message : Event
{
    Message() : Event(EventType_MESSAGE) {}

    std::string toString() const;

    std::shared_ptr<aos::jack::Message> data;

    friend void to_json(nlohmann::json& nlohmann_json_j, const Message& nlohmann_json_t);
};

//void to_json(nlohmann::json& json, const std::shared_ptr<aos::jack::Message>& msg);

struct Pursue : Event
{
    Pursue() : Event(EventType_PURSUE) {}

    std::string toString() const;

    std::string        goal;                     ///< The goal name to pursue
    bool               persistent = false;       ///< When true, attempt to achieve the goal forever
    std::shared_ptr<aos::jack::Message> message; ///< Starting parameters for the goal to pursue

    friend void to_json(nlohmann::json& nlohmann_json_j, const Pursue& nlohmann_json_t);
    //friend void from_json(const nlohmann::json& nlohmann_json_j, Pursue& nlohmann_json_t);
};

enum DropMode
{
    DropMode_NORMAL, /// Drops intention and goal if they are not persistent
    DropMode_FORCE,  /// Drops intention and goal unconditionally
};

NLOHMANN_JSON_SERIALIZE_ENUM(DropMode, {
    {DropMode_NORMAL, "NORMAL"},
    {DropMode_FORCE, "FORCE"},
})

struct Drop : Event
{
    Drop() : Event(EventType_DROP) {}

    std::string toString() const;

    std::string goal;
    std::string goalId;
    DropMode    mode = {};

    JACK_NLOHMANN_DEFINE_DERIVED_TYPE_INTRUSIVE(Drop, Event, goal, goalId, mode)
};

enum DelegationStatus
{
    DelegationStatus_PENDING,
    DelegationStatus_FAILED,
    DelegationStatus_SUCCESS,
};

NLOHMANN_JSON_SERIALIZE_ENUM(DelegationStatus, {
    {DelegationStatus_PENDING, "PENDING"},
    {DelegationStatus_FAILED, "FAILED"},
    {DelegationStatus_SUCCESS, "SUCCESS"},
})

struct Delegation : Event
{
    Delegation() : Event(EventType_DELEGATION) {}

    std::string toString() const;

    DelegationStatus   status;
    std::string        goal;           ///< Name of the goal being delegated
    std::string        goalId;         ///< UUID of the goal being delegated
    std::shared_ptr<aos::jack::Message> message;
    bool               analyse;        ///< True if this goal is an auction and a cost (score) should be calculated
    float              score;          ///< When 'analyse' is true, the cost of the delegation, lowest number is most preferred
    std::string        team;           ///< Name of the team that requested the delegation
    std::string        teamId;         ///< UUID of the team that requested the delegation

    friend void to_json(nlohmann::json& nlohmann_json_j, const Delegation& nlohmann_json_t);
    friend void from_json(const nlohmann::json& nlohmann_json_j, Delegation& nlohmann_json_t);
};

/******************************************************************************
 * Protocol
 ******************************************************************************/
struct Register : Event
{
    Register() : Event(EventType_REGISTER)  {}

    std::string toString() const;

    bool        proxy = true;  /// (Optional) The BDI entity should be instantiated as a proxy. Valid if address type is SERVICE/TEAM/AGENT.
    BusAddress  address;       /// The address of the BDI entity to register
    std::string templateType;  /// (Optional) The BDI template to register. Valid if address type is SERVICE/TEAM/AGENT.
    bool        start = false; /// (Optional) The entity should be "started" if it was created through this event. Valid if address type is SERVICE/TEAM/AGENT.
    BusAddress  team;          /// (Optional) The starting team the agent should be a member of if it was created through this event. Valid if address type is TEAM/AGENT.

    JACK_NLOHMANN_DEFINE_DERIVED_TYPE_INTRUSIVE(Register, Event, proxy, address, templateType, start, team)
};

struct Deregister : Event
{
    Deregister() : Event(EventType_DEREGISTER) {}

    std::string toString() const;

    std::string id;
    NodeType    nodeType = {};

    JACK_NLOHMANN_DEFINE_DERIVED_TYPE_INTRUSIVE(Deregister, Event, id, nodeType)
};

struct AgentJoinTeam : Event
{
    AgentJoinTeam() : Event(EventType_AGENT_JOIN_TEAM) {}

    std::string toString() const;

    BusAddress team;
    BusAddress agent;

    JACK_NLOHMANN_DEFINE_DERIVED_TYPE_INTRUSIVE(AgentJoinTeam, Event, team, agent)
};

struct AgentLeaveTeam : Event
{
    AgentLeaveTeam() : Event(EventType_AGENT_LEAVE_TEAM) {}

    std::string toString() const;

    BusAddress team;
    BusAddress agent;

    JACK_NLOHMANN_DEFINE_DERIVED_TYPE_INTRUSIVE(AgentLeaveTeam, Event, team, agent)
};

struct ActionBegin : Event
{
    ActionBegin() : Event(EventType_ACTION_BEGIN) {}

    std::string toString() const;

    std::string              name;          ///< The name of the action to begin
    std::string              taskId;        ///< The coroutine task ID that triggered the action
    std::string              goal;
    std::string              goalId;
    std::string              intentionId;
    std::string              plan;
    std::shared_ptr<aos::jack::Message>       message;      ///< The parameters to execute the action with
    std::vector<std::string> resourceLocks;

    friend void to_json(nlohmann::json& j, const ActionBegin& e);
};

enum ActionStatus
{
    ActionStatus_SUCCESS,
    ActionStatus_FEEDBACK,
    ActionStatus_FAILED,
};

NLOHMANN_JSON_SERIALIZE_ENUM(ActionStatus, {
    {ActionStatus_SUCCESS, "SUCCESS"},
    {ActionStatus_FEEDBACK, "FEEDBACK"},
    {ActionStatus_FAILED, "FAILED"},
})

struct ActionUpdate : Event
{
    ActionUpdate() : Event(EventType_ACTION_UPDATE) {}

    std::string toString() const;

    std::string        name;
    std::string        taskId;
    std::string        goal;
    std::string        goalId;
    std::string        intentionId;
    std::string        plan;
    ActionStatus       status = {};
    std::shared_ptr<aos::jack::Message> reply; ///< The return values as a result of updating the action

    friend void to_json(nlohmann::json& j, const ActionUpdate& e);
};

enum BDILogLevel
{
    BDILogLevel_NORMAL,
    BDILogLevel_IMPORTANT,
    BDILogLevel_CRITICAL,
};

NLOHMANN_JSON_SERIALIZE_ENUM(BDILogLevel, {
    {BDILogLevel_NORMAL, "NORMAL"},
    {BDILogLevel_IMPORTANT, "IMPORTANT"},
    {BDILogLevel_CRITICAL, "CRITICAL"},
})


enum BDILogType
{
    BDILogType_GOAL_STARTED,
    BDILogType_GOAL_FINISHED,
    BDILogType_SUB_GOAL_STARTED,
    BDILogType_SUB_GOAL_FINISHED,
    BDILogType_INTENTION_STARTED,
    BDILogType_INTENTION_FINISHED,
    BDILogType_ACTION_STARTED,
    BDILogType_ACTION_FINISHED,
    BDILogType_SLEEP_STARTED,
    BDILogType_SLEEP_FINISHED,
    BDILogType_CONDITION,
};

NLOHMANN_JSON_SERIALIZE_ENUM(BDILogType, {
    {BDILogType_GOAL_STARTED, "GOAL_STARTED"},
    {BDILogType_GOAL_FINISHED, "GOAL_FINISHED"},
    {BDILogType_SUB_GOAL_STARTED, "SUB_GOAL_STARTED"},
    {BDILogType_SUB_GOAL_FINISHED, "SUB_GOAL_FINISHED"},
    {BDILogType_INTENTION_STARTED, "INTENTION_STARTED"},
    {BDILogType_INTENTION_FINISHED, "INTENTION_FINISHED"},
    {BDILogType_ACTION_STARTED, "ACTION_STARTED"},
    {BDILogType_ACTION_FINISHED, "ACTION_FINISHED"},
    {BDILogType_SLEEP_STARTED, "SLEEP_STARTED"},
    {BDILogType_SLEEP_FINISHED, "SLEEP_FINISHED"},
    {BDILogType_CONDITION, "CONDITION"},
})

enum BDILogGoalIntentionResult
{
    BDILogGoalIntentionResult_FAILED,
    BDILogGoalIntentionResult_SUCCESS,
    BDILogGoalIntentionResult_DROPPED,
};

NLOHMANN_JSON_SERIALIZE_ENUM(BDILogGoalIntentionResult, {
    {BDILogGoalIntentionResult_FAILED, "FAILED"},
    {BDILogGoalIntentionResult_SUCCESS, "SUCCESS"},
    {BDILogGoalIntentionResult_DROPPED, "DROPPED"},
})

struct BDILog : Event
{
    BDILog() : Event(EventType_BDI_LOG) {}

    std::string toString() const;

    friend void to_json(nlohmann::json& j, const BDILog& e);
    friend void from_json(const nlohmann::json& j, BDILog& e);

    BDILogLevel level;
    BDILogType  logType;

    struct Goal
    {
        std::string toString() const;

        std::string               goal;
        std::string               goalId;
        std::string               intentionId; ///< Set when type is SUB_GOAL, ignored otherwise
        std::string               taskId;
        std::string               dropReason;  ///< When the goal is dropped, this is an optional string describing the reason
        BDILogGoalIntentionResult result;      ///< Set when type is set to FINISHED variant, ignore otherwise
    };

    struct Intention
    {
        std::string toString() const;

        std::string               goal;
        std::string               goalId;
        std::string               intentionId;
        std::string               plan;
        BDILogGoalIntentionResult result; ///< Set when type is set to FINISHED variant, ignore otherwise
    };

    struct Action
    {
        std::string toString() const;

        std::string goal;
        std::string goalId;
        std::string intentionId;
        std::string plan;
        std::string taskId;
        std::string action;
        std::string reasoning;
        bool        success;   ///< Set when type is set to FINISHED variant, ignore otherwise
    };

    struct Sleep
    {
        std::string toString() const;

        std::string goal;
        std::string goalId;
        std::string intentionId;
        std::string plan;
        std::string taskId;
        uint64_t    sleepMs; ///< Set when type is set to STARTED variant, ignore otherwise
    };

    struct Condition
    {
        std::string toString() const;

        std::string goal;
        std::string goalId;
        std::string intentionId;
        std::string plan;
        std::string taskId;
        std::string condition;
        bool        success;
    };

    Goal      goal      = {}; ///< Set when type is a GOAL or SUB_GOAL
    Intention intention = {}; ///< Set when type is INTENTION
    Action    action    = {}; ///< Set when type is ACTION
    Sleep     sleep     = {}; ///< Set when type is SLEEP
    Condition condition = {}; ///< Set when type is CONDITION
};

/******************************************************************************
 * Format Functions
 ******************************************************************************/
std::string_view format_as(NodeType value);
std::string_view format_as(EventType value);
std::string_view format_as(ControlCommand value);
std::string_view format_as(AnyType value);
std::string_view format_as(DropMode value);
std::string_view format_as(DelegationStatus value);
std::string_view format_as(ActionStatus value);
std::string_view format_as(BDILogLevel value);
std::string_view format_as(BDILogGoalIntentionResult value);

std::string format_as(const BusAddress& value);
std::string format_as(const Event& value);
std::string format_as(const Control& value);
std::string format_as(const Percept& value);
std::string format_as(const Pursue& value);
std::string format_as(const Drop& value);
std::string format_as(const Delegation& value);
std::string format_as(const Register& value);
std::string format_as(const AgentJoinTeam& value);
std::string format_as(const ActionBegin& value);
std::string format_as(const ActionUpdate& value);
std::string format_as(const BDILog& value);
std::string format_as(const BDILog::Goal& value);
std::string format_as(const BDILog::Intention& value);
std::string format_as(const BDILog::Action& value);
std::string format_as(const BDILog::Sleep& value);
std::string format_as(const BDILog::Condition& value);
std::string format_as(const BDILog::Goal& value);
std::string format_as(const BDILog::Intention& value);
std::string format_as(const BDILog::Action& value);
std::string format_as(const BDILog::Sleep& value);
std::string format_as(const BDILog::Condition& value);

/******************************************************************************
 * Enum Strings
 ******************************************************************************/
/// Convert a protocol node type to its string representation
std::string_view nodeTypeString(NodeType type);

/// Convert an protocol event type to its string representation
std::string_view eventTypeString(EventType type);

/// Convert a protocol command type to its string representation
std::string_view commandTypeString(ControlCommand command);

/// Convert a protocol any type to its string representation
std::string_view anyTypeString(AnyType type);

/// Convert a protocol any type to its string representation that can be use to
/// construct the type in subsystem that can create fields from strings. See
/// @aos::jack::FieldRegistry.
///
/// For example AnyType_U8, array=true  -> "U8[]"
///             AnyType_U8, array=false -> "U8"
std::string_view anyTypePropertyString(AnyType type, bool array);

/// Convert a protocol drop mode to its string representation
std::string_view dropModeString(DropMode drop);

/// Convert a delegation status to its string representation
std::string_view delegationStatusString(DelegationStatus value);

/// Convert an action status to its string representation
/// @return String representation or, "ACTION_STATUS_BAD_ENUM_VALUE" if invalid
std::string_view actionStatusString(ActionStatus type);

/// Convert a BDI log level to its string representation
std::string_view bdiLogLevelString(BDILogLevel value);

/// Convert a BDI log type to its string representation
std::string_view bdiLogTypeString(BDILogType value);

/// Convert a BDI log intention result to its string representation
std::string_view bdiLogGoalIntentionResultString(BDILogGoalIntentionResult value);

/******************************************************************************
 * Events
 ******************************************************************************/
/// Create a string representing the address of an entity that is participating
/// on the bus in the format described by the order of the enum values in the
/// BusAddressStringField
///
/// @param uuid The ID of the entity participating on the bus
/// @param type The type of entity participating on the bus
/// @param name The name of the entity participating on the bus
/// @return A string representing the address of the entity on the bus
std::string makeBusAddressStringFrom(std::string_view uuid, NodeType type, std::string_view name, bool compact);

/// Create a string representing the address of an entity that is participating
/// on the bus.
std::string makeBusAddressString(const BusAddress &address, bool compact);

/// Convert a bus address string to a bus address. This function does *not*
/// trust the input and will fail if an invalid amount of forward slashes are
/// found in the address.
/// @return The address which may be valid or invalid depending on if the passed
/// in address is well formed.
BusAddress makeBusAddressFromCString(const char *address, size_t addressSize);
BusAddress makeBusAddressFromString(std::string_view address);

/// Determine the node type from a string, the recognised values must
/// match the ones generated by the node type string function and is case
/// sensitive.
bool nodeTypeFromCString(const char *src, size_t srcSize, NodeType &type);
bool nodeTypeFromString(std::string_view src, NodeType &type);

struct BDILogHeader
{
    protocol::BusAddress  sender;
    protocol::BDILogLevel level;
    UniqueId              eventId;
};

/// Convert a protocol any (variant type) to a JACK message
/// @param[in] src The protocol variants to convert
/// @param[in] msg The message to store the variants into
/// @return False if the conversion of any variant field failed, @see
/// protocolAnyToJACKField, true otherwise.
BDILogGoalIntentionResult finishStateToBDILogGoalIntentionResult(FinishState finishState);
}  // namespace aos::jack::protocol

/******************************************************************************
 * Standard Library Interop
 ******************************************************************************/
namespace std {
template <>
struct hash<aos::jack::protocol::BusAddress>
{
    std::size_t operator()(const aos::jack::protocol::BusAddress &address) const
    {
        return std::hash<std::string>()(address.id);
    }
};
}  // namespace std
