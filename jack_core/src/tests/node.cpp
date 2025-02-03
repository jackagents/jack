// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <jack/jack.h>
#include <gtest/gtest.h>

#include <vector>
#include <iterator>
#include <thread>
#include <any>

using namespace aos;

/**
 * @brief Test the API for setting and getting the node name
 */

TEST(NODE, NODENAME)
{
    jack::Engine bdi("PeanutNode");
    std::string name1 = bdi.name();
    EXPECT_EQ( name1, "PeanutNode");

    bdi.setName("SnoopyNode");
    std::string name2 = bdi.name();
    EXPECT_EQ( name2, "SnoopyNode");
}

/**
 * @brief Test the API for getting the node's running status
 */

TEST(NODE, NODESTATUS)
{
    jack::Engine bdi("Node");
    EXPECT_EQ(bdi.getStatus(), false);
    bdi.start();
    // give the engine some time to start
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(bdi.getStatus(), true);
    bdi.exit();
    bdi.join();
    EXPECT_EQ(bdi.getStatus(), false);
}

/**
 * @brief Test the API for getting the node's up time
 */

TEST(NODE, NODETIME)
{
    jack::Engine bdi("Node");
    EXPECT_EQ(bdi.getUpTime(), std::chrono::milliseconds(0));

    /// Try to make the OS give a time slice to the engine
    bdi.start();
    for (int tries = 0; tries < 5; tries++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (bdi.getUpTime() != std::chrono::milliseconds(0)) {
            break;
        }
    }

    EXPECT_GT(bdi.getUpTime(), std::chrono::milliseconds(0));
    JACK_DEBUG("Engine up time {} seconds", bdi.getUpTime().count());
    bdi.exit();
    bdi.join();
}

/**
 * @brief Test the API for getting the agent list and traversing the agent status
 */

TEST(NODE, AGENTLIST)
{
    // agent list
    std::vector<std::string> results;
    std::vector<std::string> expectedResult = { "bob", "alice", "mary" };

    // create the jack engine
    jack::Engine bdi("AgentNode");
    bdi.exitWhenDone();

    // create counting goals
    auto countGoal = bdi.goal("CountGoal")
        .commit();

    // create counting plans
    auto countPlan = bdi.plan("CountPlan")
                        .handles(countGoal)
                        .body(bdi.coroutine()
                                 .action("DoCount")
                                 .param("value", 1)
                                 .action("DoCount")
                                 .param("value", 2)
                                 .action("DoCount")
                                 .param("value", 3)
                                 .action("DoCount")
                                 .param("value", 6)
                                 .action("DoCount")
                                 .param("value", 7)
                                 .action("Quit"))
                         .commit();

    auto valueMsg = bdi.message("Value Msg")
                       .field<int>("value")
                       .commit();

    bdi.action("DoCount")
        .request(valueMsg.name())
        .commit();

    bdi.action("Quit")
        .commit();

    // create our simple counting agent
    auto countingAgentTemplate =
        bdi.agent("CountingAgent")
            .plan(countPlan)
            .handleAction("DoCount",
                          [&](jack::Agent& agent, jack::Message& request, jack::Message &, jack::ActionHandle handle) {
                              auto v = *request.getPtr<int>("value");
                              std::cout << agent.name() << ": counting " << v << std::endl;
                              return jack::Event::SUCCESS;
                          })
            .handleAction("Quit",
                          [&](jack::Agent& agent, jack::Message&, jack::Message &, jack::ActionHandle handle) {
                              agent.stop();
                              return jack::Event::SUCCESS;
                          })
            .commitAsAgent();

    auto bobHandle   = countingAgentTemplate.createAgent("bob");
    auto aliceHandle = countingAgentTemplate.createAgent("alice");
    auto maryHandle  = countingAgentTemplate.createAgent("mary");

    jack::Agent* bob   = bdi.getAgent(bobHandle);
    jack::Agent* alice = bdi.getAgent(aliceHandle);
    jack::Agent* mary  = bdi.getAgent(maryHandle);
    bob->start();
    bob->pursue(countGoal, jack::GoalPersistent_No);

    alice->start();
    alice->pursue(countGoal, jack::GoalPersistent_No);

    mary->start();
    mary->pursue(countGoal, jack::GoalPersistent_No);

    // if the result contains the expected result we pass
    {
        auto agentList = bdi.agentList();
        JACK_DEBUG_MSG("Agent List");
        for (const auto &handle : agentList) { JACK_DEBUG_MSG(handle.toHumanString()); }

        EXPECT_EQ(expectedResult.size(), agentList.size());
        EXPECT_EQ(agentList[0].m_name, expectedResult[0]);
        EXPECT_EQ(agentList[1].m_name, expectedResult[1]);
        EXPECT_EQ(agentList[2].m_name, expectedResult[2]);

        // just loop through the whole list for now
        for (const auto &handle : agentList) {
            const jack::Agent* agent = bdi.getAgent(handle);
            EXPECT_EQ(agent->stopped(), true);
            EXPECT_EQ(agent->running(), false);
        }
    }

    bdi.execute();

    // if the result contains the expected result we pass
    {
        auto agentList = bdi.agentList();
        JACK_DEBUG_MSG("Agent List");
        for (const auto &handle : agentList) { JACK_DEBUG_MSG(handle.toHumanString()); }

        EXPECT_EQ(expectedResult.size(), agentList.size());
        EXPECT_EQ(agentList[0].m_name, expectedResult[0]);
        EXPECT_EQ(agentList[1].m_name, expectedResult[1]);
        EXPECT_EQ(agentList[2].m_name, expectedResult[2]);

        // just loop through the whole list for now
        for (const auto &handle : agentList) {
            jack::Agent* agent = bdi.getAgent(handle);
            results.push_back(agent->name());
            EXPECT_EQ(agent->stopped(), true);
            EXPECT_EQ(agent->running(), false);
        }
    }
}

