#include <jack/jack.h>
#include "jack/event-protocol/protocol.h"

/// Adapters
#if defined(JACK_WITH_RTI_DDS) || defined(JACK_WITH_CYCLONE_DDS)
    #if defined(_WIN32)
        /// \note MinGW GCC defines NOMINMAX
        #if !defined(NOMINMAX)
            #define NOMINMAX
        #endif
        /// \note Windows exposes an ERROR macro that conflicts with aos-log
        #pragma push_macro("ERROR")
        #undef ERROR
    #endif

    #if defined(JACK_WITH_RTI_DDS)
        #include "jack/dds-adapter/rti/rtiddsadapter.h"
    #elif defined(JACK_WITH_CYCLONE_DDS)
        #include "jack/dds-adapter/cyclone/cycddsadapter.h"
    #endif

    #if defined(_WIN32)
        #pragma pop_macro("ERROR")
    #endif
#endif

/// Third Party
#include <assert.h>
#include <limits.h>
#include <float.h>
#include <memory>
#include <gtest/gtest.h>

using namespace aos;

enum Bus
{
    #if defined(JACK_WITH_RTI_DDS)
    Bus_DDS,
    #elif defined(JACK_WITH_CYCLONE_DDS)
    Bus_CYC_DDS,
    #endif
    Bus_COUNT,
};

#if defined(JACK_WITH_RTI_DDS) || defined(JACK_WITH_CYCLONE_DDS)
static int32_t nextDDSDomainId()
{
    static int32_t ddsDomainId = 0;;
    static std::mutex mutex;
    mutex.lock();
    int32_t result = ddsDomainId++;
    ddsDomainId    = ddsDomainId % 100;
    mutex.unlock();
    return result;
}
#endif

struct BusAdapterFixture : public testing::Test
{
    BusAdapterFixture(Bus bus)
    : m_bus(bus)
    , m_engineA("A-JACK")
    , m_engineB("B-JACK")
    {
        JACK_INFO("Init Adapter...");
        switch (m_bus) {
            #if defined(JACK_WITH_RTI_DDS)
            case Bus_DDS: {
                JACK_INFO("Init RTI DDS Adapter...");
                const int32_t DDS_DOMAIN_ID = nextDDSDomainId();
                m_adapterA = std::unique_ptr<RTIDDSAdapter>(new RTIDDSAdapter(DDS_DOMAIN_ID));
                m_adapterB = std::unique_ptr<RTIDDSAdapter>(new RTIDDSAdapter(DDS_DOMAIN_ID));
                JACK_INFO("Init RTI DDS Adapter...DONE");
            } break;
            #elif defined(JACK_WITH_CYCLONE_DDS)
            case Bus_CYC_DDS: {
                JACK_INFO("Init CYC DDS Adapter...");
                const int32_t DDS_DOMAIN_ID = nextDDSDomainId();
                m_adapterA = std::unique_ptr<CycDDSAdapter>(new CycDDSAdapter(DDS_DOMAIN_ID));
                m_adapterB = std::unique_ptr<CycDDSAdapter>(new CycDDSAdapter(DDS_DOMAIN_ID));
                JACK_INFO("Init CYC DDS Adapter...DONE");
            } break;
            #endif

            case Bus_COUNT: { assert(!"Invalid Code Path"); } break;
        }

        m_engineA.addMessageAdapter(m_adapterA.get());
        m_engineB.addMessageAdapter(m_adapterB.get());

        #if defined(JACK_WITH_RTI_DDS) || defined(JACK_WITH_CYCLONE_DDS)
        /// \todo Temporary work-around, DDS requires sometime for the adapters
        /// to find each other. There's a subscription matched callback that we
        /// can use to delay the usage of an adapter until we connect with
        /// someone.
        std::this_thread::sleep_for(std::chrono::seconds(1));
        #endif
    }

    void TearDown() override final
    {
        /// Dump bus directory for debugging
        JACK_INFO("{}", m_engineA.dumpBusDirectory());
        JACK_INFO("{}", m_engineB.dumpBusDirectory());
    }

    Bus                                      m_bus;  /// Configure test for which bus, Cyclone or RTI
    jack::Engine                             m_engineA;
    jack::Engine                             m_engineB;
    std::unique_ptr<jack::MessageBusAdapter> m_adapterA;
    std::unique_ptr<jack::MessageBusAdapter> m_adapterB;
};

struct PerformGoal : public BusAdapterFixture
{
    enum PerformOnEngine
    {
        PerformOnEngine_A,
        PerformOnEngine_B,
        PerformOnEngine_COUNT,
    };

    PerformGoal(Bus bus, PerformOnEngine performOnEngine)
    : BusAdapterFixture(bus)
    , m_performOnEngine(performOnEngine)
    {
    }

