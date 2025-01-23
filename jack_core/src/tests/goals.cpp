#include "bitmaskops.h"
#include "testhelpers.h"

#include <jack/jack.h>
#include <gtest/gtest.h>

#include <vector>
#include <thread>
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

/*! ***********************************************************************************************
 * \class   GoalsTest
 *
 * This google test fixture aims to provide a context for Goal specific testing
 * See "3.1.3 Goals" section of Use Cases & Requirements
 * ************************************************************************************************/
class GoalsTest : public ::testing::Test {
    protected:

    GoalsTest()
        : planCode(PlanCode::NoPlan)
        , delay(false)
        , drop(false)
    {
        bdi.exitWhenDone();
    }

    void SetUp() override
    {
        auto goal1Maintenance = bdi.goal("Goal1Maintenance")
                                    .pre([this](const jack::BeliefContext &) {
                                            return !delay;
                                    })
                                    .commit();

        auto goal1 = bdi.goal("Goal1").commit();
        auto goal2 = bdi.goal("Goal2").commit();
        auto goal3 = bdi.goal("Goal3").commit();

        auto splan1 = bdi.plan("SPlan1")
                          .handles(goal1)
                          .body(bdi.coroutine()
                                    .action("Plan1Action")
                                    .print("Goal1\n"))
                          .commit();

        auto splan2 =
            bdi.plan("SPlan2")
                .handles(goal2)
                .body(bdi.coroutine()
                          .action("Plan2Action")
                          .goal(goal3.name())
                          .print("Goal2\n"))
                .commit();

        auto plan1 = bdi.plan("Plan1")
                         .handles(goal1)
                         .body(bdi.coroutine()
                                   .action("Plan1Action")
                                   .print("Goal3\n"))
                         .commit();

        auto slowPlan1 = bdi.plan("Plan1Slow")
                             .handles(goal1)
                             .body(bdi.coroutine()
                                       .sleep(100)
                                       .action("Plan1Action"))
                             .commit();

        auto plan1Maint = bdi.plan("Plan1Maintenance")
                              .handles(goal1Maintenance)
                              .body(bdi.coroutine()
                                        .action("Plan1Action"))
                              .commit();

        bdi.plan("Plan2")
            .handles(goal2)
            .body(bdi.coroutine()
                      .action("Plan2Action"))
            .commit();

        auto plan3 = bdi.plan("Plan3")
                         .handles(goal3)
                         .body(bdi.coroutine()
                                   .action("Plan3Action"))
                         .commit();

        planList.emplace_back(splan1);
        planList.emplace_back(splan2);
        planList.emplace_back(plan3);
        shortPlanList.emplace_back(plan1);
        maintenancePlanList.emplace_back(plan1Maint);
        slowPlanList.emplace_back(slowPlan1);
    }

    // Objects declared here can be used by all tests in the test case.
    jack::Engine bdi;

    // plan code is used to verify the correct execution of plans
    PlanCode planCode;

    //used internally for support
    bool planFlag;
    bool delay;
    bool drop;

    PlanList planList;
    PlanList shortPlanList;
    PlanList maintenancePlanList;
    PlanList slowPlanList;
};

TEST_F(GoalsTest, F21_GoalPayload)
{
    /*! ***************************************************************************************
     * Verify a goal be customized with a context
     *
     * F21
     * A goal shall possibly carry a payload
     * The payload provides additional context to plans. The payload is custom and natively
     * supported by the handling plans.
     * Ref. C02, C03, C11
     * ****************************************************************************************/

    bdi .action("Plan1Action")
        .commit();

    bdi .goal("MyGoal")
        //.context
        //.payload(myPayload) //FIXME: add payload
        .commit ();

    auto myPlan = bdi.plan("MyPlan")
            .handles    ("MyGoal")
            .pre        ([&] (const jack::BeliefContext&) {
                            //FIXME: add goal var and access its payload
                            //return goal.payload() != nullptr;
                            return true;
                        })
            .body       (bdi.coroutine ()
                            .action("Plan1Action")
                            .print ("GoalExecuted\n")
                            //FIXME: add goal var and access its payload
                            //.print (goal.payload());
                        )
            .commit();

    jack::AgentHandle myAgentHandle =
        bdi.agent("MyAgentType")
            .plan(myPlan)
            .handleAction("Plan1Action",
                          [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle) {
                              planCode |= PlanCode::Plan1;
                              agent.stop();
                              // FIXME: add goal var and access its payload
                              // std::cout << "Goal: " << goal.payload() <<
                              // endl;
                              return jack::Event::SUCCESS;
                          })
            .commitAsAgent()
            .createAgent("MyAgent");

    jack::Agent *myAgent = bdi.getAgent(myAgentHandle);
    myAgent->start();

    ///std::string myPayLoad = "Hello Payload";

    myAgent->pursue ("MyGoal", jack::GoalPersistent_Yes);

    //bdi.poll (100);
    /// @todo fixme this -1 might not exit
    enginePollAndRecordSchedules(bdi, -1 /*pollCount*/);
    ASSERT_EQ(planCode, PlanCode::Plan1);
}
#if 0
TEST_F(GoalsTest, F22_GoalSuspended)
{
    /*! **************************************************************************************
     * Verify a goal can be suspended and resumed
     *
     * F22
     * It shall be possible to interrupt a goal
     * This will cause the intention execution to be delayed until further notice.
     * Ref. V02
     * ****************************************************************************************/

    bdi .action("DelayedAction")
        .commit();

    bdi .action("Resume")
        .commit();

    // delay the goal
    delay = true;

    bdi .goal ("ResumeGoal")
        .commit ();

    auto resumePlan = bdi.plan("ResumePlan")
        .handles    ("ResumeGoal")
        .body       (bdi.coroutine ()
                        .sleep(50)
                        .action("Resume")
                    )
        .commit();

    bdi.goal ("DelayedGoal")
            .pre([this](const jack::BeliefContext&){
                return !delay;
            })
            .commit ();

    auto plan1 = bdi.plan("DelayedPlan")
        .handles    ("DelayedGoal")
        .body       (bdi.coroutine ()
                        .action("DelayedAction")
                    )
        .commit();

    jack::Agent* agent = bdi.agent("TestAgent1")
            .plans          ({plan1})
            //only execute the action if not delayed
            .handleAction   ("DelayedAction", [&] (jack::Agent& agent, jack::ActionEvent &action) {
                                std::cout << "delayed action invoked" << std::endl;
                                planCode |= PlanCode::Plan1;
                                return jack::Event::SUCCESS;
                            })
            //resume goal executing setting the delay flag to false
            .handleAction   ("Resume", [&] (jack::Agent& agent, jack::ActionEvent &action) {
                                std::cout << "goal resumed" << std::endl;
                                delay = false;
                                return jack::Event::SUCCESS;
                            })
            .create         ("agent1");

    agent->start();
    auto goal = agent->pursue("DelayedGoal");
    auto goal2 = agent->pursue("ResumeGoal");

    //goal is being delayed (maintain in this case)
    //goal->suspend();
    agent->dropWithMode(goal->handle(), jack::protocol::DropMode_NORMAL);
    bdi.poll(5);
    EXPECT_EQ(planCode, PlanCode::NoPlan);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    //goal will be resumed (by the resume action)
    //goal->resume();
    auto goal3 = agent->pursue("DelayedGoal");
    bdi.poll(100);
    EXPECT_EQ(planCode, PlanCode::Plan1);

    //get rid of the goal
    agent->drop (goal3->eventId());

    //finish off execution and get rid of the agent
    bdi.destroyAgent (agent->name ());
    bdi.execute ();
}
#endif

void interruptGoalTask(jack::Agent *agent, const jack::GoalHandle& goal, int millis)
{
    std::cout << "waiting a bit before dropping the goal" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(millis));
    agent->dropWithMode(goal, jack::protocol::DropMode_NORMAL, "");
    std::cout << "goal dropped" << std::endl;
}

TEST_F(GoalsTest, F22_GoalInterrupted)
{
    /*! ***************************************************************************************
     * Verify a goal can be interruped by dropping it from another thread
     *
     * F22
     * It shall be possible to interrupt a goal
     * This will cause the intention execution to be delayed until further notice.
     * Ref. V02
     * ****************************************************************************************/
    auto action = bdi.action("Action").commit();
    auto goal = bdi.goal("InterruptedGoal").commit();

    /// \note Make a plan that infinite loops
    int  startingLabel = 0;
    auto plan1         = bdi.plan("InterruptedPlan")
                     .handles("InterruptedGoal")
                     .body(bdi.coroutine()
                               .action("Action")
                               .label(startingLabel)
                               .action("Action")
                               .onSuccess(startingLabel)  /// \note Infinitely loop
                           )
                     .commit();

    jack::Agent* agent =
        bdi.agent("TestAgent1")
            .plan(plan1)
            .handleAction(action.name(),
                          [&](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                              return jack::Event::SUCCESS;
                          })
            .commitAsAgent()
            .createAgentInstance("agent1");

    /// \note Start the agent and execute the goal
    agent->start();
    jack::UniqueId goalId       = jack::UniqueId::random();
    jack::GoalPursue goalPursue = agent->pursue(goal.name(), jack::GoalPersistent_Yes, jack::Message{}, goalId);

    /// \note On goal (drop) completion stop the agent
    bool goalFinishedSuccessfully = false;
    goalPursue.promise->then([&]() {
        std::cout << "Goal finished successfully" << std::endl;
        goalFinishedSuccessfully = true;
        agent->stop();
    }, [&]() {
        std::cout << "Goal failed" << std::endl;
        agent->stop();
    });

    /// \note Dispatch a thread that will in 75 milliseconds, drop the goal from
    /// the agent.
    std::thread t1(interruptGoalTask, agent, jack::GoalHandle{goal.name(), goalId}, 75);

    /// \note This test will time-out if the goal is never dropped.
    enginePollAndRecordSchedules(bdi, -1 /*pollCount*/);

    t1.join();
    EXPECT_FALSE(goalFinishedSuccessfully);
}

#if 0
TEST_F(GoalsTest, F22_GoalWait)
{
    /*! ***************************************************************************************
     * Verify it is possible to wait for something to happen
     *
     * F22
     * It shall be possible to interrupt a goal
     * This will cause the intention execution to be delayed until further notice.
     * Ref. V02
     * ****************************************************************************************/

    jack::BeliefSet bs;
    bs.set<int>("NoPlan", 1);
    bs.set<int>("Plan1", 0);
    bs.set<int>("Plan2", 0);
    bs.set<int>("Plan3", 0);

    bdi.goal ("Goal").commit ();

    //this plan will be selected only when no plan has been executed
    auto plan1 = bdi.plan("Plan1")
        .handles    ("Goal")
        .pre        ("NoPlan == 1")
        .body       (bdi.coroutine ()
                        .action("Plan1Action")
                        .pursue("Goal")
                        .print("Subgoal finished\n")
                    )
        .commit();

    //this plan will be selected only when plan1 has been executed
    auto plan2 = bdi.plan("Plan2")
        .pre        ("Plan1 == 1")
        .handles    ("Goal")
        .body       (bdi.coroutine ()
                        .action("Plan2Action")
                        .pursue("Goal")
                        .print("Plan2 executed\n")
                    )
        .commit();

    //this plan will be selected only when plan1 has been executed
    auto plan3 = bdi.plan("Plan3")
        .pre        ("Plan2 == 1")
        .handles    ("Goal")
        .body       (bdi.coroutine ()
                        .action("Plan3Action")
                        .print("Plan3 executed\n")
                    )
        .commit();

    //the agent will pursue goal and then wait for the plancode
    jack::Agent *agent = bdi.agent("TestAgent1")
                            .beliefs(bs)
                             .plans({plan1, plan2, plan3})
                             .main(bdi.coroutine()
                                       .pursue("Goal").nowait()
                                       .yieldUntil("Plan2 == 1")
                                       .print("yieldUntil finished\n")
                                       .drop("Goal"))
                             // only execute the action if not delayed
                             .handleAction("Plan1Action", [&](jack::Agent &agent) {
                                 agent.beliefSet().set<int>("Plan1", 1);
                                 std::cout << "plan 1 action invoked" << std::endl;
                                 planCode |= PlanCode::Plan1;
                             })
                             .handleAction("Plan2Action", [&](jack::Agent &agent) {
                                 agent.beliefSet().set<int>("Plan2", 1);
                                 std::cout << "plan 2 action invoked" << std::endl;
                                 planCode |= PlanCode::Plan2;
                             })
                             .handleAction("Plan3Action", [&](jack::Agent &agent) {
                                 agent.beliefSet().set<int>("Plan3", 1);
                                 std::cout << "plan 3 action invoked" << std::endl;
                                 planCode |= PlanCode::Plan3;
                             })
                             .create("agent1");

    agent->start();

    bdi.poll (2000);

    //bdi.execute(); //never ends!

    EXPECT_EQ (agent->finished (), true);
    EXPECT_EQ (planCode, PlanCode::Plan1 | PlanCode::Plan2);
}
#endif

