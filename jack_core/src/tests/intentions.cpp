// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include "bitmaskops.h"

#include <jack/jack.h>
#include <gtest/gtest.h>

#include <vector>
#include <thread>
#include <random>
#include <chrono>

using namespace aos;

typedef std::vector<jack::PlanBuilder> PlanList;

// define some flags for testing
enum PlanCode {
    NoPlan = 0x0,
    Plan1  = 0x1,
    Plan2  = 0x2,
    Plan3  = 0x4
};
ENABLE_BITMASK_OPERATORS(PlanCode);

// The fixture for testing class JACKTest.
class IntentionsTestF : public ::testing::Test {
    protected:
    IntentionsTestF() : planCode(PlanCode::NoPlan)
    {
        bdi.exitWhenDone();
    }

    void SetUp() override {
        // Code here will be called immediately after the constructor (right
        // before each test).
        planFlag = false;

        bdi.action("Plan1Action").commit();
        bdi.action("Plan2Action").commit();
        bdi.action("Plan3Action").commit();
        bdi.action("QuitAction").commit();

        bdi.action("IncreaseValue").commit();
        bdi.action("DecreaseValue").commit();
        bdi.action("ResetValue").commit();

        bdi.action("SetPlan1Value").commit();
        bdi.action("SetPlan2Value").commit();

        bdi.action("Action").commit();

        bdi.action("UnsetPlan1Value").commit();

        bdi.goal("Goal1").commit ();
        bdi.goal("Goal2").commit ();
        bdi.goal("Goal3").commit ();

        plan1Action = [&](jack::Agent &agent, jack::Message &, jack::Message &out, jack::ActionHandle) {
            planCode |= PlanCode::Plan1;
            return jack::Event::SUCCESS;
        };

        plan2Action = [&](jack::Agent &agent, jack::Message &, jack::Message &out, jack::ActionHandle) {
            planCode |= PlanCode::Plan2;
            return jack::Event::SUCCESS;
        };

        plan3Action = [&](jack::Agent &agent, jack::Message &, jack::Message &out, jack::ActionHandle) {
            planCode |= PlanCode::Plan3;
            return jack::Event::SUCCESS;
        };

        quitAction = [&](jack::Agent &agent, jack::Message &, jack::Message &out, jack::ActionHandle) {
            bdi.exit();
            return jack::Event::SUCCESS;
        };

        auto plan1 =
        bdi .plan       ("Plan1")
            .handles    ("Goal1")
            .body       (bdi.coroutine ()
                        .action("Plan1Action")
                        )
            .commit     ();

        auto plan2 =
        bdi.plan    ("Plan2")
        .handles    ("Goal2")
        .body       (bdi.coroutine ()
                        .action("Plan2Action")
                    )
        .commit();

        auto plan3 =
        bdi.plan    ("Plan3")
        .handles    ("Goal3")
        .body       (bdi.coroutine ()
                        .action("Plan3Action")
                    )
        .commit();

        bdi.plan    ("PPlan1")
        .handles    ("Goal1")
        .body       (bdi.coroutine ()
                        .action("PPlan1Action")
                    )
        .commit();

        bdi.plan    ("PPlan2")
        .handles    ("Goal2")
        .body       (bdi.coroutine ()
                        .action("PPlan2Action")
                    )
        .commit();

        bdi.plan("SPlan1")
        .handles ("Goal1")
        .body    (bdi.coroutine ()
                  .action("Plan1Action")
                  .goal("Goal2")
                )
        .commit();

        bdi.plan("SPlan2")
        .handles ("Goal2")
        .body    (bdi.coroutine ()
                   .action("Plan2Action")
                   .goal("Goal3")
                 )
        .commit();

        PlanList plans = { plan1, plan2, plan3 };

        m_planList.swap(plans);
    }

    // Objects declared here can be used by all tests in the test case.
    jack::Engine bdi;

    // plan code is used to verify the correct execution of plans
    PlanCode planCode;

    //used internally for support
    bool planFlag;

    jack::Agent::ActionHandlerFunc plan1Action, plan2Action, plan3Action;
    jack::Agent::ActionHandlerFunc quitAction;

    PlanList m_planList;
};

    /*! ***************************************************************************************
     * Verify that a action return type is pushed into the intention context
     *
     * @todo need to add a test to check that the intention context is cleared of messages
     * between plans selection.  However this is not working so will have to wait for that to work
     *
     * ****************************************************************************************/

//
TEST(ActionOutTest, ActionReturnProp)
{
    /*! ***************************************************************************************
     * Verify that a action return type is pushed into the intention context
     * ****************************************************************************************/

    // create the jack engine
    jack::Engine bdi;
    bdi.exitWhenDone();

    /// Message

    auto numberMsg = bdi.message("Number Message")
                        .field<int32_t>("number")
                        .commit();

    /// actions
    bdi.action("GenerateAction")
       .reply(numberMsg.name())
       .commit();

    bdi.action("SetAction")
       .request(numberMsg.name())
       .commit();

    /// goal
    bdi.goal("FiveGoal").commit ();

    /// plans
    auto planFive = bdi.plan("FivePlan")
        .handles ("FiveGoal")
        .body    (bdi.coroutine ()
                     .action("GenerateAction")
                     .action("SetAction"))
        .commit();

    /// action handlers
    int numberToGenerate = -1;

    auto generateAction = [&](jack::Agent &agent, jack::Message &request, jack::Message &reply, jack::ActionHandle) {
            reply.setFieldValue<int32_t>("number", 5);
            return jack::Event::SUCCESS;
        };

    auto setAction = [&](jack::Agent &agent, jack::Message &request, jack::Message &reply, jack::ActionHandle) {
            // take value from input message
            numberToGenerate = request.get<int32_t>("number");
            return jack::Event::SUCCESS;
        };

    jack::AgentHandle agent5Handle = bdi.agent("TestAgentFive")
                                        .plan(planFive)
                                        .handleAction("GenerateAction", generateAction)
                                        .handleAction("SetAction", setAction)
                                        .commitAsAgent()
                                        .createAgent("agent5");

    // start all agents. agent1 will pursue Goal1
    jack::Agent *agent5 = bdi.getAgent(agent5Handle);
    agent5->start();

    agent5->pursue("FiveGoal", jack::GoalPersistent_Yes).promise->then([&]() {
        bdi.exit();
    });

    bdi.execute();

    // test the result
    EXPECT_EQ(numberToGenerate, 5);
}

