/// JACK
#include <jack/json-adapter/jsonadapter.h>
#include <jack/event-protocol/protocol.h>

/// Third Party
#include <tracy/Tracy.hpp>

namespace aos
{
JSONAdapter::JSONAdapter(std::string_view filePath)
: m_filePath(std::string(filePath))
{
}

bool JSONAdapter::connect()
{
    m_file.open(m_filePath);
    bool result = !m_file.fail();
    if (result) {
        JACK_INFO("Recording JACK events to JSON file '{}'", m_filePath);
        m_eventsWritten = 0;
        if (m_emitGlobalArray) {
            m_file << "[\n";
        }
    }
    return result;
}

void JSONAdapter::disconnect()
{
    if (m_file.is_open()) {
        double fileSizeMiB = m_file.tellp() / (1024.0 * 1024.0);
        JACK_INFO("Recorded JACK events to JSON file '{}' (file is {:.3f}MiB)", m_filePath, fileSizeMiB);
        if (m_emitGlobalArray) {
            m_file << (m_eventsWritten ? "\n" : "") << "]\n";
        }
        m_file.close();
    }
}

bool JSONAdapter::sendEvent(const jack::protocol::Event* event)
{
    ZoneScoped;
    if (!m_file.is_open()) {
        JACK_WARNING_ONCE_MSG("The JSON adapter has received an event but it has not been started yet. The event will not be recorded. This error will only be reported once, subsequent events will be silently ignored until the adapter is started.");
        return false;
    }

    if (!JACK_CHECK(event)) {
        return false;
    }

    JACK_ASSERT(event->senderNode.valid());

    bool result = true;
    nlohmann::json j;
    switch (event->type) {
        /// JACK
        case jack::protocol::EventType_CONTROL:          j = *static_cast<const jack::protocol::Control*>(event); break;
        case jack::protocol::EventType_PERCEPT:          j = *static_cast<const jack::protocol::Percept*>(event); break;
        case jack::protocol::EventType_PURSUE:           j = *static_cast<const jack::protocol::Pursue*>(event); break;
        case jack::protocol::EventType_DROP:             j = *static_cast<const jack::protocol::Drop*>(event); break;
        case jack::protocol::EventType_DELEGATION:       j = *static_cast<const jack::protocol::Delegation*>(event); break;
        case jack::protocol::EventType_MESSAGE:          j = *static_cast<const jack::protocol::Message*>(event); break;
        /// Protocol
        case jack::protocol::EventType_REGISTER:         j = *static_cast<const jack::protocol::Register*>(event); break;
        case jack::protocol::EventType_DEREGISTER:       j = *static_cast<const jack::protocol::Deregister*>(event); break;
        case jack::protocol::EventType_AGENT_JOIN_TEAM:  j = *static_cast<const jack::protocol::AgentJoinTeam*>(event); break;
        case jack::protocol::EventType_AGENT_LEAVE_TEAM: j = *static_cast<const jack::protocol::AgentLeaveTeam*>(event); break;
        case jack::protocol::EventType_ACTION_BEGIN:     j = *static_cast<const jack::protocol::ActionBegin*>(event); break;
        case jack::protocol::EventType_ACTION_UPDATE:    j = *static_cast<const jack::protocol::ActionUpdate*>(event); break;
        case jack::protocol::EventType_BDI_LOG:          j = *static_cast<const jack::protocol::BDILog*>(event); break;
        case jack::protocol::EventType_NONE:             result = false; break;
        case jack::protocol::EventType_COUNT:            result = false; break;
    }

    if (j.is_null()) {
        JACK_ASSERT_FMT(!j.is_null(), "A event has been serialised to the null object, event serialisation has failed [event={}]", event->toString());
        result = false;
    }

    if (result) {
        if (m_eventsWritten) {
            if (m_emitGlobalArray) {
                m_file << ",";
            }
            m_file << "\n";
        }

        if (m_indent) {
            m_file << j.dump(m_indent);
        } else {
            m_file << j;
        }

        m_eventsWritten++;
    }
    return result;
}
} // namespace aos
