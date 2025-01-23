#if !defined(JACK_DDS_ADAPTER_NAME)
    #error JACK_DDS_ADAPTER_NAME must be defined to generate the adapter
#endif

/// Protocol
#include <jack/event-protocol/busadapter.h>
#include <jack/event-protocol/protocol.h>


/// Third Party
#include <cstdint>
#include <vector>
#include <set>
#include <map>

/// Macros
#if defined(AOS_DDS_ADAPTER_USER_CONFIG_H)
    #include AOS_DDS_ADAPTER_USER_CONFIG_H
#endif

#if defined(AOS_DDS_ADAPTER_NO_ERROR)
    #if defined(AOS_DDS_ADAPTER_ERROR)
        #undef AOS_DDS_ADAPTER_ERROR
    #endif
    #define AOS_DDS_ADAPTER_ERROR(fmt)
#else
    #if !defined(AOS_DDS_ADAPTER_ERROR)
        #define AOS_DDS_ADAPTER_ERROR(fmt) std::cerr << "\033[48;5;1m\033[38;5;0m[DDS-ERROR]\033[0;37m " << fmt << std::endl;
    #endif
#endif

#if defined(AOS_DDS_ADAPTER_NO_WARNING)
    #if defined(AOS_DDS_ADAPTER_WARNING)
        #undef AOS_DDS_ADAPTER_WARNING
    #endif
    #define AOS_DDS_ADAPTER_WARNING(fmt)
#else
    #if !defined(AOS_DDS_ADAPTER_WARNING)
        #define AOS_DDS_ADAPTER_WARNING(fmt) std::cerr << "\033[48;5;214m\033[38;5;0m[DDS-WARN ]\033[0;37m " << fmt << std::endl;
    #endif
#endif

#if defined(AOS_DDS_ADAPTER_NO_DEBUG)
    #if defined(AOS_DDS_ADAPTER_DEBUG)
        #undef AOS_DDS_ADAPTER_DEBUG
    #endif
    #define AOS_DDS_ADAPTER_DEBUG(fmt)
#else
    #if !defined(AOS_DDS_ADAPTER_DEBUG)
        #define AOS_DDS_ADAPTER_DEBUG(fmt) std::cout << "\033[48;5;247m\033[38;5;0m[DDS-DEBUG]\033[0;37m " << fmt << std::endl;
    #endif
#endif

namespace aos {
class JACK_DDS_ADAPTER_NAME : public jack::BusAdapter {
public:
    /// \brief DDSAdapter - creates publishers and subscribers for all JACK protocol events for the given domain
    /// Note that any publishers and subscribers across domains are not supported yet
    /// \param domainId - DDS domain id, defaults to zero
    JACK_DDS_ADAPTER_NAME(int32_t domainId = 0);

    /// \brief ~DDSAdapter - Unsubscribes to all topics for a good cleanup
    ~JACK_DDS_ADAPTER_NAME() override;

    /// \brief poll - sleeps for a short amount of time. Subscribes messages are not explicitely pulled here.
    /// Instead they are received via DDS callbacks in instances of the Listener class. See below
    void poll() override final;

    /// Subscribe this adapter to all topics for the protocol. No-op if this is
    /// called on an adapter that is already connected.
    void connect() override final;

    /// Unsubscribe from all adapters in the topics for the protocol. No-op if
    /// this is called on an adapter that is already disconnected.
    void disconnect() override final;

    bool broadcastEventBody(const jack::protocol::BusAddress& senderNode,
                            const jack::protocol::BusAddress& sender,
                            const jack::protocol::BusAddress& recipient,
                            std::string_view                  eventId,
                            jack::protocol::EventType         type,
                            jack::ddsevents::EventBody&&      body);

    bool controlEvent(const jack::protocol::BusAddress& senderNode,
                      const jack::protocol::BusAddress& sender,
                      const jack::protocol::BusAddress& recipient,
                      std::string_view                  eventId,
                      jack::protocol::ControlCommand    command) override final;