TEST(ActionOutTest, DISABLED_SubGoalActionReplyVisibleInParentGoal)
{
    /**************************************************************************
     * Verify that an reply message generated in an action in a subgoal
     * arrives back to the parent goal's context.
     **************************************************************************/
    jack::Engine bdi;
    bdi.exitWhenDone();

    /**************************************************************************
     * Actions
     **************************************************************************/
    const std::string   NUMBER_PARAM  = "number";
    const int32_t       TARGET_NUMBER = 0xF00BAA;


    jack::MessageBuilder numberMsg    = bdi.message("Number Message")
                                           .field<int32_t>(NUMBER_PARAM)
                                           .commit();

    jack::ActionBuilder subGoalAction = bdi.action("Sub Goal Action")
                                           .reply(numberMsg.name())
                                           .commit();

    jack::ActionBuilder parentGoalAction = bdi.action("Parent Goal Action")
                                              .request(numberMsg.name())
                                              .commit();

    /**************************************************************************
     * Goals
     **************************************************************************/
    jack::GoalBuilder goal = bdi.goal("Goal")
                                .commit();

    jack::GoalBuilder subGoal = bdi.goal("SubGoal")
                                   .commit();

    /**************************************************************************
     * Plans
     **************************************************************************/
    auto subGoalPlan = bdi.plan("Sub Goal Plan")
                         .handles(subGoal)
                         .body   (bdi.coroutine()
                                     .action(subGoalAction.name())
                                 )
                         .commit();

    auto parentPlan = bdi.plan("Parent Plan")
                         .handles(goal)
                         .body   (bdi.coroutine()
                                     .goal(subGoal.name())
                                     .action(parentGoalAction.name())
                                 )
                         .commit();


    /**************************************************************************
     * Actions
     **************************************************************************/
    int number = -1;
    jack::Agent *agent = bdi.agent("AgentTemplate")
                            .plans(std::array{parentPlan, subGoalPlan})
                            .handleAction(parentGoalAction.name(),
                            [&](jack::Agent&, jack::Message& request, jack::Message&, jack::ActionHandle) {
                                number = *request.getPtr<int32_t>(NUMBER_PARAM);
                                return jack::Event::SUCCESS;
                            })
                            .handleAction(subGoalAction.name(),
                            [&](jack::Agent&, jack::Message&, jack::Message& reply, jack::ActionHandle) {
                                reply.setFieldValue<int32_t>(NUMBER_PARAM, TARGET_NUMBER);
                                return jack::Event::SUCCESS;
                            })
                            .commitAsAgent()
                            .createAgentInstance("Agent");

    /**************************************************************************
     * Run
     **************************************************************************/
    agent->start();
    agent->pursue(goal, jack::GoalPersistent_Yes).promise->then([&]() {
        bdi.exit();
    });

    bdi.execute();
    EXPECT_EQ(number, TARGET_NUMBER);
}

TEST(ActionOutTest, DISABLED_SubGoalToDelegateActionReplyVisibleInParentGoal)
{
    /**************************************************************************
     * Verify that an reply message generated in an action in a subgoal that is
     * delegated arrives back to the parent goal's context.
     **************************************************************************/
    jack::Engine bdi;
    bdi.exitWhenDone();

    /**************************************************************************
     * Actions
     **************************************************************************/
    const std::string   NUMBER_PARAM  = "number";
    const int32_t       TARGET_NUMBER = 0xF00BAA;

    jack::MessageBuilder numberMsg    = bdi.message("Number Message")
                                           .field<int32_t>(NUMBER_PARAM)
                                           .commit();

    jack::ActionBuilder subGoalAction = bdi.action("Sub Goal Action")
                                           .reply(numberMsg.name())
                                           .commit();

    jack::ActionBuilder parentGoalAction = bdi.action("Parent Goal Action")
                                              .request(numberMsg.name())
                                              .commit();

    /**************************************************************************
     * Goals
     **************************************************************************/
    jack::GoalBuilder goal = bdi.goal("Goal")
                                .commit();

    jack::GoalBuilder subGoal = bdi.goal("SubGoal")
                                   .commit();

    /**************************************************************************
     * Roles
     **************************************************************************/
    auto agentRole = bdi.role("Role")
                        .goal(subGoal)
                        .commit();

    /**************************************************************************
     * Plans
     **************************************************************************/
    auto subGoalPlan = bdi.plan("Sub Goal Plan")
                         .handles(subGoal)
                         .body   (bdi.coroutine()
                                     .action(subGoalAction.name())
                                 )
                         .commit();

    auto parentPlan = bdi.plan("Parent Plan")
                         .handles(goal)
                         .body   (bdi.coroutine()
                                     .goal(subGoal.name())
                                     .action(parentGoalAction.name())
                                 )
                         .commit();


    /**************************************************************************
     * Actions
     **************************************************************************/
    int number = -1;
    auto *team  = bdi.agent("TeamTemplate")
                     .plan(parentPlan)
                     .handleAction(parentGoalAction.name(),
                     [&](jack::Agent&, jack::Message& request, jack::Message&, jack::ActionHandle) {
                         number = *request.getPtr<int32_t>(NUMBER_PARAM);
                         return jack::Event::SUCCESS;
                     })
                     .commitAsTeam()
                     .createTeamInstance("Team");

    jack::Agent *agent = bdi.agent("AgentTemplate")
                            .role(agentRole)
                            .plan(subGoalPlan)
                            .handleAction(subGoalAction.name(),
                            [&](jack::Agent&, jack::Message&, jack::Message& reply, jack::ActionHandle) {
                                reply.setFieldValue<int32_t>(NUMBER_PARAM, TARGET_NUMBER);
                                return jack::Event::SUCCESS;
                            })
                            .commitAsAgent()
                            .createAgentInstance("Agent");

    /**************************************************************************
     * Run
     **************************************************************************/
    team->start();
    agent->start();
    team->addMemberAgent(agent);

    team->pursue(goal, jack::GoalPersistent_Yes).promise->then([&]() {
        bdi.exit();
    });

    bdi.execute();
    EXPECT_EQ(number, TARGET_NUMBER);
}

TEST_F(IntentionsTestF, F41_CorrectPlanSelected)
{
    /*! ***************************************************************************************
     * Verify only the plan matching the given goal is selected
     *
     * F41
     * A plan shall be selected based on the context
     * Performing the action that best suits a given event
     * Ref. C02 C03 P03
     * ****************************************************************************************/
    jack::AgentHandle agent1Handle = bdi.agent("TestAgent1")
                                         .plans(m_planList)
                                         .handleAction("Plan1Action", plan1Action)
                                         .handleAction("Plan2Action", plan2Action)
                                         .handleAction("Plan3Action", plan3Action)
                                         .commitAsAgent()
                                         .createAgent("agent1");

    jack::AgentHandle agent2Handle = bdi.createAgent ("TestAgent1","agent2");

    // start all agents. agent1 will pursue Goal1
    jack::Agent *agent1 = bdi.getAgent(agent1Handle);
    jack::Agent *agent2 = bdi.getAgent(agent2Handle);
    agent1->start();
    agent2->start();

    agent1->pursue("Goal1", jack::GoalPersistent_Yes).promise->then([&]() {
        bdi.exit();
    });
    bdi.execute();

    // test the result
    EXPECT_EQ(planCode, PlanCode::Plan1);

    //reset plan code
    planCode = PlanCode::NoPlan;

    // restart the agents. agent1 will pursue Goal1, agent2 will pursue Goal2
    agent2->start();
    agent2->pursue("Goal2", jack::GoalPersistent_Yes).promise->then([&]() {
        agent2->stop();
    });
    agent1->start();
    agent1->pursue("Goal1", jack::GoalPersistent_Yes).promise->then([&]() {
        agent1->stop();
    });
    bdi.execute();

    // test the result
    EXPECT_EQ(planCode, PlanCode::Plan1 | PlanCode::Plan2);
}