    void TestBody() override final
    {
        /**********************************************************************
         * Builders
         **********************************************************************/
        /// For both BDI nodes, setup the busl templates
        const std::string agentTemplateName        = "AgentTemplate";
        const std::string teamTemplateName         = "TeamTemplate";

        const std::string agentGoalName            = "AgentGoal";
        const std::string agentGoalMessageName     = "AgentGoalMessage";
        const std::string agentParamName           = "AgentParam";
        const std::string agentNeverEndingGoalName = "AgentNeverEndingGoal";

        const std::string teamGoalName             = "TeamGoal";
        const std::string teamParamName            = "TeamParam";

        const std::string beliefSetName            = "Beliefs";
        const std::string boolBeliefName           = "BoolBelief";
        const std::string intBeliefName            = "IntBelief";
        const std::string i64BeliefName            = "I64Belief";
        const std::string f32BeliefName            = "F32Belief";

        /// For all BDI nodes, setup the busl templates which is a basic
        /// agent busl as detailed below.
        ///
        ///   [Team] ------------> [TeamGoal] ----------> [TeamPlan] -----------> [Actions]
        ///      |                                                                    |
        ///      |                                                                    V
        ///      |                                                                [START                     ]
        ///      |                        +-------------------------------------- [Goal(AgentGoal)           ]
        ///      |                        |                                       [Goal(AgentNeverEndingGoal)]
        ///      |                        |                                       [END                       ]
        ///      V                        V
        ///   [Agent Role] ----------> [Handles]
        ///      |                        |
        ///      |             +----------+
        ///      |             |
        ///      V             V
        ///   [Agent] ---------+----> [AgentGoal] ------------> [AgentPlan] -------> [Actions]
        ///      |             |           |                         |                   |
        ///  [Beliefs]         |        [Params]                 [Params]                |
        ///      |             |           |                         |                   |
        ///      |             |           |                         |                   V
        ///      V             |           V                         V                [START      ]
        /// [BoolBelief: bool] |  [AgentParam: string] -----> [AgentParam: string] -> [AgentAction]
        /// [IntBelief:  int ] |                                                      [END        ]
        /// [I64Belief:  i64 ] |
        /// [F32Belief:  f32 ] +----> [AgentNeverEndingGoal] -> [AgentNeverEndingPlan] -> [Actions]
        ///                                                                                   |
        ///                                                                                   V
        ///                                                                                [START                 ]
        ///                                                                                [AgentNeverEndingAction]
        ///                                                                                [END                   ]
        ///

        jack::Engine *engines[] = {&m_engineA, &m_engineB};
        std::string   actionCalledFromEngineName;
        std::string   paramValueFromAction;
        int           agentNeverEndingActionHitCount = 0;
        for (jack::Engine *node : engines) {
            /// Agent
            /// \note These beliefs are available to test setting of percept
            /// events over the bus.
            auto beliefs = node->message(beliefSetName)
                               .fieldWithValue<bool>(boolBeliefName, false)
                               .fieldWithValue<int>(intBeliefName, 0)
                               .fieldWithValue<int64_t>(i64BeliefName, 0)
                               .fieldWithValue<float>(f32BeliefName, 0.f)
                               .commit();

            auto agentParamMsg = node->message(agentGoalMessageName)
                                      .field<std::string>(agentParamName)
                                      .commit();

            /// \note Agent Goal
            /// A simple goal that takes some parameters to test passing
            /// parameters over the bus.
            auto agentAction = node->action("AgentAction")
                                   .request(agentParamMsg.name())
                                   .commit();

            auto agentGoalMessage = node->message(agentGoalMessageName)
                                        .field<std::string>(agentParamName)
                                        .commit();

            auto agentGoal   = node->goal(agentGoalName)
                                   .message(agentGoalMessage.name())
                                   .commit();

            auto agentPlan   = node->plan("AgentPlan")
                                 .handles(agentGoal)
                                 .body(node->coroutine()
                                           .action(agentAction.name()))
                                 .commit();

            /// \note Agent Never Ending Goal
            /// The action that executes returns pending, meaning the action is
            /// locked up forever.
            /// We will drop the team goal from a remote node and this never
            /// ending goal should get terminated.
            auto agentNeverEndingAction = node->action("AgentNeverEndingAction")
                                              .commit();

            auto agentNeverEndingGoal = node->goal(agentNeverEndingGoalName)
                                            .commit();

            auto agentNeverEndingPlan = node->plan("AgentNeverEndingPlan")
                                            .handles(agentNeverEndingGoal)
                                            .body(node->coroutine()
                                                      .action(agentNeverEndingAction.name()))
                                            .commit();


            auto agentRole = node->role("AgentRole")
                               .goals(std::array{agentGoal, agentNeverEndingGoal})
                               .commit();

            auto agentBuilder =
                node->agent(agentTemplateName)
                    .plans(std::array{agentPlan, agentNeverEndingPlan})
                    .belief(beliefs)
                    .role(agentRole)
                    .handleAction(
                        agentAction.name(),
                        [&actionCalledFromEngineName, &agentParamName, &paramValueFromAction](
                            jack::Agent&   agent,
                            jack::Message& msg,
                            jack::Message&,
                            jack::ActionHandle) {
                            if (actionCalledFromEngineName.size() || paramValueFromAction.size()) {
                                JACK_ERROR("Agent action called twice [engine=" << actionCalledFromEngineName << "]");
                            }

                            assert(actionCalledFromEngineName.empty());
                            actionCalledFromEngineName = agent.engine().name();

                            if (auto *paramValue = msg.getPtr<std::string>(agentParamName)) {
                                paramValueFromAction = *paramValue;
                            }

                            JACK_INFO("Hello world! [engine={}, agent={}]", actionCalledFromEngineName, agent.handle());
                            return jack::Event::SUCCESS;
                        })
                    .handleAction(
                        agentNeverEndingAction.name(),
                        [&agentNeverEndingActionHitCount](jack::Agent&   agent,
                                                          jack::Message& msg,
                                                          jack::Message&,
                                                          jack::ActionHandle) {
                            JACK_INFO("Never ending goal triggered! [engine={}, agent={}]", agent.engine().name(), agent.handle());
                            agentNeverEndingActionHitCount++;
                            return jack::Event::PENDING;
                        })
                    .commitAsAgent();

            /// Team
            auto teamGoal   = node->goal(teamGoalName)
                                  .message(agentGoalMessage.name())
                                  .commit();

            auto teamPlan   = node->plan("TeamPlan")
                                 .handles(teamGoal)
                                 .body(node->coroutine()
                                         .goal(agentGoal.name())
                                         .goal(agentNeverEndingGoal.name())
                                         .nowait()) /// \note Async because why not, the plan should block until the last task finishes.
                                 .commit();

            auto teamBuilder = node->agent(teamTemplateName)
                                   .plan(teamPlan)
                                   .commitAsTeam();
        }

        /**********************************************************************
         * Runtime
         **********************************************************************/
        #if 1
        aos::jack::setLog(aos::log::Severity::Debug, true);
        #endif

        /// Setup an agent and team with their equivalent proxies in the
        /// following configuration
        ///
        /// Engine | Agent | Is Proxy |
        /// -------+-------+----------+
        /// A      | Agent | No       |
        /// A      | Team  | Yes      |
        ///        |       |          |
        /// B      | Agent | Yes      |
        /// B      | Team  | No       |
        ///
        /// The test will perform a delegation in Engine A and Engine B as
        /// denoted by the PerformOnEngine enum.

        /// Create the agents
        jack::Team *      realTeam    = nullptr;
        jack::Agent *     realAgent   = nullptr;
        jack::Agent *     proxyTeam   = nullptr;
        jack::Agent *     proxyAgent  = nullptr;

        jack::AgentHandle agentHandle = m_engineA.createAgent(agentTemplateName, "Agent");
        jack::AgentHandle teamHandle  = m_engineB.createAgent(teamTemplateName, "Team");

        /// \todo The number of attempts is very high, it should be much lower,
        /// I believe this variability is because of how responsive the backend
        /// bus implementations are. We are not waiting at all when pushing and
        /// polling events which means some events may not be propagated in
        /// time.
        const int ATTEMPTS = 64;
        for (int i = 0; i < ATTEMPTS; i++) {
            m_engineA.poll();
            m_engineB.poll();
            for (jack::Engine *engine : engines) {
                auto *team  = engine->getAgent(teamHandle);
                auto *agent = engine->getAgent(agentHandle);

                if (team && agent) {
                    if (dynamic_cast<jack::ProxyAgent*>(team)) {
                        proxyTeam = team;
                    } else {
                        realTeam = dynamic_cast<jack::Team*>(team);
                        realTeam->addMemberAgent(agent);
                    }
                }

                if (agent) {
                    if (dynamic_cast<jack::ProxyAgent*>(agent)) {
                        proxyAgent = agent;
                    } else {
                        realAgent = agent;
                    }
                }
            }

            if (realAgent && realTeam && proxyAgent && proxyTeam) {
                JACK_INFO("OK {}", i);
                break;
            }
        }

        ASSERT_NE(realAgent, nullptr)  << "Concrete agent was not created in the engine";
        ASSERT_NE(proxyAgent, nullptr) << "The creation of the concrete agent did not propagate a create proxy agent message on the bus";
        ASSERT_NE(realTeam, nullptr)  << "Concrete team was not created in the engine";
        ASSERT_NE(proxyTeam, nullptr) << "The creation of the concrete team did not propagate a create proxy team message on the bus";

        /**********************************************************************
         * Verify ProxyAgents were distributed and exist in the secondary
         * instance
         **********************************************************************/
        {
            ASSERT_NE(proxyTeam, nullptr)
                << "Team was not distributed to the secondary node [" << teamHandle << "]";
            ASSERT_NE(proxyAgent, nullptr)
                << "Agent was not distributed to the secondary node [" << agentHandle << "]";

            EXPECT_NE(dynamic_cast<jack::ProxyAgent *>(proxyTeam), nullptr)
                << "Team was not instantiated as a proxy on the secondary node [" << teamHandle
                << "]";
            EXPECT_NE(dynamic_cast<jack::ProxyAgent *>(proxyAgent), nullptr)
                << "Agent was not instantiated as a proxy on the secondary node [" << agentHandle
                << "]";

            EXPECT_EQ(proxyTeam->handle(), teamHandle);
            EXPECT_EQ(proxyAgent->handle(), agentHandle);
        }

        /**********************************************************************
         * Verify start command from primary instance propagated to the proxy
         * agents
         **********************************************************************/
        /// Start the agents
        {
            realAgent->start();
            realTeam->start();
        }

        /// Verify the agents started
        for (int i = 0; i < ATTEMPTS && (!proxyTeam->running() || !proxyAgent->running()); i++) {
            m_engineA.poll();
            m_engineB.poll();
        }
        EXPECT_TRUE(proxyTeam->running());
        EXPECT_TRUE(proxyAgent->running());

        /**********************************************************************
         * Perform goal on the proxy agents (on the secondary BDI instance)
         **********************************************************************/
        /// Setup some goal parameters to test that they get distributed to the
        /// concrete agent.
        const std::string TARGET_PARAM_VALUE = "Hello World!";
        auto parameters = jack::Message(m_engineA.createMessage(agentGoalMessageName));
        parameters.setFieldValue<std::string>(agentParamName, TARGET_PARAM_VALUE);

        auto performGoalId = jack::UniqueId::random();
        if (m_performOnEngine == PerformOnEngine_A) {
            /// This should dispatch an event to the proxy agent in the secondary BDI
            /// instance. The instance should route this event onto the bus and it
            /// should be directed to the primary instance where the real agent is
            /// sitting and execute the action.
            auto *team = m_engineA.getAgent(teamHandle);
            ASSERT_NE(dynamic_cast<jack::ProxyAgent *>(team), nullptr);
            team->pursue(teamGoalName, jack::GoalPersistent_No, parameters, performGoalId);
        } else {
            auto *team = m_engineB.getAgent(teamHandle);
            ASSERT_EQ(dynamic_cast<jack::ProxyAgent *>(team), nullptr);
            team->pursue(teamGoalName, jack::GoalPersistent_No, parameters, performGoalId);
        }

        for (int i = 0; i < ATTEMPTS && actionCalledFromEngineName.empty(); i++) {
            m_engineA.poll();
            m_engineB.poll();
        }

        /**********************************************************************
         * Event should end up being called from the primary BDI instance
         **********************************************************************/
        EXPECT_EQ(actionCalledFromEngineName, m_engineA.name());

        /// Verify that the parameters were also distributed from the proxy to
        /// the concrete agent thats executing the action
        EXPECT_EQ(paramValueFromAction, TARGET_PARAM_VALUE);

        /**********************************************************************
         * Write to percept in beliefset on proxy agents (on the secondary BDI
         * instance)
         **********************************************************************/
        const int     TARGET_BOOL_BELIEF = true;
        const int32_t TARGET_INT_BELIEF  = INT_MAX;
        const int64_t TARGET_I64_BELIEF  = 123456;
        const float   TARGET_F32_BELIEF  = FLT_EPSILON;

        {
            jack::Message message = proxyAgent->engine().createMessage(beliefSetName);
            EXPECT_TRUE(message.setFieldValue<bool>(boolBeliefName, TARGET_BOOL_BELIEF));
            EXPECT_TRUE(message.setFieldValue<int>(intBeliefName, TARGET_INT_BELIEF));
            EXPECT_TRUE(message.setFieldValue<int64_t>(i64BeliefName, TARGET_I64_BELIEF));
            EXPECT_TRUE(message.setFieldValue<float>(f32BeliefName, TARGET_F32_BELIEF));
            proxyAgent->sendMessage(std::move(message), true /*broadcastToBus*/);
        }

        /**********************************************************************
         * Percept should propagate to real agent and their belief should be
         * updated by the percept
         **********************************************************************/
        bool    boolBelief = false;
        int     intBelief  = 0;
        int64_t i64Belief  = 0;
        float   f32Belief  = 0.f;
        for (int i = 0; i < ATTEMPTS; i++) {
            m_engineA.poll();
            m_engineB.poll();

            /// Pull the beliefs from the real (concrete) agent
            std::shared_ptr<jack::Message> beliefSet = realAgent->message(beliefSetName);
            boolBelief = beliefSet->get<bool>(boolBeliefName);
            intBelief = beliefSet->get<int>(intBeliefName);
            i64Belief = beliefSet->get<int64_t>(i64BeliefName);
            f32Belief = beliefSet->get<float>(f32BeliefName);

            if (boolBelief == TARGET_BOOL_BELIEF &&
                intBelief == TARGET_INT_BELIEF &&
                i64Belief == TARGET_I64_BELIEF &&
                f32Belief == TARGET_F32_BELIEF &&
                agentNeverEndingActionHitCount >= 1)
            {
                break;
            }
        }

        EXPECT_EQ(boolBelief, TARGET_BOOL_BELIEF) << "Belief did not get propagated from proxy to agent";
        EXPECT_EQ(intBelief, TARGET_INT_BELIEF) << "Belief did not get propagated from proxy to agent";
        EXPECT_EQ(i64Belief, TARGET_I64_BELIEF) << "Belief did not get propagated from proxy to agent";
        EXPECT_EQ(f32Belief, TARGET_F32_BELIEF) << "Belief did not get propagated from proxy to agent";

        /**********************************************************************
         * Check that the agent has advanced to
         **********************************************************************/
        ASSERT_EQ(agentNeverEndingActionHitCount, 1) << "The team should finish the sub-goal advance to the next action in the team plan";

        /**********************************************************************
         * Trigger a drop on the goal
         **********************************************************************/
        ASSERT_TRUE(realTeam->getDesire(performGoalId)) << "The delegated goal should persist since we make the action return a pending status";
        ASSERT_TRUE(realAgent->getDesire(agentNeverEndingGoalName)) << "The team's plan triggers the never ending goal as a subgoal and it should still be running";

        if (m_performOnEngine == PerformOnEngine_A) {
            auto *team = m_engineA.getAgent(teamHandle);
            team->drop(jack::GoalHandle{teamGoalName, performGoalId}, "" /*reason*/);
        } else {
            auto *team = m_engineB.getAgent(teamHandle);
            team->drop(jack::GoalHandle{teamGoalName, performGoalId}, "" /*reason*/);
        }

        for (int i = 0;
             i < ATTEMPTS && (realTeam->getDesire(performGoalId) || realAgent->getDesire(agentNeverEndingGoalName));
             i++)
        {
            m_engineA.poll();
            m_engineB.poll();
        }

        ASSERT_FALSE(realTeam->getDesire(performGoalId)) << "The delegated goal should be gone due to dropping the goal";
        ASSERT_FALSE(realAgent->getDesire(agentNeverEndingGoalName)) << "The delegated goal should be gone due to dropping the goal";
    }

