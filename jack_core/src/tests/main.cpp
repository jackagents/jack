// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <jack/jack.h>
#include <gtest/gtest.h>

#include <vector>
#include <iterator>
#include <thread>

using namespace aos;

/**
 * @brief Test the engine timers
 */
/*
TEST(VSTEST, TESTSLEEP)
{
    // create the jack engine
    jack::Engine bdi;
    bdi.exitWhenDone();

    // create a single goal to count to ten
    auto sleepAndWakeGoal = bdi.goal("SleepAndWakeGoal")
        .commit();

    // create a single plan that will count to ten
    auto sleepAndWakePlan = bdi.plan("SleepAndWakePlan")
        .handles (sleepAndWakeGoal)
        .body    (bdi.coroutine()
                     .print("Before Sleeping\n")
                     .sleep(1000)
                     .print("After Sleeping\n")
                     .print("Before Sleeping\n")
                     .sleep(1000)
                     .print("After Sleeping\n")
                 )
        .commit  ();

    // create our simple counting agent
    auto countingAgent = bdi.agent("CountingAgent")
        .plans ({ sleepAndWakePlan })
        .main  (bdi.coroutine()
                    .print("start main coroutine\n")
                    .goal("SleepAndWakeGoal")
                    .print("end main coroutine\n")
               )
        .create("bob");

    countingAgent->start();

    // run the jack engine
    bdi.execute();

    // if the result contains the expected result we pass
    EXPECT_EQ( true, true );
}
*/
// a simple model with observability
class MyModel
{
public:

    MyModel()
        : count(0)
        , flag(false)
    {
    }

    void setCount(const int &v)
    {
        count = v;
        for (auto ob : m_observers) {
            ob();
        }
    }

    int getCount() const { return count; }

    void addObserver(std::function<void()> &func)
    {
        m_observers.push_back(func);
    }

    int count;
    bool flag;

protected:
    // a dumb observer pattern
    std::vector<std::function<void()>> m_observers;
};

/**
 * @brief JACK Virtical Slice Demo 1
 */
/*
/// TODO: fix this to wrap the model above
class MyCutomModelWrapper : public jack::CustomBeliefSet<MyCutomModelWrapper>
{
public:

    MyCutomModelWrapper(const std::string &name)
        : jack::CustomBeliefSet<MyCutomModelWrapper>("MyCutomModelWrapper")
    {
        set<int>("count", 0);
        set<bool>("flag", false);
    }
};

TEST(VSTEST, TEST1)
{
    std::vector<int> result;
    std::vector<int> expected_result = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    // create the jack engine
    jack::Engine bdi;
    bdi.exitWhenDone();

    auto bs = bdi.beliefset<MyCutomModelWrapper>("MyCutomModelWrapper").commit();

    // create a single goal to count to ten
    auto countToTenGoal = bdi.goal("CountToTenGoal")
        .satisfied("flag")
        .commit();

    // create a single plan that will count to ten
    auto countToFivePlan = bdi.plan("CountToFivePlan")
        .handles (countToTenGoal)
        .pre("count == 0")
        .dropWhen("count > 4")
        .body    (bdi.coroutine()
                     .action("Count") // 1
                     .action("Count") // 2
                     .action("Count") // 3
                     .action("Count") // 4
                     .action("Count") // 5 we will drop before here
                     .action("Count") // 6
                     .action("Count") // 7
                 )
        .commit  ();

        // create a single plan that will count to ten
    auto countToTenPlan = bdi.plan("CountToTenPlan")
        .handles (countToTenGoal)
        .pre("count > 9")
        .body    (bdi.coroutine()
                     .action("SetFlag")
                 )
        .commit  ();

    // create a single plan that will count to ten
    auto countToNinePlan = bdi.plan("CountToNinePlan")
        .handles (countToTenGoal)
        .pre("count > 4")
        .body    (bdi.coroutine()
                     .action("Count")
                 )
        .commit  ();

    auto countAction = bdi.action("Count")
      //  .prop<int>("int", "amount")
        .commit();

    auto setFlagAction = bdi.action("SetFlag")
        .commit();

    // create our simple counting agent
    auto countingAgent = bdi.agent("CountingAgent")
        .beliefs(bs)
        .plans ({ countToFivePlan, countToTenPlan, countToNinePlan })
        .main  (bdi.coroutine()
                   .goal("CountToTenGoal")
               )
        .handleAction("SetFlag",
            [&](jack::Agent& agent, jack::Action& action) {
                agent.beliefSet("MyCutomModelWrapper")->set("flag", true);
                return jack::Action::SUCCESS;
            })
        .handleAction("Count",
            [&](jack::Agent& agent, jack::Action& action) {
                int c;
                agent.beliefSet("MyCutomModelWrapper")->get<int>("count", c);
                agent.beliefSet("MyCutomModelWrapper")->set<int>("count", c+1);
                JACK_INFO("hello world {}", c);
                result.push_back(result.size() + 1);
                return jack::Action::SUCCESS;
            })
        .create("bob");

    countingAgent->start();

    // run the jack engine
    bdi.execute();

    // print the result
    std::copy(result.begin(), result.end(), std::ostream_iterator<int>(std::cout, " "));

    // if the result contains the expected result we pass
    EXPECT_EQ( (expected_result.size() == result.size()) &&
                std::equal(std::begin(expected_result), std::end(expected_result), std::begin(result)),
                true);
}*/

