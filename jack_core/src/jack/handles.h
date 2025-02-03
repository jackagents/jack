// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#if !defined(JACK_HANDLES_H)
#define JACK_HANDLES_H

/// JACK
#include <jack/engine/uuid.h>
#include <jack/utils.h>

/// Third Party
#include <string>

/******************************************************************************
 * This header file is standalone from the concepts to avoid header
 * explosion where we can avoid having to include the heavy JACK concept headers
 * (goals.h, plans.h agents.h, etc).
 ******************************************************************************/

namespace aos::jack
{
/// \todo Add plan and agent handles as we start to convert the codebase away
/// from holding onto pointers to holding onto handles.

/******************************************************************************
 * @class GoalHandle
 * @brief Store a reference to the concrete goal that can be dereferenced
 * safely in multithreaded applications.
 ******************************************************************************/
template <typename T>
inline std::string handleToString(std::string_view prefix, const T& handle)
{
    ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
    StringBuilder builder          = StringBuilder(scratch.arena);
    builder.append(FMT_STRING("{}{{name={}, id="), prefix, handle.m_name);
    if (handle.m_id.valid()) {
        builder.appendCopy(compactString(handle.m_id.toString()));
    } else {
        builder.appendCopy("(invalid)");
    }
    builder.appendRef("}");
    std::string result = builder.toString();
    return result;
}

template <typename T>
inline std::string handleToHumanString(std::string_view prefix, const T& handle)
{
    ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
    StringBuilder builder          = StringBuilder(scratch.arena);
    if (prefix.size()) {
        builder.append(FMT_STRING("{} "), prefix);
    }

    builder.append(FMT_STRING("{} (ID: "), handle.m_name);
    if (handle.m_id.valid()) {
        builder.appendCopy(compactString(handle.m_id.toString()));
    } else {
        builder.appendRef("<none>");
    }
    builder.appendRef(")");
    std::string result = builder.toString();
    return result;
}

struct GoalHandle /// A lightweight non-owning reference to a goal
{
    bool operator< (const GoalHandle &other) const { return m_id < other.m_id; }
    bool operator!=(const GoalHandle &other) const { return m_id != other.m_id; }
    bool operator==(const GoalHandle &other) const { return m_id == other.m_id; }

    std::string toString()                                  const { return handleToString("GHandle", *this); }
    std::string toHumanString(std::string_view prefix = "") const { return handleToHumanString(prefix, *this); }

    bool valid() const { return m_name.size() && m_id.valid(); }

    std::string m_name; /// Goal name
    UniqueId    m_id;   /// The goal instance's ID we reference.
};

/******************************************************************************
 * @class TacticHandle
 * @brief Store a reference to the concrete tactic that can be dereferenced
 * safely in multithreaded applications.
 ******************************************************************************/
struct TacticHandle /// A lightweight non-owning reference to a goal
{
    bool operator< (const TacticHandle &other) const { return m_id < other.m_id; }
    bool operator!=(const TacticHandle &other) const { return m_id != other.m_id; }
    bool operator==(const TacticHandle &other) const { return m_id == other.m_id; }

    std::string toString()                                  const { return handleToString("THandle", *this); }
    std::string toHumanString(std::string_view prefix = "") const { return handleToHumanString(prefix, *this); }

    bool valid() const { return m_name.size() && m_id.valid(); }

    std::string m_name; /// Tactic name
    UniqueId    m_id;   /// The tactic instance's ID we reference.
};

/******************************************************************************
 * @class ServiceHandle
 * @brief Store a reference to the concrete service that can be dereferenced
 * safely in multithreaded applications.
 ******************************************************************************/
struct ServiceHandle
{
    bool operator< (const ServiceHandle &other) const { return m_id <  other.m_id; }
    bool operator!=(const ServiceHandle &other) const { return m_id != other.m_id; }
    bool operator==(const ServiceHandle &other) const { return m_id == other.m_id; }

    enum class Type
    {
        Agent,
        Service,
    };

    std::string toString()                                  const { return handleToString("SHandle", *this); }
    std::string toHumanString(std::string_view prefix = "") const { return handleToHumanString(prefix, *this); }

    bool valid() const { return m_name.size() && m_id.valid(); }

    std::string m_name; /// Service name
    UniqueId    m_id;   /// The service instance's ID we reference.
};

/******************************************************************************
 * @class AgentHandle
 * @brief Store a reference to the concrete agent that can be dereferenced
 * safely in multithreaded applications.
 ******************************************************************************/
using AgentHandle = ServiceHandle;

/******************************************************************************
 * @class ActionHandle
 * @brief Store a reference to the concrete action that can be dereferenced
 * safely in multithreaded applications.
 ******************************************************************************/
struct ActionHandle
{
    bool operator< (const ActionHandle &other) const { return m_id < other.m_id; }
    bool operator!=(const ActionHandle &other) const { return m_id != other.m_id; }
    bool operator==(const ActionHandle &other) const { return m_id == other.m_id; }

    std::string toString()                                  const { return handleToString("AHandle", *this); }
    std::string toHumanString(std::string_view prefix = "") const { return handleToHumanString(prefix, *this); }

    bool valid() const { return m_name.size() && m_id.valid(); }

    std::string m_name; // Action name
    UniqueId    m_id;   // The action ID we reference.
};

inline std::string format_as(const GoalHandle& handle)
{
    std::string result = handle.toString();
    return result;
}

inline std::string format_as(const TacticHandle& handle)
{
    std::string result = handle.toString();
    return result;
}

inline std::string format_as(const ServiceHandle& handle)
{
    std::string result = handle.toString();
    return result;
}

inline std::string format_as(const ActionHandle& handle)
{
    std::string result = handle.toString();
    return result;
}
} /// namespace aos::jack

namespace std {
template <>
struct hash<aos::jack::GoalHandle>
{
    std::size_t operator()(const aos::jack::GoalHandle &handle) const
    {
        /// \todo We used FNV Hash to avoid allocation, i.e. previously we
        /// called std::hash<std::string>(handle.m_id.toString()), but
        /// ultimately 128 bit UUID is overkill for node-local goals, actions
        /// and plans.
        uint64_t result = 14695981039346656037ULL; /// FNV1A64 Seed
        for (uint64_t u64 : handle.m_id.data()) {
            result = (u64 ^ result) * 1099511628211; /// Prime Number
        }
        return static_cast<std::size_t>(result);
    }

};

template <>
struct hash<aos::jack::AgentHandle>
{
    std::size_t operator()(const aos::jack::AgentHandle &handle) const
    {
        /// \todo We used FNV Hash to avoid allocation, i.e. previously we
        /// called std::hash<std::string>(handle.m_id.toString()), but
        /// ultimately 128 bit UUID is overkill for node-local goals, actions
        /// and plans.
        uint64_t result = 14695981039346656037ULL; /// FNV1A64 Seed
        for (uint64_t u64 : handle.m_id.data()) {
            result = (u64 ^ result) * 1099511628211; /// Prime Number
        }
        return static_cast<std::size_t>(result);
    }
};
}  /// namespace std
#endif /// JACK_HANDLES_H