    PerformOnEngine m_performOnEngine;
};

struct DropForce : public BusAdapterFixture
{
    DropForce(Bus bus) : BusAdapterFixture(bus) { }

    void TestBody() override final
    {
        /**********************************************************************
         * Builders
         **********************************************************************/
        /// For both BDI nodes, setup the busl templates
        std::string_view agentTemplateName        = "AgentTemplate";
        std::string_view agentGoalName            = "AgentGoal";
        std::string_view agentNeverEndingGoalName = "AgentNeverEndingGoal";
        bool actionTriggered                      = false;

        jack::Engine *engines[] = {&m_engineA, &m_engineB};
        for (jack::Engine *node : engines) {
            auto agentAction = node->action("AgentAction")
                                   .commit();

            auto agentGoal   = node->goal(agentGoalName)
                                   .commit();

            auto agentPlan   = node->plan("AgentPlan")
                                 .handles(agentGoal)
                                 .body(node->coroutine()
                                           .action(agentAction.name()))
                                 .commit();

            auto agentBuilder =
                node->agent(agentTemplateName)
                    .plan(agentPlan)
                    .handleAction(
                        agentAction.name(),
                        [&actionTriggered](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                            actionTriggered = true;
                            return jack::Event::SUCCESS;
                        })
                    .commitAsAgent();
        }

        /**********************************************************************
         * Runtime
         **********************************************************************/
        #if 1
        aos::jack::setLog(aos::log::Severity::Debug, true);
        #endif

        /// Create the agents
        jack::Agent *     realAgent   = nullptr;
        jack::Agent *     proxyAgent  = nullptr;
        jack::AgentHandle agentHandle = m_engineA.createAgent(agentTemplateName, "Agent");

        /// \todo The number of attempts is very high, it should be much lower,
        /// I believe this variability is because of how responsive the backend
        /// bus implementations are. We are not waiting at all when pushing and
        /// polling events which means some events may not be propagated in
        /// time.
        const int ATTEMPTS = 64;
        for (int i = 0; i < ATTEMPTS; i++) {
            m_engineA.poll();
            m_engineB.poll();
            for (jack::Engine *engine : engines) {
                if (auto *agent = engine->getAgent(agentHandle)) {
                    if (dynamic_cast<jack::ProxyAgent*>(agent)) {
                        proxyAgent = agent;
                    } else {
                        realAgent = agent;
                    }
                }
            }

            if (realAgent && proxyAgent) {
                JACK_INFO("OK {}", i);
                break;
            }
        }

        ASSERT_NE(realAgent, nullptr)  << "Concrete agent was not created in the engine";
        ASSERT_NE(proxyAgent, nullptr) << "The creation of the concrete agent did not propagate a create proxy agent message on the bus";

        /**********************************************************************
         * Verify ProxyAgents were distributed and exist in the secondary
         * instance
         **********************************************************************/
        {
            ASSERT_NE(proxyAgent, nullptr)
                << "Agent was not distributed to the secondary node [" << agentHandle << "]";
            EXPECT_NE(dynamic_cast<jack::ProxyAgent *>(proxyAgent), nullptr)
                << "Agent was not instantiated as a proxy on the secondary node [" << agentHandle
                << "]";
            EXPECT_EQ(proxyAgent->handle(), agentHandle);
        }

        /**********************************************************************
         * Verify start command from primary instance propagated to the proxy
         * agents
         **********************************************************************/
        /// Start the agents
        realAgent->start();

        /// Verify the agents started
        for (int i = 0; i < ATTEMPTS && !proxyAgent->running(); i++) {
            m_engineA.poll();
            m_engineB.poll();
        }
        EXPECT_TRUE(proxyAgent->running());

        /**********************************************************************
         * Pursue goal on the proxy agents (on the secondary BDI instance)
         **********************************************************************/
        /// This should dispatch an event to the proxy agent in the secondary BDI
        /// instance. The instance should route this event onto the bus and it
        /// should be directed to the primary instance where the real agent is
        /// sitting and execute the action.
        jack::GoalHandle goalHandle = {std::string(agentGoalName), jack::UniqueId::random()};
        proxyAgent->pursue(agentGoalName, jack::GoalPersistent_Yes, jack::Message{}, goalHandle.m_id);

        for (int i = 0; i < ATTEMPTS && !actionTriggered; i++) {
            m_engineA.poll();
            m_engineB.poll();
        }

        /**********************************************************************
         * Event should end up being called from the primary BDI instance
         **********************************************************************/
        EXPECT_TRUE(actionTriggered);

        /**********************************************************************
         * Trigger a normal drop on the goal
         **********************************************************************/
        ASSERT_TRUE(realAgent->getDesire(agentGoalName));
        proxyAgent->dropWithMode(goalHandle, jack::protocol::DropMode_NORMAL, "" /*reason*/);

        for (int i = 0; i < ATTEMPTS; i++) {
            m_engineA.poll();
            m_engineB.poll();
        }
        ASSERT_TRUE(realAgent->getDesire(goalHandle.m_id));

        /**********************************************************************
         * Trigger a force drop on the goal
         **********************************************************************/
        ASSERT_TRUE(realAgent->getDesire(agentGoalName));
        proxyAgent->dropWithMode(goalHandle, jack::protocol::DropMode_FORCE, "" /*reason*/);

        for (int i = 0; i < ATTEMPTS; i++) {
            m_engineA.poll();
            m_engineB.poll();
        }
        ASSERT_FALSE(realAgent->getDesire(goalHandle.m_id));
    }
};

