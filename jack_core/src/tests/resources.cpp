// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <jack/jack.h>
#include <gtest/gtest.h>

// a flag
std::atomic_bool busy;

using namespace aos;

/**
 * @brief Simulate the performing of a task - draining the battery resource
 */

void doTaskAsync(jack::Agent *agent, const jack::ActionHandle &action)
{
    if (busy) {
        std::cout << "FAIL: recharge and tasking at the same time" << std::endl;
    }

    busy = true;

    std::cout << "performing task: " << std::endl;

    // consume battery resource while we are performing this action

    for(int i = 0; i < 10; i++) {
        // send percept to change the resource
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        auto batteryResource = agent->context().resource("Battery");
        batteryResource->consume(1);
        std::cout << " x" << std::flush;
    }

    std::cout << " complete" << std::endl;

    if (!busy) {
        std::cout << "FAIL: recharge and tasking at the same time" << std::endl;
    }

    busy = false;
    agent->finishActionHandle(action, true /*success*/);
}

/**
 * @brief Simulate the charging of a battery resource
 */

void doRechargeAsync(jack::Agent *agent, const jack::ActionHandle &action)
{
    if (busy) {
        std::cout << "FAIL: recharge and tasking at the same time" << std::endl;
    }
    busy = true;

    std::cout << "recharging the Battery: " << std::endl;

    // produce battery resource while recharging
    auto batteryResource = agent->context().resource("Battery");

    // using the resource handle from the external application can cause the engine to be forced to
    // re-evaluate it's plan.

    // produce may cause a previous failed plan to evaluate if it is subscribed to this resource
    // batteryResource->produce(1);

    // a consume could cause a full replan is the total resources reserved for the current plan would
    // exhause the new total.
    // batteryResource->consume(1);

    // a set could cause a full replan is the new total is below the total resources reserved for
    // the current plan

    // In a real system this wouldn't be pulled from a sensor reading
    int batteryLevel = batteryResource->count();
    while(batteryLevel < 100) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        batteryResource->set(batteryLevel + 1);
        batteryLevel = batteryResource->count();
        if (batteryLevel % 10 == 0)
            std::cout << batteryLevel << " " << std::flush;
    }

    std::cout << std::endl;

    if (!busy) {
        std::cout << "FAIL: recharge and tasking at the same time" << std::endl;
    }
    busy = false;

    agent->finishActionHandle(action, true /*success*/);
    std::cout << "recharge complete" << std::endl;
}

