#if !defined(JACK_DDS_ADAPTER_NAME)
    #error JACK_DDS_ADAPTER_NAME must be defined to generate the adapter
#endif

#if !defined(JACK_DDS_ADAPTER_INLINE_RTI_CODE) && !defined(JACK_DDS_ADAPTER_INLINE_CYCLONE_CODE)
    #error Inline code #define must be defined to enable implementation work-arounds
#endif

/// Third Party
#include <exception>
#include <string_view>

using namespace std::literals; /// operator ""sv

namespace aos
{
#if defined(JACK_DDS_ADAPTER_INLINE_RTI_CODE)
#define DDS_ENUM_AS_U32(value) (value).underlying()
#define DDS_ENUM_TYPE(enumStruct) enumStruct##_def
#define DDS_SAMPLE_TO_OSTREAM(sample) sample
#define DDS_EVENT_TO_OSTREAM(event) event
#elif defined(JACK_DDS_ADAPTER_INLINE_CYCLONE_CODE)
#define DDS_ENUM_AS_U32(value) (value)
#define DDS_ENUM_TYPE(enumStruct) enumStruct
#define DDS_SAMPLE_TO_OSTREAM(event) sample.eventId()
#define DDS_EVENT_TO_OSTREAM(event) "(none)"sv
#else
    #error Unknown DDS implementation encountered
#endif

static bool convertToProtocolDelegationStatus(jack::ddsevents::DelegationStatus src,
                                              jack::protocol::DelegationStatus &dest)
{
    switch (DDS_ENUM_AS_U32(src)) {
        #if defined(JACK_DDS_ADAPTER_INLINE_RTI_CODE)
        case jack::ddsevents::DelegationStatus::type::DelegationStatus_SUCCESS: { dest = jack::protocol::DelegationStatus_SUCCESS; } break;
        case jack::ddsevents::DelegationStatus::type::DelegationStatus_FAILED:  { dest = jack::protocol::DelegationStatus_FAILED; } break;
        case jack::ddsevents::DelegationStatus::type::DelegationStatus_PENDING: { dest = jack::protocol::DelegationStatus_PENDING; } break;
        #elif defined(JACK_DDS_ADAPTER_INLINE_CYCLONE_CODE)
        case jack::ddsevents::DelegationStatus::DelegationStatus_SUCCESS:       { dest = jack::protocol::DelegationStatus_SUCCESS; } break;
        case jack::ddsevents::DelegationStatus::DelegationStatus_FAILED:        { dest = jack::protocol::DelegationStatus_FAILED; } break;
        case jack::ddsevents::DelegationStatus::DelegationStatus_PENDING:       { dest = jack::protocol::DelegationStatus_PENDING; } break;
        #else
        default:  { JACK_PROTOCOL_ASSERT(!"Malformed DelegationStatus given"); return false; }
        #endif
    }

    return true;
}

static bool convertToDDSDelegationStatus(jack::protocol::DelegationStatus src, jack::ddsevents::DelegationStatus &dest)
{
    switch (src) {
        #if defined(JACK_DDS_ADAPTER_INLINE_RTI_CODE)
        case jack::protocol::DelegationStatus_FAILED:  { dest = jack::ddsevents::DelegationStatus(jack::ddsevents::DelegationStatus::type::DelegationStatus_FAILED); } break;
        case jack::protocol::DelegationStatus_SUCCESS: { dest = jack::ddsevents::DelegationStatus(jack::ddsevents::DelegationStatus::type::DelegationStatus_SUCCESS); } break;
        case jack::protocol::DelegationStatus_PENDING: { dest = jack::ddsevents::DelegationStatus(jack::ddsevents::DelegationStatus::type::DelegationStatus_PENDING); } break;
        #elif defined(JACK_DDS_ADAPTER_INLINE_CYCLONE_CODE)
        case jack::protocol::DelegationStatus_FAILED:  { dest = jack::ddsevents::DelegationStatus(jack::ddsevents::DelegationStatus::DelegationStatus_FAILED); } break;
        case jack::protocol::DelegationStatus_SUCCESS: { dest = jack::ddsevents::DelegationStatus(jack::ddsevents::DelegationStatus::DelegationStatus_SUCCESS); } break;
        case jack::protocol::DelegationStatus_PENDING: { dest = jack::ddsevents::DelegationStatus(jack::ddsevents::DelegationStatus::DelegationStatus_PENDING); } break;
        #else
            #error Unknown DDS implementation encountered
        #endif
        default: { JACK_PROTOCOL_ASSERT(!"Malformed DelegationStatus given"); return false; }
    }
    return true;
}

static bool convertToProtocolActionStatus(jack::ddsevents::ActionStatus src, jack::protocol::ActionStatus &dest)
{
    switch (DDS_ENUM_AS_U32(src)) {
        #if defined(JACK_DDS_ADAPTER_INLINE_RTI_CODE)
        case jack::ddsevents::ActionStatus::type::ActionStatus_SUCCESS: { dest = jack::protocol::ActionStatus_SUCCESS; } break;
        case jack::ddsevents::ActionStatus::type::ActionStatus_FAILED:  { dest = jack::protocol::ActionStatus_FAILED; } break;
        #elif defined(JACK_DDS_ADAPTER_INLINE_CYCLONE_CODE)
        case jack::ddsevents::ActionStatus::ActionStatus_SUCCESS:       { dest = jack::protocol::ActionStatus_SUCCESS; } break;
        case jack::ddsevents::ActionStatus::ActionStatus_FAILED:        { dest = jack::protocol::ActionStatus_FAILED; } break;
        #else
        default:  { JACK_PROTOCOL_ASSERT(!"Malformed ActionStatus given"); return false; }
        #endif
    }

    return true;
}

static bool convertToDDSActionStatus(jack::protocol::ActionStatus src, jack::ddsevents::ActionStatus &dest)
{
    switch (src) {
        #if defined(JACK_DDS_ADAPTER_INLINE_RTI_CODE)
        case jack::protocol::ActionStatus_FAILED:  { dest = jack::ddsevents::ActionStatus(jack::ddsevents::ActionStatus::type::ActionStatus_FAILED); } break;
        case jack::protocol::ActionStatus_SUCCESS: { dest = jack::ddsevents::ActionStatus(jack::ddsevents::ActionStatus::type::ActionStatus_SUCCESS); } break;
        #elif defined(JACK_DDS_ADAPTER_INLINE_CYCLONE_CODE)
        case jack::protocol::ActionStatus_FAILED:  { dest = jack::ddsevents::ActionStatus(jack::ddsevents::ActionStatus::ActionStatus_FAILED); } break;
        case jack::protocol::ActionStatus_SUCCESS: { dest = jack::ddsevents::ActionStatus(jack::ddsevents::ActionStatus::ActionStatus_SUCCESS); } break;
        #else
            #error Unknown DDS implementation encountered
        #endif
        default: { JACK_PROTOCOL_ASSERT(!"Malformed ActionStatus given"); return false; }
    }
    return true;
}

static bool convertToDDSNodeType(jack::protocol::NodeType src, jack::ddsevents::NodeType &dest)
{
    switch (src) {
        case jack::protocol::NodeType_GENERIC: { dest = jack::ddsevents::NodeType(jack::ddsevents::NodeType::NodeType_GENERIC); } break;
        case jack::protocol::NodeType_NODE:    { dest = jack::ddsevents::NodeType(jack::ddsevents::NodeType::NodeType_NODE); } break;
        case jack::protocol::NodeType_SERVICE: { dest = jack::ddsevents::NodeType(jack::ddsevents::NodeType::NodeType_SERVICE); } break;
        case jack::protocol::NodeType_AGENT:   { dest = jack::ddsevents::NodeType(jack::ddsevents::NodeType::NodeType_AGENT); } break;
        case jack::protocol::NodeType_TEAM:    { dest = jack::ddsevents::NodeType(jack::ddsevents::NodeType::NodeType_TEAM); } break;
        default: {
            JACK_PROTOCOL_ASSERT(!"Malformed node type given");
            return false;
        }
    }

    return true;
}

static bool convertToDDSBDILogLevel(jack::protocol::BDILogLevel src, jack::ddsevents::BDILogLevel &dest)
{
    switch (src) {
        #if defined(JACK_DDS_ADAPTER_INLINE_RTI_CODE)
        case jack::protocol::BDILogLevel_NORMAL:    { dest = jack::ddsevents::BDILogLevel(jack::ddsevents::BDILogLevel::type::BDILogLevel_NORMAL); } break;
        case jack::protocol::BDILogLevel_IMPORTANT: { dest = jack::ddsevents::BDILogLevel(jack::ddsevents::BDILogLevel::type::BDILogLevel_IMPORTANT); } break;
        case jack::protocol::BDILogLevel_CRITICAL:  { dest = jack::ddsevents::BDILogLevel(jack::ddsevents::BDILogLevel::type::BDILogLevel_CRITICAL); } break;
        #elif defined(JACK_DDS_ADAPTER_INLINE_CYCLONE_CODE)
        case jack::protocol::BDILogLevel_NORMAL:    { dest = jack::ddsevents::BDILogLevel(jack::ddsevents::BDILogLevel::BDILogLevel_NORMAL); } break;
        case jack::protocol::BDILogLevel_IMPORTANT: { dest = jack::ddsevents::BDILogLevel(jack::ddsevents::BDILogLevel::BDILogLevel_IMPORTANT); } break;
        case jack::protocol::BDILogLevel_CRITICAL:  { dest = jack::ddsevents::BDILogLevel(jack::ddsevents::BDILogLevel::BDILogLevel_CRITICAL); } break;
        #else
            #error Unknown DDS implementation encountered
        #endif
        default: { JACK_PROTOCOL_ASSERT(!"Malformed value given"); return false; }
    }
    return true;
}

static bool convertToDDSBDILogType(jack::protocol::BDILogType src, jack::ddsevents::BDILogType &dest)
{
    switch (src) {

        #if defined(JACK_DDS_ADAPTER_INLINE_RTI_CODE)
        case jack::protocol::BDILogType_GOAL_STARTED:       { dest = jack::ddsevents::BDILogType(jack::ddsevents::BDILogType::type::BDILogType_GOAL_STARTED); } break;
        case jack::protocol::BDILogType_GOAL_FINISHED:      { dest = jack::ddsevents::BDILogType(jack::ddsevents::BDILogType::type::BDILogType_GOAL_FINISHED); } break;
        case jack::protocol::BDILogType_SUB_GOAL_STARTED:   { dest = jack::ddsevents::BDILogType(jack::ddsevents::BDILogType::type::BDILogType_SUB_GOAL_STARTED); } break;
        case jack::protocol::BDILogType_SUB_GOAL_FINISHED:  { dest = jack::ddsevents::BDILogType(jack::ddsevents::BDILogType::type::BDILogType_SUB_GOAL_FINISHED); } break;
        case jack::protocol::BDILogType_INTENTION_STARTED:  { dest = jack::ddsevents::BDILogType(jack::ddsevents::BDILogType::type::BDILogType_INTENTION_STARTED); } break;
        case jack::protocol::BDILogType_INTENTION_FINISHED: { dest = jack::ddsevents::BDILogType(jack::ddsevents::BDILogType::type::BDILogType_INTENTION_FINISHED); } break;
        case jack::protocol::BDILogType_ACTION_STARTED:     { dest = jack::ddsevents::BDILogType(jack::ddsevents::BDILogType::type::BDILogType_ACTION_STARTED); } break;
        case jack::protocol::BDILogType_ACTION_FINISHED:    { dest = jack::ddsevents::BDILogType(jack::ddsevents::BDILogType::type::BDILogType_ACTION_FINISHED); } break;
        case jack::protocol::BDILogType_SLEEP_STARTED:      { dest = jack::ddsevents::BDILogType(jack::ddsevents::BDILogType::type::BDILogType_SLEEP_STARTED); } break;
        case jack::protocol::BDILogType_SLEEP_FINISHED:     { dest = jack::ddsevents::BDILogType(jack::ddsevents::BDILogType::type::BDILogType_SLEEP_FINISHED); } break;
        case jack::protocol::BDILogType_CONDITION:          { dest = jack::ddsevents::BDILogType(jack::ddsevents::BDILogType::type::BDILogType_CONDITION); } break;
        #elif defined(JACK_DDS_ADAPTER_INLINE_CYCLONE_CODE)
        case jack::protocol::BDILogType_GOAL_STARTED:       { dest = jack::ddsevents::BDILogType(jack::ddsevents::BDILogType::BDILogType_GOAL_STARTED); } break;
        case jack::protocol::BDILogType_GOAL_FINISHED:      { dest = jack::ddsevents::BDILogType(jack::ddsevents::BDILogType::BDILogType_GOAL_FINISHED); } break;
        case jack::protocol::BDILogType_SUB_GOAL_STARTED:   { dest = jack::ddsevents::BDILogType(jack::ddsevents::BDILogType::BDILogType_SUB_GOAL_STARTED); } break;
        case jack::protocol::BDILogType_SUB_GOAL_FINISHED:  { dest = jack::ddsevents::BDILogType(jack::ddsevents::BDILogType::BDILogType_SUB_GOAL_FINISHED); } break;
        case jack::protocol::BDILogType_INTENTION_STARTED:  { dest = jack::ddsevents::BDILogType(jack::ddsevents::BDILogType::BDILogType_INTENTION_STARTED); } break;
        case jack::protocol::BDILogType_INTENTION_FINISHED: { dest = jack::ddsevents::BDILogType(jack::ddsevents::BDILogType::BDILogType_INTENTION_FINISHED); } break;
        case jack::protocol::BDILogType_ACTION_STARTED:     { dest = jack::ddsevents::BDILogType(jack::ddsevents::BDILogType::BDILogType_ACTION_STARTED); } break;
        case jack::protocol::BDILogType_ACTION_FINISHED:    { dest = jack::ddsevents::BDILogType(jack::ddsevents::BDILogType::BDILogType_ACTION_FINISHED); } break;
        case jack::protocol::BDILogType_SLEEP_STARTED:      { dest = jack::ddsevents::BDILogType(jack::ddsevents::BDILogType::BDILogType_SLEEP_STARTED); } break;
        case jack::protocol::BDILogType_SLEEP_FINISHED:     { dest = jack::ddsevents::BDILogType(jack::ddsevents::BDILogType::BDILogType_SLEEP_FINISHED); } break;
        case jack::protocol::BDILogType_CONDITION:          { dest = jack::ddsevents::BDILogType(jack::ddsevents::BDILogType::BDILogType_CONDITION); } break;
        #else
            #error Unknown DDS implementation encountered
        #endif

        default: {
            JACK_PROTOCOL_ASSERT(!"Malformed value given");
            return false;
        }
    }
    return true;
}

static bool convertToDDSBDILogGoalIntentionResult(jack::protocol::BDILogGoalIntentionResult src, jack::ddsevents::BDILogGoalIntentionResult &dest)
{
    switch (src) {
        #if defined(JACK_DDS_ADAPTER_INLINE_RTI_CODE)
        case jack::protocol::BDILogGoalIntentionResult_FAILED:  { dest = jack::ddsevents::BDILogGoalIntentionResult(jack::ddsevents::BDILogGoalIntentionResult::type::BDILogGoalIntentionResult_FAILED); } break;
        case jack::protocol::BDILogGoalIntentionResult_SUCCESS: { dest = jack::ddsevents::BDILogGoalIntentionResult(jack::ddsevents::BDILogGoalIntentionResult::type::BDILogGoalIntentionResult_SUCCESS); } break;
        case jack::protocol::BDILogGoalIntentionResult_DROPPED: { dest = jack::ddsevents::BDILogGoalIntentionResult(jack::ddsevents::BDILogGoalIntentionResult::type::BDILogGoalIntentionResult_DROPPED); } break;
        #elif defined(JACK_DDS_ADAPTER_INLINE_CYCLONE_CODE)
        case jack::protocol::BDILogGoalIntentionResult_FAILED:  { dest = jack::ddsevents::BDILogGoalIntentionResult::BDILogGoalIntentionResult_FAILED; } break;
        case jack::protocol::BDILogGoalIntentionResult_SUCCESS: { dest = jack::ddsevents::BDILogGoalIntentionResult::BDILogGoalIntentionResult_SUCCESS; } break;
        case jack::protocol::BDILogGoalIntentionResult_DROPPED: { dest = jack::ddsevents::BDILogGoalIntentionResult::BDILogGoalIntentionResult_DROPPED; } break;
        #else
            #error Unknown DDS implementation encountered
        #endif
        default: { JACK_PROTOCOL_ASSERT(!"Malformed value given"); return false; }
    }
    return true;
}


static bool convertToProtocolBDILogLevel(jack::ddsevents::BDILogLevel src, jack::protocol::BDILogLevel &dest)
{
    switch (DDS_ENUM_AS_U32(src)) {
        case jack::ddsevents::BDILogLevel::BDILogLevel_NORMAL:    { dest = jack::protocol::BDILogLevel_NORMAL; } break;
        case jack::ddsevents::BDILogLevel::BDILogLevel_IMPORTANT: { dest = jack::protocol::BDILogLevel_IMPORTANT; } break;
        case jack::ddsevents::BDILogLevel::BDILogLevel_CRITICAL:  { dest = jack::protocol::BDILogLevel_CRITICAL; } break;
        default: {
            JACK_PROTOCOL_ASSERT(!"Malformed value given");
            return false;
        }
    }
    return true;
}

static bool convertToProtocolBDILogType(jack::ddsevents::BDILogType src, jack::protocol::BDILogType &dest)
{
    switch (DDS_ENUM_AS_U32(src)) {
        case jack::ddsevents::BDILogType::BDILogType_GOAL_STARTED:       { dest = jack::protocol::BDILogType_GOAL_STARTED; } break;
        case jack::ddsevents::BDILogType::BDILogType_GOAL_FINISHED:      { dest = jack::protocol::BDILogType_GOAL_FINISHED; } break;
        case jack::ddsevents::BDILogType::BDILogType_SUB_GOAL_STARTED:   { dest = jack::protocol::BDILogType_SUB_GOAL_STARTED; } break;
        case jack::ddsevents::BDILogType::BDILogType_SUB_GOAL_FINISHED:  { dest = jack::protocol::BDILogType_SUB_GOAL_FINISHED; } break;
        case jack::ddsevents::BDILogType::BDILogType_INTENTION_STARTED:  { dest = jack::protocol::BDILogType_INTENTION_STARTED; } break;
        case jack::ddsevents::BDILogType::BDILogType_INTENTION_FINISHED: { dest = jack::protocol::BDILogType_INTENTION_FINISHED; } break;
        case jack::ddsevents::BDILogType::BDILogType_ACTION_STARTED:     { dest = jack::protocol::BDILogType_ACTION_STARTED; } break;
        case jack::ddsevents::BDILogType::BDILogType_ACTION_FINISHED:    { dest = jack::protocol::BDILogType_ACTION_FINISHED; } break;
        case jack::ddsevents::BDILogType::BDILogType_SLEEP_STARTED:      { dest = jack::protocol::BDILogType_SLEEP_STARTED; } break;
        case jack::ddsevents::BDILogType::BDILogType_SLEEP_FINISHED:     { dest = jack::protocol::BDILogType_SLEEP_FINISHED; } break;
        case jack::ddsevents::BDILogType::BDILogType_CONDITION:          { dest = jack::protocol::BDILogType_CONDITION; } break;

        default: {
            JACK_PROTOCOL_ASSERT(!"Malformed value given");
            return false;
        }
    }
    return true;
}

static bool convertToProtocolBDILogGoalIntentionResult(jack::ddsevents::BDILogGoalIntentionResult src, jack::protocol::BDILogGoalIntentionResult &dest)
{
    switch (DDS_ENUM_AS_U32(src)) {
        case jack::ddsevents::BDILogGoalIntentionResult::BDILogGoalIntentionResult_FAILED:  { dest = jack::protocol::BDILogGoalIntentionResult_FAILED; } break;
        case jack::ddsevents::BDILogGoalIntentionResult::BDILogGoalIntentionResult_SUCCESS: { dest = jack::protocol::BDILogGoalIntentionResult_SUCCESS; } break;
        case jack::ddsevents::BDILogGoalIntentionResult::BDILogGoalIntentionResult_DROPPED: { dest = jack::protocol::BDILogGoalIntentionResult_DROPPED; } break;
        default: {
            JACK_PROTOCOL_ASSERT(!"Malformed value given");
            return false;
        }
    }
    return true;
}


static bool convertToProtocolNodeType(jack::ddsevents::NodeType src, jack::protocol::NodeType &dest)
{
    switch (DDS_ENUM_AS_U32(src)) {
        case jack::ddsevents::NodeType::NodeType_GENERIC: { dest = jack::protocol::NodeType_GENERIC; } break;
        case jack::ddsevents::NodeType::NodeType_NODE:    { dest = jack::protocol::NodeType_NODE; } break;
        case jack::ddsevents::NodeType::NodeType_SERVICE: { dest = jack::protocol::NodeType_SERVICE; } break;
        case jack::ddsevents::NodeType::NodeType_AGENT:   { dest = jack::protocol::NodeType_AGENT; } break;
        case jack::ddsevents::NodeType::NodeType_TEAM:    { dest = jack::protocol::NodeType_TEAM; } break;
        default: {
            JACK_PROTOCOL_ASSERT(!"Malformed node type given");
            return false;
        }
    }

    return true;
}

static bool convertToProtocolDropMode(jack::ddsevents::DropMode src,
                                      jack::protocol::DropMode &dest)
{
    switch (DDS_ENUM_AS_U32(src)) {
        #if defined(JACK_DDS_ADAPTER_INLINE_RTI_CODE)
        case jack::ddsevents::DropMode::type::DropMode_NORMAL: { dest = jack::protocol::DropMode_NORMAL; } break;
        case jack::ddsevents::DropMode::type::DropMode_FORCE:  { dest = jack::protocol::DropMode_FORCE; } break;
        #elif defined(JACK_DDS_ADAPTER_INLINE_CYCLONE_CODE)
        case jack::ddsevents::DropMode::DropMode_NORMAL:       { dest = jack::protocol::DropMode_NORMAL; } break;
        case jack::ddsevents::DropMode::DropMode_FORCE:        { dest = jack::protocol::DropMode_FORCE; } break;
        #else
        default:  { JACK_PROTOCOL_ASSERT(!"Malformed DropMode given"); return false; }
        #endif
    }

    return true;
}

static bool convertToDDSDropMode(jack::protocol::DropMode src, jack::ddsevents::DropMode &dest)
{
    switch (src) {
        #if defined(JACK_DDS_ADAPTER_INLINE_RTI_CODE)
        case jack::protocol::DropMode_NORMAL: { dest = jack::ddsevents::DropMode(jack::ddsevents::DropMode::type::DropMode_NORMAL); } break;
        case jack::protocol::DropMode_FORCE:  { dest = jack::ddsevents::DropMode(jack::ddsevents::DropMode::type::DropMode_FORCE); } break;
        #elif defined(JACK_DDS_ADAPTER_INLINE_CYCLONE_CODE)
        case jack::protocol::DropMode_NORMAL: { dest = jack::ddsevents::DropMode::DropMode_NORMAL; } break;
        case jack::protocol::DropMode_FORCE:  { dest = jack::ddsevents::DropMode::DropMode_FORCE;  } break;
        #else
            #error Unknown DDS implementation encountered
        #endif
        default: { JACK_PROTOCOL_ASSERT(!"Malformed value given"); return false; }
    }
    return true;
}

static bool convertToProtocolBusAddress(const jack::ddsevents::BusAddress &src, jack::protocol::BusAddress &dest)
{
    jack::protocol::NodeType type = {};
    if (convertToProtocolNodeType(src.type(), type)) {
        dest = jack::protocol::BusAddress(type, src.id(), src.name());
        return true;
    }

    return false;
}

static bool convertToDDSBusAddress(const jack::protocol::BusAddress& src, jack::ddsevents::BusAddress& dest)
{
    jack::ddsevents::NodeType type = {};
    if (convertToDDSNodeType(src.type, type)) {
        dest = jack::ddsevents::BusAddress(type, src.id, src.name);
        return true;
    }

    return false;
}

#if !defined(JACK_BUS_SEND_JSON)
static bool convertToDDSField(const jack::Field& src, jack::ddsevents::Field& dest)
{
    using namespace jack;
    dest.name(src.m_name);

    const std::any& srcValue     = src.m_value;
    ddsevents::Any& destValue    = dest.value();
    ddsevents::AnyData& destData = destValue.data();
    try {
        if (srcValue.type() == typeid(int8_t)) {
            destValue.type(ddsevents::AnyType(ddsevents::AnyType::AnyType_I8));
            destData.i8Val({std::any_cast<int8_t>(src.m_value)});
        } else if (srcValue.type() == typeid(int16_t)) {
            destValue.type(ddsevents::AnyType(ddsevents::AnyType::AnyType_I16));
            destData.i16Val({std::any_cast<int16_t>(src.m_value)});
        } else if (srcValue.type() == typeid(int32_t)) {
            destValue.type(ddsevents::AnyType(ddsevents::AnyType::AnyType_I32));
            destData.i32Val({std::any_cast<int32_t>(src.m_value)});
        } else if (srcValue.type() == typeid(int64_t)) {
            destValue.type(ddsevents::AnyType(ddsevents::AnyType::AnyType_I64));
            destData.i64Val({std::any_cast<int64_t>(src.m_value)});
        } else if (srcValue.type() == typeid(uint8_t)) {
            destValue.type(ddsevents::AnyType(ddsevents::AnyType::AnyType_U8));
            destData.u8Val({std::any_cast<uint8_t>(src.m_value)});
        } else if (srcValue.type() == typeid(uint16_t)) {
            destValue.type(ddsevents::AnyType(ddsevents::AnyType::AnyType_U16));
            destData.u16Val({std::any_cast<uint16_t>(src.m_value)});
        } else if (srcValue.type() == typeid(uint32_t)) {
            destValue.type(ddsevents::AnyType(ddsevents::AnyType::AnyType_U32));
            destData.u32Val({std::any_cast<uint32_t>(src.m_value)});
        } else if (srcValue.type() == typeid(uint64_t)) {
            destValue.type(ddsevents::AnyType(ddsevents::AnyType::AnyType_U64));
            destData.u64Val({std::any_cast<uint64_t>(src.m_value)});
        } else if (srcValue.type() == typeid(float)) {
            destValue.type(ddsevents::AnyType(ddsevents::AnyType::AnyType_F32));
            destData.f32Val({std::any_cast<float>(src.m_value)});
        } else if (srcValue.type() == typeid(double)) {
            destValue.type(ddsevents::AnyType(ddsevents::AnyType::AnyType_F64));
            destData.f64Val({std::any_cast<double>(src.m_value)});
        } else if (srcValue.type() == typeid(aos::jack::V2)) {
            destValue.type(ddsevents::AnyType(ddsevents::AnyType::AnyType_V2));
            auto v2 = std::any_cast<aos::jack::V2>(src.m_value);

            aos::jack::ddsevents::V2 ddsV2 = {};
            ddsV2.push_back(v2.m_x);
            ddsV2.push_back(v2.m_y);
            destData.v2Val({ddsV2});

        } else if (srcValue.type() == typeid(bool)) {
            destValue.type(ddsevents::AnyType(ddsevents::AnyType::AnyType_Bool));
            destData.b8Val({std::any_cast<bool>(src.m_value)});
        } else if (srcValue.type() == typeid(std::string)) {
            destValue.type(ddsevents::AnyType(ddsevents::AnyType::AnyType_String));
            destData.stringVal({std::any_cast<std::string>(src.m_value)});
        } else if (srcValue.type() == typeid(jack::Message)) {
            destValue.type(ddsevents::AnyType(ddsevents::AnyType::AnyType_Message));

            const jack::Message* srcMsg = std::any_cast<jack::Message>(&src.m_value);
            std::vector<ddsevents::Message> destMsgArray;

            jack::ddsevents::Message destMsg;
            destMsg.schema() = srcMsg->m_schemaName;

            jack::ddsevents::FieldArray& fieldArray = destMsg.fields();
            for (const auto& srcField : srcMsg->fields()) {
                jack::ddsevents::Field destField;
                bool converted = convertToDDSField(srcField.second, destField);
                JACK_PROTOCOL_ASSERT(converted);
                fieldArray.push_back(destField);
            }
            destMsgArray.push_back(destMsg);
            destData.messageVal(std::move(destMsgArray));

        } else if (srcValue.type() == typeid(std::vector<jack::Message>)) {
            destValue.type(ddsevents::AnyType(ddsevents::AnyType::AnyType_Message));

            const std::vector<jack::Message>* srcMsgArray = std::any_cast<std::vector<jack::Message>>(&src.m_value);
            std::vector<ddsevents::Message> destMsgArray;

            for (const auto& srcMsg : *srcMsgArray) {
                jack::ddsevents::Message destMsg;
                destMsg.schema() = srcMsg.m_schemaName;
                jack::ddsevents::FieldArray& fieldArray = destMsg.fields();
                for (const auto& srcField : srcMsg.fields()) {
                    jack::ddsevents::Field destField;
                    bool converted = convertToDDSField(srcField.second, destField);
                    JACK_PROTOCOL_ASSERT(converted);
                    fieldArray.push_back(destField);
                }
                destMsgArray.push_back(destMsg);
            }
            destData.messageVal(std::move(destMsgArray));
        } else {
            JACK_ERROR("Internal error: we don't support converting this field's data type [field=" << src << "]");
            assert(!"Internal error: invalid code path");
        }
    } catch (const std::bad_any_cast& e) {
        JACK_WARNING("Failed to convert the field to a protocol field [field=" << src << ", reason=\"" << e.what() << "\"]");
        assert(!"Internal error: invalid code path");
        return false;
    }

    return true;
}

#if defined(JACK_DDS_ADAPTER_INLINE_RTI_CODE)
template <typename T>
using DDSArray32 = rti::core::bounded_sequence<T, 32>;
#elif defined(JACK_DDS_ADAPTER_INLINE_CYCLONE_CODE)
template <typename T>
using DDSArray32 = std::vector<T>;
#else
#error Unknown DDS implementation
#endif

#if defined(JACK_DDS_ADAPTER_INLINE_RTI_CODE)
using DDSVector2 = rti::core::bounded_sequence<float, 2>;
#elif defined(JACK_DDS_ADAPTER_INLINE_CYCLONE_CODE)
using DDSVector2 = std::vector<float>;
#else
#error Unknown DDS implementation
#endif


static bool convertToDDSParameters(const std::vector<jack::Field>& src, DDSArray32<jack::ddsevents::Field>& dest)
{
    assert(src.size() < dest.max_size());
    dest.reserve(src.size());
    for (const jack::Field& field : src) {
        jack::ddsevents::Field ddsField;
        if (!convertToDDSField(field, ddsField)) {
            return false;
        }
        dest.push_back(ddsField);
    }
    return true;
}

static bool convertToProtocolField(const jack::ddsevents::Field& src, jack::Field& dest)
{
    /// \todo The src data pointer returned is in little endian format. On
    /// big endian machines we will need to do a byteswap.

    const jack::ddsevents::AnyData &srcData = src.value().data();
    dest.m_name                             = src.name();

    switch (DDS_ENUM_AS_U32(src.value().type())) {
        case jack::ddsevents::AnyType::AnyType_I8: {
            const auto& array  = srcData.i8Val();
            bool receivedArray = array.size() > 1 || array.empty();
            dest.m_type        = jack::protocol::anyTypePropertyString(jack::protocol::AnyType_I8, receivedArray);
            if (receivedArray) {
                dest.m_value = array;
            } else {
                dest.m_value = array[0];
            }
        } break;

        case jack::ddsevents::AnyType::AnyType_I16: {
            const auto& array  = srcData.i16Val();
            bool receivedArray = array.size() > 1 || array.empty();
            dest.m_type        = jack::protocol::anyTypePropertyString(jack::protocol::AnyType_I16, receivedArray);
            if (receivedArray) {
                dest.m_value = array;
            } else {
                dest.m_value = array[0];
            }
        } break;

        case jack::ddsevents::AnyType::AnyType_I32: {
            const auto& array  = srcData.i32Val();
            bool receivedArray = array.size() > 1 || array.empty();
            dest.m_type        = jack::protocol::anyTypePropertyString(jack::protocol::AnyType_I32, receivedArray);
            if (receivedArray) {
                dest.m_value = array;
            } else {
                dest.m_value = array[0];
            }
        } break;

        case jack::ddsevents::AnyType::AnyType_I64: {
            const auto& array  = srcData.i64Val();
            bool receivedArray = array.size() > 1 || array.empty();
            dest.m_type  = jack::protocol::anyTypePropertyString(jack::protocol::AnyType_I64, receivedArray);
            if (receivedArray) {
                dest.m_value = array;
            } else {
                dest.m_value = array[0];
            }
        } break;

        case jack::ddsevents::AnyType::AnyType_U8: {
            const auto& array  = srcData.u8Val();
            bool receivedArray = array.size() > 1 || array.empty();
            dest.m_type        = jack::protocol::anyTypePropertyString(jack::protocol::AnyType_U8, receivedArray);
            if (receivedArray) {
                dest.m_value = array;
            } else {
                dest.m_value = array[0];
            }
        } break;

        case jack::ddsevents::AnyType::AnyType_U16: {
            const auto& array  = srcData.u16Val();
            bool receivedArray = array.size() > 1 || array.empty();
            dest.m_type        = jack::protocol::anyTypePropertyString(jack::protocol::AnyType_U16, receivedArray);
            if (receivedArray) {
                dest.m_value = array;
            } else {
                dest.m_value = array[0];
            }
        } break;

        case jack::ddsevents::AnyType::AnyType_U32: {
            const auto& array  = srcData.u32Val();
            bool receivedArray = array.size() > 1 || array.empty();
            dest.m_type        = jack::protocol::anyTypePropertyString(jack::protocol::AnyType_U32, receivedArray);
            if (receivedArray) {
                dest.m_value = array;
            } else {
                dest.m_value = array[0];
            }
        } break;

        case jack::ddsevents::AnyType::AnyType_U64: {
            const auto& array  = srcData.u64Val();
            bool receivedArray = array.size() > 1 || array.empty();
            dest.m_type        = jack::protocol::anyTypePropertyString(jack::protocol::AnyType_U64, receivedArray);
            if (receivedArray) {
                dest.m_value = array;
            } else {
                dest.m_value = array[0];
            }
        } break;

        case jack::ddsevents::AnyType::AnyType_F32: {
            const auto& array  = srcData.f32Val();
            bool receivedArray = array.size() > 1 || array.empty();
            dest.m_type        = jack::protocol::anyTypePropertyString(jack::protocol::AnyType_F32, receivedArray);
            if (receivedArray) {
                dest.m_value = array;
            } else {
                dest.m_value = array[0];
            }
        } break;

        case jack::ddsevents::AnyType::AnyType_F64: {
            const auto& array  = srcData.f64Val();
            bool receivedArray = array.size() > 1 || array.empty();
            dest.m_type        = jack::protocol::anyTypePropertyString(jack::protocol::AnyType_F64, receivedArray);
            if (receivedArray) {
                dest.m_value = array;
            } else {
                dest.m_value = array[0];
            }
        } break;

        case jack::ddsevents::AnyType::AnyType_V2: {
            const auto& array  = srcData.v2Val();
            bool receivedArray = array.size() > 1 || array.empty();
            dest.m_type        = jack::protocol::anyTypePropertyString(jack::protocol::AnyType_V2, receivedArray);
            if (array.size() > 1 || array.empty()) {
                std::vector<aos::jack::V2> destArray;
                destArray.reserve(array.size());
                for (DDSVector2 v2 : array) {
                    destArray.push_back(aos::jack::V2(v2[0], v2[1]));
                }

                dest.m_value = destArray;
            } else {
                DDSVector2 v2 = array[0];
                dest.m_value  = aos::jack::V2(v2[0], v2[1]);
            }
        } break;

        case jack::ddsevents::AnyType::AnyType_Bool: {
            const auto& array  = srcData.b8Val();
            bool receivedArray = array.size() > 1 || array.empty();
            dest.m_type        = jack::protocol::anyTypePropertyString(jack::protocol::AnyType_Bool, receivedArray);
            if (receivedArray) {
                dest.m_value = array;
            } else {
                dest.m_value = array[0];
            }
        } break;

        case jack::ddsevents::AnyType::AnyType_String: {
            const auto& array  = srcData.stringVal();
            bool receivedArray = array.size() > 1 || array.empty();
            dest.m_type        = jack::protocol::anyTypePropertyString(jack::protocol::AnyType_String, receivedArray);
            if (receivedArray) {
                dest.m_value = array;
            } else {
                dest.m_value = array[0];
            }
        } break;

        case jack::ddsevents::AnyType::AnyType_Message: {
            const auto& array  = srcData.messageVal();
            bool receivedArray = array.size() > 1 || array.empty();
            dest.m_type        = jack::protocol::anyTypePropertyString(jack::protocol::AnyType_Message, receivedArray);

            std::vector<aos::jack::Message> destMsgArray;
            destMsgArray.reserve(array.size());

            for (const jack::ddsevents::Message& srcMsg : array) {
                jack::Message destMsg;
                destMsg.m_schemaName = srcMsg.schema(); /// \todo Verify that all the message schemas are the same

                for (const jack::ddsevents::Field& srcField : srcMsg.fields()) {
                    jack::Field destField;
                    bool converted = convertToProtocolField(srcField, destField);
                    JACK_PROTOCOL_ASSERT(converted);
                    destMsg.setField(destField, aos::jack::Message::SetFlags_CREATE);
                }
                destMsgArray.push_back(destMsg);
            }

            if (receivedArray) {
                dest.m_value = destMsgArray;
            } else {
                dest.m_value = destMsgArray[0];
            }
        } break;

    }
    return true;
}

static bool convertToProtocolFields(const DDSArray32<jack::ddsevents::Field>& src, std::vector<jack::Field>& dest)
{
    dest.reserve(src.size());
    for (const jack::ddsevents::Field &field : src) {
        jack::Field eventField = {};
        if (!convertToProtocolField(field, eventField)) {
            return false;
        }

        dest.push_back(eventField);
    }
    return true;
}
#endif /// !defined(JACK_BUS_SEND_JSON)

const std::string *getEventTopicName(jack::protocol::EventType type)
{
    /// Build the topic table
    static std::string topics[jack::protocol::EventType_COUNT] = {};
    for (static bool init = false; !init; init = true) {
        for (int index = 0; index < jack::protocol::EventType_COUNT; index++) {
            std::string& topic = topics[index];
            topic += "jack/"sv;
            topic += jack::protocol::eventTypeString(static_cast<jack::protocol::EventType>(index));
        }
    }

    std::string *result = nullptr;
    if (type >= 0 && type < jack::protocol::EventType_COUNT) {
        result = topics + type;
    }
    return result;
}

JACK_DDS_ADAPTER_NAME::JACK_DDS_ADAPTER_NAME(int32_t domainId)
    : m_participant(domainId)
    , m_listener(*this)
{
    connect();
}

JACK_DDS_ADAPTER_NAME::~JACK_DDS_ADAPTER_NAME()
{
    disconnect();
}

void JACK_DDS_ADAPTER_NAME::poll()
{
    /// \note DDS runs on multiple threads by default and has callbacks for
    /// populating the new events into the queue.
}

void JACK_DDS_ADAPTER_NAME::connect()
{
    /// \todo Add generic topic add and remove. We've hardcoded the JACK topics
    for (int i = 0; i < jack::protocol::EventType_COUNT; i++){
        const std::string *topic = getEventTopicName(static_cast<jack::protocol::EventType>(i));
        if (m_stops.count(*topic)) {
            continue;
        }

        dds::pub::qos::DataWriterQos writerQos = {};
        dds::sub::qos::DataReaderQos readerQos = {};

        #if defined(JACK_DDS_ADAPTER_INLINE_RTI_CODE)
        /// \note DDS has an internal pool that they allocate samples (our
        /// events) from. In general DDS likes to have bounded arrays, in JACK
        /// we're a bit more generic than that as we have arbitrary messages and
        /// arrays of data in that message.
        ///
        /// Yes we could lock down these sizes, and that'd most likely be better
        /// but requires some design. Since DDS bounds the arrays it can
        /// pre-allocate a fixed chunk of memory, however note that JACK
        /// messages are potentially recursive data structures which causes the
        /// memory budget to blow out.
        ///
        /// In RTI DDS we can configure the following property on QoS to be more
        /// conservative on the buffer size allocated for the pool to avoid
        /// blowing out our memory budget and instead dynamically allocate
        /// messages as they arrive.
        ///
        /// Note that RTI DS does not expose these properties in the C++ API,
        /// it's available through XML and also the generic key-value strings in
        /// the C++ API.
        ///
        /// See: RTI_ConnextDDS_CoreLibraries_UsersManual.pdf 3.2.7.2 Unbounded
        /// Built-in Types pg 109.

        auto& writerProperty = writerQos.policy<rti::core::policy::Property>();
        writerProperty.set(rti::core::policy::Property::Entry("dds.data_writer.history.memory_manager.fast_pool.pool_buffer_max_size", "65536")); // 64k

        auto& readerProperty = readerQos.policy<rti::core::policy::Property>();
        readerProperty.set(rti::core::policy::Property::Entry("dds.data_reader.history.memory_manager.fast_pool.pool_buffer_max_size", "65536")); // 64k

        auto& readerResourceLimits = readerQos.policy<rti::core::policy::DataReaderResourceLimits>();
        readerResourceLimits.dynamically_allocate_fragmented_samples(true);
        #endif

        std::pair<BusStopMap::iterator, bool> result = m_stops.insert(std::make_pair(*topic, EventBusStop(m_participant, *topic, writerQos, readerQos)));
        if (result.second) {
            // Subscribe for incoming message events
            BusStopMap::iterator                          it     = result.first;
            dds::sub::DataReader<jack::ddsevents::Event>& reader = it->second.m_reader;
            reader.listener(&m_listener, dds::core::status::StatusMask::all());
        } else {
            AOS_DDS_ADAPTER_WARNING("Topic could not be subscribed to [topic=" << *topic << "]");
        }
    }
}

void JACK_DDS_ADAPTER_NAME::disconnect()
{
    for (auto iter = m_stops.begin(); iter != m_stops.end(); ++iter) {
        EventBusStop& stop = iter->second;
        stop.m_reader.listener(nullptr, dds::core::status::StatusMask::none());
    }
    m_stops.clear();
}

void JACK_DDS_ADAPTER_NAME::createTopic(const std::string &topic)
{

}

void JACK_DDS_ADAPTER_NAME::deleteTopic(const std::string &topic)
{

}

void JACK_DDS_ADAPTER_NAME::subscribeTopic(const std::string &topic)
{

}

void JACK_DDS_ADAPTER_NAME::unsubscribeTopic(const std::string &topic)
{

}

std::set<std::string> JACK_DDS_ADAPTER_NAME::subscriptions()
{
    return std::set<std::string>();
}

void JACK_DDS_ADAPTER_NAME::sendEvent(const jack::protocol::Event *event)
{
    if (event == nullptr) {
        AOS_DDS_ADAPTER_DEBUG("Null event given [event=" << *event << "]");
        return;
    }

    using namespace jack;
    ddsevents::Event ddsEvent;
    const std::string *topic = getEventTopicName(event->type);

    if (!topic) {
        AOS_DDS_ADAPTER_ERROR("Internal error: Topic does not exist for event [event=" << *event << "]");
        return;

    }

    BusStopMap::iterator iter = m_stops.find(*topic);
    if (iter == m_stops.end()) {
        return;
    }

    switch (event->type) {
        /**********************************************************************
         * CDDI
         **********************************************************************/
        case protocol::EventType_CONTROL: {
            const auto &jackEvent      = static_cast<const protocol::Control &>(*event);
            ddsevents::ControlCommand command = {};
            switch (jackEvent.command) {
                case protocol::ControlCommand_START: { command = ddsevents::ControlCommand::ControlCommand_START; } break;
                case protocol::ControlCommand_PAUSE: { command = ddsevents::ControlCommand::ControlCommand_PAUSE; } break;
                case protocol::ControlCommand_STOP:  { command = ddsevents::ControlCommand::ControlCommand_STOP; } break;
                default: { JACK_PROTOCOL_ASSERT(!"Invalid Code Path"); return; } break;
            }
            ddsEvent.body().control(ddsevents::Control(command));
        } break;

        case protocol::EventType_PERCEPT: {
            const auto &jackEvent = static_cast<const protocol::Percept &>(*event);
            #if defined(JACK_BUS_SEND_JSON)
            ddsEvent.body().percept(jack::ddsevents::Percept(jackEvent.beliefSet, jackEvent.fieldJSON));
            #else
            jack::ddsevents::Field field;
            if (!convertToDDSField(jackEvent.field, field)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert percept field to DDS parameters [event=" << jackEvent << "]");
                return; /// \todo Error code
            }
            ddsEvent.body().percept(jack::ddsevents::Percept(jackEvent.beliefSet, field);
            #endif

        } break;

        case protocol::EventType_MESSAGE: {
            const auto &jackEvent = static_cast<const protocol::Message &>(*event);
            ddsevents::Message eventBody;
            #if defined(JACK_BUS_SEND_JSON)
            eventBody = jackEvent.json;
            #else
            eventBody.schema() = jackEvent.schema;
            if (!convertToDDSParameters(jackEvent.fields, eventBody.fields())) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event fields to DDS parameters [event=" << jackEvent << "]");
                return; /// \todo Error code
            }
            #endif

            ddsEvent.body().message(std::move(eventBody));
        } break;

        case protocol::EventType_PURSUE: {
            const auto &jackEvent = static_cast<const protocol::Pursue &>(*event);
            ddsevents::Pursue eventBody;
            eventBody.goal()       = jackEvent.goal;
            eventBody.persistent() = jackEvent.persistent;
            #if defined(JACK_BUS_SEND_JSON)
            eventBody.parameters() = jackEvent.parametersJSON;
            #else
            if (!convertToDDSParameters(jackEvent.parameters, eventBody.parameters())) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event parameters to DDS parameters [event=" << jackEvent << "]");
                return; /// \todo Error code
            }
            #endif
            ddsEvent.body().pursue(std::move(eventBody));
        } break;

        case protocol::EventType_DROP: {
            const auto &jackEvent = static_cast<const protocol::Drop &>(*event);
            ddsevents::Drop eventBody;
            eventBody.goal()   = jackEvent.goal;
            eventBody.goalId() = jackEvent.goalId;
            if (!convertToDDSDropMode(jackEvent.mode, eventBody.mode())) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event drop mode to DDS drop mode");
                return; /// \todo Error code
            }
            ddsEvent.body().drop(std::move(eventBody));
        } break;