struct RemoteService : public BusAdapterFixture
{
    RemoteService(Bus bus) : BusAdapterFixture(bus) {}

    void TestBody() override final
    {
        /**********************************************************************
         * Builders
         **********************************************************************/
        const std::string agentTemplateName   = "Agent Template";
        const std::string serviceTemplateName = "Service Template";
        const std::string agentGoalName       = "Agent Goal";
        const std::string remoteValueName     = "Remote Value";

        const int32_t EXPECTED_REMOTE_VALUE = 0xF00BAA;
        int32_t       remoteValue           = 0;
        int           agentActionHitCount   = 0;
        int           serviceActionHitCount = 0;
        jack::Engine* engines[]             = {&m_engineA, &m_engineB};
        for (jack::Engine *node : engines) {
            /// Service
            auto remoteValueMsg = node->message("Remote Value Message")
                                      .field<int32_t>(remoteValueName)
                                      .commit();

            auto serviceAction = node->action("Service Action")
                                     .reply(remoteValueMsg.name())
                                     .commit();

            auto serviceBuilder = node->service(serviceTemplateName)
                                      .handleAction(
                                          serviceAction.name(),
                                          [&serviceActionHitCount, &remoteValueName, EXPECTED_REMOTE_VALUE](jack::Service&, jack::Message&, jack::Message& reply, jack::ActionHandle) {
                                              serviceActionHitCount++;
                                              reply.setFieldValue<int32_t>(remoteValueName, EXPECTED_REMOTE_VALUE);
                                              return jack::Event::SUCCESS;
                                          })
                                      .commit();

            /// Agent
            auto agentGoal   = node->goal(agentGoalName)
                                   .commit();

            auto agentAction = node->action("Agent Action")
                                   .request(remoteValueMsg.name())
                                   .commit();

            auto agentPlan   = node->plan("Agent Plan")
                                 .handles(agentGoal)
                                 .body(node->coroutine()
                                         .action(serviceAction.name())
                                         .action(agentAction.name())
                                      )
                                 .commit();
            auto agentBuilder =
                node->agent(agentTemplateName)
                    .plan(agentPlan)
                    .handleAction(
                        agentAction.name(),
                        [&agentActionHitCount, &remoteValueName, &remoteValue](jack::Agent&, jack::Message& request, jack::Message&, jack::ActionHandle) {
                            agentActionHitCount++;
                            remoteValue = request.get<int32_t>(remoteValueName);
                            return jack::Event::SUCCESS;
                        })
                    .commitAsAgent();
        }

        /**********************************************************************
         * Runtime
         **********************************************************************/
        #if 1
        aos::jack::setLog(aos::log::Severity::Debug, true);
        #endif

        jack::Agent*        realAgent     = nullptr;
        jack::ProxyAgent*   proxyAgent    = nullptr;
        jack::Service*      proxyService  = nullptr;
        jack::Service*      realService   = nullptr;
        jack::AgentHandle   agentHandle   = m_engineA.createAgent(agentTemplateName, "Agent");
        jack::ServiceHandle serviceHandle = m_engineB.createService(serviceTemplateName, "Service", false /*proxy*/);

        /// \todo The number of attempts is very high, it should be much lower,
        /// I believe this variability is because of how responsive the backend
        /// bus implementations are. We are not waiting at all when pushing and
        /// polling events which means some events may not be propagated in
        /// time.
        const int ATTEMPTS = 128;
        for (int i = 0; i < ATTEMPTS; i++) {
            m_engineA.poll();
            m_engineB.poll();
            for (jack::Engine *engine : engines) {
                auto *agent = engine->getAgent(agentHandle);
                if (agent) {
                    if (auto *proxyInstance = dynamic_cast<jack::ProxyAgent*>(agent)) {
                        proxyAgent = proxyInstance;
                    } else {
                        realAgent = agent;
                    }
                }

                auto* service = engine->getService(serviceHandle);
                if (service) {
                    if (service->isProxy()) {
                        proxyService = service;
                    } else {
                        realService = service;
                    }
                }
            }

            if (realAgent && realService && proxyAgent && proxyService) {
                break;
            }
        }

        ASSERT_NE(realAgent, nullptr)  << "Concrete agent was not created in the engine";
        ASSERT_NE(proxyAgent, nullptr) << "The creation of the concrete agent did not propagate a create proxy agent message on the bus";
        ASSERT_NE(realService, nullptr)  << "Concrete service was not created in the engine";
        ASSERT_NE(proxyService, nullptr) << "The creation of the concrete service did not propagate a create proxy service message on the bus";

        /**********************************************************************
         * Verify start command from real agent propagated to the proxy
         *                           proxy service propagated to the real
         **********************************************************************/
        realAgent->start();    /// Start the agent
        proxyService->start(); /// Start the service
        for (int i = 0;
             i < ATTEMPTS && (!realAgent->running()   || !proxyAgent->running() ||
                              !realService->running() || !proxyService->running());
             i++)
        {
            m_engineA.poll();
            m_engineB.poll();
        }
        ASSERT_TRUE(realAgent->running());
        ASSERT_TRUE(proxyAgent->running());
        ASSERT_TRUE(realService->running());
        ASSERT_TRUE(proxyService->running());

        /**********************************************************************
         * Perform goal on the proxy agents (on the secondary BDI instance)
         **********************************************************************/
        /// Goal should propagate to the agent, the agent should execute its
        /// plan which triggers a service.
        realAgent->pursue(agentGoalName, jack::GoalPersistent_No, jack::Message{});
        for (int i = 0;
             i < ATTEMPTS && (agentActionHitCount <= 0 || serviceActionHitCount <= 0);
             i++)
        {
            m_engineA.poll();
            m_engineB.poll();
        }
        EXPECT_EQ(serviceActionHitCount, 1) << "The goal will execute a plan with [service action -> agent action], the service action should be hit once.";
        EXPECT_EQ(agentActionHitCount, 1) << "The goal will execute a plan with [service action -> agent action], the agent action should be hit once.";
        EXPECT_EQ(remoteValue, EXPECTED_REMOTE_VALUE) << "The goal will execute a plan with [service action -> agent action], the agent should receive the value written by the service.";
    }
};