/**
 * @brief JACK Context test
 */
/*
TEST(VSTEST, TEST2)
{
    std::vector<int> result;
    std::vector<int> expected_result = { 3, 6, 9, 12 };

    // create the jack engine
    jack::Engine bdi;
    bdi.exitWhenDone();

    bdi .activity("IncrementCount")
        .main([](const jack::BeliefContext &context, jack::BeliefSet &out) {

            int amount;
            context.get<int>("amount", amount);
            std::cout << "amount: " << amount << std::endl;

            int count;
            context.get<int>("count", count);
            std::cout << "count: " << count << std::endl;

            // set the output
            out.set<int>("count", count + amount);
        })
        .commit();

    auto bs = bdi.beliefset("ABC")
        .var<int>("count", 0)
        .var<int>("amount", 3)
        .commit();

    // create a single goal to count to ten
    auto countToNGoal = bdi.goal("CountToNGoal")
        // .prop<int>("target") // need type in the model
        /// \todo formally model the parameters of the goal
        .satisfied([](const jack::BeliefContext &context) {

            assert(context.goal() != nullptr);

            int count = 0;
            context.beliefSet("ABC")->get<int>("count", count);

            int target = 1;
            context.goal()->get<int>("target", target);
            return count > target;
        })
        .commit();

    // create a single plan that will count to ten
    auto countToNPlan = bdi.plan("CountToNPlan")
        .handles (countToNGoal)
        .body    (bdi.coroutine()
                    // an example of an action with a literal parameter
                    .action("IncrementCount").param("count", 0)
                    // an example of an activity
                    .activity("IncrementCount")
                    // an example of an action with a context parameter
                    .action("WriteCount").param("count")
                 )
        .commit  ();

    auto countAction = bdi.action("WriteCount")
        .prop("count", "int")
        .commit();

    auto increamentAction = bdi.action("IncrementCount")
        .prop("count", "int")
        .commit();

    // create our simple counting agent
    auto countingAgent = bdi.agent("CountingAgent")
        .beliefs(bs)
        .plans ({ countToNPlan })
        .main  (bdi.coroutine()
                   .goal("CountToNGoal").param("target", 10)
               )
        .handleAction("WriteCount",
            [&](jack::Agent& agent, jack::Action& action) {
                int count = action.get<int>("count");
                agent.beliefSet("ABC")->set<int>("count", count);
                result.push_back(count);
                return jack::Action::SUCCESS;
            })
        .handleAction("IncrementCount",
            [&](jack::Agent& agent, jack::Action& action) {
                int amount = action.get<int>("count");
                int count;
                if (agent.beliefSet("ABC")->get<int>("count", count)) {
                    agent.beliefSet("ABC")->set<int>("count", count + amount);
                }
                return jack::Action::SUCCESS;
            })
        .create("bob");

    countingAgent->start();

    // run the jack engine
    bdi.execute();

    // print the result
    std::copy(result.begin(), result.end(), std::ostream_iterator<int>(std::cout, " "));

    // if the result contains the expected result we pass
    EXPECT_EQ( (expected_result.size() == result.size()) &&
                std::equal(std::begin(expected_result), std::end(expected_result), std::begin(result)),
                true);
}*/