    bool perceptEvent(const jack::protocol::BusAddress& senderNode,
                      const jack::protocol::BusAddress& sender,
                      const jack::protocol::BusAddress& recipient,
                      std::string_view                  eventId,
                      std::string_view                  beliefSet,
                      #if defined(JACK_BUS_SEND_JSON)
                      std::string_view                  percept
                      #else
                      const jack::Field&                percept
                      #endif
                      ) override final;


    bool pursueEvent(const jack::protocol::BusAddress& senderNode,
                     const jack::protocol::BusAddress& sender,
                     const jack::protocol::BusAddress& recipient,
                     std::string_view                  eventId,
                     std::string_view                  goal,
                     bool                              persistent,
                     #if defined(JACK_BUS_SEND_JSON)
                     std::string_view                  parameters
                     #else
                     const std::vector<jack::Field>&   parameters
                     #endif
                     ) override final;

    bool dropEvent(const jack::protocol::BusAddress& senderNode,
                   const jack::protocol::BusAddress& sender,
                   const jack::protocol::BusAddress& recipient,
                   std::string_view                  eventId,
                   std::string_view                  goal,
                   std::string_view                  goalId,
                   jack::protocol::DropMode          dropMode) override final;

    bool delegationEvent(const jack::protocol::BusAddress& senderNode,
                         const jack::protocol::BusAddress& sender,
                         const jack::protocol::BusAddress& recipient,
                         std::string_view                  eventId,
                         jack::protocol::DelegationStatus  status,
                         std::string_view                  goal,
                         std::string_view                  goalId,
                         bool                              analyse,
                         float                             score,
                         std::string_view                  team,
                         std::string_view                  teamId,
                         #if defined(JACK_BUS_SEND_JSON)
                         std::string_view                  parameters
                         #else
                         const std::vector<jack::Field>&   parameters
                         #endif
                         ) override final;

    bool heartbeatEvent(const jack::protocol::BusAddress& senderNode,
                        const jack::protocol::BusAddress& sender,
                        const jack::protocol::BusAddress& recipient,
                        std::string_view                  eventId,
                        int64_t                           timestamp) override final;

    bool registerEvent(const jack::protocol::BusAddress& senderNode,
                       std::string_view                  eventId,
                       bool                              proxy,
                       const jack::protocol::BusAddress& address,
                       std::string_view                  templateType,
                       bool                              start,
                       const jack::protocol::BusAddress& team) override final;

    bool deregisterEvent(const jack::protocol::BusAddress& senderNode,
                         const jack::protocol::BusAddress& sender,
                         const jack::protocol::BusAddress& recipient,
                         std::string_view                  eventId,
                         const jack::protocol::NodeType&   type,
                         std::string_view                  id) override final;

    bool agentJoinTeamEvent(const jack::protocol::BusAddress& senderNode,
                            std::string_view                  eventId,
                            const jack::protocol::BusAddress& team,
                            const jack::protocol::BusAddress& agent) override final;

    bool agentLeaveTeamEvent(const jack::protocol::BusAddress& senderNode,
                             const jack::protocol::BusAddress& sender,
                             const jack::protocol::BusAddress& recipient,
                             std::string_view eventId,
                             const jack::protocol::BusAddress& team,
                             const jack::protocol::BusAddress& agent) override final;

    bool actionBeginEvent(const jack::protocol::BusAddress& senderNode,
                          const jack::protocol::BusAddress& sender,
                          const jack::protocol::BusAddress& recipient,
                          std::string_view                  eventId,
                          std::string_view                  name,
                          std::string_view                  taskId,
                          std::string_view                  goalId,
                          std::string_view                  goal,
                          std::string_view                  intentionId,
                          std::string_view                  plan,
                          #if defined(JACK_BUS_SEND_JSON)
                          std::string_view                  parameters,
                          #else
                          const std::vector<jack::Field>&   parameters,
                          #endif
                          const std::vector<std::string>&   resourceLocks) override final;

