// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include "bitmaskops.h"
#include "testhelpers.h"

#include <tests/meta/testsproject.h>

#include <tests/meta/messages/agentplanswitchmeta.h>
#include <tests/meta/messages/dummyflagmeta.h>
#include <tests/meta/messages/pingmeta.h>

#include <chrono>
#include <thread>
#include <vector>

using namespace aos;

typedef std::vector<jack::GoalBuilder> GoalList;
typedef std::vector<jack::PlanBuilder> PlanList;

// define some flags for testing
enum PlanCode {
    NoPlan = 0x0,
    Plan1  = 0x1
};

ENABLE_BITMASK_OPERATORS(PlanCode);

/*! ***********************************************************************************************
 * \class   AgentsTest
 *
 * This google test fixture aims to provide a context for Agent specific testing
 * See "3.1.1 Agent" section of Use Cases & Requirements
 * ************************************************************************************************/
class AgentsTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        bdi.exitWhenDone();
        bdi.action("Plan1Action")
            .commit();

        // prepare the goal that the agent will be using
        jack::GoalBuilder goal = bdi.goal("Goal1")
                                     .commit();

        // the plan is expected to invoke a predefined agent action
        jack::PlanBuilder plan = bdi.plan("Plan1")
                                     .handles(goal)
                                     .body(bdi.coroutine()
                                               .action("Plan1Action"))
                                     .commit();

        planList.emplace_back(plan);
        desireList.emplace_back(goal);
    }

    tests    bdi;                         //!< engine bdi initialization
    PlanCode planCode = PlanCode::NoPlan; //!< plan code is used to verify the correct execution of plans
    PlanList planList;
    GoalList desireList;
};

TEST_F(AgentsTest, F01_AgentClean)
{
    /*! ***************************************************************************************
     * Verify clean up of fixture
     * ****************************************************************************************/

    // picked up during memcheck
    EXPECT_EQ(true, true);
}

TEST_F(AgentsTest, AgentStoppedByDefault)
{
    /*! ***************************************************************************************
     * Verify the agent is stopped by default
     *
     * F??
     * Need a requirement
     * ****************************************************************************************/
    jack::AgentHandle agentHandle =
        bdi.agent("TestAgent1")
            .plans(planList)
            .handleAction("Plan1Action",
                          [&](jack::Agent& agent, jack::Message&, jack::Message&, jack::ActionHandle) {
                              planCode |= PlanCode::Plan1;
                              return jack::Event::SUCCESS;
                          })
            .commitAsAgent()
            .createAgent("agent1");

    jack::Agent* agent = bdi.getAgent(agentHandle);

    EXPECT_EQ(agent->running(), false);
    EXPECT_EQ(planCode, PlanCode::NoPlan);
    
}

TEST_F(AgentsTest, F01_AgentStartStop)
{
    /*! ***************************************************************************************
     * Verify if the agent's start and stop states
     * are properly interpreted by the engine.
     *
     * F01
     * An agent shall be enabled, disabled
     * A disabled agent cannot pursue goals
     * Ref. P21
     * ****************************************************************************************/
    jack::AgentHandle agentHandle = bdi.agent("TestAgent1")
                                        .plans(planList)
                                        .handleAction("Plan1Action",
                                                      [&](jack::Agent& agent, jack::Message& msg, jack::Message& out, jack::ActionHandle) {
                                                          planCode |= PlanCode::Plan1;
                                                          return jack::Event::SUCCESS;
                                                      })
                                        .commitAsAgent()
                                        .createAgent("agent1");

    /// Run the goal on a stoppped agent
    jack::Agent* agent = bdi.getAgent(agentHandle);
    agent->pursue("Goal1", jack::GoalPersistent_Yes);
    for (int i = 0; i < 100; i++) {
        bdi.poll();
    }
    EXPECT_EQ(planCode, PlanCode::NoPlan) << "The plan should not have been executed since the agent is not started";

    /// Run the goal but start the agent this time
    {
        agent->start();
        bdi.poll();
        EXPECT_EQ(agent->running(), true) << "The agent should be running now";
        for (int i = 0; i < 100; i++) {
            bdi.poll();
        }
        EXPECT_EQ(planCode, PlanCode::Plan1) << "The plan should have executed";
    }

    /// \note Stop the agent. The plan can still execute as left-over actions in
    /// agent can still execute. However, the agent should be stopped by the end
    /// of it with no desires or intentions.
    {
        agent->stop();
        for (int i = 0; !agent->stopped() && i < 100; i++) {
            bdi.poll();
        }
        EXPECT_TRUE(agent->desires().empty()) << "No desires after stopping";
        EXPECT_TRUE(agent->intentions().empty()) << "No intentions after stopping";
    }

    /// Start and ensure desires don't magically appear somehow
    agent->start();
    for (int i = 0; i < 50; i++) {
        bdi.poll();
    }
    EXPECT_TRUE(agent->desires().empty()) << "Desires should still be gone";
    EXPECT_TRUE(agent->intentions().empty()) << "Intentions should still be gone";

    /// Actually pursue the goal and finish off the test
    agent->pursue("Goal1", jack::GoalPersistent_Yes);
    for (int i = 0; i < 50; i++) {
        bdi.poll();
    }
    EXPECT_EQ(planCode, PlanCode::Plan1) << "The plan should have executed";
    EXPECT_TRUE(agent->desires().size())
        << "Goal is completed, since the goal was pursued (i.e. is persistent) the desire should still be in our list";
}