        case protocol::EventType_DELEGATION: {
            const auto &jackEvent = static_cast<const protocol::Delegation &>(*event);
            ddsevents::Delegation eventBody;

            ddsevents::DelegationStatus status = {};
            if (!convertToDDSDelegationStatus(jackEvent.status, status)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event delegation status to DDS delegation status [event=" << jackEvent << "]");
                return; /// \todo Error code
            }

            eventBody.status()  = status;
            eventBody.goal()    = jackEvent.goal;
            eventBody.goalId()  = jackEvent.goalId;
            eventBody.analyse() = jackEvent.analyse;
            eventBody.score()   = jackEvent.score;
            eventBody.team()    = jackEvent.team;
            eventBody.teamId()  = jackEvent.teamId;
            #if defined(JACK_BUS_SEND_JSON)
            eventBody.parameters() = jackEvent.parametersJSON;
            #else
            if (!convertToDDSParameters(jackEvent.parameters, eventBody.parameters())) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event parameters to DDS parameters [event=" << jackEvent << "]");
                return; /// \todo Error code
            }
            #endif

            ddsEvent.body().delegation(std::move(eventBody));
        } break;

        /**********************************************************************
         * Protocol
         **********************************************************************/
        case protocol::EventType_HEARTBEAT: {
            const auto &jackEvent = static_cast<const protocol::Heartbeat &>(*event);
            ddsEvent.body().heartbeat(ddsevents::Heartbeat(jackEvent.timestamp));
        } break;

        case protocol::EventType_REGISTER: {
            const auto &          jackEvent = static_cast<const protocol::Register &>(*event);
            ddsevents::BusAddress address   = {};
            if (!convertToDDSBusAddress(jackEvent.address, address)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event address type to DDS address [event=" << jackEvent << "]");
                return; /// \todo Error code
            }

            ddsevents::BusAddress team   = {};
            if (!convertToDDSBusAddress(jackEvent.team, team)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event bus address to DDS address [event=" << jackEvent << "]");
                return; /// \todo Error code
            }

            ddsEvent.body().reg(ddsevents::Register(jackEvent.proxy, address, jackEvent.templateType, jackEvent.start, team));
        } break;

        case protocol::EventType_DEREGISTER: {
            const auto &        jackEvent = static_cast<const protocol::Deregister &>(*event);
            ddsevents::NodeType nodeType      = {};
            if (!convertToDDSNodeType(jackEvent.nodeType, nodeType)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event node type to DDS node type [event=" << jackEvent << "]");
                return; /// \todo Error code
            }
            ddsEvent.body().dereg(ddsevents::Deregister(jackEvent.id, nodeType));
        } break;

        case protocol::EventType_AGENT_JOIN_TEAM: {
            const auto &          jackEvent = static_cast<const protocol::AgentJoinTeam &>(*event);
            ddsevents::BusAddress team = {}, agent = {};
            if (!convertToDDSBusAddress(jackEvent.team, team) ||
                !convertToDDSBusAddress(jackEvent.agent, agent)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event bus address to DDS address [event=" << jackEvent << "]");
                return; /// \todo Error code
            }
            ddsEvent.body().agentJoinTeam(ddsevents::AgentJoinTeam(team, agent));
        } break;

        case protocol::EventType_AGENT_LEAVE_TEAM: {
            const auto &          jackEvent = static_cast<const protocol::AgentLeaveTeam &>(*event);
            ddsevents::BusAddress team = {}, agent = {};
            if (!convertToDDSBusAddress(jackEvent.team, team) ||
                !convertToDDSBusAddress(jackEvent.agent, agent)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event bus address to DDS address [event=" << jackEvent << "]");
                return; /// \todo Error code
            }
            ddsEvent.body().agentLeaveTeam(ddsevents::AgentLeaveTeam(team, agent));
        } break;

        case protocol::EventType_ACTION_BEGIN: {
            const auto &jackEvent = static_cast<const protocol::ActionBegin &>(*event);
            ddsevents::ActionBegin eventBody;
            #if defined(JACK_BUS_SEND_JSON)
            eventBody.parameters() = jackEvent.parametersJSON;
            #else
            if (!convertToDDSParameters(jackEvent.parameters, eventBody.parameters())) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event parameters to DDS parameters [event=" << jackEvent << "]");
                return; /// \todo Error code
            }
            #endif

            eventBody.resourceLocks() = jackEvent.resourceLocks;
            eventBody.name()          = jackEvent.name;
            eventBody.taskId()        = jackEvent.taskId;
            eventBody.goal()          = jackEvent.goal;
            eventBody.goalId()        = jackEvent.goalId;
            eventBody.intentionId()   = jackEvent.intentionId;
            eventBody.plan()          = jackEvent.plan;
            ddsEvent.body().actionBegin(std::move(eventBody));
        } break;

        case protocol::EventType_ACTION_UPDATE: {
            const auto &jackEvent = static_cast<const protocol::ActionUpdate &>(*event);
            ddsevents::ActionStatus status = {};
            if (!convertToDDSActionStatus(jackEvent.status, status)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event status to DDS status [event=" << jackEvent << "]");
                return; /// \todo Error code
            }

            ddsevents::ActionUpdate eventBody = {};
            #if defined(JACK_BUS_SEND_JSON)
            eventBody.reply() = jackEvent.replyJSON;
            #else
            if (!convertToDDSParameters(jackEvent.reply, eventBody.reply())) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event reply to DDS reply [event=" << jackEvent << "]");
                return; /// \todo Error code
            }
            #endif

            eventBody.name()        = jackEvent.name;
            eventBody.taskId()      = jackEvent.taskId;
            eventBody.goal()        = jackEvent.goal;
            eventBody.goalId()      = jackEvent.goalId;
            eventBody.intentionId() = jackEvent.intentionId;
            eventBody.plan()        = jackEvent.plan;
            eventBody.status()      = status;
            ddsEvent.body().actionUpdate(std::move(eventBody));
        } break;

        case protocol::EventType_BDI_LOG: {
            const auto &jackEvent = static_cast<const protocol::BDILog &>(*event);
            ddsevents::BDILog log = {};
            if (!convertToDDSBDILogLevel(jackEvent.level, log.level())) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event level to DDS level [event=" << jackEvent << "]");
                return; /// \todo Error code
            }

            if (!convertToDDSBDILogType(jackEvent.logType, log.logType())) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event log type to DDS log type [event=" << jackEvent << "]");
                return; /// \todo Error code
            }

            ddsevents::BDILogPayload payload = {};
            switch (jackEvent.logType) {
                case jack::protocol::BDILogType_GOAL_STARTED: /*FALLTHRU*/
                case jack::protocol::BDILogType_GOAL_FINISHED: {
                    ddsevents::BDILogGoal value = {};
                    value.goal()        = jackEvent.goal.goal;
                    value.goalId()      = jackEvent.goal.goalId;
                    value.dropReason()  = jackEvent.goal.dropReason;
                    if (jackEvent.logType == jack::protocol::BDILogType_GOAL_STARTED) {
                        payload.goalStarted(std::move(value));
                    } else {
                        if (!convertToDDSBDILogGoalIntentionResult(jackEvent.goal.result, value.result())) {
                            AOS_DDS_ADAPTER_ERROR("Failed to convert JACK goal result enum value [value=" << jack::protocol::bdiLogGoalIntentionResultString(jackEvent.intention.result) << "]");
                            return;
                        }
                        payload.goalFinished(std::move(value));
                    }
                } break;

                case jack::protocol::BDILogType_SUB_GOAL_STARTED: /*FALLTHRU*/
                case jack::protocol::BDILogType_SUB_GOAL_FINISHED: {
                    ddsevents::BDILogGoal value = {};
                    value.goal()        = jackEvent.goal.goal;
                    value.goalId()      = jackEvent.goal.goalId;
                    value.intentionId() = jackEvent.goal.intentionId;
                    value.taskId()      = jackEvent.goal.taskId;
                    value.dropReason()  = jackEvent.goal.dropReason;

                    if (jackEvent.logType == jack::protocol::BDILogType_SUB_GOAL_STARTED) {
                        payload.subGoalStarted(std::move(value));
                    } else {
                        if (!convertToDDSBDILogGoalIntentionResult(jackEvent.goal.result, value.result())) {
                            AOS_DDS_ADAPTER_ERROR("Failed to convert JACK goal result enum value [value=" << jack::protocol::bdiLogGoalIntentionResultString(jackEvent.intention.result) << "]");
                            return;
                        }
                        payload.subGoalFinished(std::move(value));
                    }
                } break;

                case jack::protocol::BDILogType_INTENTION_STARTED: /*FALLTHRU*/
                case jack::protocol::BDILogType_INTENTION_FINISHED: {
                    ddsevents::BDILogIntention value = {};
                    value.goal()        = jackEvent.intention.goal;
                    value.goalId()      = jackEvent.intention.goalId;
                    value.intentionId() = jackEvent.intention.intentionId;
                    value.plan()        = jackEvent.intention.plan;

                    if (jackEvent.logType == jack::protocol::BDILogType_INTENTION_STARTED) {
                        payload.intentionStarted(std::move(value));
                    } else {
                        if (!convertToDDSBDILogGoalIntentionResult(jackEvent.intention.result, value.result())) {
                            AOS_DDS_ADAPTER_ERROR("Failed to convert JACK intention log result enum value [value=" << jack::protocol::bdiLogGoalIntentionResultString(jackEvent.intention.result) << "]");
                            return;
                        }
                        payload.intentionFinished(std::move(value));
                    }
                } break;

                case jack::protocol::BDILogType_ACTION_STARTED: /*FALLTHRU*/
                case jack::protocol::BDILogType_ACTION_FINISHED: {
                    ddsevents::BDILogAction value = {};
                    value.goal()        = jackEvent.action.goal;
                    value.goalId()      = jackEvent.action.goalId;
                    value.intentionId() = jackEvent.action.intentionId;
                    value.plan()        = jackEvent.action.plan;
                    value.taskId()      = jackEvent.action.taskId;
                    value.action()      = jackEvent.action.action;
                    value.reasoning()   = jackEvent.action.reasoning;

                    if (jackEvent.logType == jack::protocol::BDILogType_ACTION_STARTED) {
                        payload.actionStarted(std::move(value));
                    } else {
                        value.success() = jackEvent.action.success;
                        payload.actionFinished(std::move(value));
                    }
                } break;

                case jack::protocol::BDILogType_SLEEP_STARTED: /*FALLTHRU*/
                case jack::protocol::BDILogType_SLEEP_FINISHED: {
                    ddsevents::BDILogSleep value = {};
                    value.goal()        = jackEvent.sleep.goal;
                    value.goalId()      = jackEvent.sleep.goalId;
                    value.plan()        = jackEvent.sleep.plan;
                    value.taskId()      = jackEvent.sleep.taskId;
                    value.intentionId() = jackEvent.sleep.intentionId;

                    if (jackEvent.logType == jack::protocol::BDILogType_SLEEP_STARTED) {
                        value.sleepMs() = jackEvent.sleep.sleepMs;
                        payload.sleepStarted(std::move(value));
                    } else {
                        payload.sleepFinished(std::move(value));
                    }
                } break;

                case jack::protocol::BDILogType_CONDITION: {
                    ddsevents::BDILogCondition value = {};
                    value.goal()        = jackEvent.condition.goal;
                    value.goalId()      = jackEvent.condition.goalId;
                    value.intentionId() = jackEvent.condition.intentionId;
                    value.plan()        = jackEvent.condition.plan;
                    value.taskId()      = jackEvent.condition.taskId;
                    value.condition()   = jackEvent.condition.condition;
                    value.success()     = jackEvent.condition.success;
                    payload.condition(std::move(value));
                } break;
            }

            log.payload(std::move(payload));
            ddsEvent.body().bdiLog(std::move(log));
        } break;

        default: {
            JACK_PROTOCOL_ASSERT(!"Unhandled event type");
            AOS_DDS_ADAPTER_ERROR("Internal error: Unhandled event type [event=" << event << "]");
            return; /// \todo Error code
        } break;
    }

    if (!convertToDDSBusAddress(event->senderNode, ddsEvent.senderNode()) ||
        !convertToDDSBusAddress(event->sender, ddsEvent.sender()) ||
        !convertToDDSBusAddress(event->recipient, ddsEvent.recipient())) {
        AOS_DDS_ADAPTER_WARNING("Failed to convert JACK event's bus address to DDS address [event=" << *event << ", ddsEvent=" << DDS_EVENT_TO_OSTREAM(ddsEvent) << "]");
        return; /// \todo Return an error
    }

    ddsEvent.timestampUs() = event->timestampUs;
    ddsEvent.eventId(event->eventId);

    EventBusStop& busStop = iter->second;
    dds::pub::DataWriter<jack::ddsevents::Event>& writer = busStop.m_writer;

    try {
        writer.write(ddsEvent);
    } catch (const std::exception& e) {
        AOS_DDS_ADAPTER_ERROR("Writing event to dds-adapter failed [reason=" << e.what() << ", event=" << *event << "]");
        /// \todo Return an error
    }

    #if defined(JACK_DDS_ADAPTER_INLINE_CYCLONE_CODE)
    /// \todo Cyclone DDS doesn't support the wait for acknowledgement API yet
    #else
    /// \todo Javascript RTI connector does not send acknowledgement causing 1s
    /// timeout every write which we don't know how to fix
    #if 0
    try {
        /// \todo Are we ready to wait 1 sec for acks?
        iter->second.m_writer.wait_for_acknowledgments(dds::core::Duration::from_secs(1));
    } catch (const std::exception& e) {
        AOS_DDS_ADAPTER_WARNING("Event written to dds-adapter, waiting for acknowledgement failed [reason=" << e.what() << ", event=" << *event << "]");
        /// \todo Return an error
    }
    #endif
    #endif
}