TEST_F(GoalsTest, F23_ExecuteSubGoals)
{
    /*! ***************************************************************************************
     * Verify if the agent is correctly executing the subgoals.
     *
     * F23
     * A goal shall be pursued either as a sub-goal or root-goal
     * Sub-goals are meant to live in the parent’s goal-intention context.
     * Root goals will survive the parent.
     * Ref. V07, P12, P14
     * ****************************************************************************************/

    bdi.action("Plan1Action").commit();
    bdi.action("Plan2Action").commit();
    bdi.action("Plan3Action").commit();

    jack::AgentHandle agentHandle =
        bdi.agent("TestAgent1")
            .plans(planList)
            .handleAction("Plan1Action",
                          [&](jack::Agent &agent, jack::Message &, jack::Message &out, jack::ActionHandle) {
                              planCode |= PlanCode::Plan1;
                              std::cout << "plan1 action" << std::endl;
                              //subgoal2
                              auto goal2 = agent.pursue("Goal2", jack::GoalPersistent_Yes);
                              goal2.promise->then([&](){
                                 std::cout << "goal2 finished" << std::endl;
                              });
                              return jack::Event::SUCCESS;
                          })
            .handleAction("Plan2Action",
                          [&](jack::Agent &agent, jack::Message &, jack::Message &out, jack::ActionHandle) {
                              planCode |= PlanCode::Plan2;
                              std::cout << "plan2 action" << std::endl;
                              return jack::Event::SUCCESS;
                          })
            .handleAction("Plan3Action",
                          [&](jack::Agent &agent, jack::Message &, jack::Message &out, jack::ActionHandle) {
                              planCode |= PlanCode::Plan3;
                              std::cout << "plan3 action" << std::endl;
                              agent.stop();
                              return jack::Event::SUCCESS;
                          })
            .commitAsAgent()
            .createAgent("agent1");

    jack::Agent *agent = bdi.getAgent(agentHandle);
    agent->pursue("Goal1", jack::GoalPersistent_Yes);
    agent->start();

    enginePollAndRecordSchedules(bdi, 4 /*pollCount*/);
    EXPECT_EQ(planCode, PlanCode::Plan1);

    enginePollAndRecordSchedules(bdi, 4 /*pollCount*/);
    EXPECT_EQ(planCode, PlanCode::Plan1 | PlanCode::Plan2);

    enginePollAndRecordSchedules(bdi, 10 /*pollCount*/);
    EXPECT_EQ(planCode, PlanCode::Plan1 | PlanCode::Plan2 | PlanCode::Plan3);

    enginePollAndRecordSchedules(bdi);
    EXPECT_EQ(planCode, PlanCode::Plan1 | PlanCode::Plan2 | PlanCode::Plan3);

    EXPECT_EQ (agent->stopped(), true);
}

TEST_F(GoalsTest, F23_DropSubGoals)
{
    /*! ***************************************************************************************
     * Verify if the agent is correctly dropping subgoals
     *
     * F23
     * A goal shall be pursued either as a sub-goal or root-goal
     * Sub-goals are meant to live in the parent’s goal-intention context.
     * Root goals will survive the parent.
     * Ref. V07, P12, P14
     * ****************************************************************************************/

    const std::string ACTION_PLAN_1       = "Plan1Action";
    const std::string ACTION_PLAN_2_START = "Plan2Action";
    const std::string ACTION_PLAN_2       = "Plan2StartingAction";
    const std::string GOAL_1_NAME         = "Goal1";

    bdi.action(ACTION_PLAN_1).commit();
    bdi.action(ACTION_PLAN_2_START).commit();
    bdi.action(ACTION_PLAN_2).commit();

    auto plana = bdi.plan("Plan1_PursueGoal2")
                     .handles(GOAL_1_NAME)
                     .body(bdi.coroutine()
                              .action(ACTION_PLAN_1)
                              .goal("Goal2")
                              .print("Goal2 was successful"))
                     .commit();

    bool plan2_started = false;
    auto planb = bdi.plan("Plan2_SlowAction")
        .handles ("Goal2")
        .body    (bdi.coroutine ()
                   .action(ACTION_PLAN_2_START)
                   .sleep(100)
                   .print("goal2 done sleeping\n")
                   .action(ACTION_PLAN_2)
                 )
        .commit();

    jack::AgentHandle agentHandle = bdi.agent("TestAgent1")
            .plans  (std::array{plana, planb})
            .handleAction(ACTION_PLAN_1,
                          [&] (jack::Agent& agent, jack::Message &msg, jack::Message &out, jack::ActionHandle) {
                              planCode |= PlanCode::Plan1;
                              std::cout << "plan1 action" << std::endl;
                              return jack::Event::SUCCESS;
            })
            .handleAction(ACTION_PLAN_2_START,
                          [&plan2_started] (jack::Agent&, jack::Message &, jack::Message &out, jack::ActionHandle) {
                              plan2_started = true;
                              std::cout << "plan2 started" << std::endl;
                              return jack::Event::SUCCESS;
            })
            .handleAction(ACTION_PLAN_2,
                          [&] (jack::Agent& agent, jack::Message &, jack::Message &out, jack::ActionHandle) {
                              planCode |= PlanCode::Plan2;
                              std::cout << "plan2 action" << std::endl;
                              return jack::Event::SUCCESS;
            })
            .commitAsAgent()
            .createAgent("agent1");

    jack::Agent *agent = bdi.getAgent(agentHandle);
    agent->start();

    jack::UniqueId goalId = jack::UniqueId::random();
    auto goal = agent->pursue(GOAL_1_NAME, jack::GoalPersistent_Yes, jack::Message{}, goalId);
    goal.promise->then([&]() {
        std::cout << "goal1 finished" << std::endl;
        agent->stop();
    });

    //TODO: provide agent active goals
    //example to drop one active goal
    //agent->drop(agent->activeGoals()[0]);
    //wait_for(agent->goal().finished());
    //goal->onDrop(); ??

    // Start agent, expect Goal1 to be finished and that Goal2 (sub-goal of
    // Goal1) has started.

    enginePollAndRecordSchedules(bdi, 10 /*pollCount*/);
    EXPECT_EQ(planCode, PlanCode::Plan1);
    EXPECT_TRUE(plan2_started);

    // Drop event is queued, but not processed until engine is ticked
    jack::GoalHandle goalHandle = {GOAL_1_NAME, goalId};
    agent->dropWithMode(goalHandle, jack::protocol::DropMode_NORMAL, "" /*reason*/);
    EXPECT_EQ(agent->intentions().size(), 2) << "We should have Goal1 and Goal2 executing";
    for (const aos::jack::IntentionExecutor *intentionExecutor : agent->intentions()) {
        EXPECT_TRUE(intentionExecutor->isRunning());
    }

    // Tick engine, the intentions and goals should be deleted
    enginePollAndRecordSchedules(bdi, 8 /*pollCount*/);
    EXPECT_EQ(agent->intentions().size(), 0);

    // Finish execution
    enginePollAndRecordSchedules(bdi, -1);

    // The plan co-routine should finish and Plan2 should never complete.
    EXPECT_EQ(planCode, PlanCode::Plan1);
    EXPECT_EQ(agent->stopped(), true);
}


TEST_F(GoalsTest, N1_GoalFailsWithOnePlan)
{
    /*! ***************************************************************************************
     * Verify a perform goal fails with a single failing plan
     * ****************************************************************************************/

    bdi.action("FailingAction").commit();

    /// The failing plan
    auto planFail = bdi.plan("FailingPlan")
                        .handles("FailingGoal")
                        .body(bdi.coroutine()
                                 .action("FailingAction")
                                 .print("Will Never Print"))
                        .commit();

    auto goalFail = bdi.goal("FailingGoal").commit ();

    // count the number of time the plan action runs
    int planRuns = 0;

    jack::Agent* agent =
        bdi.agent("TestAgent1")
            .plans(std::array{planFail})
            .handleAction("FailingAction",
                          [&](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                              planRuns++;
                              if (planRuns > 1) { return jack::Event::SUCCESS; }
                              return jack::Event::FAIL;
                          })
            .commitAsAgent()
            .createAgentInstance("agent1");

    int didFail = -1;
    agent->start();
    auto goalPursue = agent->pursue(goalFail, jack::GoalPersistent_No);

    goalPursue.promise->then(
        [&]() {
            didFail = 0;
            agent->stop();
        },
        [&]() {
            didFail = 1;
            agent->stop();
        });

    bdi.execute();
    EXPECT_EQ(didFail, 1);
    EXPECT_EQ(planRuns, 1);
}

TEST_F(GoalsTest, N2_GoalFailsWithTwoPlans)
{
    /**************************************************************************
     * Verify that a perform goal with 2 plans and a exclude policy tries each
     * plan once and does not repeat
     **************************************************************************/

    auto goal        = bdi.goal("Failing Goal").commit();
    auto plan1Action = bdi.action("Failing Action 1").commit();
    auto plan2Action = bdi.action("Failing Action 2").commit();

    auto planFail1 = bdi.plan("Failing Plan 1")
                         .handles(goal.name())
                         .body(bdi.coroutine()
                                  .action(plan1Action.name()))
                         .commit();

    auto planFail2 = bdi.plan("Failing Plan 2")
                         .handles(goal.name())
                         .body(bdi.coroutine()
                                  .action(plan2Action.name()))
                         .commit();

    /// \note Count the number of time the plan action runs
    std::vector<std::string> executionList;
    jack::Agent* agent =
        bdi.agent("TestAgent1")
            .plans(std::array{planFail1, planFail2})
            .handleAction(plan1Action.name(),
                          [&](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                              executionList.push_back(plan1Action.name());
                              return jack::Event::FAIL;
                          })
            .handleAction(plan2Action.name(),
                          [&](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                              executionList.push_back(plan2Action.name());
                              return jack::Event::FAIL;
                          })
            .commitAsAgent()
            .createAgentInstance("agent1");

    agent->start();
    auto goalPursue = agent->pursue(goal, jack::GoalPersistent_No);

    int failed = -1;
    goalPursue.promise->then(
        [&]() {
            failed = 0;
            agent->stop();
        },
        [&]() {
            failed = 1;
            agent->stop();
        });

    bdi.execute();

    ASSERT_EQ(executionList.size(), 2);
    EXPECT_EQ(executionList[0], plan1Action.name());
    EXPECT_EQ(executionList[1], plan2Action.name());
}