TEST_F(AgentsTest, F01_AgentStartStopMultiThreaded)
{
    /*! ***************************************************************************************
     * Verify if the agent's started and stopped flags (enable/disable)
     * are properly interpreted by the engine.
     *
     * F01
     * An agent shall be enabled or disabled
     * A disabled agent cannot pursue goals
     * Ref. P21
     * ****************************************************************************************/

    // the startup goal
    auto goal = bdi.goal("StartupGoal").commit();

    // the startup plan
    auto plan = bdi.plan("StartupPlan")
                    .handles(goal)
                    .body(bdi.coroutine()
                              .sleep(1000)
                              .action("Plan1Action"))
                    .commit();

    jack::AgentHandle agentHandle = bdi.agent("TestAgent1")
                                        .plan(plan)
                                        .handleAction("Plan1Action",
                                                      [&](jack::Agent& agent, jack::Message& msg, jack::Message& out, jack::ActionHandle) {
                                                          planCode |= PlanCode::Plan1;
                                                          return jack::Event::SUCCESS;
                                                      })
                                        .commitAsAgent()
                                        .createAgent("agent1");

    // pursue the desire of the agent
    jack::Agent* agent = bdi.getAgent(agentHandle);
    agent->pursue(goal, jack::GoalPersistent_Yes);

    // start the agent
    agent->start();

    // start the engine
    bdi.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // stop the agent
    agent->stop();

    // wait for the agent to finish
    while (!agent->stopped()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    };

    EXPECT_TRUE(agent->stopped());

    // plan was executed
    EXPECT_EQ(planCode, PlanCode::NoPlan);
}

TEST_F(AgentsTest, F01_AgentPauseResume)
{
    /*! ***************************************************************************************
     * Verify if the agent's paused and resumed flags (enable/disable)
     * are properly interpreted by the engine.
     *
     * F01
     * An agent shall be enabled or disabled
     * A disabled agent cannot pursue goals
     * Ref. P21
     * ****************************************************************************************/
    jack::AgentHandle agentHandle = bdi.agent("TestAgent1")
                                        .plans(planList)
                                        .handleAction("Plan1Action",
                                                      [&](jack::Agent& agent, jack::Message& msg, jack::Message& out, jack::ActionHandle) {
                                                          planCode |= PlanCode::Plan1;
                                                          agent.stop();
                                                          return jack::Event::SUCCESS;
                                                      })
                                        .commitAsAgent()
                                        .createAgent("agent1");

    jack::Agent* agent = bdi.getAgent(agentHandle);
    agent->pursue("Goal1", jack::GoalPersistent_Yes);

    agent->start();
    bdi.poll();
    EXPECT_EQ(agent->running(), true);
    EXPECT_EQ(agent->stopped(), false);
    EXPECT_EQ(planCode, PlanCode::NoPlan);

    agent->pause();
    bdi.poll();
    EXPECT_EQ(agent->running(), false);
    EXPECT_EQ(agent->stopped(), false);
    EXPECT_EQ(planCode, PlanCode::NoPlan);

    agent->start();
    bdi.poll();
    EXPECT_EQ(agent->running(), true);
    EXPECT_EQ(agent->stopped(), false);
    EXPECT_EQ(planCode, PlanCode::NoPlan);

    enginePollAndRecordSchedules(bdi, -1 /*pollCount*/);
    bdi.execute(); // step 2 and 3 and finish

    EXPECT_EQ(agent->running(), false);
    EXPECT_EQ(agent->stopped(), true);
    EXPECT_EQ(planCode, PlanCode::Plan1);
}

// verify if an agent is correctly set as stopped
TEST_F(AgentsTest, F02_AgentStopped)
{
    /*! ***************************************************************************************
     * Verify if the agent is stopped when all its goals
     * have been executed.
     *
     * F02
     * An agent shall be created or destroyed
     * It shall be possible to create agents as well as destroy them when not necessary
     * Ref. P21
     * ****************************************************************************************/
    auto agentTemplate = bdi.agent("TestAgent1")
                             .plans(planList)
                             .handleAction("Plan1Action", [&](jack::Agent& agent, jack::Message& msg, jack::Message& out, jack::ActionHandle) {
                                 planCode |= PlanCode::Plan1;
                                 agent.stop();
                                 return jack::Event::SUCCESS;
                             })
                             .commitAsAgent();

    jack::AgentHandle agentHandle = {};
    for (int i = 0; i < 5; ++i) {
        agentHandle        = agentTemplate.createAgent("agent1");
        jack::Agent* agent = bdi.getAgent(agentHandle);

        agent->start();
        agent->pursue("Goal1", jack::GoalPersistent_Yes);
        bdi.poll();
        EXPECT_EQ(agent->stopped(), false);

        bdi.execute();
        EXPECT_EQ(agent->stopped(), true);

        bdi.destroyAgent(agentHandle);
        bdi.destroyAgent(agentHandle); // Do it again to test BDI is being sensible

        /// \note After the destroy calls the agent pointer is invalidated! The
        /// agent should no longer exist in the engine.
        EXPECT_EQ(bdi.getAgent(agentHandle), nullptr);
    }
    bdi.exit();
}

