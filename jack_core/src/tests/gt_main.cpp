// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <jack/jack.h>
#include <gtest/gtest.h>

#include <vector>

using namespace aos;

// The fixture for testing class JACKTest.
class JACKTest : public ::testing::Test {
    protected:
    // You can remove any or all of the following functions if its body
    // is empty.

    JACKTest() : didRun(false) {
        // You can do set-up work for each test here.
        bdi.exitWhenDone();
    }

    ~JACKTest() override {
        // You can do clean-up work that doesn't throw exceptions here.
    }

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following methods:

    void SetUp() override {
        // Code here will be called immediately after the constructor (right
        // before each test).

        bdi.action("doStuffAction").commit();
        bdi.action("shutdownBDIAction").commit();

        jack::GoalBuilder testGoal = bdi.goal("TestGoal")
                                  .pre([&](const jack::BeliefContext &) {
                                      return !didRun;
                                  })
                                  .satisfied([&](const jack::BeliefContext &) {
                                      return didRun;
                                  })
                                  .commit();

        jack::PlanBuilder testPlan = bdi.plan("TestPlan")
                                  .handles(testGoal)
                                  .body(bdi.coroutine()
                                            .action("doStuffAction")
                                            .action("shutdownBDIAction"))
                                  .commit();

        bdi.agent("TestAgent")
            .plan(testPlan)
            .handleAction("doStuffAction",
                [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle handle) {
                didRun = true;
                return jack::Event::SUCCESS;
            })
            .handleAction("shutdownBDIAction",
                [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle handle) {
                agent.stop();
                agent.stop(); // force bdi engine to shutdown
                return jack::Event::SUCCESS;
            })
            .commitAsAgent();
    }

    void TearDown() override {
        // Code here will be called immediately after each test (right
        // before the destructor).
    }

    // Objects declared here can be used by all tests in the test case.
    jack::Engine bdi;
    bool didRun;
};

// create an agent and pursue a simple goal with plan
TEST_F(JACKTest, AgentWillRunPlan)
{
    jack::AgentHandle handle = bdi.createAgent("TestAgent", "bob");
    jack::Agent *     agent  = bdi.getAgent(handle);
    agent->start();
    agent->pursue("TestGoal", jack::GoalPersistent_Yes);
    bdi.execute();
    EXPECT_EQ(didRun, true);
}

// create and destroy an agent
TEST_F(JACKTest, CreateAndDestroyAgent)
{
    jack::AgentHandle handle = bdi.createAgent("TestAgent", "bob");
    bdi.destroyAgent(handle);
    EXPECT_EQ(bdi.agentList().size(), 0);
}

// create an agent
TEST_F(JACKTest, CreateAgent)
{
    bdi.createAgent("TestAgent", "bob");

    EXPECT_EQ(bdi.agentList().size(), 1);
}