/**
 * @brief Test the API for getting the agent's up time
 */

TEST(NODE, AGENTUPTIME)
{
    /// Create Agent Model
    jack::Engine bdi("AgentNode");

    auto valueMsg = bdi.message("Value Msg")
                       .field<int>("value")
                       .commit();

    auto finishAction = bdi.action("Finish")
                           .request(valueMsg.name())
                           .commit();

    auto sleepGoal = bdi.goal("SleepGoal").commit();

    int TARGET_SLEEP_MS = 100;
    auto sleepPlan = bdi.plan("SleepPlan")
                         .handles(sleepGoal)
                         .body(bdi.coroutine()
                                   .sleep(TARGET_SLEEP_MS) /// Force the plan to take atleast 1 second
                                   .action(finishAction.name()))
                         .commit();

    /// Create Agent
    std::string bobName   = "bob";
    std::string aliceName = "alice";
    std::string maryName  = "mary";

    std::atomic<bool> bobFinished(false);
    std::atomic<bool> aliceFinished(false);
    std::atomic<bool> maryFinished(false);

    auto agentTemplate = bdi.agent("SleepingAgent")
                            .plan(sleepPlan)
                            .handleAction(finishAction.name(),
                                          [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle) {
                                              if (agent.name() == bobName)        { bobFinished = true; }
                                              else if (agent.name() == aliceName) { aliceFinished = true; }
                                              else if (agent.name() == maryName)  { maryFinished = true; }
                                              return jack::Event::SUCCESS;
                                          })
                            .commitAsAgent();

    jack::AgentHandle bobHandle   = agentTemplate.createAgent(bobName);
    jack::AgentHandle aliceHandle = agentTemplate.createAgent(aliceName);
    jack::AgentHandle maryHandle  = agentTemplate.createAgent(maryName);

    /// Start Test
    jack::Agent* bob   = bdi.getAgent(bobHandle);
    jack::Agent* alice = bdi.getAgent(aliceHandle);
    jack::Agent* mary  = bdi.getAgent(maryHandle);
    bdi.start(); /// Start engine in separate thread

    /// For each agent, let them run the sleep goal and check their uptime
    /// results are correct.
    bob->pursue(sleepGoal, jack::GoalPersistent_Yes);
    bob->start();
    while (!bobFinished) { std::this_thread::sleep_for(std::chrono::milliseconds(TARGET_SLEEP_MS)); }
    auto bobT0 = bob->getUpTime();
    EXPECT_GE(bobT0.count(), TARGET_SLEEP_MS);

    alice->pursue(sleepGoal, jack::GoalPersistent_Yes);
    alice->start();
    while (!aliceFinished) { std::this_thread::sleep_for(std::chrono::milliseconds(TARGET_SLEEP_MS)); }
    auto bobT1   = bob->getUpTime();
    auto aliceT0 = alice->getUpTime();
    EXPECT_GT(bobT1, bobT0);
    EXPECT_GE(aliceT0.count(), TARGET_SLEEP_MS);

    bob->stop();
    mary->pursue(sleepGoal, jack::GoalPersistent_Yes);
    mary->start();
    while (!maryFinished) { std::this_thread::sleep_for(std::chrono::milliseconds(TARGET_SLEEP_MS)); }
    while (!bob->stopped()) { std::this_thread::sleep_for(std::chrono::milliseconds(TARGET_SLEEP_MS)); }
    auto bobT2   = bob->getUpTime();
    auto aliceT1 = alice->getUpTime();
    auto maryT0  = mary->getUpTime();
    EXPECT_GT(aliceT1, aliceT0);
    EXPECT_GE(bobT2, bobT1);
    EXPECT_GE(maryT0.count(), TARGET_SLEEP_MS);

    bdi.exit();
    bdi.join();
}