TEST_F(AgentsTest, AgentsInitialDesires)
{
    /*! ***************************************************************************************
     * An agent should execute desires that are initially set by the builder
     * ****************************************************************************************/
    auto agentTemplate = bdi.agent("TestAgent1")
                             .plans(planList)
                             .desires(desireList)
                             .handleAction("Plan1Action", [&](jack::Agent& agent, jack::Message& msg, jack::Message& out, jack::ActionHandle) {
                                 planCode |= PlanCode::Plan1;
                                 agent.stop();
                                 return jack::Event::SUCCESS;
                             })
                             .commitAsAgent();

    jack::AgentHandle agentHandle = agentTemplate.createAgent("Agent007");
    jack::Agent*      agent       = bdi.getAgent(agentHandle);
    agent->start();

    bdi.execute();
    bdi.exit();

    EXPECT_EQ(planCode, PlanCode::Plan1);
}

TEST_F(AgentsTest, AgentPlanSwitch)
{
    /*! ***************************************************************************************
     * An agent should transition to a new plan if it needs to
     * 1. Create 1 goal, "Goal" with 2 candidate plans, "PlanA" and "PlanB".
     *    The PlanA is active when PlanB is not active. PlanB is active when PlanA is not.
     *    We use a beliefset to represent which plan should be executed.
     * 2. The agent is setup to do "PlanA" "ActionA" and halts on completing the action because we
     *    mark the action as pending.
     * 3. Run test
     * ****************************************************************************************/

    bool planAExecuted = false;
    bool planBExecuted = false;

    // 1. Create 1 goal, "Goal" with 2 candidate plans, "PlanA" and "PlanB".
    jack::GoalBuilder   goal    = bdi.goal("Goal").commit();
    jack::ActionBuilder actionA = bdi.action("ActionA").commit();
    jack::PlanBuilder   planA   = bdi.plan("PlanA")
                                  .pre([&](const jack::BeliefContext& context) -> bool {
                                      const auto beliefset = context.getMessageAsPtr<const AgentPlanSwitch>();
                                      return beliefset->switchPlans == false;
                                  })
                                  .handles(goal)
                                  .body(bdi.coroutine()
                                            .action(actionA.name()))
                                  .commit();

    jack::ActionBuilder actionB = bdi.action("ActionB").commit();
    jack::PlanBuilder   planB   = bdi.plan("PlanB")
                                  .pre([&](const jack::BeliefContext& context) -> bool {
                                      const auto beliefset = context.getMessageAsPtr<const AgentPlanSwitch>();
                                      return beliefset->switchPlans == true;
                                  })
                                  .handles(goal)
                                  .body(bdi.coroutine()
                                            .action(actionB.name()))
                                  .commit();

    // 2. The agent is setup to do "PlanA" "ActionA" and halts on completing the action because we
    //    mark the action as pending.
    jack::AgentBuilder agentBuilder =
        bdi.agent("AgentTemplate")
            .plans(std::array{planA, planB})
            .beliefName(AgentPlanSwitch::MODEL_NAME)
            .handleAction(actionA.name(), [&](jack::Agent& agent, jack::Message&, jack::Message& out, jack::ActionHandle) {
                // set the agent belief
                const auto beliefset   = agent.messageAsPtr<AgentPlanSwitch>();
                beliefset->switchPlans = true;

                /// @note Updating a belief alone will not trigger the agent to re-schedule
                /// In order to re-schedule, i.e. switch to PlanB
                /// we need to force a re-schedule of the agent
                agent.forceReschedule();

                /// record success
                planAExecuted = true;

                /// Halt the action from completing immediately by returning
                /// PENDING. This will stop the agent automatically selecting the next plan
                return jack::Event::PENDING;
            })
            .handleAction(actionB.name(), [&](jack::Agent& agent, jack::Message&, jack::Message&, jack::ActionHandle) {
                agent.stop();
                planBExecuted = true;
                return jack::Event::SUCCESS;
            })
            .commitAsAgent();

    // 3. Run test
    jack::AgentHandle agentHandle = agentBuilder.createAgent("Agent");
    jack::Agent*      agent       = bdi.getAgent(agentHandle);
    agent->pursue(goal, jack::GoalPersistent_Yes);
    agent->start();

    enginePollUntilDone(bdi);

    EXPECT_TRUE(planAExecuted);
    EXPECT_TRUE(planBExecuted);
}