TEST_F(GoalsTest, F24_GoalFinished)
{
    /*! ***************************************************************************************
     * Verify if a goal correctly tells when it's finished.
     *
     * F24
     * Goals shall indicate when they have finished
     * This signal should be observable
     * Ref. C29, P24
     * ****************************************************************************************/

    bdi.action("Plan1Action").commit();
    bdi.action("Quit").commit();

    jack::AgentHandle agentHandle =
        bdi.agent("TestAgent1")
            .plans(shortPlanList)
            .handleAction("Plan1Action",
                          [&](jack::Agent &agent, jack::Message &, jack::Message &out, jack::ActionHandle) {
                              planCode |= PlanCode::Plan1;
                              return jack::Event::SUCCESS;
                          })
            .handleAction("Quit",
                          [&](jack::Agent &agent, jack::Message &, jack::Message &out, jack::ActionHandle) {
                              bdi.exit();
                              return jack::Event::SUCCESS;
                          })
            .commitAsAgent()
            .createAgent("agent1");

    jack::Agent *agent = bdi.getAgent(agentHandle);
    agent->start();

    bool successful = false;

    auto goalPursue = agent->pursue("Goal1", jack::GoalPersistent_Yes);
    goalPursue.promise->then([&]() {
        successful = true;
        agent->stop();
    });

    bdi.poll();
    EXPECT_EQ(successful, false);

    bdi.execute();

    //TODO: add a "promise" status code like the agent
    //promise->finished()
    EXPECT_EQ(successful, true);
    EXPECT_EQ(planCode, PlanCode::Plan1);
}

TEST_F(GoalsTest, F25_GoalDropped)
{
    /*! ***************************************************************************************
     * Verify a goal can be dropped at any time
     *
     * F25
     * It shall be possible to drop a goal.
     * A dropped goal will not generate further intentions.
     * Ref. C35, P02, P11, P14
     * ****************************************************************************************/
    auto id = jack::UniqueId::random();
    id.setTag("GoalId_1234");

    jack::ActionBuilder planAction = bdi.action("Plan1Action").commit();
    jack::ActionBuilder stopAction = bdi.action("Stop").commit();

    auto slowGoal = bdi.goal("SlowGoal").commit();
    auto slowPlan = bdi.plan("SlowPlan")
                        .handles(slowGoal)
                        .body(bdi.coroutine()
                                .sleep(100)
                                .action(planAction.name()))
                        .commit();

    auto startGoal = bdi.goal("StartGoal").commit();
    auto startPlan = bdi.plan("StartPlan")
                         .handles(startGoal)
                         .body(bdi.coroutine()
                                 .sleep(50)
                                 .drop(jack::GoalHandle{slowGoal.name(), id})
                                 .sleep(50)
                                 .action(stopAction.name()))
                         .commit();

    jack::AgentHandle agentHandle =
        bdi.agent("TestAgent1")
            .plans(std::array{slowPlan, startPlan})
            .handleAction(planAction.name(),
                          [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle) {
                              // should never get called
                              planCode |= PlanCode::Plan1;
                              return jack::Event::SUCCESS;
                          })
            .handleAction(stopAction.name(),
                          [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle) {
                              agent.stop();
                              return jack::Event::SUCCESS;
                          })
            .commitAsAgent()
            .createAgent("agent1");

    jack::Agent *agent = bdi.getAgent(agentHandle);
    agent->start();
    agent->pursue(slowGoal.name(),  jack::GoalPersistent_No, jack::Message{}, id);
    agent->pursue(startGoal.name(), jack::GoalPersistent_Yes);

    /// \note We use the poll API to ensure our timing is fairly granular, not
    /// perfect but so that sleep granularity doesn't shoot us past the 50 and
    /// 100ms range in one go causing both goals to queue up and the test to
    /// fail.
    // Ensure both intentions are starteed
    while (agent->intentions().size() != 2) { bdi.poll(); }

    // Then we can poll 50ms to trigger *just* the drop
    bdi.poll(std::chrono::milliseconds(50));

    // Finish off execution
    while (bdi.getStatus()) { bdi.poll(); }

    EXPECT_EQ(planCode, PlanCode::NoPlan);
}

// varify that a goal can be suspended
// a maintenance goal will suspend and lower priority normal goal
TEST_F(GoalsTest, EM1_BatteryCharge)
{
    //FIXME: move this test to the battery charge example
    //int batteryLevel = 100;
    int globalTasksCompleted = 0;
    const int tasksToDo = 10;

    auto bs = bdi.message("Vehicle")
                 .fieldWithValue<int>("batteryLevel", 100)
                 .fieldWithValue<int>("tasksCompleted", 0)
                 .commit();

    // action messages
    bdi.action("StartWork").commit();
    bdi.action("StopWork").commit();
    bdi.action("DoTask").commit();
    bdi.action("Recharge").commit();

    // the maintenance goal that will suspend task goals and recharge the battery
    // if the battery goes below 25%
    auto rechargeGoal =
        bdi .goal("RechargeBatteryGoal")
            .pre([](const jack::BeliefContext& context) {
                const std::shared_ptr<jack::Message> vehicle = context.message("Vehicle");
                int batteryLevel                             = *vehicle->getConstPtr<int>("batteryLevel");
                return batteryLevel >= 25;
            })
            .commit ();

    // plan to recharge battery
    auto rechargePlan = bdi.plan("RechargeBatteryPlan")
                            .handles(rechargeGoal)
                            .body(bdi.coroutine().action("Recharge"))
                            .commit();

    // An achievement goal to perform all the tasks
    auto startWorkGoal = bdi.goal("StartWorkGoal")
                             .satisfied([&](const jack::BeliefContext& context) {
                                 std::shared_ptr<jack::Message> vehicle = context.message("Vehicle");
                                 auto tasksCompleted = *vehicle->getPtr<int>("tasksCompleted");
                                 return (tasksCompleted >= 10);
                             })
                             .commit();

    // plan to do a task
    // This plan will drop if the battery level drops below 25%
    // or all 10 tasks are completed
    // uses a recursion for iteration
    auto doTasksPlan =
        bdi.plan("DoTasksPlan")
            .handles(startWorkGoal)
            .effects([](jack::BeliefContext &context) {
                //std::cout << "applying effects " << std::endl;
                std::shared_ptr<jack::Message> bs = context.message("Vehicle");
                if (bs) {
                    int batteryLevel = *bs->getPtr<int>("batteryLevel");
                    bs->setFieldValue("batteryLevel", batteryLevel - 10);
                    batteryLevel = *bs->getPtr<int>("batteryLevel");

                    auto tasksCompleted = *bs->getPtr<int>("tasksCompleted");
                    bs->setFieldValue("tasksCompleted", tasksCompleted+1);
                }
            })
            .body(bdi.coroutine()
                      .action("DoTask")
                  )
            .commit();

    jack::AgentHandle agentHandle =
        bdi.agent("TestAgent1")
            .belief(bs)
            .plans(std::array{rechargePlan, doTasksPlan /*, shutdownPlan*/})
            .handleAction("DoTask",
                          [&](jack::Agent&   agent,
                              jack::Message& msg,
                              jack::Message& out,
                              jack::ActionHandle) {
                              jack::BeliefContext& context = agent.context();

                              jack::Message vehicle      = *context.message("Vehicle");
                              int           batteryLevel = *vehicle.getPtr<int>("batteryLevel");
                              batteryLevel -= 10;

                              vehicle.setFieldValue("batteryLevel", batteryLevel);
                              std::cout << "battery level(" << batteryLevel << ")" << std::endl;
                              if (batteryLevel >= 0) {
                                  auto tasksCompleted = *vehicle.getPtr<int>("tasksCompleted");
                                  vehicle.setFieldValue("tasksCompleted", tasksCompleted + 1);
                                  std::cout << "task(" << tasksCompleted + 1 << ") complete"
                                            << std::endl;
                              }
                              globalTasksCompleted++;
                              if (globalTasksCompleted >= 10) { agent.stop(); }

                              agent.sendMessage(std::move(vehicle), false /*broadcastToBus*/);
                              return jack::Event::SUCCESS;
                          })
            .handleAction(
                "Recharge",
                [&](jack::Agent& agent, jack::Message&, jack::Message&, jack::ActionHandle) {
                    jack::BeliefContext& context      = agent.context();
                    jack::Message        vehicle      = *context.message("Vehicle");
                    auto                 batteryLevel = *vehicle.getPtr<int>("batteryLevel");
                    std::cout << "recharging battery from " << batteryLevel << " -> 100"
                              << std::endl;
                    vehicle.setFieldValue("batteryLevel", 100);
                    agent.sendMessage(std::move(vehicle), false /*broadcastToBus*/);
                    return jack::Event::SUCCESS;
                })
            .commitAsAgent()
            .createAgent("agent1");

    jack::Agent *agent = bdi.getAgent(agentHandle);
    agent->start();

    agent->pursue(rechargeGoal,  jack::GoalPersistent_Yes);
    agent->pursue(startWorkGoal, jack::GoalPersistent_Yes);

    enginePollAndRecordSchedules(bdi);

    EXPECT_EQ(globalTasksCompleted, tasksToDo);
}

// varify that a goal can be suspended
// a maintenance goal will suspend and lower priority normal goal
TEST_F(GoalsTest, EM1_MultiplePlans)
{
    //FIXME: move this test to the battery charge example
    int tasksCompleted = 0;
    const int tasksToDo = 2;

    // action messages
    bdi.action("DoTask").commit();

    // a goal to perform a task
    auto doTasksGoal =
        bdi .goal("DoTasksGoal")
            .satisfied([&](const jack::BeliefContext &) {
                return (tasksCompleted >= tasksToDo);
            })
            .commit ();

    // plan to do a task
    // This plan will drop if the battery level drops below 25%
    // or all 10 tasks are completed
    auto doTasksPlan1 =
        bdi .plan       ("DoTasksPlan1")
            .handles    (doTasksGoal)
            .body       (bdi.coroutine ()
                            .print("DoTasksPlan1\n")
                            .action("DoTask")
                        )
            .commit     ();

     auto doTasksPlan2 =
        bdi .plan       ("DoTasksPlan2")
            .handles    (doTasksGoal)
            .body       (bdi.coroutine ()
                            .print("DoTasksPlan2\n")
                            .action("DoTask")
                        )
            .commit     ();

    auto doTasksPlan3 =
        bdi .plan       ("DoTasksPlan3")
            .handles    (doTasksGoal)
            .body       (bdi.coroutine ()
                            .print("DoTasksPlan3\n")
                            .action("DoTask")
                        )
            .commit     ();

    jack::AgentHandle agentHandle = bdi.agent("TestAgent1")
            .plans          (std::array{ doTasksPlan1, doTasksPlan2, doTasksPlan3 })
            .handleAction   ("DoTask", [&] (jack::Agent& agent, jack::Message &msg, jack::Message &out, jack::ActionHandle) {
                                tasksCompleted++;
                                std::cout << "task(" << tasksCompleted <<  ") complete" << std::endl;
                                return jack::Event::SUCCESS;
                            })
            .commitAsAgent()
            .createAgent("agent1");

    jack::Agent *agent = bdi.getAgent(agentHandle);
    agent->start();
    auto goalPursue = agent->pursue(doTasksGoal, jack::GoalPersistent_Yes);

    // when the startWorkGoal concludes shutdown the agent.
    goalPursue.promise->then([&](){
        std::cout << "work satisfied" << std::endl;
        std::cout << "shutting down agent" << std::endl;
        agent->stop();
    });

    enginePollAndRecordSchedules(bdi);

    EXPECT_EQ(tasksCompleted, tasksToDo);
}