/**
 * @brief Trigger a goal to pursue when an id changes
 * If the id also changes while the current goal is actiive it will drop
 * and pursue the new goal
 */

TEST(VSTEST, TEST3)
{
    std::vector<int> result;
    std::vector<int> expected_result = { 20, 30 };

    // create the jack engine
    jack::Engine bdi;
    bdi.exitWhenDone();

    jack::MessageBuilder bs = bdi.message("ABC")
                                 .fieldWithValue<int>("current_id", -1)
                                 .commit();

    jack::MessageBuilder achieveMissionMsg = bdi.message("AchieveMission")
                                                .field<int>("mission_id")
                                                .commit();

    // create a goal to achieve the mission
    auto achieveMission = bdi.goal("AchieveMission")
        .message(achieveMissionMsg.name())
        .dropWhen([&](const jack::BeliefContext &context) {
            int mission_id = 0;
            context.get<int>("mission_id", mission_id);

            int current_id = 0;
            context.get("current_id", current_id);

            if (mission_id != current_id) {
                std::cout << "dropping goal id: " << mission_id << " current: " << current_id <<  std::endl;
            }

            return mission_id != current_id;
        })
        .commit();

    auto performTestGoal = bdi.goal("PerformTestGoal").commit();

    // create a single plan that will count to ten
    auto doMissionPlan = bdi.plan("DoMissionPlan")
        .handles (achieveMission)
        .body    (bdi.coroutine()
                    .sleep(1000)
                    .action("PerformMission"))
        .commit  ();

    // create a single plan that will count to ten
    auto performTestPlan = bdi.plan("PerformTestGoal")
        .handles (performTestGoal)
        .body    (bdi.coroutine()
                    .action("SetCurrentMission")
                    .param("mission_id", 10)
                    .goal("AchieveMission")
                    .param("mission_id", 10).nowait()
                    .sleep(1) // not enough time it will be dropped before being completed
                    .action("SetCurrentMission")
                    .param("mission_id", 20)
                    .goal("AchieveMission")
                    .param("mission_id", 20).nowait()
                    .sleep(1500) // plenty of time to finish the mission
                    .action("SetCurrentMission")
                    .param("mission_id", 30)
                    .goal("AchieveMission")
                    .param("mission_id", 30).nowait()
                    .sleep(1500) // plenty of time to finish the mission
                    .action("SetCurrentMission")
                    .param("mission_id", -1)
                    .action("Quit")
                 )
        .commit  ();

    auto countAction = bdi.action("PerformMission")
        .request(achieveMissionMsg.name())
        .commit();

    auto setMission = bdi.action("SetCurrentMission")
        .request(achieveMissionMsg.name())
        .commit();

    auto quitMission = bdi.action("Quit")
        .commit();

    // create our simple counting agent
    auto countingAgentHandle =
        bdi.agent("CountingAgent")
            .belief(bs)
            .plans(std::array{doMissionPlan, performTestPlan})
            .handleAction("PerformMission",
                          [&](jack::Agent &, jack::Message &request,
                              jack::Message &reply, jack::ActionHandle) {
                            auto id = *request.getPtr<int>("mission_id");
                            std::cout << "Performing mission " << id << "\n";
                            result.push_back(id);
                            return jack::Event::SUCCESS;
                          })
            .handleAction("SetCurrentMission",
                          [&](jack::Agent &agent, jack::Message &msg,
                              jack::Message &out, jack::ActionHandle handle) {
                            int id = *msg.getPtr<int>("mission_id");
                            std::cout << "Setting mission " << id << std::endl;
                            agent.message("ABC")->setFieldValue("current_id",
                                                                id);
                            return jack::Event::SUCCESS;
                          })
            .handleAction("Quit",
                          [&](jack::Agent &agent, jack::Message &msg,
                              jack::Message &out, jack::ActionHandle handle) {
                            agent.stop();
                            return jack::Event::SUCCESS;
                          })
            .commitAsAgent()
            .createAgent("bob");

    jack::Agent* countingAgent = bdi.getAgent(countingAgentHandle);
    countingAgent->start();
    countingAgent->pursue(performTestGoal.name(), jack::GoalPersistent_No);

    // run the jack engine
    bdi.execute();

    // print the result
    std::copy(result.begin(), result.end(), std::ostream_iterator<int>(std::cout, " "));

    // if the result contains the expected result we pass
    EXPECT_EQ(expected_result, result);
}