TEST_F(AgentsTest, AppliedEffectsShouldNotSendPerceptEvent)
{
    jack::ResourceBuilder batteryResource = bdi.resource("Battery")
                                                .min(0)
                                                .max(100)
                                                .commit();
    bool effectsApplied = false;

    jack::ActionBuilder action = bdi.action("Action").commit();
    jack::GoalBuilder   goal =
        bdi.goal("Goal")
            .satisfied([&](const jack::BeliefContext& context) -> bool {
                return effectsApplied;
            })
            .commit();

    jack::PlanBuilder plan =
        bdi.plan("Plan")
            .handles(goal)
            .effects([&](jack::BeliefContext& context) -> void {
                // Normally updating a resource triggers a precept event. JACK when applying effects
                // should ideally not send a percept here.
                std::shared_ptr<jack::Resource> battery = context.resource(batteryResource.name());
                battery->set(battery->min());

                effectsApplied = true;
            })
            .body(bdi.coroutine()
                      .action(action.name()))
            .commit();

    /// Agent
    jack::AgentHandle agentHandle =
        bdi.agent("AgentWithBatteryTemplate")
            .plan(plan)
            .resource(batteryResource)
            .handleAction(
                action.name(),
                [&](jack::Agent& agent, jack::Message& msg, jack::Message& out, jack::ActionHandle) {
                    return jack::Event::SUCCESS;
                })
            .commitAsAgent()
            .createAgent("AgentWithBattery");

    jack::Agent* agent = bdi.getAgent(agentHandle);
    agent->start();
    agent->pursue(goal, jack::GoalPersistent_No);
    while (!effectsApplied) {
        bdi.poll();
    }

    EXPECT_FALSE(agent->haveEvents()) << "When applying effects and modifying resource, we should "
                                         "not send any resource percept events to the agent";
}

TEST_F(AgentsTest, ExecutorRunningState)
{
    jack::ActionBuilder actions[] = {
        bdi.action("Action1").commit(),
        bdi.action("Action2").commit(),
    };

    auto goal = bdi.goal("CustomGoal")
                    .commit();

    // the plan is expected to invoke a predefined agent action
    auto plan = bdi.plan("CustomPlan")
                    .handles(goal)
                    .body(bdi.coroutine()
                              .action(actions[0].name())
                              .action(actions[1].name()))
                    .commit();

    bool              secondActionStarted = true;
    jack::AgentHandle agentHandle =
        bdi.agent("AgentTemplate")
            .plan(plan)
            .handleAction(actions[0].name(),
                          [&](jack::Agent& agent, jack::Message& msg, jack::Message& out, jack::ActionHandle) {
                              return jack::Event::SUCCESS;
                          })
            .handleAction(actions[1].name(),
                          [&](jack::Agent& agent, jack::Message& msg, jack::Message& out, jack::ActionHandle) {
                              secondActionStarted = true;
                              return jack::Event::PENDING;
                          })
            .commitAsAgent()
            .createAgent("Agent");

    jack::Agent* agent = bdi.getAgent(agentHandle);
    agent->start();
    agent->pursue(goal, jack::GoalPersistent_No);
    while (agent->intentions().empty()) {
        EXPECT_EQ(agent->runningState(), jack::AgentExecutor::RunningState::IDLE);
        enginePollAndRecordSchedules(bdi, 1);
    }

    while (agent->intentions().size()) {
        if (secondActionStarted) {
            if (agent->runningState() == jack::AgentExecutor::RunningState::BUSY_WAITING_ON_EXECUTOR) {
                break;
            } else {
                EXPECT_EQ(agent->runningState(), jack::AgentExecutor::RunningState::EXECUTING);
            }
        } else {
            EXPECT_EQ(agent->runningState(), jack::AgentExecutor::RunningState::EXECUTING);
        }
        enginePollAndRecordSchedules(bdi, 1);
    }
    enginePollAndRecordSchedules(bdi, 1);
}

TEST_F(AgentsTest, CreateAgentKnownTemplate)
{
    /*! ***************************************************************************************
     * Confirm we can create an agent with a known template
     * ****************************************************************************************/
    // Create a known template
    jack::AgentBuilder agentTemplate =
        bdi.agent("AgentTemplate")
            .commitAsAgent();

    // Confirm we can create an agent with known template
    auto id = jack::UniqueId::random();
    id.setTag("1234");

    jack::AgentHandle agentHandle = bdi.createAgent("AgentTemplate", "AgentName", id);
    jack::Agent*      agent       = bdi.getAgent(agentHandle);
    EXPECT_TRUE(agent != NULL);
}