// varify that a goal can be suspended
// a maintenance goal will suspend and lower priority normal goal
TEST_F(GoalsTest, EM1_MultiplePlansFail)
{
    //FIXME: move this test to the battery charge example

    /// Test that when a goal's plan fails, DoTasksPlan1, it gets excluded from
    /// the plan list, and the goal will cycle to the next plan in the list,
    /// DoTasksPlan2 and succeed.
    int tasksCompleted = 0;
    const int tasksToDo = 2;
    int failedTasks = 0;

    // action messages
    bdi.action("DoTask").commit();
    bdi.action("DoFailTask").commit();

    // a goal to perform the task
    auto doTasksGoal =
        bdi .goal("DoTasksGoal")
            .commit ();

    // This plan will run first and fail
    auto doTasksPlan1 =
        bdi .plan       ("DoTasksPlan1")
            .handles    (doTasksGoal)
            .body       (bdi.coroutine ()
                            .print("DoTasksPlan1\n")
                            .action("DoFailTask")
                            .print("SHOULD NEVER PRINT")
                            .action("DoFailTask") // This should never run
                        )
            .commit     ();

    // This plan will run second and succeed
    auto doTasksPlan2 =
        bdi .plan       ("DoTasksPlan2")
            .handles    (doTasksGoal)
            .body       (bdi.coroutine ()
                            .print("DoTasksPlan2\n")
                            .action("DoTask")
                            .action("DoTask")
                        )
            .commit     ();

    // This plan will never run because the previous plan should succeed
    auto doTasksPlan3 =
        bdi .plan       ("DoTasksPlan3")
            .handles    (doTasksGoal)
            .body       (bdi.coroutine ()
                            .print("DoTasksPlan3\n")
                            .action("DoFailTask")
                            .action("DoFailTask")
                        )
            .commit     ();

    jack::AgentHandle agentHandle = bdi.agent("TestAgent1")
            .plans          (std::array{ doTasksPlan1, doTasksPlan2, doTasksPlan3 })
            .handleAction   ("DoTask", [&] (jack::Agent& agent, jack::Message &msg, jack::Message &out, jack::ActionHandle) {
                                tasksCompleted++;
                                std::cout << "task(" << tasksCompleted <<  ") complete" << std::endl;
                                return jack::Event::SUCCESS;
                            })
            .handleAction   ("DoFailTask", [&] (jack::Agent& agent, jack::Message &msg, jack::Message &out, jack::ActionHandle) {
                                failedTasks++;
                                std::cout << "task(failed)" << std::endl;
                                return jack::Event::FAIL;
                            })
            .commitAsAgent()
            .createAgent("agent1");

    jack::Agent *agent = bdi.getAgent(agentHandle);
    agent->start();
    auto goalPursue = agent->pursue(doTasksGoal, jack::GoalPersistent_Yes);

    // when the startWorkGoal concludes shutdown the agent.
    goalPursue.promise->then([&](){
        std::cout << "work satisfied" << std::endl;
        std::cout << "shutting down agent" << std::endl;
        agent->stop();
    });

    enginePollAndRecordSchedules(bdi);

    EXPECT_EQ(failedTasks, 1);
    EXPECT_EQ(tasksCompleted, tasksToDo);
}

/// \todo Maintenance goals no longer work because we now drop goals that cannot
/// be allocated after trying to schedule all goals. Before we would inject the
/// maintenance goal and permit the scheduler to try again. Since we delete the
/// goal early, the maintenance goal never gets scheduled after the goal that
/// violated it. See schedule.cpp
TEST_F(GoalsTest, DISABLED_MaintenanceGoal)
{
    // \note This test does not use any of the inherited class variables in GoalsTest

    /*
       Ontology
       +---------------------+    +---------------------+    +-----------------------+
       | RechargeBatteryGoal | -> | RechargeBatteryPlan | -> | RechargeBatteryAction |
       +---------------------+    +---------------------+    +-----------------------+
                |                   | Uses
                |                   V
                | Maintains       +---------+
                +---------------->| Battery |
                                  +---------+
                                    ^
                                    | Uses
       +--------------------+    +--------------------+    +----------------------+
       | ConsumeBatteryGoal | -> | ConsumeBatteryPlan | -> | ConsumeBatteryAction |
       +--------------------+    +--------------------+    +----------------------+
         ^
         | Performs
       +--------------------+
       | AgentWithBattery   |
       +--------------------+

       An agent's battery is initialized to an amount that is initially insufficient to perform
       ConsumeBatteryGoal without triggering a resource violation (i.e. after ConsumeBatteryGoal is
       performed, the battery value falls to -1).

       The scheduler should be able to detect this battery resource violation and trigger the
       RechargeBatteryGoal (because it's configured to maintain the Battery).

       The agent's battery will recharge and allow it to schedule ConsumeBatteryGoal after
       RechargeBatteryGoal is completed.

       We do this by checking the engine's internal clock to ensure that at some point the consuming
       intention was executed at some point after the maintenance goal was triggered (recharging).
       Prior to the fix to this problem the JACK's intention chain it planned in the scheduler was
       set to run all the chain elements in parallel.
     */

    // Setup
    int BATTERY_UNITS_TO_CONSUME          = 75;
    jack::ResourceBuilder batteryResource = bdi.resource("Battery")
                                                .min(0)
                                                .max(100)
                                                .commit();

    // \todo It's unusual that we have to specifically mark the battery as
    // locked rather than deriving it from the fact that we update the battery.
    // Is there a way to make JACK discover the resource conflicts naturally?
    //
    // By marking the plan with locks, JACK is able to correctly reorder
    // the result of the scheduler to run as many intentions possible in
    // parallel.
    jack::ResourceBuilder batteryLock = bdi.resource("BatteryLock")
                                                .min(0)
                                                .max(1)
                                                .commit();

    const std::string CONSUMED_BATTERY_POLL_COUNT  = "ConsumedBatteryOnPollCount";
    const std::string RECHARGED_BATTERY_POLL_COUNT = "RechargedBatteryOnPollCount";

    jack::MessageBuilder beliefSet =
        bdi.message("Beliefset")
            .fieldWithValue<size_t>(CONSUMED_BATTERY_POLL_COUNT, 0)
            .fieldWithValue<size_t>(RECHARGED_BATTERY_POLL_COUNT, 0)
            .commit();

    //
    // ConsumeBattery
    //
    jack::ActionBuilder consumeBatteryAction  = bdi.action("ConsumeBatteryAction").commit();
    jack::GoalBuilder consumeBatteryGoal =
        bdi.goal("ConsumeBatteryGoal")
            .satisfied([&CONSUMED_BATTERY_POLL_COUNT](const jack::BeliefContext &context) -> bool {
                size_t result = 0;
                bool  found  = context.get(CONSUMED_BATTERY_POLL_COUNT, result);
                EXPECT_TRUE(found);
                return result != 0;
            })
            .commit();

    jack::PlanBuilder consumeBatteryPlan =
        bdi.plan("ConsumeBatteryPlan")
            .handles(consumeBatteryGoal)
            .lock(batteryLock)
            .effects([&batteryResource,
                      &CONSUMED_BATTERY_POLL_COUNT,
                      &BATTERY_UNITS_TO_CONSUME,
                      &beliefSet](jack::BeliefContext &context) -> void {
                // Simulate consuming the battery after this plan is complete.
                std::shared_ptr<jack::Resource> battery = context.resource(batteryResource.name());
                battery->set(battery->count() - BATTERY_UNITS_TO_CONSUME);

                // Set the clock to any non-zero flag to simulate us completing the plan.
                std::shared_ptr<jack::Message> beliefs = context.message(beliefSet.name());
                beliefs->setFieldValue<size_t>(CONSUMED_BATTERY_POLL_COUNT, 1ULL /*any non-zero value*/);
            })
            .body(bdi.coroutine().action(consumeBatteryAction.name()))
            .commit();

    //
    // RechargeBattery
    //
    jack::ActionBuilder rechargeBatteryAction = bdi.action("RechargeBatteryAction").commit();
    jack::GoalBuilder rechargeGoal =
        bdi.goal("RechargeGoal")
            .satisfied([&RECHARGED_BATTERY_POLL_COUNT](const jack::BeliefContext &context) -> bool {
                size_t result = 0;
                bool  found  = context.get<decltype(result)>(RECHARGED_BATTERY_POLL_COUNT, result);
                EXPECT_TRUE(found);
                return result != 0;
            })
            .commit();

    jack::PlanBuilder rechargeBatteryPlan =
        bdi.plan("RechargeBatteryPlan")
            .handles(rechargeGoal)
            .lock(batteryLock)
            .effects([&batteryResource, &RECHARGED_BATTERY_POLL_COUNT, &beliefSet](
                         jack::BeliefContext &context) -> void {
                // Simulate recharging the battery to 100% after this plan is complete.
                std::shared_ptr<jack::Resource> battery = context.resource(batteryResource.name());
                battery->set(battery->max());

                // Set the clock to any non-zero flag to simulate us completing the plan.
                std::shared_ptr<jack::Message> beliefs = context.message(beliefSet.name());
                beliefs->setFieldValue<size_t>(RECHARGED_BATTERY_POLL_COUNT, 1ULL /*any non-zero value*/);
            })
            .body(bdi.coroutine().action(rechargeBatteryAction.name()))
            .commit();

    //
    // Agent
    //
    jack::AgentHandle agentHandle =
        bdi.agent("AgentWithBatteryTemplate")
            .plans(std::array{consumeBatteryPlan, rechargeBatteryPlan})
            .maintains(batteryResource, rechargeGoal)
            .resource(batteryResource)
            .belief(beliefSet)
            .handleAction(consumeBatteryAction.name(),
                          [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle) {
                              std::shared_ptr<aos::jack::Resource> battery =
                                  agent.context().resource(batteryResource.name());
                              battery->set(battery->count() - BATTERY_UNITS_TO_CONSUME);

                              // Record the poll count when this action occurred (i.e. the intention was executing)
                              std::shared_ptr<jack::Message> beliefs = agent.context().message(beliefSet.name());
                              beliefs->setFieldValue<size_t>(CONSUMED_BATTERY_POLL_COUNT, agent.engine().pollCount());
                              return jack::Event::SUCCESS;
                          })
            .handleAction(rechargeBatteryAction.name(),
                          [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle) {
                              std::shared_ptr<aos::jack::Resource> battery =
                                  agent.context().resource(batteryResource.name());
                              battery->set(battery->max());

                              // Record the poll count when this action occurred (i.e. the intention was executing)
                              std::shared_ptr<jack::Message> beliefs = agent.context().message(beliefSet.name());
                              beliefs->setFieldValue<size_t>(RECHARGED_BATTERY_POLL_COUNT, agent.engine().pollCount());
                              return jack::Event::SUCCESS;
                          })
            .commitAsAgent()
            .createAgent("AgentWithBattery");

    jack::Agent *agent = bdi.getAgent(agentHandle);

    // Validate maintenance goal is added and available
    {
        const std::map<std::string, std::string> &resourcesToGoals = agent->resourcesToGoals();
        EXPECT_EQ(resourcesToGoals.size(), 1);
        EXPECT_EQ(resourcesToGoals.count(batteryResource.name()), 1);
    }

    // Initialise battery to amount insufficient to consume
    std::shared_ptr<jack::Resource> battery = agent->context().resource(batteryResource.name());
    const int startingBattery               = battery->max() - (BATTERY_UNITS_TO_CONSUME + 1);
    battery->set(startingBattery);
    ASSERT_GE(battery->count(), battery->min())
        << "Unexpected battery min/max has insufficient range to consume validly";

    // The battery is insufficient to consume, on attempting to consume the
    // battery we will violate the resource and recharging should be triggered
    // followed by consuming.
    {
        agent->pursue(consumeBatteryGoal, jack::GoalPersistent_No);
        agent->start();
        size_t rechargedAtPollCount = 0;
        size_t consumedAtPollCount  = 0;

        // Here we check the engine's clock when the actions were executed. We
        // expect recharging to come first followed by consuming.
        //
        // Consuming should occur at a later time in the engine's clock because
        // it must block the consuming intention from executing until atleast
        // the next tick after recharging has occurred.

        while (rechargedAtPollCount == 0 || consumedAtPollCount == 0) {
            enginePollAndRecordSchedules(bdi, 1 /*pollCount*/);
            bool found = true;
            if (rechargedAtPollCount == 0) {
                found &= agent->context().get<size_t>(RECHARGED_BATTERY_POLL_COUNT, rechargedAtPollCount);
                if (rechargedAtPollCount != 0) {
                    std::shared_ptr<jack::Resource> battery = agent->context().resource(batteryResource.name());
                    EXPECT_EQ(battery->count(), battery->max()) << "Recharging just occurred but battery not set to max amount?";
                }
            }

            if (consumedAtPollCount == 0) {
                found &= agent->context().get<size_t>(CONSUMED_BATTERY_POLL_COUNT, consumedAtPollCount);
                if (consumedAtPollCount != 0) {
                    std::shared_ptr<jack::Resource> battery = agent->context().resource(batteryResource.name());
                    EXPECT_NE(battery->count(), battery->max()) << "Battery just consumed but battery is still set to the max amount?";
                }
            }

            assert(found); (void)found;
        }

        EXPECT_LT(rechargedAtPollCount, consumedAtPollCount)
            << "Consuming battery plan creates violation, meaning recharging action must occur "
               "first followed by eventually doing the consuming action which should be have been "
               "planned to execute *after* the recharge has occured.\n\nIf the timestamps are the "
               "same, then, the planner is still trying to schedule and execute both plans "
               "simultaneously which is invalid.";
        enginePollAndRecordSchedules(bdi, 1 /*pollCount*/);
    }
}

