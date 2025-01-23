#include "bitmaskops.h"


#include <jack/jack.h>
#include <gtest/gtest.h>

#include <csignal>
#include <chrono>
#include <thread>
#include <cstdint>
#include <cstdio>
#include <string>
#include <cstdlib>

TEST(Tactics, counter)
{
    srand(0xABCD1235);
    aos::jack::Engine engine;

    aos::jack::MessageBuilder countBeliefs =
        engine.message("countBeliefs")
            .fieldWithValue<uint16_t>("count", 0)
            .fieldWithValue<uint16_t>("countSlow", 0)
            .commit();

    aos::jack::ActionBuilder cntFastAction = engine.action("countFastAction").commit();
    aos::jack::ActionBuilder cntSlowAction = engine.action("countSlowAction").commit();

    aos::jack::GoalBuilder countGoal =
        engine.goal("countGoal")
            .pre([&](const aos::jack::BeliefContext &context) {
                return true;
            })
            .satisfied([&](const aos::jack::BeliefContext &context) {
                return false;
            })
            .commit();

    aos::jack::PlanBuilder countSlowPlan =
        engine.plan("countSlowPlan")
            .handles("countGoal")
            .body(engine.coroutine().action("countSlowAction"))
            .commit();

    aos::jack::PlanBuilder countFastPlan =
        engine.plan("countFastPlan")
            .handles("countGoal")
            .body(engine.coroutine().action("countFastAction"))
            .commit();

    aos::jack::TacticHandle fastTactic =
        engine.tactic("fastTactic")
            .goal("countGoal")
            .planName("countFastPlan")
            .commit();

    aos::jack::TacticHandle slowTactic =
        engine.tactic("slowTactic")
            .goal("countGoal")
            .planName("countSlowPlan")
            .commit();

    aos::jack::AgentHandle counterAgentHandle =
        engine.agent("counterAgent")
            .belief(countBeliefs)
            .desire(countGoal)
            .plans(std::array{countFastPlan, countSlowPlan})
            .handleAction("countFastAction",
                          [&](aos::jack::Agent &agent, aos::jack::Message &, aos::jack::Message &out, aos::jack::ActionHandle) {
                              std::shared_ptr<aos::jack::Message> beliefs = agent.message("countBeliefs");

                              uint16_t count = *beliefs->getPtr<uint16_t>("count");
                              count += 1;
                              beliefs->setFieldValue("count", count);
                              return aos::jack::Event::SUCCESS;
                          })
            .handleAction("countSlowAction",
                          [&](aos::jack::Agent &agent, aos::jack::Message &, aos::jack::Message &out, aos::jack::ActionHandle) {
                              std::shared_ptr<aos::jack::Message> beliefs = agent.message("countBeliefs");
                              uint16_t count     = *beliefs->getPtr<uint16_t>("count");
                              uint16_t countSlow = *beliefs->getPtr<uint16_t>("countSlow");

                              if (countSlow >= 10) {
                                  count += 1;
                                  countSlow = 0;
                              }
                              countSlow += 1;

                              beliefs->setFieldValue("count", count);
                              beliefs->setFieldValue("countSlow", countSlow);
                              return aos::jack::Event::SUCCESS;
                          })
            .tactic(fastTactic)
            .commitAsAgent()
            .createAgent("Counter");

    aos::jack::Agent *counterAgent = engine.getAgent(counterAgentHandle);
    counterAgent->pursue("countGoal", aos::jack::GoalPersistent_Yes);
    counterAgent->start();

    std::shared_ptr<aos::jack::Message> beliefs = counterAgent->context().message("countBeliefs");

    uint16_t cnt = 0;
    uint16_t cntSlow = 0;
    uint16_t time = 0;

    bool slowSet = false;

    while(1)
    {
        auto deltaTimeToMs = static_cast<int>(time * 1000.f);
        engine.poll(std::chrono::milliseconds(deltaTimeToMs));
        time += 1;

        cnt     = *beliefs->getPtr<decltype(cnt)>("count");
        cntSlow = *beliefs->getPtr<decltype(cntSlow)>("countSlow");

        if (cnt < 50) {
            ASSERT_EQ(cntSlow, 0) << "Count slow activated when it shouldn't be";
        }

        if (cnt == 50 && slowSet == false) {
            counterAgent->selectTactic("slowTactic");
            slowSet = true;
        }

        if (cnt > 51) {
            ASSERT_GT(cntSlow, 0) << "Count slow not activated";
        }

        if (cnt >= 100) {
            break;
        }
    }
}