TEST_F(IntentionsTestF, F41_NoPlanSelected_SEGFAULT)
{
    /*! ***************************************************************************************
     *  A Segfault bug - this test should not crash
     * ****************************************************************************************/

    // If the goal and plan are commited it will seg fault
    // maybe the goal can't handle if there is no plan
    // should be a warning at most
    auto goal = bdi.goal ("TestGoal").commit();
    auto plan = bdi.plan ("TestPlan").commit();

    jack::AgentHandle agentHandle = bdi.agent("TestAgent1")
            .plan(plan)
            .commitAsAgent()
            .createAgent("agent1");

    jack::Agent *agent = bdi.getAgent(agentHandle);
    agent->start();
    agent->pursue("TestGoal", jack::GoalPersistent_Yes).promise->then([&]() {
        agent->stop();
    });

    for (int i = 0; i < 100; i++) {
        bdi.poll();
    }

    EXPECT_EQ(planCode, PlanCode::NoPlan);
}

TEST_F(IntentionsTestF, F41_NoPlanSelected)
{
    /*! ***************************************************************************************
     * Verify no plan is selected if a non matching goal is submitted
     *
     * F41
     * A plan shall be selected based on the context
     * Performing the action that best suits a given event
     * Ref. C02 C03 P03
     * ****************************************************************************************/

    auto goal = bdi.goal ("TestGoal");
    auto plan = bdi.plan ("TestPlan");

    jack::AgentHandle agentHandle = bdi.agent("TestAgent1")
            .plan(plan)
            .commitAsAgent()
            .createAgent("agent1");

    jack::Agent *agent = bdi.getAgent(agentHandle);
    agent->start();
    agent->pursue("TestGoal", jack::GoalPersistent_Yes);

    for (int i = 0; i < 10; i++) {
        bdi.poll();
    }

    EXPECT_EQ(planCode, PlanCode::NoPlan);
}

TEST_F(IntentionsTestF, ClimbStairs)
{
    /*! ***************************************************************************************
     * Test the conditional task in a plan
     * ****************************************************************************************/

    int current_step = 0;
    int num_steps = 10;
    int labelA = -1;

    bdi.action("ClimbStep").commit();

    auto climbStairsGoal = bdi.goal("ClimbStairs")
                            .satisfied ([&](const jack::BeliefContext &context) { return current_step >= num_steps; })
                            .commit();

    auto increaseValuePlan = bdi.plan ("ClimbStairs")
            .handles    (climbStairsGoal)
            .body       (bdi.coroutine ()
                            .print("starting plan\n")
                            .action("ClimbStep").label(labelA)
                            .cond([&](const jack::BeliefContext& context) {
                                return current_step >= num_steps;
                            }).onFail(labelA)
                            .print("ending plan\n")
                        )
            .commit();

    jack::AgentHandle agentHandle =
        bdi.agent("StairClimbingAgent")
            .plan(increaseValuePlan)
            .handleAction("ClimbStep",
                          [&](jack::Agent &agent, jack::Message &, jack::Message &out, jack::ActionHandle) {
                              current_step++;
                              std::cout << "Action: climb step =" << current_step << std::endl;
                              return jack::Event::SUCCESS;
                          })
            .commitAsAgent()
            .createAgent("agent1");

    jack::Agent *agent = bdi.getAgent(agentHandle);
    agent->start();
    agent->pursue(climbStairsGoal, jack::GoalPersistent_Yes);

    for (int i = 0; i < 1000; i++) {
        bdi.poll();
    }

    EXPECT_EQ (current_step, num_steps);
}


TEST_F(IntentionsTestF, F41_Precondition)
{
    /*! ***************************************************************************************
     * Verify if the intention is selected based on the precondition
     *
     * F41
     * A plan shall be selected based on the context
     * Performing the action that best suits a given event
     * Ref. C02 C03 P03
     * ****************************************************************************************/

    auto checkValueGoal = bdi.goal("CheckValue").commit();
    auto recheckValueGoal = bdi.goal("ReCheckValue").commit();
    auto startGoal = bdi.goal("StartGoal").commit();

    int value = 0;
    auto increaseValuePlan = bdi.plan ("IncreaseValuePlan")
            .handles    (checkValueGoal)
            .pre        ([&](const jack::BeliefContext&) { return 0 == value; })
            .body       (bdi.coroutine ()
                         .print("increasing value\n")
                         .action("IncreaseValue")
                         )
            .commit();

    auto increaseMoreValuePlan = bdi.plan ("IncreaseMoreValuePlan")
            .handles    (recheckValueGoal)
            .pre        ([&](const jack::BeliefContext&) { return 0 >= value; })
            .body           (bdi.coroutine ()
                             .print("decreasing value\n")
                             .action("DecreaseValue")
                             )
            .commit();

    auto resetValuePlan = bdi.plan ("ResetValuePlan")
            .handles    (recheckValueGoal)
            //this will never be selected
            .pre        ([&](const jack::BeliefContext&) { return false; })
            .body           (bdi.coroutine ()
                             .print("resetting value\n")
                             .action("ResetValue")
                             )
            .commit();

    auto startPlan = bdi.plan ("StartPlan")
            .handles    (startGoal)
            .body           (bdi.coroutine ()
                                .goal("CheckValue")   //1
                                .goal("ReCheckValue") //not executed
                             )
            .commit();

    jack::AgentHandle agentHandle =
        bdi.agent("TestAgent1")
            .plans(std::array{increaseValuePlan, increaseMoreValuePlan, resetValuePlan, startPlan})
            .handleAction("IncreaseValue",
                          [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle) {
                              ++value;
                              std::cout << "value=" << value << std::endl;
                              return jack::Event::SUCCESS;
                          })
            .handleAction("DecreaseValue",
                          [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle) {
                              --value;
                              std::cout << "value=" << value << std::endl;
                              return jack::Event::SUCCESS;
                          })
            .handleAction("ResetValue",
                          [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle) {
                              value = 0;
                              std::cout << "value reset=" << value << std::endl;
                              return jack::Event::SUCCESS;
                          })
            .commitAsAgent()
            .createAgent("agent1");

    jack::Agent *agent = bdi.getAgent(agentHandle);
    agent->start();
    agent->pursue(startGoal, jack::GoalPersistent_Yes);

    for (int i = 0; i < 100; i++) {
        bdi.poll();
    }

    EXPECT_EQ (value, 1);
}