struct ConnectDisconnectHeartbeatCheck : public BusAdapterFixture
{
    ConnectDisconnectHeartbeatCheck(Bus bus) : BusAdapterFixture(bus) {}

    void TestBody() override final
    {
        #if 1
        aos::jack::setLog(aos::log::Severity::Debug, true);
        #endif

        /// \note Check that the engine's bus directory is empty initially
        const std::unordered_map<jack::protocol::BusAddress, jack::Engine::BusAddressableEntity>& busDirectoryEngineA = m_engineA.busDirectory();
        const std::unordered_map<jack::protocol::BusAddress, jack::Engine::BusAddressableEntity>& busDirectoryEngineB = m_engineB.busDirectory();
        ASSERT_TRUE(busDirectoryEngineA.empty());
        ASSERT_TRUE(busDirectoryEngineB.empty());

        std::chrono::milliseconds heartbeatMs = jack::Engine::heartbeatTimerPeriod();
        m_engineA.poll(heartbeatMs);
        m_engineB.poll(heartbeatMs);

        /// \note Tick the engine until the heartbeat arrives. Since we do not
        /// advance the clock (delta time is 0) then the heartbeats will be
        /// registered at exactly the time point at Engine::heartbeatTimer
        for (int i = 0; i < 16; i++) {
            m_engineA.poll(std::chrono::milliseconds(0));
            m_engineB.poll(std::chrono::milliseconds(0));
            if (busDirectoryEngineA.size() && busDirectoryEngineB.size()) {
                break;
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }
        }

        /// \note After the heartbeat duration has elapsed, both nodes should
        /// see each other.
        EXPECT_EQ(busDirectoryEngineA.size(), 1);
        EXPECT_EQ(busDirectoryEngineB.size(), 1);
        jack::Engine::BusAddressableEntity preDisconnectBusEntryEngineB = busDirectoryEngineA.at(m_engineB.busAddress());
        jack::Engine::BusAddressableEntity preDisconnectBusEntryEngineA = busDirectoryEngineB.at(m_engineA.busAddress());
        EXPECT_EQ(preDisconnectBusEntryEngineA.m_lastMessageClockTime.count(), heartbeatMs.count());
        EXPECT_EQ(preDisconnectBusEntryEngineB.m_lastMessageClockTime.count(), heartbeatMs.count());

        /// \note Disconnect engine A from the bus, engineA should no longer
        /// appear on the bus, heart beats should no longer be sent.
        for (jack::MessageBusAdapter* adapter : m_engineA.busAdapters()) {
            adapter->disconnect();
        }

        /// \note Poll the engine to send more heartbeats
        for (int i = 0; i < 8; i++) {
            m_engineA.poll(heartbeatMs);
            m_engineB.poll(heartbeatMs);
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }

        /// \note Since EngineA is disconnected, all engines should never see
        /// each other's heartbeat. The bus directory should be the same.
        EXPECT_EQ(busDirectoryEngineA.size(), 1);
        EXPECT_EQ(busDirectoryEngineB.size(), 1);
        jack::Engine::BusAddressableEntity postDisconnectBusEntryEngineB = busDirectoryEngineA.at(m_engineB.busAddress());
        jack::Engine::BusAddressableEntity postDisconnectBusEntryEngineA = busDirectoryEngineB.at(m_engineA.busAddress());

        EXPECT_EQ(preDisconnectBusEntryEngineB.m_lastMessageClockTime.count(), postDisconnectBusEntryEngineB.m_lastMessageClockTime.count());
        EXPECT_EQ(preDisconnectBusEntryEngineA.m_lastMessageClockTime.count(), postDisconnectBusEntryEngineA.m_lastMessageClockTime.count());

        /// \note Reconnect engine A back into the bus
        for (jack::MessageBusAdapter* adapter : m_engineA.busAdapters()) {
            adapter->connect();
        }

        /// \note Poll the engine and wait for heartbeats to appear back to the
        /// engines again
        for (int i = 0; i < 8; i++) {
            m_engineA.poll(heartbeatMs);
            m_engineB.poll(heartbeatMs);
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }

        /// \note Check that the heartbeat was recently received
        EXPECT_EQ(busDirectoryEngineA.size(), 1);
        EXPECT_EQ(busDirectoryEngineB.size(), 1);
        jack::Engine::BusAddressableEntity postConnectBusEntryEngineB = busDirectoryEngineA.at(m_engineB.busAddress());
        jack::Engine::BusAddressableEntity postConnectBusEntryEngineA = busDirectoryEngineB.at(m_engineA.busAddress());

        EXPECT_LT(postDisconnectBusEntryEngineB.m_lastMessageClockTime.count(), postConnectBusEntryEngineB.m_lastMessageClockTime.count());
        EXPECT_LT(postDisconnectBusEntryEngineA.m_lastMessageClockTime.count(), postConnectBusEntryEngineA.m_lastMessageClockTime.count());
    }
};