TEST_F(GoalsTest, GoalDAGDependency)
{
    // \note This test does not use any of the inherited class variables in GoalsTest

    /*
       Given 2 goals that lock the battery resource and are performed by the agent
         - ConsumeBatteryGoal_0
         - ConsumeBatteryGoal_1

       We expect the agent to queue up execution of the 2 goals in sequence,
       with ConsumeBatteryGoal_1 a dependency of ConsumeBatteryGoal_0

       The DAG generated from the schedule should first run
       ConsumeBatteryGoal_0, and only after it's finished, by closing the DAG
       node, ConsumeBatteryGoal_1 shall execute.
     */

    // Setup
    // \todo It's unusual that we have to specifically mark the battery as
    // locked rather than deriving it from the fact that we update the battery.
    // Is there a way to make JACK discover the resource conflicts naturally?
    //
    // By marking the plan with locks, JACK is able to correctly reorder
    // the result of the scheduler to run as many intentions possible in
    // parallel.
    jack::ResourceBuilder batteryLock = bdi.resource("BatteryLock")
                                                .min(0)
                                                .max(1)
                                                .commit();

    //
    // ConsumeBattery
    //
    const std::string CONSUMED_BATTERY_POLL_COUNT_0 = "ConsumedBatteryOnPollCount_0";
    const std::string CONSUMED_BATTERY_POLL_COUNT_1 = "ConsumedBatteryOnPollCount_1";

    jack::MessageBuilder beliefSet =
        bdi.message("Beliefset")
           .field<size_t>(CONSUMED_BATTERY_POLL_COUNT_0)
           .field<size_t>(CONSUMED_BATTERY_POLL_COUNT_1)
           .commit();

    //
    // ConsumeBattery 0
    //
    jack::ActionBuilder           consumeBatteryAction_0 = bdi.action("ConsumeBatteryAction_0").commit();
    jack::GoalBuilder consumeBatteryGoal_0 =
        bdi.goal("ConsumeBatteryGoal_0")
            .satisfied([&CONSUMED_BATTERY_POLL_COUNT_0](
                           const jack::BeliefContext &context) -> bool {
                size_t result = 0;
                bool   found = context.get<decltype(result)>(CONSUMED_BATTERY_POLL_COUNT_0, result);
                EXPECT_TRUE(found);
                return result != 0;
            })
            .commit();

    jack::PlanBuilder consumeBatteryPlan_0 =
        bdi.plan("ConsumeBatteryPlan_0")
            .handles(consumeBatteryGoal_0)
            .lock(batteryLock)
            .body(bdi.coroutine().action(consumeBatteryAction_0.name()))
            .commit();

    //
    // ConsumeBattery 1
    //
    jack::ActionBuilder consumeBatteryAction_1 = bdi.action("ConsumeBatteryAction_1").commit();
    jack::GoalBuilder consumeBatteryGoal_1 =
        bdi.goal("ConsumeBatteryGoal_1")
            .satisfied([&CONSUMED_BATTERY_POLL_COUNT_1](const jack::BeliefContext &context) -> bool {
                size_t result = 0;
                bool   found = context.get<decltype(result)>(CONSUMED_BATTERY_POLL_COUNT_1, result);
                EXPECT_TRUE(found);
                return result != 0;
            })
            .commit();

    jack::PlanBuilder consumeBatteryPlan_1 =
        bdi.plan("ConsumeBatteryPlan_1")
            .handles(consumeBatteryGoal_1)
            .lock(batteryLock)
            .body(bdi.coroutine().action(consumeBatteryAction_1.name()))
            .commit();


    //
    // Agent
    //
    jack::AgentBuilder agentBuilder =
        bdi.agent("AgentWithBatteryTemplate")
            .plans(std::array{consumeBatteryPlan_0, consumeBatteryPlan_1})
            .belief(beliefSet)
            .handleAction(consumeBatteryAction_0.name(),
                          [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle) {
                              std::shared_ptr<jack::Message> beliefs = agent.context().message(beliefSet.name());
                              beliefs->setFieldValue(CONSUMED_BATTERY_POLL_COUNT_0, agent.engine().pollCount());
                              return jack::Event::SUCCESS;
                          })
            .handleAction(consumeBatteryAction_1.name(),
                          [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle) {
                              std::shared_ptr<jack::Message> beliefs = agent.context().message(beliefSet.name());
                              beliefs->setFieldValue(CONSUMED_BATTERY_POLL_COUNT_1, agent.engine().pollCount());
                              return jack::Event::SUCCESS;
                          })
            .commitAsAgent();

    {
        jack::AgentHandle agentHandle = agentBuilder.createAgent("AgentWithBattery");
        jack::Agent *agent = bdi.getAgent(agentHandle);
        agent->pursue(consumeBatteryGoal_0, jack::GoalPersistent_No);
        agent->pursue(consumeBatteryGoal_1, jack::GoalPersistent_No);
        agent->start();

        size_t consumeBatteryAtPollCount_0 = 0;
        size_t consumeBatteryAtPollCount_1 = 0;

        while (consumeBatteryAtPollCount_0 == 0 || consumeBatteryAtPollCount_1 == 0) {
            enginePollAndRecordSchedules(bdi, 1 /*pollCount*/);
            bool found = true;

            // Query the poll counters from each goal's beliefset, indicating when the goal was complete
            if (consumeBatteryAtPollCount_0 == 0) {
                found &= agent->context().get<size_t>(CONSUMED_BATTERY_POLL_COUNT_0,
                                                      consumeBatteryAtPollCount_0);
            }

            if (consumeBatteryAtPollCount_1 == 0) {
                found &= agent->context().get<size_t>(CONSUMED_BATTERY_POLL_COUNT_1,
                                                      consumeBatteryAtPollCount_1);
            }

            // Ensure that at most, we only have 1 intention running at a time
            if (agent->desires().size() >= 1) {
                EXPECT_GT(agent->intentions().size(), 0)
                    << "If we have 2 goals that are dependent on the battery, meaning only 1 can "
                       "execute at a time, then at most, we can only have 1 intention running at a "
                       "time. The 2nd goal dependent on the battery will only execute after the first "
                       "intention has completed.";

                EXPECT_LT(agent->intentions().size(), 2)
                    << "If we have 2 goals that are dependent on the battery, meaning only 1 can "
                       "execute at a time, then at most, we can only have 1 intention running at a "
                       "time. The 2nd goal dependent on the battery will only execute after the first "
                       "intention has completed.";
            }

            assert(found); (void)found;
        }
        enginePollAndRecordSchedules(bdi, 1 /*pollCount*/, writer, true /*writeToFile*/);
        EXPECT_LT(consumeBatteryAtPollCount_0, consumeBatteryAtPollCount_1)
            << "Consuming the battery should happen with the 0th goal first before the 1st goal "
               "because they are both dependent on the battery lock. They should also happen one "
               "after another, not simultaneously on the same tick because of said lock conflict.";
    }
}

TEST_F(GoalsTest, RepeatedGoalMerge_NoParameters)
{
    /*! ***************************************************************************************
     * Verify a repeated perform goal is merged correct and calls the promise onces
     * Req:  <#>
     * ****************************************************************************************/

    // the count var to determine if we are successful
    int count = 0;

    jack::ActionBuilder action1 = bdi.action("PlanAction1").commit();
    jack::ActionBuilder action2 = bdi.action("PlanAction2").commit();

    jack::GoalBuilder goal = bdi.goal("BeSatisfiedGoal")
                                .satisfied([&](const jack::BeliefContext &context) -> bool {
                                    if (count > 5) {
                                        std::cout << "Goal Satisfied" << std::endl;
                                    }
                                    return count > 5;
                                })
                                .commit();

    auto plan1 = bdi.plan("BeSatisfiedPlan")
                    .handles(goal.name())
                    .body(bdi.coroutine()
                             .action(action1.name()))
                    .commit();

    jack::TacticHandle tactic = bdi.tactic("Tactic")
                                   .goal(goal.name())
                                   .loopPlansInfinitely()
                                   .commit();

    jack::AgentHandle agentHandle = bdi.agent("TestAgent1")
                                       .plan(plan1)
                                       .handleAction(action1.name(), [&] (jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                                           count++;
                                           std::cout << "count = " << count << std::endl;
                                           return jack::Event::SUCCESS;
                                       })
                                       .commitAsAgent()
                                       .createAgent("agent1");

    jack::Agent *agent = bdi.getAgent(agentHandle);
    agent->start();
    agent->selectTactic(tactic.m_name);

    int promiseCount = 0;
    bool done = false;

    auto goalId = jack::UniqueId::random();
    while(!done) {
        auto goalPursue = agent->pursue(goal.name(), jack::GoalPersistent_No, jack::Message{}, goalId);
        goalPursue.promise->then([&]() {
            std::cout << "Goal finished" << std::endl;
            done = true;
            promiseCount++;
        });
        bdi.poll();
    };

    EXPECT_EQ(done, true);
    EXPECT_EQ(count, 6);
    EXPECT_EQ(promiseCount, 1);
}

TEST_F(GoalsTest, GoalResourcesGetLocked)
{
    /*! ***************************************************************************************
     * A plan that is marked to engaged a lock in the PlanBuilder should
     * automatically be locked when the plan is executed.
     *
     * After the agent has completed the plan, the lock should no longer be
     * engaged.
     * ****************************************************************************************/
    jack::ResourceBuilder lockResource = bdi.resource("lock")
                                             .min(0)
                                             .max(1)
                                             .commit();

    //
    // Goal
    //
    jack::ActionBuilder action  = bdi.action("action").commit();
    jack::GoalBuilder   goal    = bdi.goal("Goal").commit();
    jack::PlanBuilder   plan    = bdi.plan("Plan")
                                     .handles(goal)
                                     .lock(lockResource)
                                     .effects([](jack::BeliefContext &context) -> void {})
                                     .body(bdi.coroutine().action(action.name()))
                                     .commit();

    //
    // Agent
    //
    bool planComplete = false;
    jack::AgentHandle agentHandle = bdi.agent("AgentTemplate")
                                       .plan(plan)
                                       .resource(lockResource)
                                       .handleAction(action.name(),
                                                     [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle) {
                                                         std::shared_ptr<jack::Resource> lock = agent.context().resource(lockResource.name());
                                                         EXPECT_EQ(lock->count(), 0) << "Expected lock to be engaged, plan just started";
                                                         planComplete = true;
                                                         return jack::Event::SUCCESS;
                                                     })
                                       .commitAsAgent()
                                       .createAgent("Agent");

    jack::Agent *agent = bdi.getAgent(agentHandle);

    // Ensure the lock is open at the beginning and can be locked once the agent
    // starts going through the plan
    std::shared_ptr<jack::Resource> lock = agent->context().resource(lockResource.name());
    ASSERT_EQ(lock->count(), lock->max()) << "Locks are expected to be unlocked on startup";

    // Run the agent until its finished
    agent->pursue(goal, jack::GoalPersistent_No);
    agent->start();
    while(!planComplete)
        bdi.poll();

    // Check the lock was released
    EXPECT_EQ(lock->count(), lock->max())
        << "After completing the plan, the lock should be unlocked.";
}