TEST_F(IntentionsTestF, F42_RepeatIntention)
{
    /*! ***************************************************************************************
     * Pursuing the same goal more times will generate different intentions.
     * All the goals and relative intentions should be executed as issued by the coroutine.
     *
     * F42
     * It shall be possible to run intentions in parallel or exclusively
     * A maximum number of parallel intentions could be specified .
     * Ref. C12 C21 C25 C26 C31 V01 V03 V13 V23 P02 P04
     * ****************************************************************************************/

    int value = 0;
    auto increaseValueGoal = bdi.goal("IncreaseValue")
                                 .pre([&](const jack::BeliefContext &) { return value < 11; })
                                 .commit();

    auto increaseValuePlan = bdi.plan("IncreaseValuePlan")
                                 .handles(increaseValueGoal)
                                 .body(bdi.coroutine()
                                           .action("IncreaseValue"))
                                 .commit();

    jack::AgentHandle agentHandle =
        bdi.agent("TestAgent1")
            .plan(increaseValuePlan)
            .handleAction("IncreaseValue",
                          [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle) {
                              ++value;
                              std::cout << "value = " << value << std::endl;
                              return jack::Event::SUCCESS;
                          })
            .commitAsAgent()
            .createAgent("agent1");

    jack::Agent *agent = bdi.getAgent(agentHandle);
    agent->start();
    for (int i = 0; i < 100; i++) {
        bdi.poll();
    }
    EXPECT_EQ(value, 0);
    EXPECT_TRUE(agent->running());

    agent->stop();
    agent->start();

    for (int i = 0; i < 100; i++) {
        bdi.poll();
    }
    EXPECT_EQ(value, 0);
    EXPECT_TRUE(agent->running());

    // Need to force the events through because the above events will clear the
    // pursue goals queue. this is because they are in a different queue at the
    // moment
    agent->stop();
    agent->start();
    bdi.poll();
    EXPECT_TRUE(agent->running());

    agent->pursue("IncreaseValue", jack::GoalPersistent_No);
    agent->pursue("IncreaseValue", jack::GoalPersistent_No);
    agent->pursue("IncreaseValue", jack::GoalPersistent_No);
    agent->pursue("IncreaseValue", jack::GoalPersistent_No);

    for (int i = 0; i < 100; i++) {
        bdi.poll();
    }
    EXPECT_EQ(value, 4);
}

TEST_F(IntentionsTestF, F42_QueuedExecution)
{
    /*! ***************************************************************************************
     * Verify three goals are correctly enqueued for execution
     *
     * F42
     * It shall be possible to run intentions in parallel or exclusively
     * A maximum number of parallel intentions could be specified .
     * Ref. C12 C21 C25 C26 C31 V01 V03 V13 V23 P02 P04
     *
     * ****************************************************************************************/
    jack::AgentHandle agentHandle = bdi.agent("TestAgent1")
                                        .plans(m_planList)
                                        .handleAction("Plan1Action", plan1Action)
                                        .handleAction("Plan2Action", plan2Action)
                                        .handleAction("Plan3Action", plan3Action)
                                        .commitAsAgent()
                                        .createAgent("agent1");

    // start the agent and perform the goals
    jack::Agent *agent = bdi.getAgent(agentHandle);
    agent->start();
    agent->pursue("Goal1", jack::GoalPersistent_No);
    agent->pursue("Goal2", jack::GoalPersistent_No);
    agent->pursue("Goal3", jack::GoalPersistent_No);

    // Make sure that all the intentions are being executed at the same time
    // we check after every tick of the bdi engine
    bool done = false;
    while (!done) {
        // tick once
        bdi.poll();

        // check if any plans have finished
        if (planCode != PlanCode::NoPlan) {

            // they should all be done at the same time
            EXPECT_EQ(planCode, PlanCode::Plan1 | PlanCode::Plan2 | PlanCode::Plan3);
            done = true;
        }
    }
}

TEST_F(IntentionsTestF, InversePreconditionPlans)
{
    /**************************************************************************
     * - Create an agent with 2 plans and a goal to handle it.
     * - FirstPlan  is valid when count <  2, on success count += 2
     * - SecondPlan is valid when count >= 2, on success count += 3
     * - Agent pursues the goal and executes FirstPlan
     * - The goal should succeed because there is no satisfied condition on the
     *   goal
     * - The value of count must be 2
     **************************************************************************/

    jack::Engine bdi("Engine");

    bdi.exitWhenDone();

    auto action = bdi.action("Action").commit();
    auto altAction = bdi.action("AltAction").commit();

    auto bs = bdi.message("CountBelief")
                 .fieldWithValue<uint32_t>("count", 0)
                 .commit();

    auto commonGoal = bdi.goal("CommonGoal").commit();

    auto plan1 = bdi.plan("FirstPlan")
            .handles(commonGoal)
            .body(bdi.coroutine()
                     .action(action.name()))
            .pre([](const jack::BeliefContext &context) {
                std::cout << "Plan 1 precondition evaluated" << std::endl;
                auto     bs    = context.message("CountBelief");
                uint32_t count = *bs->getPtr<uint32_t>("count");
                return count < 2;
            })
            .commit();

    auto plan2 = bdi.plan("SecondPlan")
            .handles(commonGoal)
            .body(bdi.coroutine()
                     .action(altAction.name()))
            .pre([](const jack::BeliefContext &context) {
                std::cout << "Plan 2 precondition evaluated" << std::endl;

                auto bs    = context.message("CountBelief");
                auto count = *bs->getPtr<uint32_t>("count");
                return count >= 2;
            })
            .commit();

    auto actionFunc =
        [](jack::Agent& agent, jack::Message&, jack::Message& out, jack::ActionHandle) {
            jack::Message countBelief = *agent.context().message("CountBelief");
            uint32_t      count       = *countBelief.getPtr<uint32_t>("count");
            countBelief.setFieldValue<uint32_t>("count", count + 2);
            agent.sendMessage(std::move(countBelief), false /*broadcastToBus*/);
            return jack::Event::Status::SUCCESS;
        };

    auto altActionFunc =
        [](jack::Agent& agent, jack::Message&, jack::Message& out, jack::ActionHandle) {
            jack::Message countBelief = *agent.context().message("CountBelief");
            uint32_t      count       = *countBelief.getPtr<uint32_t>("count");
            countBelief.setFieldValue<uint32_t>("count", count + 3);
            agent.sendMessage(std::move(countBelief), false /*broadcastToBus*/);
            return jack::Event::Status::SUCCESS;
        };

    jack::AgentHandle agentHandle = bdi.agent("Agent")
                                        .belief(bs)
                                        .plans(std::array{plan1, plan2})
                                        .handleAction(action.name(), actionFunc)
                                        .handleAction(altAction.name(), altActionFunc)
                                        .commitAsAgent()
                                        .createAgent("agent");

    jack::Agent *agent = bdi.getAgent(agentHandle);
    agent->start();

    bool promiseExecuted = false;
    auto goalPursue = agent->pursue(commonGoal, jack::GoalPersistent_No);
    goalPursue.promise->then([&]() {
        auto     msg    = agent->context().message(bs.name());
        uint32_t count  = *msg->getPtr<uint32_t>("count");
        promiseExecuted = true;
        EXPECT_EQ(count, 2);
    });

    for (int i = 0; !promiseExecuted && i < 100; i++) {
        bdi.poll();
    }

    EXPECT_TRUE(promiseExecuted);
}


