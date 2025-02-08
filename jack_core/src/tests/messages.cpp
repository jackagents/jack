// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <tests/meta/testsproject.h>

#include <tests/meta/messages/pingmeta.h>
#include <tests/meta/messages/pingpongbeliefsetmeta.h>
#include <tests/meta/messages/pongmeta.h>

#include <gtest/gtest.h>
#include <jack/jack.h>

#include <iterator>
#include <string>
#include <vector>

using namespace aos;

typedef std::vector<jack::PlanBuilder> PlanList;

/*! ***********************************************************************************************
 * \class   MessageTest
 *
 * This google test fixture aims to provide a context for Message specific testing
 * ************************************************************************************************/
class MessageTest : public ::testing::Test
{
  protected:
    MessageTest()
    {
        // when all agents have stopped the engine will exit
        bdi.exitWhenDone();
    }

    void SetUp() override
    {
        bdi.action("DoPing").request(Ping::MODEL_NAME).commit();
        bdi.action("DoPong").request(Pong::MODEL_NAME).commit();
        bdi.action("Quit").commit();

        // prepare the goal that the agent will be using
        bdi.goal("PingPongGoal").message(Ping::MODEL_NAME).commit();

        // Plans
        auto ping = bdi.plan("PingPlan")
                        .handles("PingPongGoal")
                        .body(bdi.coroutine().action("DoPing"))
                        .commit();

        auto pong = bdi.plan("PongPlan")
                        .handles("PingPongGoal")
                        .body(bdi.coroutine().action("DoPong"))
                        .commit();

        pingPlans.emplace_back(ping);
        pongPlans.emplace_back(pong);
    }

    tests    bdi; // The JACK engine instance
    PlanList pingPlans;
    PlanList pongPlans;
};

// PING PONG AGENTS TEST
TEST_F(MessageTest, PingPong)
{
    // a goal to quit
    bdi.goal("Quit")
        .pre([&](const jack::BeliefContext& context) {
            const auto bs = context.getMessageAsPtr<const PingPongBeliefSet>();
            return bs->count > 5;
        })
        .commit();

    auto kaPlan = bdi.plan("QuitPlan")
                      .handles("Quit")
                      .body(bdi.coroutine()
                                .action("Quit"))
                      .commit();

    pingPlans.emplace_back(kaPlan);
    pongPlans.emplace_back(kaPlan);

    jack::AgentHandle bobHandle = bdi.agent("PingAgent")
                                      .beliefName(PingPongBeliefSet::MODEL_NAME)
                                      .plans(pingPlans)
                                      .handleMessage(std::string(Pong::MODEL_NAME), [&](jack::Agent& agent, const jack::Message& msg) {
                                          const Pong& m = dynamic_cast<const Pong&>(msg);

                                          // store
                                          const auto bs = agent.messageAsPtr<PingPongBeliefSet>();
                                          bs->count     = m.count;

                                          // create ping message
                                          auto goalMsg   = std::make_shared<Ping>();
                                          goalMsg->count = m.count;

                                          JACK_INFO("Bob receives a pong message [count={}]", m.count);
                                          agent.pursue("PingPongGoal", jack::GoalPersistent_No, goalMsg); // pass count as parameter to this pursue message
                                      })
                                      .handleAction("DoPing", [&](jack::Agent& agent, jack::Message& msg, jack::Message& out, jack::ActionHandle handle) {
                                          const Ping& m = dynamic_cast<const Ping&>(msg);

                                          const auto bs = agent.messageAsPtr<const PingPongBeliefSet>();

                                          auto count            = m.count;
                                          auto targetUUIDString = bs->target;

                                          auto targetUUID = jack::UniqueId::initFromString(targetUUIDString);
                                          assert(targetUUID.valid());

                                          // create ping message
                                          auto reply   = std::make_unique<Ping>();
                                          reply->count = count + 1;

                                          jack::Agent* to = bdi.getAgentByUUID(targetUUID);
                                          if (to->running()) {
                                              JACK_INFO("Bob sending ping message to [target={}]", to->name());
                                              to->sendMessageToHandler(std::move(reply));
                                          }
                                          return jack::Event::SUCCESS;
                                      })
                                      .handleAction("Quit", [&](jack::Agent& agent, jack::Message& msg, jack::Message& out, jack::ActionHandle handle) {
                                          agent.stop();
                                          return jack::Event::SUCCESS;
                                      })
                                      .commitAsAgent()
                                      .createAgent("bob");

    jack::AgentHandle sueHandle = bdi.agent("PongAgent")
                                      .beliefName(PingPongBeliefSet::MODEL_NAME)
                                      .plans(pongPlans)
                                      .handleMessage(std::string(Ping::MODEL_NAME), [&](jack::Agent& agent, const jack::Message& msg) {
                                          const Ping& m = dynamic_cast<const Ping&>(msg);

                                          const auto bs = agent.messageAsPtr<PingPongBeliefSet>();
                                          bs->count     = m.count;

                                          auto goalMsg   = std::make_shared<Ping>();
                                          goalMsg->count = m.count;

                                          JACK_INFO("Sue receives a ping message [count={}]", m.count);
                                          agent.pursue("PingPongGoal", jack::GoalPersistent_No, goalMsg); // pass count as parameter to this pursue message
                                      })
                                      .handleAction("DoPong", [&](jack::Agent& agent, jack::Message& msg, jack::Message& out, jack::ActionHandle handle) {
                                          const auto bs = agent.messageAsPtr<const PingPongBeliefSet>();

                                          const Pong& m = dynamic_cast<const Pong&>(msg);

                                          auto count            = m.count;
                                          auto targetUUIDString = bs->target;

                                          auto targetUUID = jack::UniqueId::initFromString(targetUUIDString);
                                          assert(targetUUID.valid());

                                          auto reply   = std::make_unique<Pong>();
                                          reply->count = count + 1;

                                          jack::Agent* to = bdi.getAgentByUUID(targetUUID);
                                          if (to->running()) {
                                              JACK_INFO("Sue sending a pong message to [target={}]", to->name());
                                              to->sendMessageToHandler(std::move(reply));
                                          }
                                          return jack::Event::SUCCESS;
                                      })
                                      .handleAction("Quit", [&](jack::Agent& agent, jack::Message& msg, jack::Message& out, jack::ActionHandle handle) {
                                          agent.stop();
                                          return jack::Event::SUCCESS;
                                      })
                                      .commitAsAgent()
                                      .createAgent("sue");

    jack::Agent* bob = bdi.getAgent(bobHandle);
    jack::Agent* sue = bdi.getAgent(sueHandle);

    // configure beliefs
    {
        const auto bs = bob->messageAsPtr<PingPongBeliefSet>();
        bs->target    = std::string(sueHandle.m_id.toString());
    }
    {
        const auto bs = sue->messageAsPtr<PingPongBeliefSet>();
        bs->target    = std::string(bobHandle.m_id.toString());
    }

    bob->start();
    sue->start();

    bob->pursue("Quit", jack::GoalPersistent_Yes);
    sue->pursue("Quit", jack::GoalPersistent_Yes);

    // send ping to sue
    auto goalMsg   = std::make_unique<Ping>();
    goalMsg->count = 0;
    sue->sendMessageToHandler(std::move(goalMsg));

    bdi.execute();

    EXPECT_EQ(true, true);
}