struct MessagesWithArrays : public BusAdapterFixture
{
    enum PerformOnEngine
    {
        PerformOnEngine_A,
        PerformOnEngine_B,
        PerformOnEngine_COUNT,
    };

    MessagesWithArrays(Bus bus)
    : BusAdapterFixture(bus)
    {
    }

    void TestBody() override final
    {
        /**********************************************************************
         * Builders
         **********************************************************************/
        /// For both BDI nodes, setup the busl templates
        std::string_view agentTemplateName = "AgentTemplate";
        std::string_view messageName       = "BeliefsSoA";
        std::string_view boolBeliefName    = "BoolBeliefArray";
        std::string_view intBeliefName     = "IntBeliefArray";
        std::string_view i64BeliefName     = "I64BeliefArray";
        std::string_view f32BeliefName     = "F32BeliefArray";

        jack::Engine *engines[] = {&m_engineA, &m_engineB};
        for (jack::Engine *node : engines) {
            /// \note These beliefs are available to test setting of percept
            /// events over the bus.
            auto beliefs = node->message(messageName)
                               .fieldWithValue<std::vector<bool>>(boolBeliefName, {false})
                               .fieldWithValue<std::vector<int>>(intBeliefName, {0})
                               .fieldWithValue<std::vector<int64_t>>(i64BeliefName, {0})
                               .fieldWithValue<std::vector<float>>(f32BeliefName, {0.f})
                               .commit();

            auto agentBuilder = node->agent(agentTemplateName)
                                    .belief(beliefs)
                                    .commitAsAgent();
        }

        /**********************************************************************
         * Runtime
         **********************************************************************/
        #if 1
        aos::jack::setLog(aos::log::Severity::Debug, true);
        #endif

        /// Setup an agent and team with their equivalent proxies in the
        /// following configuration
        ///
        /// Engine | Agent | Is Proxy |
        /// -------+-------+----------+
        /// A      | Agent | No       |
        /// A      | Team  | Yes      |
        ///        |       |          |
        /// B      | Agent | Yes      |
        /// B      | Team  | No       |
        ///
        /// The test will perform a delegation in Engine A and Engine B as
        /// denoted by the PerformOnEngine enum.

        /// Create the agents
        jack::Agent *     realAgent   = nullptr;
        jack::Agent *     proxyAgent  = nullptr;
        jack::AgentHandle agentHandle = m_engineA.createAgent(agentTemplateName, "Agent");

        /// \todo The number of attempts is very high, it should be much lower,
        /// I believe this variability is because of how responsive the backend
        /// bus implementations are. We are not waiting at all when pushing and
        /// polling events which means some events may not be propagated in
        /// time.
        const int ATTEMPTS = 64;
        for (int i = 0; i < ATTEMPTS; i++) {
            m_engineA.poll();
            m_engineB.poll();
            for (jack::Engine *engine : engines) {
                auto *agent = engine->getAgent(agentHandle);
                if (agent) {
                    if (dynamic_cast<jack::ProxyAgent*>(agent)) {
                        proxyAgent = agent;
                    } else {
                        realAgent = agent;
                    }
                }
            }

            if (realAgent && proxyAgent) {
                JACK_INFO("OK {}", i);
                break;
            }
        }

        ASSERT_NE(realAgent, nullptr)  << "Concrete agent was not created in the engine";
        ASSERT_NE(proxyAgent, nullptr) << "The creation of the concrete agent did not propagate a create proxy agent message on the bus";

        /**********************************************************************
         * Verify ProxyAgents were distributed and exist in the secondary
         * instance
         **********************************************************************/
        {
            ASSERT_NE(proxyAgent, nullptr)
                << "Agent was not distributed to the secondary node [" << agentHandle << "]";

            EXPECT_NE(dynamic_cast<jack::ProxyAgent *>(proxyAgent), nullptr)
                << "Agent was not instantiated as a proxy on the secondary node [" << agentHandle
                << "]";

            EXPECT_EQ(proxyAgent->handle(), agentHandle);
        }

        /**********************************************************************
         * Verify start command from primary instance propagated to the proxy
         * agents
         **********************************************************************/
        /// Start the agents
        realAgent->start();

        /// Verify the agents started
        for (int i = 0; i < ATTEMPTS && !proxyAgent->running(); i++) {
            m_engineA.poll();
            m_engineB.poll();
        }
        EXPECT_TRUE(proxyAgent->running());

        /**********************************************************************
         * Write to percept in beliefset on proxy agents (on the secondary BDI
         * instance)
         **********************************************************************/
        const std::vector<bool>    TARGET_BOOL_BELIEF = {true, false, true};
        const std::vector<int>     TARGET_INT_BELIEF  = {INT_MAX, 0, INT_MIN};
        const std::vector<int64_t> TARGET_I64_BELIEF  = {123456, 0xF00BAA};
        const std::vector<float>   TARGET_F32_BELIEF  = {FLT_EPSILON, 0.31212f};

        {
            jack::Message message = proxyAgent->engine().createMessage(messageName);
            EXPECT_TRUE(message.setFieldValue<std::vector<bool>>(boolBeliefName, {TARGET_BOOL_BELIEF}));
            EXPECT_TRUE(message.setFieldValue<std::vector<int>>(intBeliefName, {TARGET_INT_BELIEF}));
            EXPECT_TRUE(message.setFieldValue<std::vector<int64_t>>(i64BeliefName, {TARGET_I64_BELIEF}));
            EXPECT_TRUE(message.setFieldValue<std::vector<float>>(f32BeliefName, {TARGET_F32_BELIEF}));
            proxyAgent->sendMessage(std::move(message), true /*broadcastToBus*/);
        }

        /**********************************************************************
         * Percept should propagate to real agent and their belief should be
         * updated by the percept
         **********************************************************************/
        std::vector<bool>    boolBelief;
        std::vector<int>     intBelief;
        std::vector<int64_t> i64Belief;
        std::vector<float>   f32Belief;
        for (int i = 0; i < ATTEMPTS; i++) {
            m_engineA.poll();
            m_engineB.poll();

            /// Pull the beliefs from the real (concrete) agent
            std::shared_ptr<jack::Message> beliefSet = realAgent->message(messageName);
            boolBelief = beliefSet->get<std::vector<bool>>(boolBeliefName);
            intBelief = beliefSet->get<std::vector<int>>(intBeliefName);
            i64Belief = beliefSet->get<std::vector<int64_t>>(i64BeliefName);
            f32Belief = beliefSet->get<std::vector<float>>(f32BeliefName);

            if (boolBelief == TARGET_BOOL_BELIEF &&
                intBelief == TARGET_INT_BELIEF &&
                i64Belief == TARGET_I64_BELIEF &&
                f32Belief == TARGET_F32_BELIEF)
            {
                break;
            }
        }

        EXPECT_EQ(boolBelief, TARGET_BOOL_BELIEF) << "Belief did not get propagated from proxy to agent";
        EXPECT_EQ(intBelief, TARGET_INT_BELIEF) << "Belief did not get propagated from proxy to agent";
        EXPECT_EQ(i64Belief, TARGET_I64_BELIEF) << "Belief did not get propagated from proxy to agent";
        EXPECT_EQ(f32Belief, TARGET_F32_BELIEF) << "Belief did not get propagated from proxy to agent";
    }