// same as InversePreconditionPlans
// but with a satisfied condition on the goal
// also the FirstPlan has an extra task so that it doesn't finish after the first
// action is executed

TEST_F(IntentionsTestF, InversePreconditionPlansSatisfiedGoal)
{
    jack::Engine bdi("Engine");

    bdi.exitWhenDone();

    auto action = bdi.action("Action").commit();
    auto altAction = bdi.action("AltAction").commit();

    auto bs = bdi.message("CountBelief")
                 .fieldWithValue<uint32_t>("count", 0)
                 .commit();

    auto commonGoal = bdi.goal("CommonGoal")
                          .satisfied([&](const jack::BeliefContext& context) {
                              auto bs    = context.message("CountBelief");
                              auto count = *bs->getPtr<uint32_t>("count");
                              return count >= 2;
                          })
                          .commit();

    auto plan1 = bdi.plan("FirstPlan")
            .handles(commonGoal)
            .body(bdi.coroutine().action(action.name()).print("last task"))
            .pre([](const jack::BeliefContext &context) {
                std::cout << "Plan 1 precondition evaluated" << std::endl;

                auto bs    = context.message("CountBelief");
                auto count = *bs->getPtr<uint32_t>("count");
                return count < 2;
            })
            .commit();

    auto plan2 = bdi.plan("SecondPlan")
            .handles(commonGoal)
            .body(bdi.coroutine().action(altAction.name()))
            .pre([](const jack::BeliefContext &context) {
                std::cout << "Plan 2 precondition evaluated" << std::endl;

                auto bs    = context.message("CountBelief");
                auto count = *bs->getPtr<uint32_t>("count");
                return count >= 2;
            })
            .commit();

    auto actionFunc =
        [](jack::Agent& agent, jack::Message&, jack::Message& out, jack::ActionHandle) {
            auto     bs    = agent.context().message("CountBelief");
            uint32_t count = *bs->getPtr<uint32_t>("count");
            bs->setFieldValue<uint32_t>("count", count + 2);
            return jack::Event::Status::SUCCESS;
        };

    auto altActionFunc =
        [](jack::Agent& agent, jack::Message&, jack::Message& out, jack::ActionHandle) {
            auto bs    = agent.context().message("CountBelief");
            uint32_t count = *bs->getPtr<uint32_t>("count");
            bs->setFieldValue<uint32_t>("count", count + 3);
            return jack::Event::Status::SUCCESS;
        };

    jack::AgentHandle agentHandle = bdi.agent("Agent")
                                        .belief(bs)
                                        .plans(std::array{plan1, plan2})
                                        .handleAction(action.name(), actionFunc)
                                        .handleAction(altAction.name(), altActionFunc)
                                        .commitAsAgent()
                                        .createAgent("agent");

    jack::Agent *agent = bdi.getAgent(agentHandle);
    agent->start();

    auto goalPursue = agent->pursue(commonGoal, jack::GoalPersistent_No);
    goalPursue.promise->then([&]() {
        auto bs    = agent->context().message("CountBelief");
        auto count = *bs->getPtr<uint32_t>("count");
        EXPECT_EQ(count, 2);
    });

    for (int i = 0; i < 100; i++) {
        bdi.poll();
    }
}

TEST_F(IntentionsTestF, TeamInversePreconditionPlans)
{
    jack::Engine bdi("Engine");

    bdi.exitWhenDone();

    auto action = bdi.action("Action").commit();
    auto altAction = bdi.action("AltAction").commit();

    auto bs = bdi.message("CountBelief")
                 .fieldWithValue<uint32_t>("count", 0)
                 .commit();

    auto teamGoal = bdi.goal("TeamGoal").commit();
    auto commonGoal = bdi.goal("CommonGoal").commit();

    auto plan1 = bdi.plan("FirstPlan")
                     .handles(commonGoal)
                     .body(bdi.coroutine().action(action.name()))
                     .pre([](const jack::BeliefContext &context) {
                         std::cout << "Plan 1 precondition evaluated" << std::endl;

                         auto bs    = context.message("CountBelief");
                         auto count = *bs->getPtr<uint32_t>("count");
                         return count < 2;
                     })
                     .commit();

    auto plan2 = bdi.plan("SecondPlan")
                     .handles(commonGoal)
                     .body(bdi.coroutine().action(altAction.name()))
                     .pre([](const jack::BeliefContext &context) {
                         std::cout << "Plan 2 precondition evaluated" << std::endl;

                         auto bs    = context.message("CountBelief");
                         auto count = *bs->getPtr<uint32_t>("count");
                         return count >= 2;
                     })
                     .commit();

    auto teamPlan = bdi.plan("TeamPlan")
                        .handles(teamGoal)
                        .body(bdi.coroutine()
                                 .goal(commonGoal.name()))
                        .commit();

    auto agentRole = bdi.role("AgentRole")
                    .goal(commonGoal)
                    .commit();

    auto actionFunc =
        [](jack::Agent& agent, jack::Message&, jack::Message& out, jack::ActionHandle) {
            jack::Message countBelief = *agent.context().message("CountBelief");
            uint32_t      count       = *countBelief.getPtr<uint32_t>("count");
            countBelief.setFieldValue<uint32_t>("count", count + 2);
            agent.sendMessage(std::move(countBelief), false /*broadcastToBus*/);
            return jack::Event::Status::SUCCESS;
        };

    auto altActionFunc =
        [](jack::Agent& agent, jack::Message&, jack::Message& out, jack::ActionHandle) {
            jack::Message countBelief = *agent.context().message("CountBelief");
            uint32_t      count       = *countBelief.getPtr<uint32_t>("count");
            countBelief.setFieldValue<uint32_t>("count", count + 3);
            agent.sendMessage(std::move(countBelief), false /*broadcastToBus*/);
            return jack::Event::Status::SUCCESS;
        };

    jack::AgentHandle agentHandle = bdi.agent("Agent")
                                        .belief(bs)
                                        .plans(std::array{plan1, plan2})
                                        .role(agentRole)
                                        .handleAction(action.name(), actionFunc)
                                        .handleAction(altAction.name(), altActionFunc)
                                        .commitAsAgent()
                                        .createAgent("agent");

    jack::AgentHandle teamHandle = bdi.agent("Team")
            .plan(teamPlan)
            .commitAsTeam()
            .createTeam("team");

    jack::Agent *agent = bdi.getAgent(agentHandle);
    auto *       team  = static_cast<jack::Team *>(bdi.getAgent(teamHandle));
    team->addMemberAgent(agent);

    agent->start();
    team->start();

    auto goalPursue = team->pursue(teamGoal, jack::GoalPersistent_No);
    goalPursue.promise->then([&]() {
        auto bs    = agent->context().message("CountBelief");
        auto count = *bs->getPtr<uint32_t>("count");
        EXPECT_EQ(count, 2);
    });

    for (int i = 0; i < 100; i++) {
        bdi.poll();
    }
}