void JACK_DDS_ADAPTER_NAME::Listener::on_data_available(dds::sub::DataReader<jack::ddsevents::Event>& reader)
{
    using SampleContainer = dds::sub::LoanedSamples<jack::ddsevents::Event>;

    SampleContainer samples = reader.take();
    for (auto sampleIter = samples.begin(); sampleIter != samples.end(); ++sampleIter) {
        if (sampleIter->info().valid()) {
            jack::protocol::Event *event = m_adapter.deserialise(sampleIter->data());
            if (event && (!event->isOk() || !m_adapter.m_incomingEvents.enqueue(event))) {
                AOS_DDS_ADAPTER_WARNING("Error deserialising or enqueuing incoming event [event" << event << "]");
                delete event;
            }
        }
    }
}

jack::protocol::Event *JACK_DDS_ADAPTER_NAME::deserialise(const jack::ddsevents::Event &sample)
{
    jack::protocol::Event *result = nullptr;

    /// Event sender/recipient information
    jack::protocol::BusAddress senderNode = {}, sender = {}, recipient = {};
    if (!convertToProtocolBusAddress(sample.senderNode(), senderNode) ||
        !convertToProtocolBusAddress(sample.sender(), sender) ||
        !convertToProtocolBusAddress(sample.recipient(), recipient))
    {
        AOS_DDS_ADAPTER_WARNING("Failed to convert DDS event's bus address to protocol address [sample=" << DDS_SAMPLE_TO_OSTREAM(sample) << "]");
        return result; /// \todo Error code
    }

    /// Event Body
    const jack::ddsevents::EventBody &sampleBody = sample.body();
    switch (DDS_ENUM_AS_U32(sampleBody._d())) {
        /**********************************************************************
         * CDDI
         **********************************************************************/
        case jack::ddsevents::EventType::EventType_CONTROL: {
            const jack::ddsevents::Control & body    = sampleBody.control();
            jack::protocol::ControlCommand command = {};
            switch (DDS_ENUM_AS_U32(body.command())) {
                case jack::ddsevents::ControlCommand::ControlCommand_START: { command = jack::protocol::ControlCommand_START; } break;
                case jack::ddsevents::ControlCommand::ControlCommand_PAUSE: { command = jack::protocol::ControlCommand_PAUSE; } break;
                case jack::ddsevents::ControlCommand::ControlCommand_STOP:  { command = jack::protocol::ControlCommand_STOP; } break;
                default: { JACK_PROTOCOL_ASSERT(!"Invalid Code Path"); return result; } break;
            }

            auto *event    = new jack::protocol::Control;
            event->command = command;
            result         = event;
        } break;

        case jack::ddsevents::EventType::EventType_PERCEPT: {
            const jack::ddsevents::Percept& body = sampleBody.percept();
            const jack::ddsevents::Field& field  = body.field();

            #if defined(JACK_BUS_SEND_JSON)
            auto *event      = new jack::protocol::Percept;
            event->fieldJSON = field;
            #else
            jack::Field eventField;
            if (!convertToProtocolField(field, eventField)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event field to protocol field [event=" << DDS_SAMPLE_TO_OSTREAM(sample) << "]");
                return nullptr; /// \todo Error code
            }
            auto *event  = new jack::protocol::Percept;
            event->field = std::move(eventField);
            #endif

            event->beliefSet = body.beliefSet();
            result           = event;
        } break;

        case jack::ddsevents::EventType::EventType_MESSAGE: {
            const jack::ddsevents::Message& body = sampleBody.message();

            #if defined(JACK_BUS_SEND_JSON)
            auto *event = new jack::protocol::Message;
            event->json = body;
            #else
            std::vector<jack::Field> fields;
            if (!convertToProtocolFields(body.fields(), fields)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event fields to protocol fields [event=" << sample.eventId() << "]");
                return nullptr; /// \todo Error code
            }
            auto *event   = new jack::protocol::Message;
            event->fields = std::move(fields);
            event->schema = body.schema();
            #endif

            result        = event;
        } break;

        case jack::ddsevents::EventType::EventType_PURSUE: {
            const jack::ddsevents::Pursue& body = sampleBody.pursue();

            #if defined(JACK_BUS_SEND_JSON)
            auto* event       = new jack::protocol::Pursue;
            event->parametersJSON = body.parameters();
            #else
            std::vector<jack::Field> parameters;
            if (!convertToProtocolFields(body.parameters(), parameters)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event parameters to protocol parameters [event=" << DDS_SAMPLE_TO_OSTREAM(sample) << "]");
                return nullptr; /// \todo Error code
            }
            auto* event       = new jack::protocol::Pursue;
            event->parameters = std::move(parameters);
            #endif

            event->goal       = body.goal();
            event->persistent = body.persistent();
            result            = event;
        } break;

        case jack::ddsevents::EventType::EventType_DROP: {
            const jack::ddsevents::Drop& body  = sampleBody.drop();

            jack::protocol::DropMode dropMode = {};
            if (!convertToProtocolDropMode(body.mode(), dropMode)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event drop mode to DDS drop mode");
                return nullptr; /// \todo Error code
            }

            auto* event   = new jack::protocol::Drop;
            event->goal   = body.goal();
            event->goalId = body.goalId();
            event->mode   = dropMode;
            result        = event;
        } break;

        case jack::ddsevents::EventType::EventType_DELEGATION: {
            const jack::ddsevents::Delegation& body = sampleBody.delegation();

            jack::protocol::DelegationStatus status = {};
            if (!convertToProtocolDelegationStatus(body.status(), status)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert delegation status to protocol status [event=" << DDS_SAMPLE_TO_OSTREAM(sample) << "]");
                return nullptr; /// \todo Error code
            }

            #if defined(JACK_BUS_SEND_JSON)
            auto* event           = new jack::protocol::Delegation;
            event->parametersJSON = body.parameters();
            #else
            std::vector<jack::Field> parameters;
            if (!convertToProtocolFields(body.parameters(), parameters)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event parameters to protocol parameters [event=" << DDS_SAMPLE_TO_OSTREAM(sample) << "]");
                return nullptr; /// \todo Error code
            }
            auto* event       = new jack::protocol::Delegation;
            event->parameters = std::move(parameters);
            #endif

            event->status     = status;
            event->goal       = body.goal();
            event->goalId     = body.goalId();
            event->analyse    = body.analyse();
            event->score      = body.score();
            event->team       = body.team();
            event->teamId     = body.teamId();
            result            = event;
        } break;

        /**********************************************************************
         * Protocol
         **********************************************************************/
        case jack::ddsevents::EventType::EventType_HEARTBEAT: {
            const jack::ddsevents::Heartbeat& body  = sampleBody.heartbeat();
            auto*                             event = new jack::protocol::Heartbeat;
            event->timestamp                        = body.time();
            result                                  = event;
        } break;

        case jack::ddsevents::EventType::EventType_REGISTER: {
            const jack::ddsevents::Register& body    = sampleBody.reg();
            jack::protocol::BusAddress       address = {};
            if (!convertToProtocolBusAddress(body.address(), address)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event body address to protocol address [event=" << DDS_SAMPLE_TO_OSTREAM(sample) << "]");
                return nullptr; /// \todo Error code
            }

            jack::protocol::BusAddress team = {};
            if (!convertToProtocolBusAddress(body.team(), team)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event team address to protocol address [event=" << DDS_SAMPLE_TO_OSTREAM(sample) << "]");
                return nullptr; /// \todo Error code
            }

            auto* event         = new jack::protocol::Register;
            event->proxy        = body.proxy();
            event->address      = std::move(address);
            event->templateType = body.templateType();
            event->start        = body.start();
            event->team         = std::move(team);
            result              = event;
        } break;

        case jack::ddsevents::EventType::EventType_DEREGISTER: {
            const jack::ddsevents::Deregister& body = sampleBody.dereg();
            jack::protocol::NodeType           nodeType = {};
            if (!convertToProtocolNodeType(body.nodeType(), nodeType)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event deregister to protocol deregister [event=" << DDS_SAMPLE_TO_OSTREAM(sample) << "]");
                return nullptr; /// \todo Error code
            }

            auto* event = new jack::protocol::Deregister;
            event->id   = body.id();
            event->nodeType = nodeType;
            result      = event;
        } break;

        case jack::ddsevents::EventType::EventType_AGENT_JOIN_TEAM: {
            const jack::ddsevents::AgentJoinTeam& body = sampleBody.agentJoinTeam();
            jack::protocol::BusAddress team = {}, agent = {};
            if (!convertToProtocolBusAddress(body.team(), team) ||
                !convertToProtocolBusAddress(body.agent(), agent)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event bus address to protocol bus address [event=" << DDS_SAMPLE_TO_OSTREAM(sample) << "]");
                return nullptr; /// \todo Error code
            }

            auto* event  = new jack::protocol::AgentJoinTeam;
            event->team  = std::move(team);
            event->agent = std::move(agent);
            result       = event;
        } break;

        case jack::ddsevents::EventType::EventType_AGENT_LEAVE_TEAM: {
            const jack::ddsevents::AgentLeaveTeam &body = sampleBody.agentLeaveTeam();
            jack::protocol::BusAddress             team = {}, agent = {};
            if (!convertToProtocolBusAddress(body.team(), team) ||
                !convertToProtocolBusAddress(body.agent(), agent)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event teama/agent address to protocol address [event=" << DDS_SAMPLE_TO_OSTREAM(sample) << "]");
                return nullptr; /// \todo Error code
            }

            auto* event  = new jack::protocol::AgentLeaveTeam;
            event->team  = std::move(team);
            event->agent = std::move(agent);
            result       = event;
        } break;

        case jack::ddsevents::EventType::EventType_ACTION_BEGIN: {
            const jack::ddsevents::ActionBegin& body = sampleBody.actionBegin();

            #if defined(JACK_BUS_SEND_JSON)
            auto* event           = new jack::protocol::ActionBegin;
            event->parametersJSON = body.parameters();
            #else
            std::vector<jack::Field> parameters;
            if (!convertToProtocolFields(body.parameters(), parameters)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event parameters to protocol parameters [event=" << DDS_SAMPLE_TO_OSTREAM(sample) << "]");
                return nullptr; /// \todo Error code
            }
            auto* event          = new jack::protocol::ActionBegin;
            event->parameters    = std::move(parameters);
            #endif

            event->name          = body.name();
            event->taskId        = body.taskId();
            event->goal          = body.goal();
            event->goalId        = body.goalId();
            event->intentionId   = body.intentionId();
            event->plan          = body.plan();
            event->resourceLocks.reserve(body.resourceLocks().size());
            for (const std::string &entry : body.resourceLocks()) {
                event->resourceLocks.push_back(entry);
            }

            result               = event;
        } break;

        case jack::ddsevents::EventType::EventType_ACTION_UPDATE: {
            const jack::ddsevents::ActionUpdate& body = sampleBody.actionUpdate();

            jack::protocol::ActionStatus status = {};
            if (!convertToProtocolActionStatus(body.status(), status)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event parameters to protocol parameters [event=" << DDS_SAMPLE_TO_OSTREAM(sample) << "]");
                return nullptr; /// \todo Error code
            }

            #if defined(JACK_BUS_SEND_JSON)
            auto* event      = new jack::protocol::ActionUpdate;
            event->replyJSON = body.reply();
            #else
            std::vector<jack::Field> reply;
            if (!convertToProtocolFields(body.reply(), reply)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event reply to protocol reply [event=" << sample.eventId() << "]");
                return nullptr; /// \todo Error code
            }
            auto* event        = new jack::protocol::ActionUpdate;
            event->reply       = std::move(reply);
            #endif

            event->name        = body.name();
            event->status      = status;
            event->taskId      = body.taskId();
            event->goal        = body.goal();
            event->goalId      = body.goalId();
            event->intentionId = body.intentionId();
            event->plan        = body.plan();
            result             = event;
        } break;

        case jack::ddsevents::EventType::EventType_BDI_LOG: {
            const jack::ddsevents::BDILog& body = sampleBody.bdiLog();

            jack::protocol::BDILogLevel level = {};
            if (!convertToProtocolBDILogLevel(body.level(), level)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert BDI log level to protocol log level [event=" << DDS_SAMPLE_TO_OSTREAM(sample) << "]");
                return nullptr; /// \todo Error code
            }

            jack::protocol::BDILogType logType = {};
            if (!convertToProtocolBDILogType(body.logType(), logType)) {
                AOS_DDS_ADAPTER_WARNING("Failed to convert event log type to DDS log type [event=" << DDS_SAMPLE_TO_OSTREAM(sample) << "]");
                return nullptr; /// \todo Error code
            }

            auto* event    = new jack::protocol::BDILog;
            *event         = {};
            event->level   = level;
            event->logType = logType;

            const jack::ddsevents::BDILogPayload& payload = body.payload();
            switch (DDS_ENUM_AS_U32(body.payload()._d())) {
                case DDS_ENUM_TYPE(jack::ddsevents::BDILogType)::BDILogType_GOAL_STARTED: /*FALLTHRU*/
                case DDS_ENUM_TYPE(jack::ddsevents::BDILogType)::BDILogType_GOAL_FINISHED: {
                    bool started = DDS_ENUM_AS_U32(body.payload()._d()) == DDS_ENUM_TYPE(jack::ddsevents::BDILogType)::BDILogType_GOAL_STARTED;
                    const jack::ddsevents::BDILogGoal& value = started ? payload.goalStarted() : payload.goalFinished();
                    event->goal.goal                         = value.goal();
                    event->goal.goalId                       = value.goalId();
                    event->goal.dropReason                   = value.dropReason();
                    if (started) {
                        event->logType = jack::protocol::BDILogType_GOAL_STARTED;
                    } else {
                        event->logType = jack::protocol::BDILogType_GOAL_FINISHED;
                        if (!convertToProtocolBDILogGoalIntentionResult(value.result(), event->goal.result)) {
                            AOS_DDS_ADAPTER_WARNING("Failed to convert BDI goal result to protocol log level [event=" << DDS_SAMPLE_TO_OSTREAM(sample) << "]");
                            return nullptr;
                        }
                    }
                } break;

                case DDS_ENUM_TYPE(jack::ddsevents::BDILogType)::BDILogType_SUB_GOAL_STARTED: /*FALLTHRU*/
                case DDS_ENUM_TYPE(jack::ddsevents::BDILogType)::BDILogType_SUB_GOAL_FINISHED: {
                    bool started = DDS_ENUM_AS_U32(body.payload()._d()) == DDS_ENUM_TYPE(jack::ddsevents::BDILogType)::BDILogType_SUB_GOAL_STARTED;
                    const jack::ddsevents::BDILogGoal& value = started ? payload.subGoalStarted() : payload.subGoalFinished();
                    event->goal.goal                         = value.goal();
                    event->goal.goalId                       = value.goalId();
                    event->goal.intentionId                  = value.intentionId();
                    event->goal.taskId                       = value.taskId();
                    event->goal.dropReason                   = value.dropReason();

                    if (started) {
                      event->logType = jack::protocol::BDILogType_SUB_GOAL_STARTED;
                    } else {
                      event->logType = jack::protocol::BDILogType_SUB_GOAL_FINISHED;
                      if (!convertToProtocolBDILogGoalIntentionResult(value.result(), event->goal.result)) {
                          AOS_DDS_ADAPTER_WARNING("Failed to convert BDI goal result to protocol log level [event=" << DDS_SAMPLE_TO_OSTREAM(sample) << "]");
                          return nullptr;
                      }
                    }
                } break;

                case DDS_ENUM_TYPE(jack::ddsevents::BDILogType)::BDILogType_INTENTION_STARTED: /*FALLTHRU*/
                case DDS_ENUM_TYPE(jack::ddsevents::BDILogType)::BDILogType_INTENTION_FINISHED: {
                    bool started = DDS_ENUM_AS_U32(body.payload()._d()) == DDS_ENUM_TYPE(jack::ddsevents::BDILogType)::BDILogType_INTENTION_STARTED;
                    const jack::ddsevents::BDILogIntention& value = started ? payload.intentionStarted() : payload.intentionFinished();
                    event->intention.goal                         = value.goal();
                    event->intention.goalId                       = value.goalId();
                    event->intention.intentionId                  = value.intentionId();
                    event->intention.plan                         = value.plan();

                    if (started) {
                        event->logType = jack::protocol::BDILogType_INTENTION_STARTED;
                    } else {
                        jack::protocol::BDILogGoalIntentionResult result = {};
                        if (!convertToProtocolBDILogGoalIntentionResult(value.result(), result)) {
                            AOS_DDS_ADAPTER_WARNING("Failed to convert BDI intention result to protocol log level [event=" << DDS_SAMPLE_TO_OSTREAM(sample) << "]");
                            return nullptr;
                        }

                        event->logType          = jack::protocol::BDILogType_INTENTION_FINISHED;
                        event->intention.result = result;
                    }
                } break;

                case DDS_ENUM_TYPE(jack::ddsevents::BDILogType)::BDILogType_ACTION_STARTED: /*FALLTHRU*/
                case DDS_ENUM_TYPE(jack::ddsevents::BDILogType)::BDILogType_ACTION_FINISHED: {
                    bool started = DDS_ENUM_AS_U32(body.payload()._d()) == DDS_ENUM_TYPE(jack::ddsevents::BDILogType)::BDILogType_ACTION_STARTED;
                    const jack::ddsevents::BDILogAction& value = started ? payload.actionStarted() : payload.actionFinished();
                    event->action.goal                         = value.goal();
                    event->action.goalId                       = value.goalId();
                    event->action.intentionId                  = value.intentionId();
                    event->action.plan                         = value.plan();
                    event->action.taskId                       = value.taskId();
                    event->action.action                       = value.action();
                    event->action.reasoning                    = value.reasoning();

                    if (started) {
                        event->logType = jack::protocol::BDILogType_ACTION_STARTED;
                    } else {
                        event->logType = jack::protocol::BDILogType_ACTION_FINISHED;
                        event->action.success = value.success();
                    }
                } break;

                case DDS_ENUM_TYPE(jack::ddsevents::BDILogType)::BDILogType_SLEEP_STARTED: /*FALLTHRU*/
                case DDS_ENUM_TYPE(jack::ddsevents::BDILogType)::BDILogType_SLEEP_FINISHED: {
                    bool started = DDS_ENUM_AS_U32(body.payload()._d()) == DDS_ENUM_TYPE(jack::ddsevents::BDILogType)::BDILogType_SLEEP_STARTED;
                    const jack::ddsevents::BDILogSleep& value = started ? payload.sleepStarted() : payload.sleepFinished();
                    event->sleep.goal                         = value.goal();
                    event->sleep.goalId                       = value.goalId();
                    event->sleep.intentionId                  = value.intentionId();
                    event->sleep.plan                         = value.plan();
                    event->sleep.taskId                       = value.taskId();

                    if (started) {
                        event->logType = jack::protocol::BDILogType_SLEEP_STARTED;
                        event->sleep.sleepMs = value.sleepMs();
                    } else {
                        event->logType = jack::protocol::BDILogType_SLEEP_FINISHED;
                    }
                } break;

                case DDS_ENUM_TYPE(jack::ddsevents::BDILogType)::BDILogType_CONDITION: {
                    const jack::ddsevents::BDILogCondition& value = payload.condition();
                    event->logType                                = jack::protocol::BDILogType_CONDITION;
                    event->condition.goal                         = value.goal();
                    event->condition.goalId                       = value.goalId();
                    event->condition.intentionId                  = value.intentionId();
                    event->condition.plan                         = value.plan();
                    event->condition.taskId                       = value.taskId();
                    event->condition.condition                    = value.condition();
                    event->condition.success                      = value.success();
                } break;
            }

            result = event;
        } break;

        default: {
            JACK_PROTOCOL_ASSERT(!"Unhandled enum type");
            AOS_DDS_ADAPTER_ERROR("Internal error: Unhandled event type [event=" << DDS_SAMPLE_TO_OSTREAM(sample) << "]");
            return nullptr; /// \todo Error code
        } break;
    }

    if (result) {
        result->timestampUs = sample.timestampUs();
        result->senderNode  = std::move(senderNode);
        result->sender      = std::move(sender);
        result->recipient   = std::move(recipient);
        result->eventId     = sample.eventId();
    }

    return result;
}