    PerformOnEngine m_performOnEngine;
};

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    /// Register the test for each distribution method
    for (int busInt = 0; busInt < Bus_COUNT; busInt++) {
        char const *testSuffix = nullptr;
        switch(busInt) {
            #if defined(JACK_WITH_RTI_DDS)
            case Bus_DDS: {
                testSuffix = "RTI_DDS";
            } break;
            #elif defined(JACK_WITH_CYCLONE_DDS)
            case Bus_CYC_DDS: {
                testSuffix = "CYC_DDS";
            } break;
            #endif
            case Bus_COUNT: { assert(!"Invalid Code Path"); } break;
        }

        for (int performOnEngineInt = 0;
             performOnEngineInt < PerformGoal::PerformOnEngine_COUNT;
             performOnEngineInt++)
        {
            const char *testPrefix = nullptr;
            if (performOnEngineInt == PerformGoal::PerformOnEngine_A) {
                testPrefix = "DISABLED_PerformGoalFromProxyTeam";
            } else {
                testPrefix = "DISABLED_PerformGoalFromRealTeam";
            }

            testing::RegisterTest(/*test suite name*/ "BusAdapters",
                                  /*test name*/ (std::string(testPrefix) + testSuffix).c_str(),
                                  /*type param*/ nullptr,
                                  /*value param*/ nullptr,
                                  /*file*/ __FILE__,
                                  /*line*/ __LINE__,
                                  [=]() -> PerformGoal * { return new PerformGoal(static_cast<Bus>(busInt), static_cast<PerformGoal::PerformOnEngine>(performOnEngineInt)); });
        }

        testing::RegisterTest(/*test suite name*/ "BusAdaptersService",
                              /*test name*/ (std::string("DISABLED_RemoteService") + testSuffix).c_str(),
                              /*type param*/ nullptr,
                              /*value param*/ nullptr,
                              /*file*/ __FILE__,
                              /*line*/ __LINE__,
                              [=]() -> RemoteService * { return new RemoteService(static_cast<Bus>(busInt)); });

        testing::RegisterTest(/*test suite name*/ "BusAdaptersHeartbeat",
                              /*test name*/ (std::string("DISABLED_ConnectDisconnectHeartbeatCheck") + testSuffix).c_str(),
                              /*type param*/ nullptr,
                              /*value param*/ nullptr,
                              /*file*/ __FILE__,
                              /*line*/ __LINE__,
                              [=]() -> ConnectDisconnectHeartbeatCheck * { return new ConnectDisconnectHeartbeatCheck(static_cast<Bus>(busInt)); });

        testing::RegisterTest(/*test suite name*/ "BusAdaptersMessages",
                              /*test name*/ (std::string("DISABLED_MessagesWithArrays") + testSuffix + "_BROKEN").c_str(),
                              /*type param*/ nullptr,
                              /*value param*/ nullptr,
                              /*file*/ __FILE__,
                              /*line*/ __LINE__,
                              [=]() -> MessagesWithArrays * { return new MessagesWithArrays(static_cast<Bus>(busInt)); });
    }

    return RUN_ALL_TESTS();
}