// varify that a goal's parameters are available in the plan pre-condition
TEST_F(GoalsTest, G101_GoalParamInPlanPre)
{
    bool actionOneCalled = false;
    bool actionTwoCalled = false;
    bool preOneFailed = false;
    bool preTwoFailed = false;

    // action messages
    bdi.action("DoTask1").commit();
    bdi.action("DoTask2").commit();

    auto doTaskGoalMsg = bdi.message("DoTaskGoal")
       .field<bool>("testA")
       .commit();

    auto doAnotherTaskGoalMsg = bdi.message("DoAnotherTaskGoal")
       .field<bool>("testB")
       .commit();

    // a goal to perform a task
    auto doTaskGoal = bdi.goal("DoTaskGoal")
                         .message(doTaskGoalMsg.name())
                         .commit();

    auto doTaskAnotherGoal = bdi.goal("DoAnotherTaskGoal")
                                .message(doAnotherTaskGoalMsg.name())
                                .commit();

    // plan to do a task
    // This plan will drop if the battery level drops below 25%
    // or all 10 tasks are completed
    auto doTaskPlan =
        bdi .plan       ("DoTaskPlan")
            .pre([&](const aos::jack::BeliefContext &context) {

                std::cout << "DoTaskPlan pre-condition" << std::endl;

                bool result = false;
                if(context.get<bool>("testA", result)) {
                    std::cout << "Plan pre-condition has goal parameter" << std::endl;
                    return true;
                } else {
                    preOneFailed = true;
                    std::cout << "Plan pre-condition is missing goal parameter" << std::endl;
                    return false;
                }
            })
            .handles    (doTaskGoal)
            .body       (bdi.coroutine ()
                            .action("DoTask1")
                        )
            .commit     ();

    auto doTaskAnotherPlan =
        bdi .plan       ("DoTaskAnotherPlan")
            .pre([&](const aos::jack::BeliefContext &context) {

                std::cout << "DoTaskAnotherPlan pre-condition" << std::endl;

                bool result = false;
                if(context.get<bool>("testB", result)) {
                    std::cout << "Plan pre-condition has goal parameter" << std::endl;
                    return true;
                } else {
                    std::cout << "Plan pre-condition is missing goal parameter" << std::endl;
                    preTwoFailed = true;
                    return false;
                }
            })
            .handles    (doTaskAnotherGoal)
            .body       (bdi.coroutine ()
                            .action("DoTask2")
                        )
            .commit     ();

    jack::AgentHandle agentHandle = bdi.agent("TestAgent1")
            .plans          (std::array{ doTaskPlan, doTaskAnotherPlan })
            .handleAction   ("DoTask1", [&] (jack::Agent& agent, jack::Message &msg, jack::Message &out, jack::ActionHandle) {
                                actionOneCalled = true;
                                std::cout << "DoTask done" << std::endl;
                                return jack::Event::SUCCESS;
                            })
            .handleAction   ("DoTask2", [&] (jack::Agent& agent, jack::Message &msg, jack::Message &out, jack::ActionHandle) {
                                actionTwoCalled = true;
                                std::cout << "DoTask done" << std::endl;
                                return jack::Event::SUCCESS;
                            })
            .commitAsAgent()
            .createAgent("agent1");

    jack::Agent *agent = bdi.getAgent(agentHandle);
    agent->start();

    auto bs1 = jack::Message(*bdi.getMessageSchema(doTaskGoalMsg.name()));
    bs1.setFieldValue("testA", true);
    auto goalPursue1 = agent->pursue("DoTaskGoal", jack::GoalPersistent_No, bs1);

    auto bs2 = jack::Message(*bdi.getMessageSchema(doAnotherTaskGoalMsg.name()));
    bs2.setFieldValue("testB", true);
    auto goalPursue2 = agent->pursue("DoAnotherTaskGoal", jack::GoalPersistent_No, bs2);

    // when the startWorkGoal concludes shutdown the agent.
    goalPursue1.promise->then([&](){
        std::cout << "DoTaskGoal done" << std::endl;
    });

    goalPursue2.promise->then([&](){
        std::cout << "DoAnotherTaskGoal done" << std::endl;
        agent->stop();
    });

    bdi.execute();

    EXPECT_EQ(preOneFailed, false);
    EXPECT_EQ(preTwoFailed, false);
    EXPECT_EQ(actionOneCalled, true);
    EXPECT_EQ(actionTwoCalled, true);
}

TEST_F(GoalsTest, SubGoalHeuristicChoosesPlanWithCheapestEffect)
{
    /*
       +--------------+              +--------------+
       |   MainGoal   |              |   SubGoal    |
       +--------------+              +--------------+
              |                             |
              |                     +-------+--------+
              |                     |                |
              V                     V                V
       +--------------+     +--------------+ +--------------+
       |   MainPlan   |     |   SubPlanA   | |   SubPlanB   |
       +--------------+     +--------------+ +--------------+
              |
              V
              O Start
              |
              V
       +--------------+
       |   SubGoal    |
       +--------------+
              |
              V
       +--------------+
       | FinishAction |
       +--------------+
              |
              V
              O End

        - SubGoal has 2 plans that handle it, SubPlanA and SubPlanB
        - SubGoal has a heuristic that wants to choose the plan that produces
          the cheapest cost by letting the plans model effects.
        - The plans set a cost value in the beliefset which the goal heuristic
          will read and return to the planner.
        - SubPlanB is defined to produce a lower cost than SubPlanA, hence
          SubPlanB should be executed.
     */

    const std::string COST_BELIEF = "cost";
    auto beliefSet = bdi.message("Beliefs")
                        .fieldWithValue<float>(COST_BELIEF, 0.f)
                        .commit();

    auto subGoal = bdi.goal("SubGoal")
                      .heuristic([&beliefSet, &COST_BELIEF](const jack::BeliefContext &context) -> float {
                          std::shared_ptr<jack::Message> beliefs = context.message(beliefSet.name());
                          float result = *beliefs->getPtr<float>(COST_BELIEF);
                          return result;
                      })
                      .commit();

    auto subPlanAAction = bdi.action("SubGoalPlanAAction").commit();
    auto subPlanBAction = bdi.action("SubGoalPlanBAction").commit();
    auto finishAction       = bdi.action("FinishAction").commit();

    auto subPlanA = bdi.plan("SubGoalPlanA")
                           .handles(subGoal)
                           .effects([&beliefSet, &COST_BELIEF](jack::BeliefContext &context) {
                                std::shared_ptr<jack::Message> beliefs = context.message(beliefSet.name());
                                beliefs->setFieldValue(COST_BELIEF, 2.0f);
                           })
                           .body(bdi.coroutine()
                                    .action(subPlanAAction.name()))
                           .commit();

    auto subPlanB = bdi.plan("SubGoalPlanB")
                           .handles(subGoal)
                           .effects([&beliefSet, &COST_BELIEF](jack::BeliefContext &context) {
                                std::shared_ptr<jack::Message> beliefs = context.message(beliefSet.name());
                                beliefs->setFieldValue(COST_BELIEF, 1.0f);
                           })
                           .body(bdi.coroutine()
                                    .action(subPlanBAction.name()))
                           .commit();


    auto mainGoal = bdi.goal("MainGoal").commit();
    auto mainPlan = bdi.plan("MainPlan")
                           .handles(mainGoal)
                           .body(bdi.coroutine()
                                    .goal(subGoal.name())
                                    .action(finishAction.name()))
                           .commit();

    bool subPlanAExecuted = false;
    bool subPlanBExecuted = false;

    jack::AgentHandle agentHandle =
        bdi.agent("Agent")
            .plans(std::array{mainPlan, subPlanA, subPlanB})
            .belief(beliefSet)
            .handleAction(subPlanAAction.name(),
                          [&subPlanAExecuted](jack::Agent &agent, jack::Message &, jack::Message &out, jack::ActionHandle) {
                              subPlanAExecuted = true;
                              return jack::Event::SUCCESS;
                          })
            .handleAction(subPlanBAction.name(),
                          [&subPlanBExecuted](jack::Agent &agent, jack::Message &, jack::Message &out, jack::ActionHandle) {
                              subPlanBExecuted = true;
                              return jack::Event::SUCCESS;
                          })
            .handleAction(finishAction.name(),
                          [](jack::Agent &agent, jack::Message &, jack::Message &out, jack::ActionHandle) {
                              agent.stop();
                              return jack::Event::SUCCESS;
                          })
            .commitAsAgent()
            .createAgent("agent");

    jack::Agent *agent = bdi.getAgent(agentHandle);
    agent->start();
    agent->pursue(mainGoal.name(), jack::GoalPersistent_Yes);

    enginePollAndRecordSchedules(bdi);
    EXPECT_FALSE(subPlanAExecuted);
    EXPECT_TRUE(subPlanBExecuted);
}

struct GoalsStrictPlanPolicy : public ::testing::Test {
    void SetUp() override
    {
        m_bdi.exitWhenDone();
        m_solveWorldHungerGoal = m_bdi.goal("Solve World Hunger Goal").commit().name();
        m_oneMealADayAction    = m_bdi.action("One Meal A Day Action").commit().name();

        auto oneMealADayPlan =
            m_bdi.plan("One Meal A Day")
                .handles(m_solveWorldHungerGoal)
                .body(m_bdi.coroutine()
                         .action(m_oneMealADayAction))
                .commit();
        m_oneMealADayPlan = oneMealADayPlan.name();

        m_donateToCharityAction = m_bdi.action("Donate To Charity Action").commit().name();
        auto donateToCharityPlan =
            m_bdi.plan("Donate To Charity Plan")
                .handles(m_solveWorldHungerGoal)
                .body(m_bdi.coroutine()
                         .action(m_donateToCharityAction))
                .commit();
        m_donateToCharityPlan = donateToCharityPlan.name();

        m_deliverFoodAction = m_bdi.action("Deliver Food Action").commit().name();
        auto deliverFoodPlan =
            m_bdi.plan("Deliver Food Plan")
                .handles(m_solveWorldHungerGoal)
                .body(m_bdi.coroutine()
                         .action(m_deliverFoodAction))
                .commit();
        m_deliverFoodPlan = deliverFoodPlan.name();

        m_agent =
            m_bdi.agent("AgentTemplate")
                .plans(std::array{oneMealADayPlan, donateToCharityPlan, deliverFoodPlan})
                .handleAction(
                    m_oneMealADayAction,
                    [&](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                        m_actionsExecuted[m_actionsExecutedCount++] = m_oneMealADayAction;
                        assert(m_actionsExecutedCount <= jack::arrayCountUSize(m_actionsExecuted));
                        return jack::Event::FAIL;
                    })
                .handleAction(
                    m_donateToCharityAction,
                    [&](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                        m_actionsExecuted[m_actionsExecutedCount++] = m_donateToCharityAction;
                        assert(m_actionsExecutedCount <= jack::arrayCountUSize(m_actionsExecuted));
                        return jack::Event::FAIL;
                    })
                .handleAction(
                    m_deliverFoodAction,
                    [&](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                        m_actionsExecuted[m_actionsExecutedCount++] = m_deliverFoodAction;
                        assert(m_actionsExecutedCount <= jack::arrayCountUSize(m_actionsExecuted));
                        return jack::Event::FAIL;
                    })
                .commitAsAgent()
                .createAgentInstance("Agent");
        m_agent->start();
    }