bool JACK_DDS_ADAPTER_NAME::broadcastEventBody(const jack::protocol::BusAddress& senderNode,
                                               const jack::protocol::BusAddress& sender,
                                               const jack::protocol::BusAddress& recipient,
                                               std::string_view                  eventId,
                                               jack::protocol::EventType         type,
                                               jack::ddsevents::EventBody&&      body)
{
    const std::string *topic = getEventTopicName(type);
    if (!topic) {
        AOS_DDS_ADAPTER_ERROR("Internal error: Topic does not exist for event [type=" << jack::protocol::eventTypeString(type) << "]");
        return false;
    }

    jack::ddsevents::Event event;
    event.body()        = body;
    event.timestampUs() = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    BusStopMap::iterator iter = m_stops.find(*topic);
    if (iter == m_stops.end()) {
        /// \note Adapter is disconnected
        return false;
    }

    if (!convertToDDSBusAddress(senderNode, event.senderNode())) {
        AOS_DDS_ADAPTER_WARNING("Failed to convert sender node address to DDS address [sender=" << senderNode << ", ddsEvent=" << DDS_EVENT_TO_OSTREAM(event) << "]");
        return false; /// \todo Return an error
    }

    if (!convertToDDSBusAddress(sender, event.sender())) {
        AOS_DDS_ADAPTER_WARNING("Failed to convert JACK sender address to DDS address [sender=" << sender << ", ddsEvent=" << DDS_EVENT_TO_OSTREAM(event) << "]");
        return false; /// \todo Return an error
    }

    if (!convertToDDSBusAddress(recipient, event.recipient())) {
        AOS_DDS_ADAPTER_WARNING("Failed to convert JACK recipient address to DDS address [recipient=" << recipient << ", ddsEvent=" << DDS_EVENT_TO_OSTREAM(event) << "]");
        return false; /// \todo Return an error
    }

    event.eventId(std::string(eventId));

    EventBusStop& busStop = iter->second;
    dds::pub::DataWriter<jack::ddsevents::Event>& writer = busStop.m_writer;

    try {
        writer.write(event);
    } catch (const std::exception& e) {
        AOS_DDS_ADAPTER_ERROR("Writing event to dds-adapter failed [topic=" << *topic << ", reason=" << e.what() << "]");
        /// \todo Return an error
    }

    #if defined(JACK_DDS_ADAPTER_INLINE_CYCLONE_CODE)
    /// \todo Cyclone DDS doesn't support the wait for acknowledgement API yet
    #else
    /// \todo Javascript RTI connector does not send acknowledgement causing 1s
    /// timeout every write which we don't know how to fix
    #if 0
    try {
        /// \todo Are we ready to wait 1 sec for acks?
        iter->second.m_writer.wait_for_acknowledgments(dds::core::Duration::from_secs(1));
    } catch (const std::exception& e) {
        AOS_DDS_ADAPTER_WARNING("Event written to dds-adapter, waiting for acknowledgement failed [reason=" << e.what() << ", event=" << *event << "]");
        /// \todo Return an error
    }
    #endif
    #endif
    return true;
}