TEST_F(AgentsTest, CreateAgentUnknownTemplate)
{
    /*! ***************************************************************************************
     * Confirm engine::createAgent returns NULL if template unrecognised
     * ****************************************************************************************/
    auto id = jack::UniqueId::random();
    id.setTag("1111");
    jack::AgentHandle handle = bdi.createAgent("UnknownTemplate", "Unknown", id);
    jack::Agent*      agent  = bdi.getAgent(handle);
    EXPECT_TRUE(agent == NULL);
}

TEST_F(AgentsTest, DontCrashEngineIfAgentMissing)
{
    /*! ***************************************************************************************
     * Do not allow engine to start if there if a critical error has occured previously
     * ****************************************************************************************/

    // Create and start a valid agent
    jack::AgentBuilder agentTemplate =
        bdi.agent("AgentTemplate")
            .commitAsAgent();

    auto id1234 = jack::UniqueId::random();
    id1234.setTag("1234");

    auto id1111 = jack::UniqueId::random();
    id1111.setTag("1111");

    jack::AgentHandle agentHandle = bdi.createAgent("AgentTemplate", "AgentName", id1234);
    jack::Agent*      agent       = bdi.getAgent(agentHandle);
    agent->start();

    // Attempt to create an agent with an unknown template
    bdi.createAgent("UnknownTemplate", "Unknown", id1111);

    // Confirm engine will not start now that there is a critical error
    bdi.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(bdi.getStatus());
}

TEST_F(AgentsTest, DISABLED_DoubleSchedulingOfFinishedDesire)
{
    /**************************************************************************
     * /// \todo description
     **************************************************************************/
    /// \todo This task is meant to fail at the moment! Because it's not
    /// a critical error, JACK still handles this ok because we merge intentions
    /// together for the same goal- and this will continue to work whilst
    /// agent's can complete processing a schedule in the same tick that the
    /// schedule was generated.

    // const char DUMMY_FLAG[]            = "DummyFlag";

    // jack::MessageBuilder beliefs = bdi.message("Beliefs")
    //                                   .fieldWithValue<bool>(DUMMY_FLAG, false)
    //                                   .commit();

    jack::GoalBuilder goal = bdi.goal("Goal")
                                 .commit();

    jack::ActionBuilder action = bdi.action("Action")
                                     .commit();

    jack::PlanBuilder plan = bdi.plan("Plan")
                                 .handles(goal)
                                 .body(bdi.coroutine()
                                           .action(action.name()))
                                 .commit();

    jack::ActionHandle actionHandle;
    int                actionInvokeCount = 0;
    jack::AgentHandle  agentHandle       = bdi.agent("AgentTemplate")
                                        .plan(plan)
                                        .beliefName(DummyFlag::MODEL_NAME)
                                        .handleAction(action.name(),
                                                      [&actionHandle, &actionInvokeCount](jack::Agent& agent, jack::Message& msg, jack::Message& out, jack::ActionHandle handle) {
                                                          actionHandle = handle;
                                                          actionInvokeCount++;
                                                          return jack::Event::PENDING;
                                                      })
                                        .commitAsAgent()
                                        .createAgent("Agent");

    jack::Agent* agent = bdi.getAgent(agentHandle);
    agent->start();
    for (int i = 0; i < 16; i++) {
        bdi.poll(); /// Startup the engine and get it ticking into a nice state
    }

    agent->pursue(goal, jack::GoalPersistent_No);
    for (int i = 0; i < 16; i++) {
        bdi.poll(); /// Tick the engine enough to trigger the agent action
    }

    /// Ensure the engine is in the state that we expected
    EXPECT_EQ(actionInvokeCount, 1)
        << "The goal was not pursued, otherwise the lambda would have incremented the counter";

    /// Complete the action, but dirty the schedule as well, these 2 operations
    /// will create 2 events that are handled in the same tick.
    {
        agent->finishActionHandle(actionHandle, true /*success*/);
        const auto beliefSet = agent->messageAsPtr<DummyFlag>();
        beliefSet->flag      = true;
        // how to dirty schedule??
        // beliefSet->setFieldValue<bool>(DUMMY_FLAG, true); /// Dirty the schedule
    }

    /// Tick the engine, this will complete the plan- and the engine will start
    /// trying to clean it up. The beliefset will be dirtied and the agent will
    /// try and make a new schedule.
    bdi.poll();

    EXPECT_EQ(agent->intentions().size(), 1)
        << "The agent should still have the intention to do the action, but it should enter the "
           "concluding phase because we returned success in the action handler";

    EXPECT_TRUE(agent->intentions()[0]->currentIntention()->finished())
        << "The agent should still have the intention to do the action, but it should enter the "
           "finishing state because we returned success in the action handler";

    EXPECT_TRUE(agent->desires().empty())
        << "When we successfully finish the intention, the desire should get deleted immediately; "
           "Otherwise since the Agent's schedule is dirty, the schedule will plan with the desire- "
           "even though we've already finished.\n\n"
        << "This is supported of course in the engine, the executor will merge any new intentions "
           "that are the same as the ones currently running- but the fact that the engine thinks "
           "that the desire needs to still be planned again is semantically wrong. This doesn't "
           "ordinarily cause problems for solo agents, but for teams this causes incorrect "
           "behaviour.";

    /// Tick the engine, this will "conclude" the intention and delete the
    /// desire in the one tick
    bdi.poll();

    EXPECT_TRUE(agent->intentions().empty());

    EXPECT_TRUE(agent->desires().empty());

    /// Tick the engine, some arbitrary amount to make sure there's nothing
    /// hiding in the scheduler and the agent is truly finished
    bdi.poll();

    EXPECT_TRUE(agent->intentions().empty());

    EXPECT_TRUE(agent->desires().empty());

    EXPECT_EQ(actionInvokeCount, 1)
        << "Somehow the action got invoked twice, but we only performed the action once.";
}

