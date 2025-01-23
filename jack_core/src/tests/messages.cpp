#include <jack/jack.h>
#include <gtest/gtest.h>

#include <vector>
#include <iterator>
#include <string>

using namespace aos;

typedef std::vector<jack::PlanBuilder> PlanList;

/*! ***********************************************************************************************
 * \class   MessageTest
 *
 * This google test fixture aims to provide a context for Message specific testing
 * ************************************************************************************************/
class MessageTest : public ::testing::Test {
    protected:
    MessageTest()
    {
        bdi.exitWhenDone();
    }

    void SetUp() override
    {
        bdi.action("DoPing").commit();
        bdi.action("DoPong").commit();
        bdi.action("Quit").commit();

        // prepare the goal that the agent will be using
        bdi.goal("PingPongGoal").commit ();

        // the plan is expected to invoke a predefined agent action
        auto ping = bdi.plan("PingPlan")
                        .handles("PingPongGoal")
                        .body(bdi.coroutine()
                                  // TODO: pass in count param from the goal context
                                  .action("DoPing")
                              // action("DoPing").param("count", "$goal.count");
                              )
                        .commit();

        auto pong = bdi.plan("PongPlan")
                        .handles("PingPongGoal")
                        .body(bdi.coroutine().action("DoPong"))
                        .commit();

        pingPlans.emplace_back(ping);
        pongPlans.emplace_back(pong);
    }

    jack::Engine    bdi;  //!< engine bdi initialization
    PlanList pingPlans;
    PlanList pongPlans;
};

// PING PONG AGENTS
TEST_F(MessageTest, PingPong)
{
    auto pingMsg = bdi.message("Ping")
                      .field<int>("count")
                      .commit();

    auto pongMsg = bdi.message("Pong")
                      .field<int>("count")
                      .commit();

    // a goal to quit
    bdi.goal("Quit")
        .pre([&](const jack::BeliefContext& context) {
            auto       bs    = context.message("PingPongBeliefSet");
            const auto count = *bs->getPtr<int>("count");
            return count > 5;
        })
        .commit();

    auto kaPlan = bdi.plan("QuitPlan")
                     .handles("Quit")
                     .body(bdi.coroutine()
                              .action("Quit"))
                     .commit();

    pingPlans.emplace_back(kaPlan);
    pongPlans.emplace_back(kaPlan);

    jack::MessageBuilder bs = bdi.message("PingPongBeliefSet")
                                 .fieldWithValue<int>("count", 0)
                                 .field<std::string>("target")
                                 .commit();

    // NOTE: I think an agent does need a context - for configuration
    jack::AgentHandle bobHandle = bdi.agent("PingAgent")
                                     .belief(bs)
                                     .plans(pingPlans)
                                     .handleMessage("Pong", [&](jack::Agent &agent, const jack::Message &msg) {
                                         int count = *msg.getConstPtr<int>("count");
                                         agent.message("PingPongBeliefSet")->setFieldValue<int>("count", count);

                                         JACK_DEBUG("Bob got a pong message [count={}]", count);
                                         agent.pursue("PingPongGoal", jack::GoalPersistent_No); // pass count as parameter to this pursue message
                                     })
                                     .handleAction("DoPing", [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle handle) {
                                         std::shared_ptr<jack::Message> bSet = agent.message("PingPongBeliefSet");

                                         int count             = *bSet->getPtr<int>("count");
                                         auto targetUUIDString = *bSet->getPtr<std::string>("target");

                                         auto targetUUID = jack::UniqueId::initFromString(targetUUIDString);
                                         assert(targetUUID.valid());

                                         jack::Message reply = bdi.createMessage("Ping");
                                         reply.setFieldValue("count", count + 1);
                                         JACK_DEBUG("Bob sends a ping message to [target={}]", targetUUIDString);

                                         // TODO: should be possible to send this from the plan - protocols too
                                         jack::Agent *to = bdi.getAgentByUUID(targetUUID);
                                         to->sendMessageToHandler(std::move(reply));
                                         return jack::Event::SUCCESS;
                                     })
                                     .handleAction("Quit", [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle handle) {
                                         agent.stop();
                                         return jack::Event::SUCCESS;
                                     })
                                     .commitAsAgent()
                                     .createAgent("bob");

    jack::AgentHandle sueHandle = bdi.agent("PongAgent")
                                     .belief(bs)
                                     .plans(pongPlans)
                                     .handleMessage("Ping", [&] (jack::Agent &agent, const jack::Message& msg) {
                                         int count = *msg.getConstPtr<int>("count");
                                         agent.message("PingPongBeliefSet")->setFieldValue<int>("count", count);

                                         JACK_DEBUG("Sue got a ping message [count={}]", count);
                                         agent.pursue("PingPongGoal", jack::GoalPersistent_No); // pass count as parameter to this pursue message
                                     })
                                     .handleAction("DoPong", [&] (jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle handle) {
                                         int count = *agent.message("PingPongBeliefSet")->getPtr<int>("count");

                                         jack::Message reply = bdi.createMessage("Pong");
                                         reply.setFieldValue("count", count + 1);
                                         JACK_DEBUG_MSG("Sue sends a pong message to bob");

                                         std::string targetUUIDString = *agent.message("PingPongBeliefSet")->getPtr<std::string>("target");
                                         auto targetUUID = jack::UniqueId::initFromString(targetUUIDString);
                                         assert(targetUUID.valid());

                                         jack::Agent *to = bdi.getAgentByUUID(targetUUID);
                                         to->sendMessageToHandler(std::move(reply));
                                         return jack::Event::SUCCESS;
                                     })
                                     .handleAction("Quit", [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle handle) {
                                        agent.stop();
                                        return jack::Event::SUCCESS;
                                     })
                                     .commitAsAgent()
                                     .createAgent("sue");

    jack::Agent *bob = bdi.getAgent(bobHandle);
    jack::Agent *sue = bdi.getAgent(sueHandle);

    // the agent context ??
    bob->message("PingPongBeliefSet")->setFieldValue<std::string>("target", std::string(sueHandle.m_id.toString()));
    sue->message("PingPongBeliefSet")->setFieldValue<std::string>("target", std::string(bobHandle.m_id.toString()));

    bob->start();
    sue->start();

    bob->pursue("Quit", jack::GoalPersistent_Yes);
    sue->pursue("Quit", jack::GoalPersistent_Yes);

    // get bob to perform the first ping
    bob->pursue("PingPongGoal", jack::GoalPersistent_No);

    bdi.execute();

    EXPECT_EQ(true, true);
}