bool JACK_DDS_ADAPTER_NAME::controlEvent(const jack::protocol::BusAddress& senderNode,
                                 const jack::protocol::BusAddress& sender,
                                 const jack::protocol::BusAddress& recipient,
                                 std::string_view                  eventId,
                                 jack::protocol::ControlCommand    command)
{
    jack::ddsevents::ControlCommand ddsCommand = {};
    switch (command) {
        case jack::protocol::ControlCommand_START: { ddsCommand = jack::ddsevents::ControlCommand(jack::ddsevents::ControlCommand::ControlCommand_START); } break;
        case jack::protocol::ControlCommand_PAUSE: { ddsCommand = jack::ddsevents::ControlCommand(jack::ddsevents::ControlCommand::ControlCommand_PAUSE); } break;
        case jack::protocol::ControlCommand_STOP:  { ddsCommand = jack::ddsevents::ControlCommand(jack::ddsevents::ControlCommand::ControlCommand_STOP); } break;
        default: { JACK_PROTOCOL_ASSERT(!"Invalid Code Path"); return false; } break;
    }

    jack::ddsevents::EventBody body;
    body.control(jack::ddsevents::Control(ddsCommand));
    bool result = broadcastEventBody(senderNode, sender, recipient, eventId, jack::protocol::EventType_CONTROL, std::move(body));
    return result;
}