    bool actionUpdateEvent(const jack::protocol::BusAddress& senderNode,
                           const jack::protocol::BusAddress& sender,
                           const jack::protocol::BusAddress& recipient,
                           std::string_view                  eventId,
                           std::string_view                  name,
                           std::string_view                  taskId,
                           std::string_view                  goalId,
                           std::string_view                  goal,
                           std::string_view                  intentionId,
                           std::string_view                  plan,
                           jack::protocol::ActionStatus      status,
                           #if defined(JACK_BUS_SEND_JSON)
                           std::string_view                reply
                           #else
                           const std::vector<jack::Field>& reply
                           #endif
                           ) override final;

    // Not enough clarity on create, delete, subscribe and unsubscribe topic methods below
    void createTopic(const std::string &topic) override final;
    void deleteTopic(const std::string &topic) override final;
    void subscribeTopic(const std::string &topic) override final;
    void unsubscribeTopic(const std::string &topic) override final;
    std::set<std::string> subscriptions() override final;

    // Serialise and send event via the adapter
    std::vector<std::pair<std::string, std::string>> getMessages() override final;

protected:
    // Translate the given protocol event to what's understood on the bus and publish it
    ///
    /// \brief sendEvent - translates th given JACK protocol event to it's equivalent representation on the DDS
    /// bus and publishes it
    /// \param event - the outgoing event to translate and publish
    ///
    void sendEvent(const jack::protocol::Event *event) override final;

    /// \brief sendMessage - Not implemented for DDS bus. DDS bus publishes objects directly on to the bus so
    /// serialising them to byte buffers isn't needed
    void sendMessage(const std::string &topic, const void *data, size_t size) override final { }

private:
    ///
    /// \brief The Listener class - The callback interface to receive messages from the bus
    /// Instances of this interface are subscribe to topics and thus receive any available messages
    /// on the bus via callbacks. Meta information about the condition o reception is also avaiable
    ///
    class Listener : public dds::sub::NoOpDataReaderListener<jack::ddsevents::Event> {
    public:
        Listener(JACK_DDS_ADAPTER_NAME &adapter) : m_adapter(adapter) {}

        /// \brief on_data_available - callback for receiving DDS topic samples from the bus
        /// \param reader - subscriber data reader
        void on_data_available(dds::sub::DataReader<jack::ddsevents::Event> &reader) override final;

    private:
        JACK_DDS_ADAPTER_NAME &m_adapter;
    };

    /// \brief deserialise - translates the given DDS event sample to JACK protocol event
    /// \param sample - event sample received from the DDS bus based on the IDL structure of events
    /// \return instance of the translated JACK event. The event instance is "given" to the caller.
    /// Must be deallocated by it
    jack::protocol::Event* deserialise(const jack::ddsevents::Event &sample);

protected:
    dds::domain::DomainParticipant m_participant;

    template <typename TopicType>
    struct BusStop {
        BusStop(dds::domain::DomainParticipant& participant,
                const std::string& topic,
                const dds::pub::qos::DataWriterQos& writerQos,
                const dds::sub::qos::DataReaderQos& readerQos)
            : m_topic(participant, topic)
            , m_writer(dds::pub::Publisher(participant), m_topic, writerQos)
            , m_reader(dds::sub::Subscriber(participant), m_topic, readerQos)
        {}

        dds::topic::Topic<TopicType>    m_topic;
        dds::pub::DataWriter<TopicType> m_writer;
        dds::sub::DataReader<TopicType> m_reader;
    };

    using EventBusStop = BusStop<jack::ddsevents::Event>;
    using BusStopMap = std::map<std::string, EventBusStop>;

private:
    BusStopMap m_stops;
    Listener   m_listener;
};
} // namespace aos