#if 0
TEST_F(IntentionsTestF, F42_ParallelExecution)
{
    /*! ***************************************************************************************
     * Verify two goals are queued and executed in parallel
     *
     * F42
     * It shall be possible to run intentions in parallel or exclusively
     * A maximum number of parallel intentions could be specified .
     * Ref. C12 C21 C25 C26 C31 V01 V03 V13 V23 P02 P04
     * ****************************************************************************************/
    bdi.goal("IncreaseGoal").commit ();
    bdi.goal("DecreaseGoal").commit ();

    //TODO: add a wait for value in the plan
    int value = 0;
    auto increasePlan = bdi.plan("IncreaseValuePlan")
            .handles    ("IncreaseGoal")
            .body       (bdi.coroutine ()
                         .action("SetPlan1Value")
                         .action("IncreaseValue")
                         .action("IncreaseValue")
                         .action("IncreaseValue")
                         .action("IncreaseValue")
                         .action("IncreaseValue")
                         .action("IncreaseValue")
                         .action("IncreaseValue")
                         .action("IncreaseValue")
                         )
            .commit();

    auto decreasePlan = bdi.plan   ("DecreaseValuePlan")
            .handles    ("DecreaseGoal")
            .body       (bdi.coroutine ()
                         .action("SetPlan2Value")
                         .action("DecreaseValue")
                         .action("DecreaseValue")
                         .action("DecreaseValue")
                         .action("DecreaseValue")
                         .action("DecreaseValue")
                         .action("DecreaseValue")
                         .action("DecreaseValue")
                         .action("DecreaseValue")
                         )
            .commit();
    jack::Agent* agent = bdi.agent("TestAgent1")
            .plans  ({ increasePlan, decreasePlan })
            .handleAction("IncreaseValue",
                          [&](jack::Agent& agent, jack::ActionEvent &action) {
                              ++value;
                              return jack::Event::SUCCESS;
                          })
            .handleAction("DecreaseValue",
                          [&](jack::Agent& agent, jack::ActionEvent &action) {
                                //only decrease the value if it's less than one
                                //this will fail if the increase plan is called more
                                //TODO: refactor this when the wait for will be implemented
                              if(1<=value) {
                                  --value;
                              }
                              return jack::Event::SUCCESS;
                          })
            .handleAction("SetPlan1Value",
                          [&] (jack::Agent& agent, jack::ActionEvent &action) {
                                planCode |= PlanCode::Plan1;
                                return jack::Event::SUCCESS;
                         })
            .handleAction("SetPlan2Value",
                        [&] (jack::Agent& agent, jack::ActionEvent &action) {
                            planCode |= PlanCode::Plan2;
                            return jack::Event::SUCCESS;
                        })
            .create("agent1");

    agent->start();
    agent->pursue("IncreaseGoal");
    agent->pursue("DecreaseGoal");

    bdi.execute();

    EXPECT_EQ(0, value);
    EXPECT_EQ(planCode, PlanCode::Plan1 | PlanCode::Plan2);
}

#if 0
/// \todo JC - Disabled test since I'm not sure what it should be doing
TEST_F(IntentionsTestF, F43_UniqueInstance)
{
    /*! ***************************************************************************************
     * Verify if the specific intention is executed only once.
     * Further intentions are dropped until the initial one is finished.
     * In this version of the test I'm using a precondition to prevent intentions
     * to be started until the last action of the plan is invoked.
     *
     * F43
     * It shall be possible to specify if an intention will run exclusively on different
     * parameters.
     * For example, execute two intentions IA and IB running in parallel, respectively one for
     * parameter A, one for B. The next intention on A is dropped since IA is still working on it.
     * Ref. C22 V02 F22
     * ****************************************************************************************/
    bdi.goal("UniqueGoal").commit ();

    int value = 0;
    auto increasePlan = bdi.plan("UniquePlan")
            .handles    ("UniqueGoal")
            .pre        ([&](const jack::BeliefContext &){ return planCode != Plan1; })
            .body       (bdi.coroutine ()
                         .action("SetPlan1Value")
                         .print("waiting\n")
                         .sleep (2)
                         .action("UnsetPlan1Value")
                         )
            .commit();

    jack::Agent* agent = bdi.agent("TestAgent1")
            .plans  ({ increasePlan })
            .main(bdi.coroutine ()
                  .goal ("UniqueGoal").nowait () //1
                  .sleep (1)
                  .goal ("UniqueGoal").nowait () //2
                  .sleep (1)
                  .goal ("UniqueGoal").nowait () //3
                  .sleep (1)
                  .goal ("UniqueGoal").nowait () //4
                  .sleep (1)
                  .goal ("UniqueGoal").nowait () //5
                  .sleep (1)
                  .goal ("UniqueGoal").nowait () //6
                  .sleep (1)
                  .goal ("UniqueGoal").nowait () //7
                  .sleep (1)
                  .goal ("UniqueGoal").nowait () //8
                  .sleep (1)
                  .goal ("UniqueGoal").nowait () //9
                  .sleep (1)
                  .goal ("UniqueGoal").nowait () //10
                  .sleep (1)
                  .yieldUntil ([&](const jack::BeliefContext &){
                        //std::cout << "value = " << value << std::endl;
                        return value > 9;
                    } )
                  )
            .handleAction("SetPlan1Value",
                          [&] (jack::Agent& agent, jack::ActionEvent &action) {
                                ++value;
                                planCode |= PlanCode::Plan1;
                                std::cout << "Plan1Value value=" << value << std::endl;
                                return jack::Event::SUCCESS;
                         })
            .handleAction("UnsetPlan1Value",
                        [&] (jack::Agent& agent, jack::ActionEvent &action) {
                            planCode = PlanCode::NoPlan;
                            std::cout << "NoPlanValue" << std::endl;
                            return jack::Event::SUCCESS;
                        })
            .create("agent1");

    agent->start();

    bdi.execute();

    EXPECT_EQ(planCode, PlanCode::NoPlan);
    EXPECT_EQ(value, 10);
}
#endif