TEST_F(AgentsTest, DISABLED_InitOrderMustHaveZeroSideEffects)
{
    /**************************************************************************
     * /// \todo description
     **************************************************************************/
    const std::string agentAName = "AgentA";
    const std::string agentBName = "AgentB";

    jack::GoalBuilder goal = bdi.goal("Goal")
                                 .commit();

    jack::ActionBuilder action = bdi.action("Action")
                                     .commit();

    jack::PlanBuilder plan = bdi.plan("Plan")
                                 .handles(goal)
                                 .body(bdi.coroutine()
                                           .action(action.name()))
                                 .commit();

    bool              agentBSentMsg     = false;
    bool              agentAReceivedMsg = false;
    jack::AgentHandle agentHandleA      = {};
    jack::AgentHandle agentHandleB      = {};

    // auto pingMsg      = bdi.message("Ping").commit();
    auto agentBuilder = bdi.agent("AgentTemplate")
                            .plan(plan)
                            .handleAction(action.name(),
                                          [&](jack::Agent& agent, jack::Message& msg, jack::Message& out, jack::ActionHandle handle) {
                                              /// In the middle of a engine tick, we send a message
                                              /// to Agent A. Note that in this test we initialised
                                              /// Agent A after Agent B.
                                              ///
                                              /// By sending a message to Agent A from Agent B,
                                              /// Agent A should not receive that message within the
                                              /// same tick.
                                              ///
                                              /// Note that if we swapped the init order, Agent A
                                              /// then B, then sending a message to Agent A from B,
                                              /// Agent A will receive it in the next tick.
                                              ///
                                              /// This inconsistency depending on the initialisation
                                              /// order should not exist.
                                              assert(agent.handle() == agentHandleB && "This test should only execute the action for agent B");
                                              jack::Agent* agentA = agent.engine().getAgent(agentHandleA);

                                              auto msgToSend = std::make_unique<Ping>();
                                              agentA->sendMessageToHandler(std::move(msgToSend));

                                              agentBSentMsg = true;
                                              return jack::Event::SUCCESS;
                                          })
                            .handleMessage(std::string(Ping::MODEL_NAME), [&](jack::Agent& agent, const jack::Message& message) {
                                assert(agent.handle() == agentHandleA && "This test should only send a message to agent A");
                                agentAReceivedMsg = true;
                            });

    /// NOTE: Initialise AgentB before AgentA!
    agentHandleB = agentBuilder.createAgent("AgentB");
    agentHandleA = agentBuilder.createAgent("AgentA");

    jack::Agent* agentB = bdi.getAgent(agentHandleB);
    jack::Agent* agentA = bdi.getAgent(agentHandleA);

    agentA->start();
    agentB->start();

    agentB->pursue(goal.name(), jack::GoalPersistent_Yes);
    while (!agentBSentMsg) {
        bdi.poll();
    }

    EXPECT_FALSE(agentAReceivedMsg)
        << "Agent B sent a message to Agent A and the engine stopped polling, Agent A should not "
           "receive the message until the next tick, the message should not have been processed "
           "and should be sitting in Agent A's event queue.";
}

TEST(Agents, Perform100Goals)
{
    jack::Engine        bdi;
    jack::ActionBuilder action = bdi.action("Action").commit();
    jack::GoalBuilder   goal   = bdi.goal("Goal")
                                 .commit();
    jack::PlanBuilder plan = bdi.plan("Plan")
                                 .handles(goal)
                                 .body(bdi.coroutine()
                                           .action(action.name()))
                                 .commit();

    const int    GOAL_COUNT        = 100;
    int          actionInvokeCount = 0;
    jack::Agent* agent             = bdi.agent("AgentTemplate")
                             .plan(plan)
                             .handleAction(action.name(),
                                           [&actionInvokeCount](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                                               actionInvokeCount++;
                                               return jack::Event::SUCCESS;
                                           })
                             .commitAsAgent()
                             .createAgentInstance("Agent");

    agent->start();
    for (int i = 0; i < GOAL_COUNT; i++) {
        agent->pursue(goal, jack::GoalPersistent_No);
    }

    while (actionInvokeCount != GOAL_COUNT) {
        bdi.poll();
    }
}