    void TearDown() override {
        /// \note Dump the order that the actions (plans) were executed.
        std::cout << "-- Begin Action Execute List (" << m_actionsExecutedCount << ") --\n";
        for (size_t index = 0; index < m_actionsExecutedCount; index++) {
            std::cout << "  " << m_actionsExecuted[index] << "\n";
        }
        std::cout << "-- End Action Execute List (" << m_actionsExecutedCount << ") --\n";
    }

    jack::Agent* m_agent;
    jack::Engine m_bdi;

    std::string  m_actionsExecuted[128] = {};
    int          m_actionsExecutedCount = 0;

    std::string  m_solveWorldHungerGoal;

    std::string  m_oneMealADayPlan;
    std::string  m_donateToCharityPlan;
    std::string  m_deliverFoodPlan;

    std::string  m_oneMealADayAction;
    std::string  m_donateToCharityAction;
    std::string  m_deliverFoodAction;
};

TEST_F(GoalsStrictPlanPolicy, PerformOutOfOrder3Plans)
{
    /**************************************************************************
     * A goal that uses a fixed plan policy executes all 3 plans in the order
     * specified by the plan list.
     **************************************************************************/

    /// \note Setup a non-persistent fixed plan policy
    jack::TacticHandle tacticHandle = m_bdi.tactic("Tactic")
         .goal(m_solveWorldHungerGoal)
         .planNames(std::array{std::string_view(m_donateToCharityPlan),
                               std::string_view(m_deliverFoodPlan),
                               std::string_view(m_oneMealADayPlan)})
         .planOrder(jack::Tactic::PlanOrder::Strict)
         .commit();

    /// \note Pursue to goal
    m_agent->selectTactic(tacticHandle.m_name);
    auto goalPursue = m_agent->pursue(m_solveWorldHungerGoal, jack::GoalPersistent_No);
    goalPursue.promise->then([&]() { m_agent->stop(); });

    /// \note Execute the test
    enginePollAndRecordSchedules(bdi);

    /// \note Check test result
    const jack::Tactic* tactic = m_bdi.getTactic(tacticHandle.m_name);
    ASSERT_EQ(m_actionsExecutedCount, tactic->plans().size()) << "Non persistent fixed plan policy, we should try all 3 plans and fail";
    EXPECT_EQ(m_actionsExecuted[0], m_donateToCharityAction);
    EXPECT_EQ(m_actionsExecuted[1], m_deliverFoodAction);
    EXPECT_EQ(m_actionsExecuted[2], m_oneMealADayAction);
}

TEST_F(GoalsStrictPlanPolicy, Perform2OutOf3Plans)
{
    /**************************************************************************
     * A goal that uses a fixed plan policy with 2 plans in the list. Both plans
     * should be attempted, failed and the goal should exit.
     **************************************************************************/

    /// \note Setup a non-persistent fixed plan policy
    jack::TacticHandle tacticHandle = m_bdi.tactic("Tactic")
         .goal(m_solveWorldHungerGoal)
         .planNames(std::array{std::string_view(m_deliverFoodPlan),
                               std::string_view(m_oneMealADayPlan)})
         .planOrder(jack::Tactic::PlanOrder::Strict)
         .commit();

    /// \note Pursue to goal
    m_agent->selectTactic(tacticHandle.m_name);
    auto goalPursue = m_agent->pursue(m_solveWorldHungerGoal, jack::GoalPersistent_No);
    goalPursue.promise->then([&]() { m_agent->stop(); });

    /// \note Execute the test
    enginePollAndRecordSchedules(bdi);

    /// \note Check test result
    const jack::Tactic* tactic = m_bdi.getTactic(tacticHandle.m_name);
    ASSERT_EQ(m_actionsExecutedCount, tactic->plans().size()) << "Non persistent fixed plan policy, we should try 2 out of 3 plans and fail it otherwise";
    EXPECT_EQ(m_actionsExecuted[0], m_deliverFoodAction);
    EXPECT_EQ(m_actionsExecuted[1], m_oneMealADayAction);
}

TEST_F(GoalsStrictPlanPolicy, PerformPlanListWithRepeats)
{
    /**************************************************************************
     * A goal that uses a fixed plan policy where the plan list repeats the
     * plans to be executed.
     **************************************************************************/

    /// \note Setup a non-persistent fixed plan policy
    jack::TacticHandle tacticHandle = m_bdi.tactic("Tactic")
         .goal(m_solveWorldHungerGoal)
         .planNames(std::array{std::string_view(m_deliverFoodPlan),
                               std::string_view(m_oneMealADayPlan),
                               std::string_view(m_deliverFoodPlan),
                               std::string_view(m_donateToCharityPlan),
                               std::string_view(m_donateToCharityPlan)})
         .planOrder(jack::Tactic::PlanOrder::Strict)
         .commit();

    /// \note Pursue to goal
    m_agent->selectTactic(tacticHandle.m_name);
    auto goalPursue = m_agent->pursue(m_solveWorldHungerGoal, jack::GoalPersistent_No);
    goalPursue.promise->then([&]() { m_agent->stop(); });

    /// \note Execute the test
    enginePollAndRecordSchedules(bdi);

    /// \note Check test result
    const jack::Tactic* tactic = m_bdi.getTactic(tacticHandle.m_name);
    ASSERT_EQ(m_actionsExecutedCount, tactic->plans().size()) << "Non persistent fixed plan policy, we repeat the plans in the plan list";
    EXPECT_EQ(m_actionsExecuted[0], m_deliverFoodAction);
    EXPECT_EQ(m_actionsExecuted[1], m_oneMealADayAction);
    EXPECT_EQ(m_actionsExecuted[2], m_deliverFoodAction);
    EXPECT_EQ(m_actionsExecuted[3], m_donateToCharityAction);
    EXPECT_EQ(m_actionsExecuted[4], m_donateToCharityAction);
}

TEST_F(GoalsStrictPlanPolicy, Pursue3Plans)
{
    /**************************************************************************
     * A goal that uses a fixed plan policy with 3 plans. We will cycle the
     * engine to ensure atleast 6 plans are ran and verify that the plans are
     * executed in the correct order.
     **************************************************************************/

    /// \note Setup a persistent fixed plan policy
    jack::TacticHandle tacticHandle = m_bdi.tactic("Tactic")
         .goal(m_solveWorldHungerGoal)
         .planNames(std::array{std::string_view(m_deliverFoodPlan),
                               std::string_view(m_oneMealADayPlan),
                               std::string_view(m_donateToCharityPlan)})
         .planOrder(jack::Tactic::PlanOrder::Strict)
         .loopPlansInfinitely()
         .commit();

    /// \note Pursue to goal
    m_agent->selectTactic(tacticHandle.m_name);
    auto goalPursue = m_agent->pursue(m_solveWorldHungerGoal, jack::GoalPersistent_Yes);
    goalPursue.promise->then([&]() { m_agent->stop(); });

    /// \note Execute the test
    const jack::Tactic* tactic = m_bdi.getTactic(tacticHandle.m_name);
    while (m_actionsExecutedCount != tactic->plans().size() * 2) {
        enginePollAndRecordSchedules(m_bdi, 1 /*pollCount*/);
    }

    /// \note Check test result
    ASSERT_EQ(m_actionsExecutedCount, tactic->plans().size() * 2) << "Persistent strict plan policy, we run until 6 actions are executed.";
    EXPECT_EQ(m_actionsExecuted[0], m_deliverFoodAction);
    EXPECT_EQ(m_actionsExecuted[1], m_oneMealADayAction);
    EXPECT_EQ(m_actionsExecuted[2], m_donateToCharityAction);

    /// \note We cycle the plan list twice, so they should repeat in this order.
    EXPECT_EQ(m_actionsExecuted[3], m_deliverFoodAction);
    EXPECT_EQ(m_actionsExecuted[4], m_oneMealADayAction);
    EXPECT_EQ(m_actionsExecuted[5], m_donateToCharityAction);
}

TEST(Goals, StrictPlanPolicyChainPlansCorrectOrder)
{
    /**************************************************************************
     * A goal that uses a fixed plan policy chains the plans in the fixed order
     * and interleaves any other goal inbetween.
     **************************************************************************/

    jack::Engine bdi;
    bdi.exitWhenDone();

    /**************************************************************************
     * Goal
     *************************************************************************/
    const std::string plansChainedBelief = "plansChained";
    jack::MessageBuilder beliefSet = bdi.message("Beliefset")
                                        .fieldWithValue<int>(plansChainedBelief, 0)
                                        .commit();

    jack::GoalBuilder solveWorldHungerGoal =
        bdi.goal("Solve World Hunger Goal")
            .satisfied([&beliefSet, plansChainedBelief](const jack::BeliefContext& context) {
                std::shared_ptr<jack::Message> bSet = context.message(beliefSet.name());
                int  plansChained                   = *bSet->getPtr<int>(plansChainedBelief);
                bool result                         = plansChained == 3;
                return result;
            })
            .commit();

    /**************************************************************************
     * Plans & Action
     *************************************************************************/
    jack::ActionBuilder oneMealADayAction = bdi.action("One Meal A Day Action").commit();
    jack::PlanBuilder oneMealADayPlan =
        bdi.plan("One Meal A Day")
            .handles(solveWorldHungerGoal)
            .effects([&plansChainedBelief, &beliefSet](jack::BeliefContext& context) {
                std::shared_ptr<jack::Message> bSet = context.message(beliefSet.name());
                const int* plansChained             = bSet->getPtr<int>(plansChainedBelief);
                bSet->setFieldValue<int>(plansChainedBelief, *plansChained + 1);
            })
            .body(bdi.coroutine()
                     .action(oneMealADayAction.name()))
            .commit();


    jack::ActionBuilder donateToCharityAction = bdi.action("Donate To Charity Action").commit();
    jack::PlanBuilder donateToCharityPlan =
        bdi.plan("Donate To Charity Plan")
            .handles(solveWorldHungerGoal)
            .effects([&plansChainedBelief, &beliefSet](jack::BeliefContext& context) {
                std::shared_ptr<jack::Message> bSet = context.message(beliefSet.name());
                const int* plansChained             = bSet->getPtr<int>(plansChainedBelief);
                bSet->setFieldValue<int>(plansChainedBelief, *plansChained + 1);
            })
            .body(bdi.coroutine()
                     .action(donateToCharityAction.name()))
            .commit();

    jack::ActionBuilder deliverFoodAction = bdi.action("Deliver Food Action").commit();
    jack::PlanBuilder deliverFoodPlan =
        bdi.plan("Deliver Food Plan")
            .handles(solveWorldHungerGoal)
            .effects([&plansChainedBelief, &beliefSet](jack::BeliefContext& context) {
                std::shared_ptr<jack::Message> bSet = context.message(beliefSet.name());
                const int* plansChained             = bSet->getPtr<int>(plansChainedBelief);
                bSet->setFieldValue<int>(plansChainedBelief, *plansChained + 1);
            })
            .body(bdi.coroutine()
                     .action(deliverFoodAction.name()))
            .commit();

    std::string actionsExecuted[128] = {};
    int         actionsExecutedCount = 0;

    /**************************************************************************
     * Agent
     *************************************************************************/
    jack::Agent* agent =
        bdi.agent("AgentTemplate")
            .plans(std::array{oneMealADayPlan,
                              donateToCharityPlan,
                              deliverFoodPlan})
            .belief(beliefSet)
            .handleAction(
                oneMealADayAction.name(),
                [&](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                    actionsExecuted[actionsExecutedCount++] = oneMealADayAction.name();
                    assert(actionsExecutedCount <= jack::arrayCountUSize(actionsExecuted));
                    return jack::Event::FAIL;
                })
            .handleAction(
                donateToCharityAction.name(),
                [&](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                    actionsExecuted[actionsExecutedCount++] = donateToCharityAction.name();
                    assert(actionsExecutedCount <= jack::arrayCountUSize(actionsExecuted));
                    return jack::Event::SUCCESS;
                })
            .handleAction(
                deliverFoodAction.name(),
                [&](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                    actionsExecuted[actionsExecutedCount++] = deliverFoodAction.name();
                    assert(actionsExecutedCount <= jack::arrayCountUSize(actionsExecuted));
                    return jack::Event::FAIL;
                })
            .commitAsAgent()
            .createAgentInstance("Agent");
    agent->start();

    /**************************************************************************
     * Test
     *************************************************************************/
    /// \note Setup a persistent fixed plan policy
    jack::TacticHandle tacticHandle = bdi.tactic("Tactic")
                                         .goal(solveWorldHungerGoal.name())
                                         .plans(std::array{deliverFoodPlan,
                                                           oneMealADayPlan,
                                                           donateToCharityPlan})
                                         .planOrder(jack::Tactic::PlanOrder::Strict)
                                         .commit();

    /// \note Pursue to goal
    agent->selectTactic(tacticHandle.m_name);
    auto goalPursue = agent->pursue(solveWorldHungerGoal, jack::GoalPersistent_Yes);
    goalPursue.promise->then([&]() { agent->stop(); });

    /// \note Execute the test
    const jack::Tactic* tactic = bdi.getTactic(tacticHandle.m_name);
    while (actionsExecutedCount != tactic->plans().size()) {
        enginePollAndRecordSchedules(bdi, 1 /*pollCount*/);
    }

    /// \note Check test result
    ASSERT_EQ(actionsExecutedCount, tactic->plans().size()) << "Persistent fixed plan policy, we run until 3 plans are chained.";
    EXPECT_EQ(actionsExecuted[0], deliverFoodAction.name());
    EXPECT_EQ(actionsExecuted[1], oneMealADayAction.name());
    EXPECT_EQ(actionsExecuted[2], donateToCharityAction.name());
}