// an ugly way to convert a Belief to a string
std::string to_string(const std::any& belief)
{
    try {
        return std::to_string(std::any_cast<int>(belief));
    } catch(std::bad_any_cast e) {
    }

    try {
        return std::to_string(std::any_cast<bool>(belief));
    } catch(std::bad_any_cast e) {
    }

    try {
        return std::any_cast<std::string>(belief);
    } catch(std::bad_any_cast e) {
    }

    return std::string("unknown belief type");
}

/**
 * @brief Test the API for getting the agent's beliefs
 */
TEST(NODE, AGENTBELIEFS)
{
    std::vector<std::string> expected_sets = { "CountingBS", "BlahBS" };
    std::vector<std::string> expected_beliefs = { "count", "target", "a", "b" };
    std::vector<std::string> result_beliefs;

    // create the jack engine
    jack::Engine bdi("AgentNode");

    // create counting goals
    auto countGoal = bdi.goal("CountGoal")
        .commit();

    // create counting plans
    auto countPlan = bdi.plan("CountPlan")
        .handles (countGoal)
        .body    (bdi.coroutine()
                    .action("DoCount").param("value", 1)
                    .action("DoCount").param("value", 2)
                    .action("DoCount").param("value", 3)
                    .action("DoCount").param("value", 6)
                    .action("DoCount").param("value", 7)
                 )
        .commit  ();

    auto valueMsg = bdi.message("Value Msg")
                       .field<int>("value")
                       .commit();

    auto countAction = bdi.action("DoCount")
                          .request(valueMsg.name())
                          .commit();

        // create a belief set template
    auto bs1 = bdi.message("CountingBS")
                  .fieldWithValue<int>("count", 0)
                  .fieldWithValue<std::string>("target", "hello")
                  .commit();

    auto bs2 = bdi.message("BlahBS")
                  .fieldWithValue<int>("a", 0)
                  .fieldWithValue<std::string>("b", "world")
                  .commit();

    // create our simple counting agent
    auto builder = bdi.agent("CountingAgent")
        .beliefs(std::array{bs1, bs2})
        .plan(countPlan)
        .handleAction("DoCount",
            [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle) {
                int v = *msg.getPtr<int>("value");
                agent.message("CountingBS")->setFieldValue("count", v);
                fmt::print("{ agent: {}, beliefsets: ", agent.name());

                // print belief sets for the agent
                fmt::print("[ ");
                auto bsNames2 = agent.beliefSetNames();
                for(auto s : bsNames2) {
                    std::cout << s << ": { ";
                    auto& context = agent.context();
                    auto bs = context.message(s);
                    bs->toString(""sv);
                }
                fmt::println(" ] }");
                return jack::Event::SUCCESS;
            })
        .commitAsAgent();

    jack::AgentHandle bobHandle = builder.createAgent("bob");
    jack::Agent*      bob       = bdi.getAgent(bobHandle);
    auto              bsNames2  = bob->beliefSetNames();
    for(auto s : bsNames2) {
        const auto& context = bob->context();
        const auto& bs      = context.message(s);
        const auto& fields  = bs->fields();
        for (const auto& it : fields) {
            result_beliefs.push_back(it.first);
        }
    }

    bob->start();

    bdi.start();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    EXPECT_EQ( (expected_sets.size() == bsNames2.size()) &&
                std::equal(std::begin(expected_sets), std::end(expected_sets), std::begin(bsNames2)),
                true);

    EXPECT_EQ( (expected_beliefs.size() == result_beliefs.size()) &&
                std::equal(std::begin(expected_beliefs), std::end(expected_beliefs), std::begin(result_beliefs)),
                true);

    // shutdown test
    bdi.exit();
    bdi.join();
}