TEST(Agents, Perform1kGoals)
{
    jack::Engine        bdi;
    jack::ActionBuilder action = bdi.action("Action").commit();
    jack::GoalBuilder   goal   = bdi.goal("Goal")
                                 .commit();
    jack::PlanBuilder plan = bdi.plan("Plan")
                                 .handles(goal)
                                 .body(bdi.coroutine().action(action.name()))
                                 .commit();

    const int    GOAL_COUNT        = 1000;
    int          actionInvokeCount = 0;
    jack::Agent* agent             = bdi.agent("AgentTemplate")
                             .plan(plan)
                             .handleAction(action.name(),
                                           [&actionInvokeCount](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                                               actionInvokeCount++;
                                               return jack::Event::SUCCESS;
                                           })
                             .commitAsAgent()
                             .createAgentInstance("Agent");

    agent->start();
    for (int i = 0; i < GOAL_COUNT; i++) {
        agent->pursue(goal, jack::GoalPersistent_No);
    }

    while (actionInvokeCount != GOAL_COUNT) {
        bdi.poll();
    }
}

TEST(Agents, PerformFailingPlan)
{
    jack::Engine      bdi;
    jack::GoalBuilder goal = bdi.goal("Goal")
                                 .commit();

    jack::ActionBuilder actionA = bdi.action("ActionA").commit();
    jack::ActionBuilder actionB = bdi.action("ActionB").commit();

    jack::PlanBuilder planA = bdi.plan("PlanA")
                                  .handles(goal)
                                  .body(bdi.coroutine()
                                            .action(actionA.name()))
                                  .commit();

    jack::PlanBuilder planB = bdi.plan("PlanB")
                                  .handles(goal)
                                  .body(bdi.coroutine()
                                            .action(actionB.name()))
                                  .commit();

    jack::TacticHandle tactic = bdi.tactic("Tactic")
                                    .goal(goal.name())
                                    .plans(std::array{planA, planB})
                                    .commit();

    int          actionInvokeCountA = 0;
    int          actionInvokeCountB = 0;
    jack::Agent* agent              = bdi.agent("AgentTemplate")
                             .plans(std::array{planA, planB})
                             .handleAction(actionA.name(),
                                           [&actionInvokeCountA](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                                               actionInvokeCountA++;
                                               return jack::Event::FAIL;
                                           })
                             .handleAction(actionB.name(),
                                           [&actionInvokeCountB](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                                               actionInvokeCountB++;
                                               return jack::Event::SUCCESS;
                                           })
                             .commitAsAgent()
                             .createAgentInstance("Agent");
    agent->start();
    for (int i = 0; i < 16; i++) {
        bdi.poll(); /// Startup the engine and get it ticking into a nice state
    }

    agent->selectTactic(tactic.m_name);
    agent->pursue(goal.name(), jack::GoalPersistent_No);
    for (int i = 0; i < 16; i++) {
        bdi.poll(); /// Tick the engine enough to trigger the agent action
    }

    /// \note Both actions should invoke if we have a failing plan policy, i.e.
    /// loop all plans until success.
    EXPECT_EQ(actionInvokeCountA, 1);
    EXPECT_EQ(actionInvokeCountB, 1);
}

TEST(AgentTest, AttachedServices)
{
    tests bdi;

    /**************************************************************************
     * Setup BDI Model
     **************************************************************************/
    jack::GoalBuilder goal = bdi.goal("Goal")
                                 .commit();

    jack::ActionBuilder action = bdi.action("Action")
                                     .commit();

    jack::PlanBuilder plan = bdi.plan("Plan")
                                 .handles(goal)
                                 .body(bdi.coroutine()
                                           .action(action.name()))
                                 .commit();

    bool           serviceActionCalled = false;
    jack::Service* service             = bdi.service("Service Template")
                                 .handleAction(action.name(),
                                               [&](jack::Service&, jack::Message&, jack::Message&, jack::ActionHandle) {
                                                   serviceActionCalled = true;
                                                   return jack::Event::SUCCESS;
                                               })
                                 .commit()
                                 .createInstance("Service", false /*proxy*/);

    jack::Agent* agent = bdi.agent("Agent Template")
                             .plan(plan)
                             .beliefName(DummyFlag::MODEL_NAME)
                             .commitAsAgent()
                             .createAgentInstance("Agent");

    EXPECT_TRUE(agent->attachService(service->handle(), false));

    /**************************************************************************
     * Execute
     **************************************************************************/
    agent->start();
    service->start();
    agent->pursue(goal, jack::GoalPersistent_No);

    for (int iteration = 0; !serviceActionCalled && iteration < 32 /*tries*/; iteration++) {
        bdi.poll();
    }
    EXPECT_TRUE(serviceActionCalled);

    /**************************************************************************
     * Attach/Detach service
     **************************************************************************/
    /// Can't reattach the service that's already connected
    EXPECT_FALSE(agent->attachService(service->handle(), false /*force*/));



    /// \note Force can't reattach either, force only works if we have different
    /// service instances of the same type.
    EXPECT_FALSE(agent->attachService(service->handle(), true /*force*/));

    EXPECT_TRUE(agent->detachService(service->handle()));

    /// \note Double detach must fail as the service has already been detached.
    EXPECT_FALSE(agent->detachService(service->handle()));

    /**************************************************************************
     * Send Message
     **************************************************************************/
    /// Reattach the service
    EXPECT_TRUE(agent->attachService(service->handle(), false /*force*/));

    /// Verify that the flag in the message is false
    {
        const auto agentMsg = agent->messageAsPtr<const DummyFlag>();
        EXPECT_FALSE(agentMsg->flag);
    }

    /// Send the message through the service
    auto msgToSend  = std::make_shared<DummyFlag>();
    msgToSend->flag = true;

    service->sendMessage(msgToSend, false /*broadcastToBus*/, nullptr /*recipient*/);

    /// Advance the engine to make sure the message gets processed
    for (int iteration = 0; iteration < 32 /*tries*/; iteration++) {
        bdi.poll();
    }

    /// Verify that agent received the message and the field changed to true
    {
        const auto agentMsg = agent->messageAsPtr<const DummyFlag>();
        EXPECT_TRUE(agentMsg->flag);
    }
}