TEST(RESOURCES, DISABLED_ResourceTest1)
{
    /*! ***************************************************************************************
     * Demonstrate the consumption of a battery resource
     * ****************************************************************************************/

    // create the jack engine
    jack::Engine bdi("AgentNode");
    bdi.exitWhenDone();

    busy = false;

    int globalTasksCompleted = 0;
    const int tasksToDo = 10;

    /// a battery resource
    jack::ResourceBuilder batteryResource = bdi.resource("Battery")
                                               .max(100)
                                               .min(0)
                                               .commit();

    /// a resource for blocking goals from being active at the same time
    jack::ResourceBuilder exclusiveResource = bdi.resource("Exclusive")
                                                 .max(1)
                                                 .min(0)
                                                 .commit();

    auto bs = bdi.message("Vehicle")
                 .fieldWithValue<int>("tasksCompleted", 0)
                 .commit();

    // action messages
    bdi.action("StartTask").commit();
    bdi.action("EndTask").commit();
    bdi.action("Recharge").commit();

    // the maintenance goal that will suspend task goals and recharge the battery
    // if the battery goes below 25%
    auto rechargeGoal =
        bdi.goal("RechargeBatteryGoal")
            .pre([](const jack::BeliefContext &context) {
                // reactive condition
                auto batteryResource = context.resource("Battery");
                return (batteryResource->count() < 25);
            })
            // add the heuristic
            .heuristic([](const jack::BeliefContext &context) {
                // estimate that this goal will require 1 'cost'
                return 1.0f;
            })
            .commit();

    // plan to recharge battery
    auto rechargePlan =
        bdi.plan("RechargeBatteryPlan")
            .handles(rechargeGoal)
            .lock(exclusiveResource) // lock the exclusive flag during the life time of this plan
            .body(bdi.coroutine().action("Recharge"))
            .effects([](jack::BeliefContext &context) {
                // model the battery being consumed
                auto batteryResource = context.resource("Battery");

                /// the battery will be fully recharged after this plan
                if (batteryResource) {
                  batteryResource->set(100);
                }
            })

            .commit();

    // An achievement goal to perform all the tasks
    auto startWorkGoal = bdi.goal("StartWorkGoal")
                             .pre([](const jack::BeliefContext& context) {
                                 // this goal requires that we haven't completed 10 tasks
                                 std::shared_ptr<jack::Message> bs = context.message("Vehicle");
                                 auto tasksCompleted = *bs->getPtr<int>("tasksCompleted");
                                 return (tasksCompleted < 10);
                             })
                             .satisfied([&](const jack::BeliefContext& context) {
                                 std::shared_ptr<jack::Message> bs = context.message("Vehicle");
                                 auto tasksCompleted = *bs->getPtr<int>("tasksCompleted");
                                 return (tasksCompleted >= 10);
                             })
                             .heuristic([](const jack::BeliefContext& context) {
                                 std::shared_ptr<jack::Message> bs = context.message("Vehicle");
                                 auto tasksCompleted = *bs->getPtr<int>("tasksCompleted");
                                 // estimate that this goal will require 1 cost per task left
                                 return 10.f - tasksCompleted;
                             })
                             .commit();

    auto doTaskPlan =
        bdi.plan("DoTaskPlan")
            .handles(startWorkGoal)
            .lock(exclusiveResource) // lock the exclusive flag during the life time of this plan
            .pre([](const jack::BeliefContext &context) {
                auto batteryResource = context.resource("Battery");
                bool enoughBattery = (batteryResource->count() >= 10);

                // model the task being completed
                auto bs = context.message("Vehicle");

                if (bs) {
                  auto tasksCompleted = *bs->getPtr<int>("tasksCompleted");
                  return enoughBattery && (tasksCompleted <= 9);
                } else {
                  return false;
                }
            })
            .effects([](jack::BeliefContext &context) {
                // model the battery being consumed
                auto rs = context.resource("Battery");

                if (rs) {
                  rs->consume(10);
                }

                // model the task being completed
                auto bs = context.message("Vehicle");

                if (bs) {
                  auto tasksCompleted = *bs->getPtr<int>("tasksCompleted");
                  bs->setFieldValue("tasksCompleted", tasksCompleted + 1);
                }
            })
            .body(bdi.coroutine().action("StartTask").action("EndTask"))
            .commit();

    jack::AgentHandle agentHandle = bdi.agent("TestAgent1")
                                       .belief(bs)
                                       .resources(std::array{batteryResource, exclusiveResource})
                                       .plans(std::array{doTaskPlan, rechargePlan})
                                       .handleAction("EndTask", [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle handle) {
                                           jack::BeliefContext &context = agent.context();

                                           auto tasksCompleted = *context.message("Vehicle")->getPtr<int>("tasksCompleted");
                                           context.message("Vehicle")->setFieldValue("tasksCompleted", tasksCompleted+1);
                                           std::cout << "task(" << tasksCompleted+1 << ") complete" << std::endl;

                                           globalTasksCompleted++;
                                           std::cout << "task(" << globalTasksCompleted << ") complete" << std::endl;
                                           if (globalTasksCompleted >= 10) {
                                               agent.stop();
                                           }

                                           return jack::Event::SUCCESS;
                                       })
                                       .handleAction("StartTask", [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle handle) {
                                           // perform this action asynchronously
                                           std::thread t1(doTaskAsync, &agent, handle);
                                           t1.detach();
                                           return jack::Event::PENDING;
                                       })
                                       .handleAction("Recharge", [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle handle) {
                                           // perform this action asynchronously
                                           std::thread t2(doRechargeAsync, &agent, handle);
                                           t2.detach();
                                           return jack::Event::PENDING;
                                       })
                                       .commitAsAgent()
                                       .createAgent("agent1");

    // Set the initial resource value
    jack::Agent *agent     = bdi.getAgent(agentHandle);
    auto         battery   = agent->context().resource(batteryResource.name());
    auto         exclusive = agent->context().resource(exclusiveResource.name());
    ASSERT_TRUE(battery != nullptr);
    ASSERT_TRUE(exclusive !=  nullptr);
    battery->set(50);
    exclusive->set(exclusive->max());

    agent->start();

    agent->pursue(rechargeGoal, jack::GoalPersistent_Yes);
    agent->pursue(startWorkGoal, jack::GoalPersistent_Yes);

    bdi.execute();

    EXPECT_EQ(globalTasksCompleted, tasksToDo);
}