TEST_F(IntentionsTestF, F44_DropWhenCondition)
{
    /*! ***************************************************************************************
     * Verify if the intention is dropped when a specific condition is met
     *
     * F44
     * It shall be possible to specify if and when an intention is interruptible
     * Atomic steps for critical sections or transactions.
     * An entire intention could potentially be uninterruptible: it can only finish.
     * Ref. C22 V02 F22
     * ****************************************************************************************/
    auto checkValueGoal = bdi.goal("CheckValue").commit();
    int value = 0;
    const int testValue = 7;
    auto interruptablePlan = bdi.plan ("ResetValuePlan")
            .handles    (checkValueGoal)
            .dropWhen   ([&](const jack::BeliefContext&) { return value >= testValue; })
            .body       (bdi.coroutine ()
                         .print("Increasing value\n")
                         .action("IncreaseValue") //1
                         .action("IncreaseValue") //2
                         .action("IncreaseValue") //3
                         .action("IncreaseValue") //4
                         .action("IncreaseValue") //5
                         .action("IncreaseValue") //6
                         .action("IncreaseValue") //7  <- drop here please
                         .action("IncreaseValue") //8
                         .action("IncreaseValue") //9
                         .action("IncreaseValue") //10
                         .action("IncreaseValue") //11
                         .action("IncreaseValue") //12
                         .action("IncreaseValue") //13
                         .action("ResetValue")    //0
                         )
            .commit();

    jack::Agent* agent = bdi.agent("TestAgent1")
            .plans  ({ interruptablePlan })
            .main   (bdi.coroutine ()
                        .print ("Starting agent1\n")
                        .goal ("CheckValue") //1
                     )
            .handleAction("IncreaseValue",
                  [&](jack::Agent& agent, jack::ActionEvent &action) {
                      ++value;
                      std::cout << "value=" << value << std::endl;
                      return jack::Event::SUCCESS;
                  })
            .handleAction("ResetValue",
                  [&](jack::Agent& agent, jack::ActionEvent &action) {
                      value=0;
                      std::cout << "value reset=" << value << std::endl;
                      return jack::Event::SUCCESS;
                  })
            .create("agent1");

    agent->start();

    bdi.execute();

    EXPECT_EQ (value, testValue);
}

TEST_F(IntentionsTestF, F45_NeverendingIntention)
{
    /*! ***************************************************************************************
     * Verify if an intention is allowed to never end and requires a specific drop.
     *
     * F45
     * An intention shall possibly run indefinitely
     * Such intention would eventually have to be dropped explicitly
     * (either internally or by an event level call)
     * Ref. C35 P02 P11 P14
     * ****************************************************************************************/
    int value = 0;
    bdi.goal("NeverendingGoal").commit ();

    auto workPlan = bdi.plan("WorkPlan")
            .handles    ("NeverendingGoal")
            .body       (bdi.coroutine ()
                         .yieldUntil  ([&] (const jack::BeliefContext &agent) {
                            ++value;
                            return false; })
                         )
            .commit();

    jack::Agent* agent1 = bdi.agent("TestAgent1")
            .plans  ({ workPlan })
            .main(bdi.coroutine ()
                  .goal ("NeverendingGoal")
                  )
            .handleAction("Neverend",
                          [&] (jack::Agent& agent, jack::ActionEvent &action) {
                                value = -1;
                                planCode |= PlanCode::Plan1;
                                std::cout << "Decrease value" << std::endl;
                                return jack::Event::SUCCESS;
                         })
            .create("agent1");

    agent1->start();

    bdi.poll(1000);

    EXPECT_FALSE (agent1->finished ());
    agent1->drop ("NeverendingGoal");
    bdi.execute ();
    EXPECT_TRUE (agent1->finished ());

    std::cout << "worked to " << value << std::endl;
}

TEST_F(IntentionsTestF, F46_YieldUntil)
{
    /*! ***************************************************************************************
     * Pause the intention execution until a given condition is met
     *
     * F46
     * It shall be possible to pause (interrupt) the execution of an intention for a given
     * amount of time or until a condition is verified.
     * The intention execution will not proceed until specified
     * Ref. C27 C28 V03 P14
     * ****************************************************************************************/
    int value = 0;
    int testValue = 3;
    auto waitUntilPlan = bdi.plan ("WaitUntilPlan")
        .handles    ("Goal1")
        .body       (bdi.coroutine ()
                        .action("IncreaseValue") //1
                        .action("IncreaseValue") //2
                        .action("IncreaseValue") //3
                        .action("IncreaseValue") //4
                        .action("IncreaseValue") //5
                        .action("IncreaseValue") //6
                        .action("IncreaseValue") //7
                        .action("IncreaseValue") //8
                        .action("IncreaseValue") //9
                        .action("IncreaseValue") //10
                    )
        .commit     ();

    jack::Agent* agent = bdi.agent("TestAgent1")
        .plans          ({ waitUntilPlan })
        .main           (bdi.coroutine ()
                            .goal ("Goal1").nowait()
                            .yieldUntil ([&](const jack::BeliefContext &) { return value >= testValue; })
                            .drop("Goal1")
                        )
        .handleAction   ("IncreaseValue", [&](jack::Agent&, jack::ActionEvent &action) { ++value; return jack::Event::SUCCESS;})
        .create         ("agent1");

    agent->start();

    bdi.execute();

    // make sure we did yield until testValue
    EXPECT_GE (value, testValue);

    // make sure the goal was droppen
    EXPECT_LT (value, 10);
}

TEST_F(IntentionsTestF, F47_GoalSurvival)
{
    /*! ***************************************************************************************
     * Verify if a goal can survive an intention drop
     *
     * F47
     * It shall be possible to drop an intention.
     * The intention execution will not proceed and predefined trigger will be executed to
     * ensure all the allocated resources are released. The invoking event will survive and
     * could potentially instantiate a new intention.
     * Ref. C35 P02 P11 P14
     * ****************************************************************************************/
    int value = 0;
    auto goal = bdi.goal("InsistGoal")
            //expect the goal to keep instanciating intentions as long as necessary
            .satisfied ([&](const jack::BeliefContext &context){
                std::cout << "check value: " << value << std::endl;
                return value > 23;
            })
            .commit ();

    auto sleepPlan = bdi.plan ("SleepPlan")
        .handles    ("InsistGoal")
        .body       (bdi.coroutine ()
                     .print("Working on value\n")
                     .sleep(10)
                     .action("IncreaseValue")
                     //intention is over!
                    )
        .commit     ();

    jack::Agent* agent = bdi.agent("TestAgent1")
        .plans        ({ sleepPlan })
        .main         (bdi.coroutine()
                            .goal ("InsistGoal")
                            .print  ("Goal finished\n")
                      )
        .handleAction   ("IncreaseValue", [&](jack::Agent&, jack::ActionEvent &action) {
            ++value;
            std::cout << "increased value to " << value << std::endl;
            return jack::Event::SUCCESS;
            })
        .create       ("agent1");

    agent->start();

    bdi.execute ();

    EXPECT_TRUE(agent->finished ());
}