/**
 * @brief Ensure that the order of timer events within jack is correct
 */
/*
TEST(VSTEST, TEST_TIMERORDER)
{
    std::vector<int> result;
    std::vector<int> expected_result = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    // create the jack engine
    jack::Engine bdi;
    bdi.exitWhenDone();

    // create counting goals
    auto countSomeGoal = bdi.goal("CountSome")
        .commit();

    auto countSomeMoreGoal = bdi.goal("CountSomeMore")
        .commit();

    // create counting plans
    auto countSomePlan = bdi.plan("CountSomePlan")
        .handles (countSomeGoal)
        .body    (bdi.coroutine()
                    .action("DoCount").param("value", 1)
                    .sleep(10)
                    .action("DoCount").param("value", 2)
                    .action("DoCount").param("value", 3)
                    .sleep(150)
                    .action("DoCount").param("value", 6)
                    .action("DoCount").param("value", 7)
                 )
        .commit  ();

    auto countSomeMorePlan = bdi.plan("CountSomeMorePlan")
        .handles (countSomeMoreGoal)
        .body    (bdi.coroutine()
                    .sleep(100)
                    .action("DoCount").param("value", 4)
                    .action("DoCount").param("value", 5)
                    .sleep(200)
                    .action("DoCount").param("value", 8)
                 )
        .commit  ();

    auto countAction = bdi.action("DoCount")
        .prop<int>("value")
        .commit();

    // create our simple counting agent
    auto countingAgent = bdi.agent("CountingAgent")
        .plans ({ countSomePlan, countSomeMorePlan })
        .main  (bdi.coroutine()
                   .goal("CountSome").nowait()
                   .goal("CountSomeMore").nowait()
                   .sleep(400)
                   .action("DoCount").param("value", 9)
               )
        .handleAction("DoCount",
            [&](jack::Agent& agent, jack::Action& action) {
                int v = action.get<int>("value");
                result.push_back(v);
                return jack::Action::SUCCESS;
            })
        .create("bob");

    countingAgent->start();

    // run the jack engine
    bdi.execute();

    // print the result
    std::copy(result.begin(), result.end(), std::ostream_iterator<int>(std::cout, " "));

    // if the result contains the expected result we pass
    EXPECT_EQ( (expected_result.size() == result.size()) &&
                std::equal(std::begin(expected_result), std::end(expected_result), std::begin(result)),
                true);
}
*/
/**
 * @brief SoakPlans execute 1000 plans
 */