bool JACK_DDS_ADAPTER_NAME::perceptEvent(const jack::protocol::BusAddress& senderNode,
                                         const jack::protocol::BusAddress& sender,
                                         const jack::protocol::BusAddress& recipient,
                                         std::string_view                  eventId,
                                         std::string_view                  beliefSet,
                                         #if defined(JACK_BUS_SEND_JSON)
                                         std::string_view                  percept
                                         #else
                                         jack::Field&&                     percept
                                         #endif
                                         )
{
    jack::ddsevents::EventBody body;
    #if defined(JACK_BUS_SEND_JSON)
    body.percept(jack::ddsevents::Percept(std::string(beliefSet), std::string(percept)));
    #else
    jack::ddsevents::Field value;
    if (!convertToDDSField(percept, value)) {
        AOS_DDS_ADAPTER_WARNING("Failed to convert event percept to DDS percept [percept=" << percept << "]");
        return false; /// \todo Error code
    }
    body.percept(jack::ddsevents::Percept(std::string(beliefSet), std::move(value)));
    #endif

    bool result = broadcastEventBody(senderNode, sender, recipient, eventId, jack::protocol::EventType_PERCEPT, std::move(body));
    return result;
}

bool JACK_DDS_ADAPTER_NAME::pursueEvent(const jack::protocol::BusAddress& senderNode,
                                        const jack::protocol::BusAddress& sender,
                                        const jack::protocol::BusAddress& recipient,
                                        std::string_view                  eventId,
                                        std::string_view                  goal,
                                        bool                              persistent,
                                        #if defined(JACK_BUS_SEND_JSON)
                                        std::string_view                    parameters
                                        #else
                                        const std::vector<jack::Field>&     parameters
                                        #endif
                                        )
{
    jack::ddsevents::Pursue pursue;
    pursue.goal() = goal;
    pursue.persistent() = persistent;

    #if defined(JACK_BUS_SEND_JSON)
    pursue.parameters() = parameters;
    #else
    if (!convertToDDSParameters(parameters, pursue.parameters())) {
        AOS_DDS_ADAPTER_WARNING("Failed to convert event parameters to DDS parameters");
        return false; /// \todo Error code
    }
    #endif

    jack::ddsevents::EventBody body;
    body.pursue(std::move(pursue));
    bool result = broadcastEventBody(senderNode, sender, recipient, eventId, jack::protocol::EventType_PURSUE, std::move(body));
    return result;
}