TEST_F(IntentionsTestF, F48_IntentionDroppingItself)
{
    /*! ***************************************************************************************
     * Verify if an intention is dropped from within its implementing coroutine.
     *
     * F48
     * An intention shall be able to drop itself.
     * Dropping the intention from within the intention.
     * Ref. V07
     * ****************************************************************************************/
    auto checkValueGoal = bdi.goal("CheckValue").commit();
    int value = 0;
    const int testValue = 2;
    auto interruptablePlan = bdi.plan ("ResetValuePlan")
        .handles    (checkValueGoal)
        .body       (bdi.coroutine ()
                        .action("IncreaseValue") //1
                        .action("IncreaseValue") //2
                        .action("ResetValue")    //0
                    )
        .commit();

    jack::Agent* agent = bdi.agent("TestAgent1")
            .plans  ({ interruptablePlan })
            .main   (bdi.coroutine ()
                        .print ("Starting agent1\n")
                        .goal ("CheckValue") //1
                     )
            .handleAction("IncreaseValue",
                  [&](jack::Agent& agent, jack::ActionEvent &action) {
                      ++value;
                      std::cout << "value=" << value << std::endl;
                      if(testValue == value) {
                        //I just want the intention to quit gracefully
                        //TODO: add an $plan.success() or $plan.fail()
                        agent.drop ("CheckValue");
                      }
                      return jack::Event::SUCCESS;
                  })
            .handleAction("ResetValue",
                  [&](jack::Agent& agent, jack::ActionEvent &action) {
                      value=0;
                      std::cout << "value reset=" << value << std::endl;
                      return jack::Event::SUCCESS;
                  })
            .create("agent1");

    agent->start();

    bdi.execute();

    EXPECT_EQ (value, testValue);
}

TEST_F(IntentionsTestF, F49_IntentionFinishedEvent)
{
    /*! ***************************************************************************************
     * Verify if an intention is signalling when finished
     *
     * F49
     * Intentions shall indicate when they have finished
     * This signal should be observable, at least internally
     * Ref. C29 P24
     * ****************************************************************************************/
    int value = 0;
    auto goal = bdi.goal("Goal").commit ();

    auto sleepPlan = bdi.plan ("Plan")
        .handles    ("Goal")
        .body       (bdi.coroutine ()
                     .print("Plan is ")
                     .action("Action")
                     //trigger that intention is over (internally)
                    )
        .commit     ();

    jack::Agent* agent = bdi.agent("Agent")
        .plans        ({ sleepPlan })
        .main         (bdi.coroutine()
                            .goal ("Goal")
                      )
        .handleAction   ("Action", [&](jack::Agent&, jack::ActionEvent &action) {
            std::cout << "working" << std::endl;
            ++value;
            return jack::Event::SUCCESS;
            })
        .create       ("agent1");

    agent->start();

    bdi.poll (100);

    EXPECT_TRUE(agent->finished ());
}
#endif

TEST(IntentionsTest, DropDoesNotRestartFinishedExecutor)
{
    /**************************************************************************
     * - Create an agent with 2 plans whose preconditions are the opposite of
     *   each other
     * - Execute Plan A which runs an action that makes the preconditions
     *   opposite
     * - Before the executor for Plan A is finished dirty the schedule
     * - The schedule is dirty, new schedule is planned and merged into the
     *   executor.
     * - Executor should be in a transition mode (i.e. DROPPED), however the
     *   executor should still proceed to finish because Plan A was finished
     *   (there's only 1 action to execute in Plan A).
     * - Executor should go away, Plan B is never executed because the goal was
     *   achieved.
     **************************************************************************/

    /// \note Build the model
    jack::Engine bdi;
    jack::GoalBuilder goal = bdi.goal("Goal")
                                .commit();

    const std::string FORCE_DIRTY_BELIEF = "ForceDirty";
    jack::MessageBuilder beliefs = bdi.message("Beliefs")
                                      .fieldWithValue<bool>(FORCE_DIRTY_BELIEF, false)
                                      .commit();

    jack::ActionBuilder planAAction = bdi.action("PlanAAction").commit();
    jack::ActionBuilder planBAction = bdi.action("PlanBAction").commit();

    bool planAActive = true;
    jack::PlanBuilder planA = bdi.plan("PlanA")
                                .handles(goal)
                                .pre([&](const jack::BeliefContext&) -> bool {
                                    return planAActive;
                                })
                                .body(bdi.coroutine()
                                         .action(planAAction.name())
                                      )
                                .commit();

    jack::PlanBuilder planB = bdi.plan("PlanB")
                                .handles(goal)
                                .pre([&](const jack::BeliefContext&) -> bool {
                                    return !planAActive;
                                })
                                .body(bdi.coroutine()
                                         .action(planBAction.name())
                                      )
                                .commit();


    int actionInvokeCountA = 0;
    int actionInvokeCountB = 0;
    jack::Agent *agent = bdi.agent("AgentTemplate")
                            .plans(std::array{planA, planB})
                            .belief(beliefs)
                            .handleAction(planAAction.name(),
                                          [&planAActive, &actionInvokeCountA](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                                              actionInvokeCountA++;
                                              planAActive = false; /// Cause PlanB precondition to be valid and PlanA precondition to be invalid
                                              return jack::Event::SUCCESS;
                                          })
                            .handleAction(planBAction.name(),
                                          [&actionInvokeCountB](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                                              actionInvokeCountB++;
                                              return jack::Event::SUCCESS;
                                          })
                            .commitAsAgent()
                            .createAgentInstance("Agent");

    /// \note Start the engine
    agent->start();
    agent->pursue(goal, jack::GoalPersistent_No);

    /// \note Advance the executor to a running state
    {
        for (int attempts = 0; attempts < 4 && agent->intentions().empty(); attempts++) {
            bdi.poll();
        }

        jack::Span<const jack::IntentionExecutor*> executorList = agent->intentions();
        ASSERT_EQ(executorList.size(), 1);

        const jack::IntentionExecutor *executor = executorList[0];
        ASSERT_EQ(executor->state(), jack::IntentionExecutor::RUNNING);

        /// \note Execute the first action, this will queue an action into the
        /// event queue.
        bdi.poll();
    }

    /// \note We have a plan with only 1 action, one more tick will conclude and
    /// delete the executor within the same tick. Before this happens, we dirty
    /// the agent causing a reschedule.
    ///
    /// In the correct executor, the executor will notice that the last plan has
    /// succeeded. Meaning the goal has potentially been achieved, goal
    /// achivement must be checked.
    jack::Message msg = *agent->message(beliefs.name());
    msg.setFieldValue<bool>(FORCE_DIRTY_BELIEF, true);
    agent->sendMessage(std::move(msg), false /*broadcastToBus*/); /// Trigger a reschedule

    /// \note Poll to trigger reschedule, but, the plan should have finished,
    /// despite setting a new plan because the goal has been achieved.
    for (int attempts = 0;
         attempts < 8 && agent->intentions().size();
         attempts++)
    {
        bdi.poll();
    }

    ASSERT_EQ(actionInvokeCountA, 1) << "Verify that Plan A executed once";
    ASSERT_EQ(actionInvokeCountB, 0) << "Verify that Plan B was never executed";

    /// \note Currently in this use-case the agent fast-path's the concluding of
    /// the executor so the executor will disappear. Ensure this happens.
    ASSERT_EQ(agent->intentions().size(), 0);
}