/*TEST(VSTEST, SOAK_PLAN)
{
    int count = -1;

    // create the jack engine
    jack::Engine bdi;
    bdi.exitWhenDone();

    auto bs = bdi.beliefset("ABC")
        .var<int>("count", 1000)
        .commit();

    // create counting goals
    auto soakPlansGoal = bdi.goal("SoakPlansGoal")
        .satisfied([&](const jack::BeliefContext &context) {
            int count = 0;
            context.get<int>("count", count);
            return count <= 0;
        })
        .commit();

    // create counting plans
    auto somePlan = bdi.plan("SomePlan")
        .handles (soakPlansGoal)
        .body    (bdi.coroutine()
                    .action("DoCount").param("count")
                 )
        .commit  ();

    auto countAction = bdi.action("DoCount")
        .prop<int>("count")
        .commit();

    // create our simple counting agent
    auto countingAgent = bdi.agent("CountingAgent")
        .beliefs(bs)
        .plans ({ somePlan })
        .main  (bdi.coroutine()
                   .goal("SoakPlansGoal")
               )
        .handleAction("DoCount",
            [&](jack::Agent& agent, jack::Action& action) {
                int v = action.get<int>("count");
                agent.beliefSet("ABC")->set<int>("count", v-1);
                count = v-1;
                return jack::Action::SUCCESS;
            })
        .create("bob");

    countingAgent->start();

    // run the jack engine
    bdi.execute();

    EXPECT_EQ(count, 0);
}*/

#if 0
void pendingActionTask(std::shared_ptr<jack::ActionHandle> handle, std::vector<int> &result, bool success)
{
    std::cout << "performing pending action..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    result.push_back(2);
    if (success) {
        std::cout << "pending action success" << std::endl;
        handle->success();
    } else {
        std::cout << "pending action fail" << std::endl;
        handle->fail();
    }
}
TEST(VSTEST, PENDINGACTION)
{
    std::vector<int> result;
    std::vector<int> expected_result = { 1, 2, 3, 2 };

    /*! ***************************************************************************************
     * Verify a pending action is completed correctly
     * ****************************************************************************************/
    // delay the goal
    jack::Engine bdi;
    bdi.exitWhenDone();

    bdi .action("PendingAction")
        .prop<bool>("success")
        .commit();

    bdi .action("PostAction")
        .commit();

    bdi.goal("MyGoal")
        .satisfied([&](const jack::BeliefContext &context) {
            // TODO: use belief
            return result.size() >= 4;
        })
        .commit();

    auto plan1 = bdi.plan("MyPlanPlan")
        .handles    ("MyGoal")
        .body       (bdi.coroutine ()
                        .action("PendingAction").param("success", true) // 2
                        .action("PostAction") // 3
                        .action("PendingAction").param("success", false) // 2
                        .action("PostAction") // this will not be called
                    )
        .commit();

    jack::Agent* agent = bdi.agent("TestAgent1")
            .plans          ({plan1})
            .main           (bdi.coroutine()
                                .goal("MyGoal"))
            //only execute the action if not delayed
            .handleAction   ("PendingAction", [&] (jack::Agent& agent, jack::Action &action) {
                                std::cout << "pending action invoked" << std::endl;
                                bool success = action.get<bool>("success");
                                std::thread t1(pendingActionTask, action.handle(), std::ref(result), success);
                                t1.detach();
                                return jack::Action::PENDING;
                            })
            //resume goal executing setting the delay flag to false
            .handleAction   ("PostAction", [&] (jack::Agent& agent, jack::Action &action) {
                                std::cout << "post action invoked" << std::endl;
                                result.push_back(3);
                                return jack::Action::SUCCESS;
                            })
            .create         ("agent1");

    result.push_back(1);
    agent->start();

    bdi.execute ();

    // print the result
    std::copy(result.begin(), result.end(), std::ostream_iterator<int>(std::cout, " "));

    // if the result contains the expected result we pass
    EXPECT_EQ( (expected_result.size() == result.size()) &&
                std::equal(std::begin(expected_result), std::end(expected_result), std::begin(result)),
                true);
}
#endif