TEST(AgentTest, ForceAttachedServices)
{
    tests bdi;

    /**************************************************************************
     * Setup BDI Model
     **************************************************************************/
    jack::GoalBuilder goal = bdi.goal("Goal")
                                 .commit();

    jack::ActionBuilder action = bdi.action("Action")
                                     .commit();

    jack::PlanBuilder plan = bdi.plan("Plan")
                                 .handles(goal)
                                 .body(bdi.coroutine()
                                           .action(action.name()))
                                 .commit();

    jack::ServiceHandle actionInvoker    = {};
    const std::string   SERVICE_TEMPLATE = "Service Template";
    jack::Service*      service          = bdi.service(SERVICE_TEMPLATE)
                                 .handleAction(action.name(),
                                               [&](jack::Service& service, jack::Message&, jack::Message&, jack::ActionHandle) {
                                                   actionInvoker = service.handle();
                                                   return jack::Event::SUCCESS;
                                               })
                                 .commit()
                                 .createInstance("Service", false /*proxy*/);

    jack::Service* altService = bdi.createServiceInstance(SERVICE_TEMPLATE, "Alt Service", false /*proxy*/);
    jack::Agent*   agent      = bdi.agent("Agent Template")
                             .plan(plan)
                             .commitAsAgent()
                             .createAgentInstance("Agent");

    EXPECT_TRUE(agent->attachService(service->handle(), false));

    /**************************************************************************
     * Execute on the Service
     **************************************************************************/
    agent->start();
    service->start();
    altService->start();
    agent->pursue(goal, jack::GoalPersistent_No);

    for (int iteration = 0; !actionInvoker.valid() && iteration < 32 /*tries*/; iteration++) {
        bdi.poll();
    }

    EXPECT_TRUE(actionInvoker.valid());
    EXPECT_EQ(actionInvoker, service->handle());

    /**************************************************************************
     * Attach/Detach service
     **************************************************************************/
    /// \note Duplicate service exists in the service, we can't overwrite it.
    EXPECT_FALSE(agent->attachService(altService->handle(), false /*force*/));

    /// \note We can overwrite it if we set force on
    EXPECT_TRUE(agent->attachService(altService->handle(), true /*force*/));

    /**************************************************************************
     * Execute on the Alt Service
     **************************************************************************/
    /// We deattached the original service and attached the alternative service.
    agent->pursue(goal, jack::GoalPersistent_No);
    actionInvoker = {};

    for (int iteration = 0; !actionInvoker.valid() && iteration < 32 /*tries*/; iteration++) {
        bdi.poll();
    }

    EXPECT_TRUE(actionInvoker.valid());
    EXPECT_EQ(actionInvoker, altService->handle());
}

TEST(Agents, AgentRejectsGoalIfNoPlans)
{
    jack::Engine      bdi;
    jack::GoalBuilder goal = bdi.goal("Goal")
                                 .commit();

    jack::Agent* agent = bdi.agent("AgentTemplate")
                             .commitAsAgent()
                             .createAgentInstance("Agent");
    agent->start();
    for (int i = 0; i < 16; i++) {
        bdi.poll(); /// Startup the engine and get it ticking into a nice state
    }

    agent->pursue(goal.name(), jack::GoalPersistent_Yes);
    for (int i = 0; i < 32; i++) {
        bdi.poll(); /// Tick the engine enough to ensure nothing gets triggered
    }

    EXPECT_EQ(agent->desires(), std::vector<std::string>{});
}