bool JACK_DDS_ADAPTER_NAME::dropEvent(const jack::protocol::BusAddress& senderNode,
                                      const jack::protocol::BusAddress& sender,
                                      const jack::protocol::BusAddress& recipient,
                                      std::string_view                  eventId,
                                      std::string_view                  goal,
                                      std::string_view                  goalId,
                                      jack::protocol::DropMode          mode)
{

    jack::ddsevents::Drop drop;
    drop.goal()     = goal;
    drop.goalId()   = goalId;
    if (!convertToDDSDropMode(mode, drop.mode())) {
        AOS_DDS_ADAPTER_WARNING("Failed to convert event drop mode to DDS drop mode");
        return false; /// \todo Error code
    }

    jack::ddsevents::EventBody body;
    body.drop(std::move(drop));
    bool result = broadcastEventBody(senderNode, sender, recipient, eventId, jack::protocol::EventType_DROP, std::move(body));
    return result;
}

bool JACK_DDS_ADAPTER_NAME::delegationEvent(const jack::protocol::BusAddress& senderNode,
                                            const jack::protocol::BusAddress& sender,
                                            const jack::protocol::BusAddress& recipient,
                                            std::string_view eventId,
                                            jack::protocol::DelegationStatus delegationStatus,
                                            std::string_view goal,
                                            std::string_view goalId,
                                            bool analyse,
                                            float score,
                                            std::string_view team,
                                            std::string_view teamId,
                                            #if defined(JACK_BUS_SEND_JSON)
                                            std::string_view                    parameters
                                            #else
                                            const std::vector<jack::Field>&     parameters
                                            #endif
                                            )
{
    jack::ddsevents::Delegation delegation;
    delegation.goal()    = goal;
    delegation.goalId()  = goalId;
    delegation.analyse() = analyse;
    delegation.score()   = score;
    delegation.team()    = team;
    delegation.teamId()  = teamId;

    if (!convertToDDSDelegationStatus(delegationStatus, delegation.status())) {
        AOS_DDS_ADAPTER_WARNING("Failed to convert event delegation status to DDS delegation status");
        return false; /// \todo Error code
    }

    #if defined(JACK_BUS_SEND_JSON)
    delegation.parameters() = parameters;
    #else
    if (!convertToDDSParameters(parameters, delegation.parameters())) {
        AOS_DDS_ADAPTER_WARNING("Failed to convert event parameters to DDS parameters");
        return false; /// \todo Error code
    }
    #endif

    jack::ddsevents::EventBody body;
    body.delegation(std::move(delegation));
    bool result = broadcastEventBody(senderNode, sender, recipient, eventId, jack::protocol::EventType_DELEGATION, std::move(body));
    return result;
}