TEST(Goals, TacticPlanListTurnedIntoASetWhenChoosingBestPlan)
{
    /**************************************************************************
     * A goal that selects the 'choose best plan' policy for plans should filter
     *out duplicate plans from the list when it is added to the engine.
     **************************************************************************/
    jack::Engine bdi;
    bdi.exitWhenDone();

    jack::GoalBuilder goal     = bdi.goal("Goal").commit();
    jack::ActionBuilder action = bdi.action("Action").commit();

    jack::PlanBuilder planA =
        bdi.plan("Plan A")
            .handles(goal)
            .body(bdi.coroutine()
                     .action(action.name()))
            .commit();

    jack::PlanBuilder planB =
        bdi.plan("Plan B")
            .handles(goal)
            .body(bdi.coroutine()
                     .action(action.name()))
            .commit();

    jack::TacticHandle tacticHandle = bdi.tactic("Tactic")
                                         .goal(goal.name())
                                         .plans(std::array{planB, planB, planA})
                                         .planOrder(jack::Tactic::PlanOrder::ChooseBestPlan)
                                         .commit();

    const jack::Tactic *tactic = bdi.getTactic(tacticHandle.m_name);
    ASSERT_EQ(tactic->plans().size(), 2) << "3 plans turns into 2 plans because we had 1 duplicate 'Plan B'";

    std::set<std::string> expect = {tactic->plans().begin(), tactic->plans().end()};
    ASSERT_EQ(expect.size(), 2);
    EXPECT_EQ(expect.count(planA.name()), 1);
    EXPECT_EQ(expect.count(planB.name()), 1);
}

#if 0
TEST_F(GoalsTest, GoalSatisfiedCallsPromise)
{
    /*! ***************************************************************************************
     * Verify a performed goal will call the promise when satified
     * Req:  <#>
     *
     * Currently fails because the satisfied condition is not checked between tasks only
     * When a plan finishes
     * ****************************************************************************************/

    // delay the goal
    bool good = false;
    bool one = false;
    bool two = false;

    bdi.action("PlanAction1").commit();
    bdi.action("PlanAction2").commit();

    bdi .goal("BeSatisfiedGoal")
        .satisfied([&](const jack::BeliefContext &context) -> bool {

                if (planCode == PlanCode::Plan1) {
                    std::cout << "Goal Satisfied" << std::endl;
                }

                return planCode == PlanCode::Plan1;
            })
        .commit();

    auto plan1 = bdi.plan("BeSatisfiedPlan")
        .handles    ("BeSatisfiedGoal")
        .body       (bdi.coroutine ()
                        .action("PlanAction1")
                        .action("PlanAction2")
                    )
        .commit();

    jack::Agent* agent = bdi.agent("TestAgent1")
            .plans          ({plan1})
            .handleAction   ("PlanAction1", [&] (jack::Agent& agent, jack::ActionEvent &action) {
                                std::cout << "PlanAction1" << std::endl;
                                planCode |= PlanCode::Plan1;
                                one = true;
                                return jack::Event::SUCCESS;
                            })
            .handleAction   ("PlanAction2", [&] (jack::Agent& agent, jack::ActionEvent &action) {
                                std::cout << "PlanAction2 - should not get called" << std::endl;
                                planCode |= PlanCode::NoPlan;
                                two = true;
                                return jack::Event::SUCCESS;
                            })
            .create         ("agent1");

    agent->start();
    auto goal = agent->perform("BeSatisfiedGoal");

    goal->then([&]() {
        std::cout << "Goal Promise Called" << std::endl;
        good = true;
    });

    bdi.poll(100);

    EXPECT_EQ(one, true);
    EXPECT_EQ(two, false); // should never be true
    EXPECT_EQ(good, true);
}
#endif

TEST(Goals, QueryFunctionsMustAlwaysSeeFieldFromGoalMessage)
{
    /**************************************************************************
     * Query functions in the goal and plan must always give us a context with
     * the message that was set when the goal was performed.
     **************************************************************************/
    jack::Engine bdi;

    std::string_view fieldName = "This_Variable_Must_Exist_In_The_Belief_Context_Always";
    jack::MessageBuilder msg   = bdi.message("Message")
                                    .field<int>(fieldName)
                                    .commit();
    const int FIELD_VALUE = 10;

    /// \note Create some helper functions to use in the goal/plan query
    /// callbacks. These functions will verify that the context contains the
    /// message that we performed the goal with.
    auto queryFunctionThatReturnsTrue = [fieldName, FIELD_VALUE](const jack::BeliefContext& context) -> bool {
        [[maybe_unused]] int dummy = 0;
        [[maybe_unused]] bool found = context.get<int>(fieldName, dummy);
        assert(found);
        assert(dummy == FIELD_VALUE);
        return true;
    };

    auto queryFunctionThatReturnsFalse = [fieldName, FIELD_VALUE](const jack::BeliefContext& context) -> bool {
        [[maybe_unused]] int dummy = 0;
        [[maybe_unused]] bool found = context.get<int>(fieldName, dummy);
        assert(found);
        assert(dummy == FIELD_VALUE);
        return false;
    };

    auto queryFunctionThatReturns0f = [fieldName, FIELD_VALUE](const jack::BeliefContext& context) -> float {
        [[maybe_unused]] int dummy = 0;
        [[maybe_unused]] bool found = context.get<int>(fieldName, dummy);
        assert(found);
        assert(dummy == FIELD_VALUE);
        return 0.f;
    };

    auto queryFunctionThatReturnsVoid = [fieldName, FIELD_VALUE](const jack::BeliefContext& context) -> void {
        [[maybe_unused]] int dummy = 0;
        [[maybe_unused]] bool found = context.get<int>(fieldName, dummy);
        assert(found);
        assert(dummy == FIELD_VALUE);
    };

    /// \note Construct the agent model and goals and plans that accept the
    /// query function.
    jack::GoalBuilder goal = bdi.goal("Goal")
                                .message(msg.name())
                                .pre(queryFunctionThatReturnsTrue)
                                .satisfied(queryFunctionThatReturnsFalse)
                                .dropWhen(queryFunctionThatReturnsFalse)
                                .heuristic(queryFunctionThatReturns0f)
                                .commit();

    jack::ActionBuilder action         = bdi.action("Action")
                                             .commit();

    jack::PlanBuilder plan = bdi.plan("Plan")
                                .handles(goal)
                                .body(bdi.coroutine()
                                         .action(action.name()))
                                .pre(queryFunctionThatReturnsTrue)
                                .dropWhen(queryFunctionThatReturnsFalse)
                                .effects(queryFunctionThatReturnsVoid)
                                .commit();

    jack::Agent *agent = bdi.agent("AgentTemplate")
                            .plan(plan)
                            .handleAction(action.name(),
                                          [&](jack::Agent& agent, jack::Message&, jack::Message&, jack::ActionHandle) {
                                              agent.stop();
                                              return jack::Event::SUCCESS;
                                          })
                            .commitAsAgent()
                            .createAgentInstance("Agent");

    /// \note Create the goal and message
    auto goalMsg  = jack::Message(bdi.createMessage(msg.name()));
    bool fieldSet = goalMsg.setFieldValue(fieldName, FIELD_VALUE, jack::Message::SetFlags_NONE);
    ASSERT_TRUE(fieldSet);

    /// \note Execute the goal, the query functions will trigger and we will
    /// check if the goal message is available.
    agent->start();
    agent->pursue(goal.name(), jack::GoalPersistent_No, goalMsg);

    bdi.exitWhenDone();
    bdi.execute();
}

TEST(Goals, DropForce)
{
    jack::Engine bdi;

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

    bool actionTriggered = false;
    jack::Agent* agent = bdi.agent("Agent Template")
                            .plan(plan)
                            .handleAction(action.name(), [&actionTriggered](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                                actionTriggered = true;
                                return jack::Event::Status::SUCCESS;
                            })
                            .commitAsAgent()
                            .createAgentInstance("Agent");

    /**************************************************************************
     * Pursue the goal
     **************************************************************************/
    jack::GoalHandle goalHandle = {};
    goalHandle.m_name           = goal.name();
    goalHandle.m_id             = jack::UniqueId::random();

    agent->start();
    agent->pursue(goal, jack::GoalPersistent_Yes, jack::Message{}, goalHandle.m_id);

    for (int iteration = 0; iteration < 32 /*tries*/; iteration++)
        bdi.poll();
    EXPECT_TRUE(actionTriggered);
    EXPECT_TRUE(agent->getDesire(goalHandle.m_id));

    /**************************************************************************
     * Try drop the goal, should succeed because we force drop
     **************************************************************************/
    agent->drop(goalHandle, "" /*reason*/);
    for (int iteration = 0; iteration < 32 /*tries*/; iteration++)
        bdi.poll();
    EXPECT_FALSE(agent->getDesire(goalHandle.m_id));
}

TEST(Goals, DropNormal)
{
    jack::Engine bdi;

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

    bool actionTriggered = false;
    jack::Agent* agent = bdi.agent("Agent Template")
                            .plan(plan)
                            .handleAction(action.name(), [&actionTriggered](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                                actionTriggered = true;
                                return jack::Event::Status::SUCCESS;
                            })
                            .commitAsAgent()
                            .createAgentInstance("Agent");

    /**************************************************************************
     * Pursue the goal
     **************************************************************************/
    jack::GoalHandle goalHandle = {};
    goalHandle.m_name           = goal.name();
    goalHandle.m_id             = jack::UniqueId::random();

    agent->start();
    agent->pursue(goal, jack::GoalPersistent_Yes, jack::Message{}, goalHandle.m_id);

    for (int iteration = 0; iteration < 32 /*tries*/; iteration++)
        bdi.poll();
    EXPECT_TRUE(actionTriggered);
    EXPECT_TRUE(agent->getDesire(goalHandle.m_id));

    /**************************************************************************
     * Try drop the goal, should fail because we don't drop persistent goals
     **************************************************************************/
    agent->dropWithMode(goalHandle, jack::protocol::DropMode_NORMAL, "" /*reason*/);
    for (int iteration = 0; iteration < 32 /*tries*/; iteration++)
        bdi.poll();
    EXPECT_TRUE(agent->getDesire(goalHandle.m_id));
}