#if defined(JACK_SHARED_MEMORY_DEBUGGING)
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

/**
 * @brief Test the API for getting the agent's beliefs
 */
TEST(NODE, AGENTSHAREDMEMORY)
{
#if defined(JACK_SHARED_MEMORY_DEBUGGING)
    /// split into two processes - one to write and one to read
    if (fork() == 0)
    {
        // spy on the shared memory
        jack::shared::SharedMemoryModel model(false);

        for (int i = 0; i < 10; i++) {
            auto em = model.readEngine();
            auto engineName = em.name();

            printf("Engine name: %s\n",engineName->c_str());

            auto agentList = em->agents();
            for (int j = 0; j < agentList->Length(); j++) {
                printf("Agent: %s\n", agentList->Get(j).name()->c_str());
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        return;
    }

    std::vector<std::string> expected_sets = { "CountingBS", "BlahBS" };
    std::vector<std::string> expected_beliefs = { "count", "target", "a", "b" };
    std::vector<std::string> result_beliefs;

    // create the jack engine
    jack::Engine bdi("AgentNode");

    // create counting goals
    auto countGoal = bdi.goal("CountGoal")
        .commit();

    // create counting plans
    auto countPlan = bdi.plan("CountPlan")
        .handles (countGoal)
        .body    (bdi.coroutine()
                    .action("DoCount").param("value", 1)
                    .action("DoCount").param("value", 2)
                    .action("DoCount").param("value", 3)
                    .action("DoCount").param("value", 6)
                    .action("DoCount").param("value", 7)
                 )
        .commit  ();

    auto countAction = bdi.action("DoCount")
        .prop<int>("value")
        .commit();

        // create a belief set template
    auto bs1 = bdi.beliefset("CountingBS")
        .var<int>("count", 0)
        .var<std::string>("target", "hello")
        .commit();

    auto bs2 = bdi.beliefset("BlahBS")
        .var<int>("a", 0)
        .var<std::string>("b", "world")
        .commit();

    // create our simple counting agent
    auto builder = bdi.agent("CountingAgent")
        .beliefs(bs1)
        .beliefs(bs2)
        .plans ({ countPlan })
        .handleAction("DoCount",
            [&](jack::Agent& agent, jack::ActionEvent& action) {
                int v = action.get<int>("value");
                agent.beliefSet("CountingBS")->set<int>("count", v);
                std::cout << "{ agent: " << agent.name() << ", beliefsets: ";

                // print belief sets for the agent
                std::cout << "[ ";
                auto bsNames2 = agent.beliefSetNames();
                for(auto s : bsNames2) {
                    std::cout << s << ": { ";
                    auto& context = agent.context();
                    auto bs = context.beliefSet(s);
                    auto keys = bs->keys();
                    for(auto k : keys) {
                        auto b = bs->get(k);
                        std::cout << k << ": " << to_string(b) << ", ";
                    }
                    std::cout << " }, ";
                }
                std::cout << " ] }" << std::endl;

                return jack::Event::SUCCESS;
            })
        .commit();

    builder.create("bob")->start();
    builder.create("alice")->start();
    builder.create("mary")->start();

    jack::Agent* bob = builder.create("bob");
    auto bsNames2 = bob->beliefSetNames();
    for(auto s : bsNames2) {
        auto& context = bob->context();
        auto bs = context.beliefSet(s);
        auto keys = bs->keys();
        for(auto k : keys) {
            result_beliefs.push_back(k);
        }
    }

    bob->start();
    bob->pursue(countGoal);

    bdi.start();

    // hang around awhile
    std::this_thread::sleep_for(std::chrono::seconds(1));

    EXPECT_EQ( (expected_sets.size() == bsNames2.size()) &&
                std::equal(std::begin(expected_sets), std::end(expected_sets), std::begin(bsNames2)),
                true);

    EXPECT_EQ( (expected_beliefs.size() == result_beliefs.size()) &&
                std::equal(std::begin(expected_beliefs), std::end(expected_beliefs), std::begin(result_beliefs)),
                true);

    // shutdown test
    bdi.exit();
    bdi.join();
#endif // JACK_SHARED_MEMORY_DEBUGGING
}