bool JACK_DDS_ADAPTER_NAME::heartbeatEvent(const jack::protocol::BusAddress& senderNode,
                                   const jack::protocol::BusAddress& sender,
                                   const jack::protocol::BusAddress& recipient,
                                   std::string_view eventId,
                                   int64_t timestamp)
{
    jack::ddsevents::EventBody body;
    body.heartbeat(jack::ddsevents::Heartbeat(timestamp));
    bool result = broadcastEventBody(senderNode, sender, recipient, eventId, jack::protocol::EventType_HEARTBEAT, std::move(body));
    return result;
}

bool JACK_DDS_ADAPTER_NAME::registerEvent(const jack::protocol::BusAddress& senderNode,
                                  std::string_view                  eventId,
                                  bool                              proxy,
                                  const jack::protocol::BusAddress& address,
                                  std::string_view                  templateType,
                                  bool                              start,
                                  const jack::protocol::BusAddress& team)
{
    jack::ddsevents::BusAddress ddsAddress = {};
    if (!convertToDDSBusAddress(address, ddsAddress)) {
        AOS_DDS_ADAPTER_WARNING("Failed to convert event address to DDS address [address=" << address << "]");
        return false; /// \todo Error code
    }

    jack::ddsevents::BusAddress ddsTeam = {};
    if (!convertToDDSBusAddress(team, ddsTeam)) {
        AOS_DDS_ADAPTER_WARNING("Failed to convert event team address to DDS address [team=" << team << "]");
        return false; /// \todo Error code
    }

    jack::ddsevents::EventBody body;
    body.reg(jack::ddsevents::Register(proxy, ddsAddress, std::string(templateType), start, ddsTeam));
    bool result = broadcastEventBody(senderNode,
                                     jack::protocol::BusAddress{}, /// sender
                                     jack::protocol::BusAddress{}, /// recipient
                                     eventId,
                                     jack::protocol::EventType_REGISTER,
                                     std::move(body));
    return result;
}

bool JACK_DDS_ADAPTER_NAME::deregisterEvent(const jack::protocol::BusAddress& senderNode,
                                    const jack::protocol::BusAddress& sender,
                                    const jack::protocol::BusAddress& recipient,
                                    std::string_view eventId,
                                    const jack::protocol::NodeType& type,
                                    std::string_view id)
{
    jack::ddsevents::NodeType ddsType = {};
    if (!convertToDDSNodeType(type, ddsType)) {
        AOS_DDS_ADAPTER_WARNING("Failed to convert event node type to DDS node type [type=" << type << "]");
        return false; /// \todo Error code
    }

    jack::ddsevents::EventBody body;
    body.dereg(jack::ddsevents::Deregister(std::string(id), ddsType));
    bool result = broadcastEventBody(senderNode, sender, recipient, eventId, jack::protocol::EventType_DEREGISTER, std::move(body));
    return result;
}

bool JACK_DDS_ADAPTER_NAME::agentJoinTeamEvent(const jack::protocol::BusAddress& senderNode,
                                        std::string_view eventId,
                                        const jack::protocol::BusAddress& team,
                                        const jack::protocol::BusAddress& agent)
{
    jack::ddsevents::BusAddress ddsAgent = {};
    if (!convertToDDSBusAddress(agent, ddsAgent)) {
        AOS_DDS_ADAPTER_WARNING("Failed to convert event agent address to DDS address [agent=" << agent << "]");
        return false; /// \todo Error code
    }

    jack::ddsevents::BusAddress ddsTeam = {};
    if (!convertToDDSBusAddress(team, ddsTeam)) {
        AOS_DDS_ADAPTER_WARNING("Failed to convert event team address to DDS address [team=" << team << "]");
        return false; /// \todo Error code
    }

    jack::ddsevents::EventBody body;
    body.agentJoinTeam(jack::ddsevents::AgentJoinTeam(ddsTeam, ddsAgent));
    bool result = broadcastEventBody(senderNode,
                                     jack::protocol::BusAddress{},
                                     jack::protocol::BusAddress{},
                                     eventId,
                                     jack::protocol::EventType_AGENT_JOIN_TEAM,
                                     std::move(body));
    return result;
}

bool JACK_DDS_ADAPTER_NAME::agentLeaveTeamEvent(const jack::protocol::BusAddress& senderNode,
                                        const jack::protocol::BusAddress& sender,
                                        const jack::protocol::BusAddress& recipient,
                                        std::string_view eventId,
                                        const jack::protocol::BusAddress& team,
                                        const jack::protocol::BusAddress& agent)
{
    jack::ddsevents::BusAddress ddsAgent = {};
    if (!convertToDDSBusAddress(agent, ddsAgent)) {
        AOS_DDS_ADAPTER_WARNING("Failed to convert event agent address to DDS address [agent=" << agent << "]");
        return false; /// \todo Error code
    }

    jack::ddsevents::BusAddress ddsTeam = {};
    if (!convertToDDSBusAddress(team, ddsTeam)) {
        AOS_DDS_ADAPTER_WARNING("Failed to convert event team address to DDS address [team=" << team << "]");
        return false; /// \todo Error code
    }

    jack::ddsevents::EventBody body;
    body.agentLeaveTeam(jack::ddsevents::AgentLeaveTeam(ddsTeam, ddsAgent));
    bool result = broadcastEventBody(senderNode, sender, recipient, eventId, jack::protocol::EventType_AGENT_LEAVE_TEAM, std::move(body));
    return result;
}

bool JACK_DDS_ADAPTER_NAME::actionBeginEvent(const jack::protocol::BusAddress& senderNode,
                                     const jack::protocol::BusAddress& sender,
                                     const jack::protocol::BusAddress& recipient,
                                     std::string_view eventId,
                                     std::string_view name,
                                     std::string_view taskId,
                                     std::string_view goal,
                                     std::string_view goalId,
                                     std::string_view intentionId,
                                     std::string_view plan,
                                     #if defined(JACK_BUS_SEND_JSON)
                                     std::string_view                    parameters,
                                     #else
                                     const std::vector<jack::Field>&     parameters,
                                     #endif
                                     const std::vector<std::string>& resourceLocks)
{
    jack::ddsevents::ActionBegin actionBegin;
    actionBegin.resourceLocks() = resourceLocks;
    actionBegin.name()          = name;
    actionBegin.taskId()        = taskId;
    actionBegin.goal()          = goal;
    actionBegin.goalId()        = goalId;
    actionBegin.intentionId()   = intentionId;
    actionBegin.plan()          = plan;

    #if defined(JACK_BUS_SEND_JSON)
    actionBegin.parameters() = parameters;
    #else
    if (!convertToDDSParameters(parameters, actionBegin.parameters())) {
        AOS_DDS_ADAPTER_WARNING("Failed to convert event parameters to DDS parameters");
        return false; /// \todo Error code
    }
    #endif

    jack::ddsevents::EventBody body;
    body.actionBegin(std::move(actionBegin));
    bool result = broadcastEventBody(senderNode, sender, recipient, eventId, jack::protocol::EventType_ACTION_BEGIN, std::move(body));
    return result;
}

bool JACK_DDS_ADAPTER_NAME::actionUpdateEvent(const jack::protocol::BusAddress& senderNode,
                                              const jack::protocol::BusAddress& sender,
                                              const jack::protocol::BusAddress& recipient,
                                              std::string_view                  eventId,
                                              std::string_view                  name,
                                              std::string_view                  taskId,
                                              std::string_view                  goal,
                                              std::string_view                  goalId,
                                              std::string_view                  intentionId,
                                              std::string_view                  plan,
                                              jack::protocol::ActionStatus      actionStatus,
                                              #if defined(JACK_BUS_SEND_JSON)
                                              std::string_view                  reply
                                              #else
                                              const std::vector<jack::Field>&   reply
                                              #endif
                                              )
{
    jack::ddsevents::ActionStatus ddsActionStatus = {};
    if (!convertToDDSActionStatus(actionStatus, ddsActionStatus)) {
        AOS_DDS_ADAPTER_WARNING("Failed to convert event status to DDS status [status=" << jack::protocol::actionStatusTypeString(actionStatus) << "]");
        return false; /// \todo Error code
    }

    jack::ddsevents::ActionUpdate actionUpdate;
    #if defined(JACK_BUS_SEND_JSON)
    actionUpdate.reply() = reply;
    #else
    if (!convertToDDSParameters(reply, actionUpdate.reply())) {
        AOS_DDS_ADAPTER_WARNING("Failed to convert event parameters to DDS parameters");
        return false; /// \todo Error code
    }
    #endif

    actionUpdate.name()        = name;
    actionUpdate.taskId()      = taskId;
    actionUpdate.goal()        = goal;
    actionUpdate.goalId()      = goalId;
    actionUpdate.intentionId() = intentionId;
    actionUpdate.plan()        = plan;
    actionUpdate.status()      = ddsActionStatus;

    jack::ddsevents::EventBody body;
    body.actionUpdate(std::move(actionUpdate));
    bool result = broadcastEventBody(senderNode, sender, recipient, eventId, jack::protocol::EventType_ACTION_UPDATE, std::move(body));
    return result;
}
}; // namespace aos
