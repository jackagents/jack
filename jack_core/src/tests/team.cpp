#include "testhelpers.h"

#include <jack/jack.h>
#include <gtest/gtest.h>

#include <math.h>
#include <array>
#include <string>

using namespace aos;
/**
 * @brief A simple data structure to represent a 2D position
 * */
struct Position
{
    Position() = default;
    Position(int x, int y) : x(x), y(y) {}
    int x = 0;
    int y = 0;
};

struct DeliveryState
{
    bool delivered;
    bool delegated;
};

/// Calculate the squared distance between two Position objects
int sqrtDistanceBetween(const Position &a, const Position &b)
{
    int deltax = a.x - b.x;
    int deltay = a.y - b.y;
    return deltax * deltax + deltay * deltay;
}

/// The environment simulator for the unit tests.
///
/// The simulation contains a number of items that need to be delivered to a
/// hopper
class Environment
{
public:
    /**************************************************************************
     * Public Functions
     **************************************************************************/
    Environment(int numItems, int numHoppers);

    Position       nearestHopper(Position pos);
    Position       itemPosition (int index) { return m_items.at(index); }
    jack::UniqueId itemId       (int index) { return m_itemIds.at(index); }
    void           deliverItem  (int index);

    /**************************************************************************
     * Public Members
     **************************************************************************/
    std::vector<Position>       m_items;
    std::vector<jack::UniqueId> m_itemIds;
    int                         m_itemsDelivered = 0;
    std::vector<DeliveryState>  m_itemState;
    std::vector<Position>       m_hoppers;
    jack::Agent *               m_deliveryTeam = nullptr;
};

/******************************************************************************
 * \class   TeamDeliveryTest
 * \date    24.09.2020
 *
 * This test fixture provides a testing environment for teams of agents
 ******************************************************************************/
class TeamDeliveryTest : public ::testing::Test {
protected:
    /**************************************************************************
     * GTest Setup
     **************************************************************************/
    TeamDeliveryTest();
    void SetUp() override;

    /**************************************************************************
     * Helper Functions
     **************************************************************************/
    void processMoveToJobs();
    void executeTest(bool withBuilders);

    /**************************************************************************
     * Public Members
     **************************************************************************/
    struct MoveToJob
    {
        jack::Agent        *agent;
        jack::ActionHandle  actionHandle;
        Position            start;
        Position            end;
        int                 steps;
    };

    const std::string_view VEHICLE_MSG          = "Vehicle";
    const std::string_view AGENT_TEMPLATE       = "DeliveryAgent";
    const std::string_view TEAM_TEMPLATE        = "WorkTeam";
    const std::string_view TEAM_GOAL            = "TeamGoal";
    const std::string_view CURRENT_POSITION_KEY = "currentPosition";
    const int ITEMS_TO_DELIVER  = 10;

    jack::Engine           bdi;  //!< engine bdi initialization
    Environment            env;
    std::vector<MoveToJob> moveToJobs;
};

Environment::Environment(int numItems, int numHoppers)
{
    // initialise items
    m_itemState.resize(numItems);
    m_items.reserve(numItems);
    m_hoppers.reserve(numItems);
    for (int i = 0; i < numItems; ++i) {
        m_items.push_back(Position(rand() % 100, rand() % 100));
        auto id = jack::UniqueId::random();
        id.setTag("Item" + std::to_string(i));
        m_itemIds.push_back(id);
    }

    // initialise hoppers
    for (int i = 0; i < numHoppers; ++i) {
        m_hoppers.push_back(Position(rand() % 100, rand() % 100));
    }
}

// return the nearest hopper
Position Environment::nearestHopper(Position pos)
{
    Position bestHopper;
    int      bestDist = 100001;

    // brute force search of the nearest item
    for (auto hopperPos : m_hoppers) {
        int sqDist = sqrtDistanceBetween(hopperPos, pos);

        if (sqDist < bestDist) {
            bestDist   = sqDist;
            bestHopper = hopperPos;
        }
    }

    return bestHopper;
}

/// deliver the item
void Environment::deliverItem(int index)
{
    // make sure the item hasn't already been delivered
    if (m_itemState[index].delivered == false) {
        m_itemState[index].delivered = true;
        m_itemsDelivered++;
    }

    assert(m_deliveryTeam);
    std::shared_ptr<jack::Message> msg = m_deliveryTeam->context().message("Team");
    msg->setFieldValue<int>("itemsDelivered", m_itemsDelivered);
    m_deliveryTeam->sendMessage(msg, false /*broadcastToBus*/); // send a percept to the team agent

    // when all items are delivered stop the team
    if (m_itemsDelivered >= m_items.size()) {
        JACK_INFO("All items ({}) delivered! [agent={}]", m_itemsDelivered, m_deliveryTeam->handle());
        m_deliveryTeam->stop();
    }
}

TeamDeliveryTest::TeamDeliveryTest()
    : bdi("TeamNode")
    , env(ITEMS_TO_DELIVER, 5 /*numHoppers*/)
{
    bdi.exitWhenDone();
}

void TeamDeliveryTest::SetUp()
{
    // register the custom types
    jack::FieldRegistry& registry = jack::FieldRegistry::getInstance();
    registry.registerType<Position>("Position");

    std::string_view itemPositionMsgName = "Move To Item Message Debug"sv;
    auto itemPositionMsg = bdi.message(itemPositionMsgName)
                              .field<int>("itemIndex") /// For debugging
                              .field<Position>("itemPosition")
                              .commit();

    auto itemIndexMsg = bdi.message("Move To Item Message")
                           .field<int>("itemIndex")
                           .commit();

    // Setup the agent actions
    // These are used to change the environment
    bdi.action("MoveToItem")
       .request(itemPositionMsg.name())
       .commit();

    bdi.action("MoveToNearestHopper")
       .request(itemIndexMsg.name())
       .commit();

    bdi.action("DeliverItem")
       .request(itemIndexMsg.name())
       .commit();

    bdi.action("Stop")
       .commit();

    bdi.action("PursueAllDeliveryGoals")
       .commit();

    // belief sets
    auto teamBS = bdi.message("Team")
                     .fieldWithValue<int>("itemsDelivered", 0)
                     .commit();

    auto vehicleBS = bdi.message(VEHICLE_MSG)
                        .field<Position>(CURRENT_POSITION_KEY)
                        .commit();

    // goals
    auto teamGoal = bdi.goal("TeamGoal")
                        .pre([&](const jack::BeliefContext& context) {
                            auto teamBS         = context.message("Team");
                            int  itemsDelivered = *teamBS->getPtr<int>("itemsDelivered");
                            return itemsDelivered < ITEMS_TO_DELIVER;
                        })
                        .satisfied([&](const jack::BeliefContext& context) {
                            auto teamBS         = context.message("Team");
                            int  itemsDelivered = *teamBS->getPtr<int>("itemsDelivered");
                            return itemsDelivered >= ITEMS_TO_DELIVER;
                        })
                        .heuristic([&](const jack::BeliefContext& context) { return 0.0f; })
                        .commit();

    /// \note Setup the message schema for FetchGoal
    /// An achievement goal to fetch an item and deliver it to the nearest
    /// hopper
    auto fetchGoal = bdi.goal("FetchGoal")
                        .message(itemPositionMsgName)
                        .pre([&](const jack::BeliefContext &context) {
                            int itemIndex = 0;
                            bool found = context.get<int>("itemIndex", itemIndex);
                            assert(found);
                            return !env.m_itemState[itemIndex].delivered;
                        })
                        .satisfied([&](const jack::BeliefContext &context) {
                            int itemIndex = 0;
                            bool found = context.get<int>("itemIndex", itemIndex);
                            assert(found);
                            return env.m_itemState[itemIndex].delivered;
                        })
                        .heuristic([&](const jack::BeliefContext &context) {
                            // estimate the distance that is remaining to deliver the item

                            // the current position of the vechicle
                            std::shared_ptr<jack::Message> vehicleBS = context.message(VEHICLE_MSG);
                            assert(vehicleBS);

                            // the current position of the vechicle
                            auto currentPos = *vehicleBS->getPtr<Position>(CURRENT_POSITION_KEY);
                            auto itemIndex  = 0;
                            bool found      = context.get<int>("itemIndex", itemIndex);
                            assert(found);

                            Position itemPos   = env.itemPosition(itemIndex);
                            Position hopperPos = env.nearestHopper(itemPos);

                            int sqrtDist = sqrtDistanceBetween(currentPos, itemPos) +
                                           sqrtDistanceBetween(itemPos, hopperPos);

                            return (float)sqrt((double)sqrtDist);
                        })
                        .commit();

    // An achievement goal to fetch an item and deliver it to the nearest hopper
    auto stopGoal = bdi.goal("StopGoal")
                       .pre([&](const jack::BeliefContext &context) {
                           // when all items are delivered stop the agent
                           return env.m_itemsDelivered >= ITEMS_TO_DELIVER;
                       })
                       .commit();

    // Setup the plans
    auto teamPlan = bdi.plan("TeamPlan")
                       .handles(teamGoal)
                       .body(bdi.coroutine()
                                .action("PursueAllDeliveryGoals"))
                       .commit();

    auto fetchPlan =
        bdi.plan("FetchPlan")
            .handles(fetchGoal)
            .body(bdi.coroutine()
                      .action("MoveToItem")
                      .action("MoveToNearestHopper")
                      .action("DeliverItem"))
            .commit();

    auto stopPlan = bdi.plan("StopPlan")
                       .handles(stopGoal)
                       .body(bdi.coroutine()
                                .action("Stop"))
                       .commit();

    // define the team
    bdi.agent(TEAM_TEMPLATE)
        .desire(teamGoal)
        .plan(teamPlan)
        .belief(teamBS)
        .handleAction(
            "PursueAllDeliveryGoals",
            [&, itemPositionMsgName](jack::Agent& agent, jack::Message& msg, jack::Message& out, jack::ActionHandle) {
                jack::Engine& engine = agent.engine();
                for (int itemIndex = 0; itemIndex < env.m_itemState.size(); ++itemIndex) {
                    if (env.m_itemState[itemIndex].delegated) {
                        continue;
                    }

                    env.m_itemState[itemIndex].delegated = true;
                    jack::UniqueId goalId = env.itemId(itemIndex);
                    Position pos          = env.itemPosition(itemIndex);

                    auto msg             = engine.createMessage(itemPositionMsgName);
                    bool itemIndexSet    = msg.setFieldValue<int>("itemIndex", itemIndex);
                    bool itemPositionSet = msg.setFieldValue<Position>("itemPosition", pos);
                    assert(itemIndexSet);
                    assert(itemPositionSet);

                    JACK_INFO("Delegating delivery of item {} at {},{} [agent={}]", itemIndex, pos.x, pos.y, agent.handle().toString());
                    agent.pursue("FetchGoal", jack::GoalPersistent_No, msg, goalId);
                }
                return jack::Event::SUCCESS;
            })
        .commitAsTeam();

    auto deliveryAgentRole = bdi.role("DeliveryAgentRole")
                                .goals(std::array{teamGoal, fetchGoal, stopGoal})
                                .commit();

    // define the delivery agent
    bdi.agent(AGENT_TEMPLATE)
       .desire(stopGoal)
       .belief(vehicleBS)
       .role(deliveryAgentRole)
       .plans(std::array{fetchPlan, stopPlan})
       .handleAction("Stop",
                     [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle) {
                         agent.stop();
                         return jack::Event::SUCCESS;
                     })
       .handleAction(
           "MoveToItem",
           [&](jack::Agent &agent, jack::Message &request, jack::Message &, jack::ActionHandle handle) {
               jack::BeliefContext &context = agent.context();

               auto currentPos = *context.message(VEHICLE_MSG)->getPtr<Position>(CURRENT_POSITION_KEY);
               auto end        = *request.getPtr<Position>("itemPosition");
               auto itemIndex  = *request.getPtr<int>("itemIndex");

               /// \note Perform this action asynchronously
               MoveToJob job    = {};
               job.agent        = &agent;
               job.start        = currentPos;
               job.end          = end;
               job.actionHandle = handle;
               moveToJobs.push_back(job);
               return jack::Event::PENDING;
           })
       .handleAction(
           "MoveToNearestHopper",
           [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle handle) {
               jack::BeliefContext &context = agent.context();
               auto currentPos = *context.message(VEHICLE_MSG)->getPtr<Position>(CURRENT_POSITION_KEY);
               auto itemIndex  = *msg.getPtr<int>("itemIndex");

               /// \note Perform this action asynchronously
               MoveToJob job    = {};
               job.agent        = &agent;
               job.start        = currentPos;
               job.end          = env.nearestHopper(currentPos);
               job.actionHandle = handle;
               moveToJobs.push_back(job);
               return jack::Event::PENDING;
           })
       .handleAction(
           "DeliverItem",
           [&](jack::Agent& agent, jack::Message& request, jack::Message&, jack::ActionHandle) {
               auto itemIndex = request.get<int>("itemIndex");
               env.deliverItem(itemIndex);
               JACK_INFO("Delivered item {} [agent={}]", itemIndex, agent.handle());
               return jack::Event::SUCCESS;
           })
       .commitAsAgent();
}

void TeamDeliveryTest::processMoveToJobs()
{
    for (auto it = moveToJobs.begin(); it != moveToJobs.end();) {
        int distSquared    = sqrtDistanceBetween(it->start, it->end);
        jack::Agent *agent = it->agent;

        if (it->steps == 0)
            JACK_INFO("Moving from {},{} to {},{} [agent={}]", it->start.x, it->start.y, it->end.x, it->end.y, it->agent->handle());

        /// \note All move jobs finish in 5 steps
        if (it->steps++ >= 5) {
            std::shared_ptr<jack::Message> vehicleMsg = agent->context().message(VEHICLE_MSG);
            vehicleMsg->setFieldValue(CURRENT_POSITION_KEY, it->end);
            agent->finishActionHandle(it->actionHandle, true /*success*/);
            it = moveToJobs.erase(it);
        } else {
            it++;
        }
    }
}

void TeamDeliveryTest::executeTest(bool withBuilders)
{
    /// \note Create the team
    std::string_view teamName = "Team";
    jack::Team *team          = nullptr;
    if (withBuilders) {
        auto teamBuilder  = bdi.agent(TEAM_TEMPLATE);
        team              = teamBuilder.createTeamInstance(teamName);
    } else {
        jack::AgentHandle teamHandle = bdi.createAgent(TEAM_TEMPLATE, teamName);
        team                         = dynamic_cast<jack::Team *>(bdi.getAgent(teamHandle));
    }
    team->start();
    env.m_deliveryTeam = team;

    /// \note Create the team of delivery agents
    for (int i = 0; i < 5 /*agents*/; ++i) {
        /// \note Create the agent
        std::string agentName = "Agent " + std::to_string(i);
        jack::Agent *agent    = nullptr;
        if (withBuilders) {
            auto agentBuilder             = bdi.agent(AGENT_TEMPLATE);
            jack::AgentHandle agentHandle = agentBuilder.createAgent(agentName);
            agent                         = bdi.getAgent(agentHandle);
        } else {
            jack::AgentHandle agentHandle = bdi.createAgent(AGENT_TEMPLATE, agentName);
            agent                         = bdi.getAgent(agentHandle);
        }

        /// \note Set the agent position randomly
        jack::Message beliefSet = *agent->context().message(VEHICLE_MSG);
        beliefSet.setFieldValue(CURRENT_POSITION_KEY, Position(rand() % 100, rand() % 100));
        agent->sendMessage(std::move(beliefSet), false /*broadcastToBus*/);

        /// \note Add the agent to the team
        team->addMemberAgent(agent);
        agent->start();
    }

    /// \note Run the test
    for (;;) {
        jack::Engine::PollResult result = enginePollAndRecordSchedules(bdi, 1);
        if (result.agentsRunning <= 0 && result.agentsExecuting <= 0)
            break;
        processMoveToJobs();
    }
    EXPECT_EQ(env.m_itemsDelivered, ITEMS_TO_DELIVER);
}

TEST_F(TeamDeliveryTest, WithTemplates)
{
    /**************************************************************************
     * Delivery team example with agents created from templates
     **************************************************************************/
    executeTest(false /*withBuilders*/);
}

TEST_F(TeamDeliveryTest, WithBuilders)
{
    /**************************************************************************
     * Delivery team example with agents created from builders
     **************************************************************************/
    executeTest(true /*withBuilders*/);
}

TEST(TEAMS, TeamDeliveryTest2)
{
    // create the jack engine
    jack::Engine bdi("TeamNode");
    bdi.exitWhenDone();

    bool doneAction = false;

    // the action to be performed by the delegated agent
    bdi .action("PerformAction")
        .commit();

     // An the team goal to deliver all items
    auto teamGoal = bdi.goal("TeamGoal")
                        .pre([&](const jack::BeliefContext &context) {
                                return !doneAction;
                        })
                        .satisfied([&](const jack::BeliefContext &context) {
                                return doneAction;
                        })
                        .commit();

    // An achievement goal to fetch an item and deliver it to the nearest hopper
    auto agentGoal = bdi.goal("AgentGoal")
                        .satisfied([&](const jack::BeliefContext &context) { return doneAction; })
                        .commit();

    auto agentRole = bdi.role("AgentRole").goal(agentGoal).commit();

    auto teamPlan = bdi.plan("TeamPlan")
                       .handles(teamGoal)
                       .body(bdi.coroutine()
                                .goal("AgentGoal")  // this will be performed by a team member agent
                             )
                       .commit();

    auto agentPlan = bdi.plan("AgentPlan")
                        .handles(agentGoal)
                        .body(bdi.coroutine().action("PerformAction"))
                        .commit();

    // return
    auto teamHandle = bdi.agent("TeamAgent")
                          .desire(teamGoal)
                          .plan(teamPlan)
                          .role(agentRole)
                          .commitAsTeam()
                          .createTeam("team1");

    jack::AgentHandle agentHandle =
        bdi.agent("MemberAgent")
            .plan(agentPlan)
            .role(agentRole)
            .handleAction("PerformAction",
                          [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle) {
                              std::cout << "MoveToItem Action" << std::endl;
                              doneAction = true;
                              return jack::Event::SUCCESS;
                          })

            .commitAsTeam()
            .createAgent("agent1");

    // manually add the agent to the team
    jack::Team *team  = static_cast<jack::Team *>(bdi.getAgent(teamHandle));
    jack::Agent *agent = bdi.getAgent(agentHandle);
    team->addMemberAgent(agent);

    team->start();
    agent->start();

    enginePollAndRecordSchedules(bdi, 100 /*pollCount*/);
    EXPECT_EQ(doneAction, true);
}

// A meta team
template<typename ImplType>
class MyTeamMeta : public jack::Team
{
public:
    MyTeamMeta(jack::Engine& engine, std::string_view name)
        : jack::Team(engine, name)
    {
    }

    MyTeamMeta(const MyTeamMeta<ImplType>* other, std::string_view newName)
        : jack::Team(other, newName)
    {
    }

    static jack::AgentBuilder init(jack::Engine &bdi)
    {

        // Plans
        std::vector<jack::PlanBuilder> planList = {
            bdi.plan("TeamPlan")
        };

        // Roles
        std::vector<jack::RoleBuilder> roleList = {
            bdi.role("AgentRole")
        };

        // Goals
        std::vector<jack::GoalBuilder> desireList = {
            bdi.goal("TeamGoal")
        };

        return bdi.agent("TeamAgent")
            .desires(desireList)
            .plans(planList)
            .roles(roleList)
            .commitAsTeam<ImplType>();
    }

protected:
    jack::Agent* clone(std::string_view name) const override
    {
        return new ImplType(static_cast<const ImplType*>(this), name);
    }
};

// the my team implementation
class MyTeam : public MyTeamMeta<MyTeam>
{
public:
    MyTeam(jack::Engine& engine, std::string_view name)
        :  MyTeamMeta<MyTeam>(engine, name)
    {
    }

    MyTeam(const MyTeam* other, std::string_view newName)
        :  MyTeamMeta<MyTeam>(other, newName)
    {
    }

protected:
    jack::Agent* clone(std::string_view name) const override
    {
        return new MyTeam(static_cast<const MyTeam*>(this), name);
    }
};

TEST(TEAMS, TeamDeliveryTestAgentRoleFiltering)
{
    /*! ***************************************************************************************
     * Given 2 agents in a team and a goal, setup the goal such that only agents
     * with the "AgentRole" can complete the goal. This tests that goals are
     * only delegated to agents with the correct role within teams.
     * ****************************************************************************************/

    jack::Engine bdi("TeamNode");
    bdi.exitWhenDone();

    bool smartAgentPerformedAction     = false;
    bool dumbAgentPerformedAction      = false;
    const std::string SMART_AGENT_NAME = "SmartAgent";
    const std::string DUMB_AGENT_NAME  = "DumbAgent";
    const std::string PERFORM_ACTION   = "PerformAction";

    // The action to be performed by the delegated agent
    bdi.action(PERFORM_ACTION).commit();

    auto agentGoal = bdi.goal("AgentGoal")
                        .satisfied([&](const jack::BeliefContext &context) {
                            return smartAgentPerformedAction;
                        })
                        .commit();

    // Setup a role that can fulfill the goals, only the smartAgent will be
    // assigned this role to test role filtering when delegating to the team.
    auto agentRole = bdi.role("AgentRole")
                        .goal(agentGoal)
                        .commit();

    auto agentPlan = bdi.plan("AgentPlan")
                        .handles(agentGoal)
                        .body(bdi.coroutine()
                                 .action(PERFORM_ACTION)
                             )
                        .commit();

    auto performActionLambda = [&](jack::Agent &agent, jack::Message &, jack::Message &, jack::ActionHandle) -> jack::Event::Status {
        if (agent.name() == SMART_AGENT_NAME)
        {
            smartAgentPerformedAction = true;
        }
        else
        {
            assert(agent.name() == DUMB_AGENT_NAME);
            dumbAgentPerformedAction = true;
        }

        return jack::Event::Status::SUCCESS;
    };

    // Create the smart and dumb agent, we expect the smart agent to complete
    // the action, the dumb agent should not because the goal specifies
    // a particular role that can achieve it (a role that is only assigned to
    // the smart agent).
    jack::AgentHandle teamHandle       = bdi.agent("AgentTeamTemplate").commitAsTeam().createTeam("AgentTeam");
    jack::AgentHandle smartAgentHandle = bdi.agent("SmartAgent Template")
                                            .plan(agentPlan)
                                            .role(agentRole)
                                            .handleAction(PERFORM_ACTION, performActionLambda)
                                            .commitAsAgent()
                                            .createAgent(SMART_AGENT_NAME);
    jack::AgentHandle dumbAgentHandle = bdi.agent("DumbAgent Template")
                                           .plan(agentPlan)
                                           .handleAction(PERFORM_ACTION, performActionLambda)
                                           .commitAsAgent()
                                           .createAgent(DUMB_AGENT_NAME);

    auto *team       = static_cast<jack::Team *>(bdi.getAgent(teamHandle));
    auto *smartAgent = bdi.getAgent(smartAgentHandle);
    auto *dumbAgent  = bdi.getAgent(dumbAgentHandle);

    // Start a team with only the dumbAgent activated. The dumbAgent does NOT
    // have a sufficient role to achieve the AgentGoal. Neither action
    // should be performed.
    dumbAgent->start();

    team->addMemberAgent(dumbAgent);
    team->addMemberAgent(smartAgent);
    team->start();

    enginePollAndRecordSchedules(bdi, 100 /*pollCount*/);
    EXPECT_FALSE(smartAgentPerformedAction);
    EXPECT_FALSE(dumbAgentPerformedAction);

    // Start the smart agent who has a role that can fulfill the goal. Only the
    // smart agent can and should perform the action to hit the goal.
    smartAgent->start();
    enginePollAndRecordSchedules(bdi, 1 /*pollCount*/);
    EXPECT_TRUE(smartAgent->running());

    team->pursue(agentGoal.name(), jack::GoalPersistent_Yes);
    enginePollAndRecordSchedules(bdi, 100 /*pollCount*/);
    EXPECT_TRUE(smartAgentPerformedAction);
    EXPECT_FALSE(dumbAgentPerformedAction);
}

TEST(TEAMS, SharedBeliefs)
{
    /*! ***************************************************************************************
     * ****************************************************************************************/

    jack::Engine bdi("TeamWithSharedBeliefs");
    bdi.exitWhenDone();

    // The action to be performed by the team agent
    auto agentAction = bdi.action("AgentAction").commit();
    auto agentGoal   = bdi.goal("AgentGoal").commit();
    auto agentPlan   = bdi.plan("AgentPlan")
                         .handles(agentGoal)
                         .body(bdi.coroutine().action(agentAction.name()))
                         .commit();

    auto agentBelief = bdi.message("AgentBeliefSet")
                          .field<bool>("CanSwim")
                          .field<int>("Age")
                          .field<float>("Braveness")
                          .field<int8_t>("ID")
                          .commit();

    // Setup a role that can fulfill the goals
    auto agentReadRole = bdi.role("AgentReadRole")
                            .goal(agentGoal)
                            .beliefs(agentBelief, jack::RoleBeliefSet::ReadAccess::YES, jack::RoleBeliefSet::WriteAccess::NO)
                            .commit();

    auto agentWriteRole = bdi.role("AgentWriteRole")
                             .goal(agentGoal)
                             .beliefs(agentBelief, jack::RoleBeliefSet::ReadAccess::NO, jack::RoleBeliefSet::WriteAccess::YES)
                             .commit();

    auto agentReadWriteRole = bdi.role("AgentReadWriteRole")
                                 .goal(agentGoal)
                                 .beliefs(agentBelief, jack::RoleBeliefSet::ReadAccess::YES, jack::RoleBeliefSet::WriteAccess::YES)
                                 .commit();


    auto performActionLambda = [&](jack::Agent &agent, jack::Message &, jack::Message &, jack::ActionHandle) -> jack::Event::Status {
        return jack::Event::Status::SUCCESS;
    };

    jack::AgentHandle teamHandle          = bdi.agent("AgentTeamTemplate").commitAsTeam().createTeam("AgentTeam");
    jack::AgentHandle readMsgAgentHandle = bdi.agent("ReadMsgAgent Template")
                                               .plan(agentPlan)
                                               .role(agentReadRole)
                                               .handleAction(agentAction.name(), performActionLambda)
                                               .commitAsAgent()
                                               .createAgent("ReadMsgAgent");

    jack::AgentHandle writeMsgAgentHandle = bdi.agent("WriteMsgAgent Template")
                                                .plan(agentPlan)
                                                .role(agentWriteRole)
                                                .handleAction(agentAction.name(), performActionLambda)
                                                .commitAsAgent()
                                                .createAgent("WriteMsgAgent");

    jack::AgentHandle readWriteMsgAgentHandle = bdi.agent("ReadWriteMsgAgent Template")
                                                    .plan(agentPlan)
                                                    .role(agentReadWriteRole)
                                                    .handleAction(agentAction.name(), performActionLambda)
                                                    .commitAsAgent()
                                                    .createAgent("ReadWriteMsgAgent");

    auto *team               = static_cast<jack::Team *>(bdi.getAgent(teamHandle));
    auto *readMsgAgent      = bdi.getAgent(readMsgAgentHandle);
    auto *writeMsgAgent     = bdi.getAgent(writeMsgAgentHandle);
    auto *readWriteMsgAgent = bdi.getAgent(readWriteMsgAgentHandle);

    team->addMemberAgent(readMsgAgent);
    team->addMemberAgent(writeMsgAgent);
    team->addMemberAgent(readWriteMsgAgent);

    readMsgAgent->start();
    writeMsgAgent->start();
    readWriteMsgAgent->start();
    team->start();

    /// Set the belief of the agent to dirty the beliefset and allow it to be
    /// shared between team members.
    {
        jack::Message readMsg = *readMsgAgent->message(agentBelief.name());
        readMsg.setFieldValue<bool>("CanSwim", true);
        readMsgAgent->sendMessage(std::move(readMsg), false /*broadcastToBus*/);

        jack::Message writeMsg = *writeMsgAgent->message(agentBelief.name());
        writeMsg.setFieldValue<bool>("CanSwim", true);
        writeMsgAgent->sendMessage(std::move(writeMsg), false /*broadcastToBus*/);

        jack::Message readWriteMsg = *readWriteMsgAgent->message(agentBelief.name());
        readWriteMsg.setFieldValue<bool>("CanSwim", true);
        readWriteMsgAgent->sendMessage(std::move(readWriteMsg), false /*broadcastToBus*/);
    }

    enginePollAndRecordSchedules(bdi, 100 /*pollCount*/);
    ASSERT_EQ(team->sharedBeliefSets().count(agentBelief.name()), 1);

    // Check the team has the correct shared beliefsets
    {
        const std::vector<jack::Agent::SharedBeliefSet> &sharedBeliefs = team->sharedBeliefSets().at(agentBelief.name());
        EXPECT_EQ(sharedBeliefs.size(), 2) << "The team \"" << team->name() << "\" should have 2 beliefsets (1x WriteMsgAgent, 1x ReadWriteMsgAgent)";

        auto readMsgAgentBeliefSetIt      = std::find(sharedBeliefs.begin(), sharedBeliefs.end(), readMsgAgent->UUID());
        auto writeMsgAgentBeliefSetIt     = std::find(sharedBeliefs.begin(), sharedBeliefs.end(), writeMsgAgent->UUID());
        auto readWriteMsgAgentBeliefSetIt = std::find(sharedBeliefs.begin(), sharedBeliefs.end(), readWriteMsgAgent->UUID());

        ASSERT_EQ(readMsgAgentBeliefSetIt,      sharedBeliefs.end()) << "Should not exist. The read role agent does not share to the team";
        ASSERT_NE(writeMsgAgentBeliefSetIt,     sharedBeliefs.end());
        ASSERT_NE(readWriteMsgAgentBeliefSetIt, sharedBeliefs.end());
    }

    // Check the agent with a read role has the correct shared beliefsets
    {
        const std::vector<jack::Agent::SharedBeliefSet> &sharedBeliefs = readMsgAgent->sharedBeliefSets().at(agentBelief.name());
        EXPECT_EQ(sharedBeliefs.size(), 2) << "The team \"" << team->name() << "\" should have 2 beliefsets (1x WriteMsgAgent, 1x ReadWriteMsgAgent)";

        auto readMsgAgentBeliefSetIt      = std::find(sharedBeliefs.begin(), sharedBeliefs.end(), readMsgAgent->UUID());
        auto writeMsgAgentBeliefSetIt     = std::find(sharedBeliefs.begin(), sharedBeliefs.end(), writeMsgAgent->UUID());
        auto readWriteMsgAgentBeliefSetIt = std::find(sharedBeliefs.begin(), sharedBeliefs.end(), readWriteMsgAgent->UUID());

        ASSERT_EQ(readMsgAgentBeliefSetIt,      sharedBeliefs.end()) << "Should not exist. The team shouldn't have received the agent beliefset in the first place";
        ASSERT_NE(writeMsgAgentBeliefSetIt,     sharedBeliefs.end());
        ASSERT_NE(readWriteMsgAgentBeliefSetIt, sharedBeliefs.end());

        EXPECT_EQ(writeMsgAgentBeliefSetIt->m_memberId, writeMsgAgent->UUID());
        EXPECT_EQ(writeMsgAgentBeliefSetIt->m_memberName, writeMsgAgent->name());
        EXPECT_GT(writeMsgAgentBeliefSetIt->m_lastUpdated, std::chrono::milliseconds(0));
        {
            bool a = std::any_cast<bool>(writeMsgAgentBeliefSetIt->m_beliefSet.fields().at("CanSwim").m_value);
            bool b = std::any_cast<bool>(writeMsgAgent->message(agentBelief.name())->fields().at("CanSwim").m_value);
            EXPECT_EQ(a, b);
        }

        EXPECT_EQ(readWriteMsgAgentBeliefSetIt->m_memberId, readWriteMsgAgent->UUID());
        EXPECT_EQ(readWriteMsgAgentBeliefSetIt->m_memberName, readWriteMsgAgent->name());
        EXPECT_GT(readWriteMsgAgentBeliefSetIt->m_lastUpdated, std::chrono::milliseconds(0));
        {
            bool a = std::any_cast<bool>(readWriteMsgAgentBeliefSetIt->m_beliefSet.fields().at("CanSwim").m_value);
            bool b = std::any_cast<bool>(readWriteMsgAgent->message(agentBelief.name())->fields().at("CanSwim").m_value);
            EXPECT_EQ(a, b);
        }
    }

    // Check the agent with a write role has the correct shared beliefsets
    {
        const std::map<jack::Agent::BeliefSetName, std::vector<jack::Agent::SharedBeliefSet>> &sharedBeliefs = writeMsgAgent->sharedBeliefSets();
        EXPECT_TRUE(sharedBeliefs.empty()) << "The agent \"" << writeMsgAgent->name() << "\" should have no beliefsets, it can only write to the team";
    }

    // Check the agent with a read/write role has the correct shared beliefsets
    {
        const std::vector<jack::Agent::SharedBeliefSet> &sharedBeliefs = readWriteMsgAgent->sharedBeliefSets().at(agentBelief.name());
        EXPECT_EQ(sharedBeliefs.size(), 1) << "The agent \"" << readWriteMsgAgent->name() << "\" should have 1 beliefset (1x WriteMsgAgent)";

        auto readMsgAgentBeliefSetIt      = std::find(sharedBeliefs.begin(), sharedBeliefs.end(), readMsgAgent->UUID());
        auto writeMsgAgentBeliefSetIt     = std::find(sharedBeliefs.begin(), sharedBeliefs.end(), writeMsgAgent->UUID());
        auto readWriteMsgAgentBeliefSetIt = std::find(sharedBeliefs.begin(), sharedBeliefs.end(), readWriteMsgAgent->UUID());

        ASSERT_EQ(readMsgAgentBeliefSetIt,      sharedBeliefs.end()) << "Should not exist. The read role agent does not share to the team.";
        ASSERT_NE(writeMsgAgentBeliefSetIt,     sharedBeliefs.end());
        ASSERT_EQ(readWriteMsgAgentBeliefSetIt, sharedBeliefs.end()) << "Should not exist. The agent does not get its own beliefset shared to itself.";

        EXPECT_EQ(writeMsgAgentBeliefSetIt->m_memberId, writeMsgAgent->UUID());
        EXPECT_EQ(writeMsgAgentBeliefSetIt->m_memberName, writeMsgAgent->name());
        EXPECT_GT(writeMsgAgentBeliefSetIt->m_lastUpdated, std::chrono::milliseconds(0));
        {
            bool a = std::any_cast<bool>(writeMsgAgentBeliefSetIt->m_beliefSet.fields().at("CanSwim").m_value);
            bool b = std::any_cast<bool>(writeMsgAgent->message(agentBelief.name())->fields().at("CanSwim").m_value);
            EXPECT_EQ(a, b);
        }
    }
}

TEST(TEAMS, DISABLED_DoubleSchedulingOfFinishedDesire)
{
    /**************************************************************************
     * When a team member finishes a delegated goal, check that the goal does
     * not get scheduled again by the team, i.e. due to double scheduling bugs
     * related to timings between finishing, concluding an intention and the
     * team sending out auctions inbetween.
     **************************************************************************/
    /// \note The original bug that this test addresses is as follows.
    ///
    /// The team has 1 agent and delegates a goal to it. This action in the goal
    /// is made to be pending. We make the agent trigger success on the action
    /// *and* at the same time we dirty the schedule of the team to force it
    /// to replan in the next tick.
    ///
    /// The team will receives these events and start concluding the delegated
    /// intention, its schedule is also dirtied so it will start planning.
    /// Since the intention is concluding- the goal/desire is still a part of
    /// the team. The team will make a schedule with the desire even though its
    /// finished! That's bug 1.
    ///
    /// On the next tick, the engine will delete the desire and intention- but
    /// the team's schedule was generated using the old desire list, i.e.
    /// referencing a desire that is already finished. The team will auction off
    /// goals to the members again.
    ///
    /// Once the auction comes back, normally, if a schedule has an intention
    /// that is already executing in the intention they will be merged together
    /// and the intention will not be restarted- leading to "correct" behaviour.
    /// In the buggy implementation, since the intention is concluded and now
    /// deleted, the schedule that comes back can't merge the two and instead adds
    /// it to the team as brand new- this causes the intention to be executed
    /// twice when we only wanted to do it once.
    ///
    /// The key point here is that because the auction takes a N amount of ticks
    /// to resolve, the team's intention is deleted and when the auction is
    /// complete the team can no longer merge the intentions as it's gone, so
    /// the team believes it's free to execute the intention.

    jack::Engine bdi("TeamDoubleScheduling");
    bdi.exitWhenDone();

    auto agentAction = bdi.action("AgentAction").commit();
    auto agentGoal   = bdi.goal("AgentGoal").commit();
    auto agentPlan   = bdi.plan("AgentPlan")
                           .handles(agentGoal)
                           .body(bdi.coroutine()
                                     .action(agentAction.name())
                                )
                           .commit();

    auto agentRole = bdi.role("AgentRole")
                         .goal(agentGoal)
                         .commit();

    const std::string DUMMY_FLAG = "DummyFlag";
    auto teamBelief = bdi.message("TeamBeliefs")
                          .field<bool>(DUMMY_FLAG)
                          .commit();

    jack::AgentHandle teamHandle = bdi.agent("AgentTeamTemplate")
                                      .belief(teamBelief)
                                      .commitAsTeam()
                                      .createTeam("Team");

    int actionInvokeCount = 0;
    jack::ActionHandle actionHandle = {};
    jack::AgentHandle  agentHandle =
        bdi.agent("Agent Template")
            .plan(agentPlan)
            .role(agentRole)
            .handleAction(agentAction.name(),
                          [&actionHandle, &actionInvokeCount](jack::Agent        &agent,
                                                              jack::Message      &msg,
                                                              jack::Message      &out,
                                                              jack::ActionHandle  handle) {
                              actionInvokeCount++;
                              actionHandle = handle;
                              return jack::Event::PENDING;
                          })
            .commitAsAgent()
            .createAgent("Agent");

    auto        *team  = static_cast<jack::Team *>(bdi.getAgent(teamHandle));
    jack::Agent *agent = bdi.getAgent(agentHandle);
    team->addMemberAgent(agent);
    agent->start();
    team->start();

    enginePollAndRecordSchedules(bdi, 16 /*pollCount*/); /// Tick the engine into a nice state

    team->pursue(agentGoal, jack::GoalPersistent_No);
    enginePollAndRecordSchedules(bdi, 16 /*pollCount*/); /// Tick the engine enough to trigger the delegated agent action

    /// Ensure the engine is in the state that we expected
    EXPECT_EQ(actionInvokeCount, 1)
        << "The goal was not pursued, otherwise the lambda would have incremented the counter";

    /// Complete the action, but dirty the schedule as well, these 2 operations
    /// will create 2 events that are handled in the same tick.
    {
        agent->finishActionHandle(actionHandle, true /*success*/);
        std::shared_ptr<jack::Message> beliefSet = team->message(teamBelief.name());
        beliefSet->setFieldValue<bool>(DUMMY_FLAG, true); /// Dirty the schedule for the team
    }

    /// Tick the engine, this will complete the plan- and the engine will start
    /// trying to clean it up. The team beliefset will be dirtied and the team
    /// will try and make a new schedule.
    enginePollAndRecordSchedules(bdi, 1 /*pollCount*/);

    EXPECT_EQ(agent->intentions().size(), 1)
        << "The agent should still have the intention to do the action, but it should enter the "
           "concluding phase because we returned success in the action handler";

    EXPECT_TRUE(agent->intentions()[0]->currentIntention()->finished())
        << "The agent should still have the intention to do the action, but it should enter the "
           "finishing state because we returned success in the action handler";

    /// Tick the engine, this will "conclude" the team & agent's intention and
    /// delete the desire in the one tick
    enginePollAndRecordSchedules(bdi, 1 /*pollCount*/);

    /// Tick the engine, some arbitrary amount to make sure there's nothing
    /// hiding in the scheduler/or the team auctions and everything should be
    /// truly finished.
    enginePollAndRecordSchedules(bdi, 16 /*pollCount*/);

    EXPECT_TRUE(agent->intentions().empty());

    EXPECT_TRUE(agent->desires().empty());

    EXPECT_TRUE(team->intentions().empty());

    EXPECT_TRUE(team->desires().empty());

    EXPECT_EQ(actionInvokeCount, 1)
        << "Somehow the action got invoked twice, but we only performed the action once.";
}

TEST(TEAMS, SpawnAgent)
{
    // NOTE: Team with a SpawnGoal which will trigger the engine to queue
    // a create agent event.
    const std::string AGENT_NAME_BELIEF     = "AgentName";
    const std::string AGENT_TEMPLATE_BELIEF = "Agent Template";
    const std::string START_BELIEF          = "Start";
    const std::string TEAM_BELIEF           = "Team";

    jack::Engine bdi("SpawnAgent");

    jack::FieldRegistry& registry = jack::FieldRegistry::getInstance();
    registry.registerType<jack::AgentHandle>("AgentHandle");

    auto msg = bdi.message("Message")
                  .field<std::string>(AGENT_NAME_BELIEF)
                  .field<bool>(START_BELIEF)
                  .field<jack::AgentHandle>(TEAM_BELIEF)
                  .field<std::string>(AGENT_TEMPLATE_BELIEF)
                  .commit();

    auto spawnAction = bdi.action("SpawnAction")
                          .request(msg.name())
                          .commit();

    auto spawnGoal   = bdi.goal("SpawnGoal").commit();
    auto spawnPlan   = bdi.plan("SpawnPlan")
                          .handles(spawnGoal)
                          .body(bdi.coroutine()
                                   .action(spawnAction.name()))
                          .commit();

    auto delegateAction = bdi.action("DelegateAction").commit();
    auto delegateGoal   = bdi.goal("DelegateGoal").commit();
    auto delegatePlan   = bdi.plan("DelegatePlan")
                             .handles(delegateGoal)
                             .body(bdi.coroutine()
                                      .action(delegateAction.name()))
                             .commit();

    auto delegateTemplate = bdi.agent("DelegateTemplate")
        .plan(delegatePlan)
        .desire(delegateGoal)
        .handleAction("DelegateAction",
                      [&](jack::Agent& agent, jack::Message& msg, jack::Message &out, jack::ActionHandle handle) {
                          fmt::println("Spawned agent [agent={}]", agent.handle().toString());
                          return jack::Event::PENDING;
                      })
        .commitAsAgent();

    auto teamTemplate =
        bdi.agent("OverlordTemplate")
            .plan(spawnPlan)
            .handleAction(spawnAction.name(),
                          [&](jack::Agent& agent, jack::Message& msg, jack::Message& out, jack::ActionHandle handle) {
                              const auto *agentName     = msg.getPtr<std::string>(AGENT_NAME_BELIEF);
                              const auto *agentTemplate = msg.getPtr<std::string>(AGENT_TEMPLATE_BELIEF);
                              const auto *start         = msg.getPtr<bool>(START_BELIEF);
                              const auto *team          = msg.getPtr<jack::AgentHandle>(TEAM_BELIEF);
                              jack::AgentHandle delegate = bdi.queueCreateAgent(*agentTemplate,
                                                                                *agentName,
                                                                                jack::UniqueId::random(),
                                                                                *start,
                                                                                team);
                              (void)delegate;
                              return jack::Event::SUCCESS;
                          })
            .commitAsTeam();

    jack::AgentHandle teamHandle = teamTemplate.createTeam("Supervisor");
    auto *team = dynamic_cast<jack::Team *>(bdi.getAgent(teamHandle));
    team->start();

    const struct SpawnConfig {
        std::string       agentName;
        bool              start;
        std::string       agentTemplate;
        jack::AgentHandle team;
    } SPAWN_CONFIG[] = {
        {"Alice"   /*name*/, false /*start*/, teamTemplate.name()     /*agentTemplate*/, teamHandle /*team*/},
        {"Bob"     /*name*/, true  /*start*/, delegateTemplate.name() /*agentTemplate*/, teamHandle /*team*/},
        {"Charlie" /*name*/, false /*start*/, delegateTemplate.name() /*agentTemplate*/, teamHandle /*team*/},
    };

    // NOTE: Create the agents from a goal
    const int NUM_AGENTS = sizeof(SPAWN_CONFIG)/sizeof(SPAWN_CONFIG[0]);
    for (int i = 0; i < NUM_AGENTS; i++) {
        const SpawnConfig *config = SPAWN_CONFIG + i;
        auto beliefs = jack::Message(*bdi.getMessageSchema(msg.name()));
        beliefs.setFieldValue<std::string>(AGENT_NAME_BELIEF, config->agentName);
        beliefs.setFieldValue<std::string>(AGENT_TEMPLATE_BELIEF, config->agentTemplate);
        beliefs.setFieldValue<bool>(START_BELIEF, config->start);
        beliefs.setFieldValue<jack::AgentHandle>(TEAM_BELIEF, config->team);

        EXPECT_NE(beliefs.getPtr<decltype(config->agentName)>(AGENT_NAME_BELIEF), nullptr);
        EXPECT_NE(beliefs.getPtr<decltype(config->agentTemplate)>(AGENT_TEMPLATE_BELIEF), nullptr);
        EXPECT_NE(beliefs.getPtr<decltype(config->start)>(START_BELIEF), nullptr);
        EXPECT_NE(beliefs.getPtr<decltype(config->team)>(TEAM_BELIEF), nullptr);

        team->pursue(spawnGoal.name(), jack::GoalPersistent_No, beliefs);
    }

    // NOTE: Tick engine and wait for agents to show up
    for (int i = 0;
         i < 100 && (bdi.agentList().size() + 1/*team*/) != NUM_AGENTS;
         i++)
    {
        enginePollAndRecordSchedules(bdi, 1 /*pollCount*/);
    }
    EXPECT_TRUE((bdi.agentList().size() + 1/*team*/) != NUM_AGENTS);

    // NOTE: Let the delegated agents run their actions
    enginePollAndRecordSchedules(bdi, 10 /*pollCount*/, writer, true /*writeToFile*/);

    // NOTE: Verify the runtime agent model
    std::vector<jack::AgentHandle> agentList = bdi.agentList();
    for (int i = 1 /*skip the initial team*/; i < agentList.size(); i++) {
        const jack::AgentHandle& handle = agentList[i];
        const SpawnConfig*       config = SPAWN_CONFIG + (i - 1);
        const jack::Agent*       agent  = bdi.getAgent(handle);

        EXPECT_EQ(agent->name(), config->agentName);
        EXPECT_EQ(agent->running(), config->start);

        std::string desire = agent->plans()[0]; /// \note Each template should have 1 goal
        if (config->agentTemplate == teamTemplate.name()) {
            EXPECT_TRUE(desire == spawnPlan.name());
        } else {
            EXPECT_TRUE(desire == delegatePlan.name());
        }

        if (config->team.valid()) {
            EXPECT_GT(agent->teamMemberships().size(), 0);
        } else {
            EXPECT_EQ(agent->teamMemberships().size(), 0);
        }
    }
}

TEST(TEAMS, TeamGoalReferencesCompletedCoroutineBecauseOfNoWait)
{
    /**************************************************************************
     * Overview
     * A team's delegated sub-goal holds a reference to a coroutine that can be
     * completed quickly due to no-wait being applied to all tasks.
     *
     * The team plan create two sub goals for delegation using nowait(async).
     * This unit test will ensure that the team plan doesn't finish untill all
     * async delegations in the plan are finished.
     */

    jack::Engine bdi("TeamGoalReferencesCompletedCoroutineBecauseOfNoWait");
    bdi.exitWhenDone();

    /**************************************************************************
     * Agent A
     **************************************************************************/
    auto agentAction = bdi.action("AgentAction").commit();
    auto agentGoal   = bdi.goal("AgentGoal").commit();
    auto agentPlan   = bdi.plan("AgentPlan")
                          .handles(agentGoal)
                          .body(bdi.coroutine()
                                   .action(agentAction.name())
                                   .sleep(100)
                               )
                          .commit();

    auto agentRole = bdi.role("AgentRole")
                         .goal(agentGoal)
                         .commit();

    int agentACalls = 0;

    /**************************************************************************
     * Agent B
     **************************************************************************/
    auto agentActionB = bdi.action("AgentActionB").commit();
    auto agentGoalB   = bdi.goal("AgentGoalB").commit();
    auto agentPlanB   = bdi.plan("AgentPlanB")
                          .handles(agentGoalB)
                          .body(bdi.coroutine()
                                   .action(agentActionB.name())
                               )
                          .commit();

    auto agentRoleB = bdi.role("AgentRoleB")
                         .goal(agentGoalB)
                         .commit();

    int agentBCalls = 0;

    /**************************************************************************
     * Team
     **************************************************************************/
    auto teamGoal = bdi.goal("TeamGoal").commit();
    auto teamPlan = bdi.plan("TeamPlan")
                       .handles(teamGoal)
                       .body(bdi.coroutine()
                                .print("start team goal\n")
                                .goal(agentGoal.name())
                                .nowait()
                                .goal(agentGoalB.name())
                                .nowait()
                                .print("end team goal\n")
                               )
                         .commit();

    auto teamRole = bdi.role("TeamRole")
                       .goal(teamGoal)
                       .commit();
    /**************************************************************************
     * Create Agents
     **************************************************************************/
    jack::ActionHandle handles[2] = {};
    jack::AgentHandle teamHandle = bdi.agent("TeamTemplate")
                                      .desire(teamGoal)
                                      .plan(teamPlan)
                                      .role(teamRole)
                                      .commitAsTeam()
                                      .createTeam("Team");

    jack::AgentHandle agentAHandle =
        bdi.agent("AgentATemplate")
            .plan(agentPlan)
            .role(agentRole)
            .handleAction(agentAction.name(),
                          [&handles, &agentACalls](jack::Agent &, jack::Message &msg, jack::Message &out, jack::ActionHandle handle) {
                              handles[0] = handle;
                              agentACalls++;
                              std::cout << "called action for Agent A" << std::endl;
                              return jack::Event::PENDING;
                          })
            .commitAsTeam()
            .createTeam("AgentA");

    jack::AgentHandle agentBHandle =
        bdi.agent("AgentBTemplate")
            .plan(agentPlanB)
            .role(agentRoleB)
            .handleAction(agentActionB.name(),
                          [&handles, &agentBCalls](jack::Agent &, jack::Message &msg, jack::Message &out, jack::ActionHandle handle) {
                              handles[1] = handle;
                              agentBCalls++;
                              std::cout << "called action for Agent B" << std::endl;
                              return jack::Event::PENDING;
                          })
            .commitAsTeam()
            .createTeam("AgentB");

    /**************************************************************************
     * Engine
     **************************************************************************/
    auto *team   = static_cast<jack::Team *>(bdi.getAgent(teamHandle));
    auto *agentA = bdi.getAgent(agentAHandle);
    auto *agentB = bdi.getAgent(agentBHandle);

    team->start();
    agentA->start();
    agentB->start();

    team->addMemberAgent(agentA);
    team->addMemberAgent(agentB);

    while (handles[0].m_name.empty() || handles[1].m_name.empty()) {
        enginePollAndRecordSchedules(bdi, 1 /*pollCount*/, std::chrono::milliseconds(16));
    }

    std::cout << "spin" << std::endl;

    // spin the engine - the team plan should not have finished so we hope it
    // won't run another one
    enginePollAndRecordSchedules(bdi, 100 /*pollCount*/, std::chrono::milliseconds(16));

    EXPECT_EQ(agentACalls, 1);
    EXPECT_EQ(agentBCalls, 1);

    // make the agent plans finish so that the team plan can finish
    std::cout << "trigger finish plans" << std::endl;
    EXPECT_TRUE(agentA->finishActionHandle(handles[0], true /*success*/));
    EXPECT_TRUE(agentB->finishActionHandle(handles[1], true /*success*/));

    // The team goal/plans should run again
    enginePollAndRecordSchedules(bdi, 100 /*pollCount*/, std::chrono::milliseconds(16));

    EXPECT_EQ(agentACalls, 2);
    EXPECT_EQ(agentBCalls, 2);
}

TEST(AnAgentCanExecuteApplicablePerformGoals, DISABLED_TeamFailsPreconditionAgentSucceeds)
{
    /**************************************************************************
     * The team fails the goal precondition but its team members succeed the
     * precondition.
     **************************************************************************/
    jack::Engine bdi;
    bdi.exitWhenDone();

    /**************************************************************************
     * Beliefsets
     **************************************************************************/
    std::string_view CAN_I_RUN_THE_GOAL = "Can I run the goal?"sv;
    auto beliefs = bdi.message("Beliefs")
                      .field<bool>(std::string(CAN_I_RUN_THE_GOAL))
                      .commit();

    /**************************************************************************
     * Agent A
     **************************************************************************/
    /// The goal is configured to look at the flag in the beliefs.
    auto agentAction = bdi.action("Agent Action").commit();
    auto agentGoal   = bdi.goal("Agent Goal")
                         .pre([&](const jack::BeliefContext& context) {
                             bool result = false;
                             bool found  = context.get<bool>(std::string(CAN_I_RUN_THE_GOAL), result);
                             assert(found);
                             return result;
                         })
                         .commit();
    auto agentPlan   = bdi.plan("Agent Plan")
                          .handles(agentGoal)
                          .body(bdi.coroutine()
                                   .action(agentAction.name())
                               )
                          .commit();

    auto agentRole = bdi.role("AgentRole")
                         .goal(agentGoal)
                         .commit();
    int agentActionInvocations = 0;

    /**************************************************************************
     * Create Agents
     **************************************************************************/
    jack::Agent* agent =
        bdi.agent("Agent Template")
            .belief(beliefs)
            .plan(agentPlan)
            .role(agentRole)
            .handleAction(agentAction.name(),
                          [&agentActionInvocations](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                              agentActionInvocations++;
                              return jack::Event::SUCCESS;
                          })
            .commitAsAgent()
            .createAgentInstance("Agent A");

    auto* team = bdi.agent("Team Template")
                    .belief(beliefs)
                    .commitAsTeam()
                    .createTeamInstance("Team");

    /// The team sets a flag disallowing it from running the goal whereas the
    /// agent is allowed to run it.
    assert(team);
    team->message(beliefs.name())->setFieldValue<bool>(std::string(CAN_I_RUN_THE_GOAL), false);
    agent->message(beliefs.name())->setFieldValue<bool>(std::string(CAN_I_RUN_THE_GOAL), true);

    /**************************************************************************
     * Engine
     **************************************************************************/
    /// \todo We have to poll once before pursuing to ensure that the team and
    /// agents are members of each in order to perform the goal and conduct
    /// delegation.
    team->start();
    agent->start();
    team->addMemberAgent(agent);
    bdi.poll(std::chrono::milliseconds(16));

    team->pursue(agentGoal, jack::GoalPersistent_Yes);
    for(int i = 0; i < 100; ++i) {
        bdi.poll(std::chrono::milliseconds(16));
    }
    EXPECT_EQ(agentActionInvocations, 1);
    EXPECT_EQ(agent->desires().size(), 0);
}

TEST(AnAgentCannotExecuteNonApplicablePerformGoals, DISABLED_NoPlans)
{
    /**************************************************************************
     * The agent has no plans for the goal
     **************************************************************************/
    jack::Engine bdi;
    bdi.exitWhenDone();

    /**************************************************************************
     * Agent
     **************************************************************************/
    auto agentAction = bdi.action("Agent Action").commit();
    auto agentGoal   = bdi.goal("Agent Goal").commit();
    auto agentPlan   = bdi.plan("Agent Plan")
                          .handles(agentGoal)
                          .body(bdi.coroutine()
                                   .action(agentAction.name()))
                          .commit();
    int agentActionInvocations = 0;

    /**************************************************************************
     * Create Agents
     **************************************************************************/
    /// No plan is assigned to the agent as per the requirement.
    jack::Agent* agent =
        bdi.agent("Agent Template")
           .handleAction(agentAction.name(),
                          [&agentActionInvocations](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                              agentActionInvocations++;
                              return jack::Event::SUCCESS;
                          })
           .commitAsAgent()
           .createAgentInstance("Agent A");

    /**************************************************************************
     * Engine
     **************************************************************************/
    agent->start();
    agent->pursue(agentGoal, jack::GoalPersistent_Yes);
    for(int i = 0; i < 100; ++i) {
        bdi.poll(std::chrono::milliseconds(16));
    }
    EXPECT_EQ(agentActionInvocations, 0);
    EXPECT_EQ(agent->desires().size(), 0);
}

TEST(AnAgentCannotExecuteNonApplicablePerformGoals, DISABLED_PlansUnusable)
{
    /**************************************************************************
     * The agent has unusable plans for the goal (e.g. all plans available fail
     * the precondition)
     **************************************************************************/
    jack::Engine bdi;
    bdi.exitWhenDone();

    /**************************************************************************
     * Agent
     **************************************************************************/
    /// The plan here always fails the precondition fulfilling the test
    /// requirements.
    auto agentAction = bdi.action("Agent Action").commit();
    auto agentGoal   = bdi.goal("Agent Goal").commit();
    auto agentPlan   = bdi.plan("Agent Plan")
                          .pre([&](const jack::BeliefContext &context) {
                              return false;
                          })
                          .handles(agentGoal)
                          .body(bdi.coroutine()
                                   .action(agentAction.name()))
                          .commit();
    int agentActionInvocations = 0;

    /**************************************************************************
     * Create Agents
     **************************************************************************/
    jack::Agent* agent =
        bdi.agent("Agent Template")
           .plan(agentPlan)
           .handleAction(agentAction.name(),
                          [&agentActionInvocations](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                              agentActionInvocations++;
                              return jack::Event::SUCCESS;
                          })
           .commitAsAgent()
           .createAgentInstance("Agent A");

    /**************************************************************************
     * Engine
     **************************************************************************/
    agent->start();
    agent->pursue(agentGoal, jack::GoalPersistent_Yes);
    for(int i = 0; i < 100; ++i) {
        bdi.poll(std::chrono::milliseconds(16));
    }
    EXPECT_EQ(agentActionInvocations, 0);
    EXPECT_EQ(agent->desires().size(), 0);
}

TEST(AnAgentCannotExecuteNonApplicablePerformGoals, DISABLED_GoalPreconditionFail)
{
    /**************************************************************************
     * The agent fails the precondition for the goal.
     **************************************************************************/
    jack::Engine bdi;
    bdi.exitWhenDone();

    /**************************************************************************
     * Agent
     **************************************************************************/
    /// The goal here always fails the precondition fulfilling the test
    /// requirements.
    auto agentAction = bdi.action("Agent Action").commit();
    auto agentGoal   = bdi.goal("Agent Goal")
                          .pre([&](const jack::BeliefContext &context) {
                              return false;
                          })
                          .commit();
    auto agentPlan   = bdi.plan("Agent Plan")
                          .handles(agentGoal)
                          .body(bdi.coroutine()
                                   .action(agentAction.name()))
                          .commit();
    int agentActionInvocations = 0;

    /**************************************************************************
     * Create Agents
     **************************************************************************/
    jack::Agent* agent =
        bdi.agent("Agent Template")
           .handleAction(agentAction.name(),
                          [&agentActionInvocations](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                              agentActionInvocations++;
                              return jack::Event::SUCCESS;
                          })
           .commitAsAgent()
           .createAgentInstance("Agent A");

    /**************************************************************************
     * Engine
     **************************************************************************/
    agent->start();
    agent->pursue(agentGoal, jack::GoalPersistent_No);
    for(int i = 0; i < 100; ++i) {
        bdi.poll(std::chrono::milliseconds(16));
    }
    EXPECT_EQ(agentActionInvocations, 0);
    EXPECT_EQ(agent->desires().size(), 0);
}

TEST(AnAgentCannotExecuteNonApplicablePerformGoals, DISABLED_DelegationImpossible)
{
    /**************************************************************************
     * The team has no applicable plans and no delegates
     **************************************************************************/
    jack::Engine bdi;
    bdi.exitWhenDone();

    /**************************************************************************
     * Agent A
     **************************************************************************/
    auto agentAction = bdi.action("Agent Action").commit();
    auto agentGoal   = bdi.goal("Agent Goal").commit();
    auto agentPlan   = bdi.plan("Agent Plan")
                          .handles(agentGoal)
                          .body(bdi.coroutine()
                                   .action(agentAction.name()))
                          .commit();

    auto agentRole = bdi.role("AgentRole")
                         .goal(agentGoal)
                         .commit();
    int agentActionInvocations = 0;

    /**************************************************************************
     * Create Agents
     **************************************************************************/
    /// The agent exists but does not handle the goal
    jack::Agent* agent =
        bdi.agent("Agent Template")
            .handleAction(agentAction.name(),
                          [&agentActionInvocations](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                              agentActionInvocations++;
                              return jack::Event::SUCCESS;
                          })
            .commitAsAgent()
            .createAgentInstance("Agent A");

    auto* team = bdi.agent("Team Template")
                    .commitAsTeam()
                    .createTeamInstance("Team");
    /**************************************************************************
     * Engine
     **************************************************************************/
    /// Add the agent but the team can not delegate it as per requirements.
    ///
    /// \todo We have to poll once before pursuing to ensure that the team and
    /// agents are members of each in order to perform the goal and conduct
    /// delegation.
    team->start();
    agent->start();
    team->addMemberAgent(agent);
    bdi.poll(std::chrono::milliseconds(16));

    team->pursue(agentGoal, jack::GoalPersistent_No);
    for(int i = 0; i < 100; ++i) {
        bdi.poll(std::chrono::milliseconds(16));
    }
    EXPECT_EQ(agentActionInvocations, 0);
    EXPECT_EQ(team->desires().size(), 0);
    EXPECT_EQ(agent->desires().size(), 0);
}

TEST(AnAgentCannotExecuteNonApplicablePerformGoals, DISABLED_DelegatorsUnusable)
{
    /**************************************************************************
     * The team has delegates, but all delegates fail the precondition for the
     * plan.
     **************************************************************************/
    jack::Engine bdi;
    bdi.exitWhenDone();

    /**************************************************************************
     * Agent A
     **************************************************************************/
    /// The delegator's plan will always fail as per requirements.
    auto agentAction = bdi.action("Agent Action").commit();
    auto agentGoal   = bdi.goal("Agent Goal").commit();
    auto agentPlan   = bdi.plan("Agent Plan")
                          .pre([&](const jack::BeliefContext& context) {
                              return false;
                          })
                          .handles(agentGoal)
                          .body(bdi.coroutine()
                                   .action(agentAction.name()))
                          .commit();

    auto agentRole = bdi.role("AgentRole")
                         .goal(agentGoal)
                         .commit();
    int agentActionInvocations = 0;

    /**************************************************************************
     * Create Agents
     **************************************************************************/
    /// The agent exists but does not handle the goal
    jack::Agent* agent =
        bdi.agent("Agent Template")
           .plan(agentPlan)
           .role(agentRole)
           .handleAction(agentAction.name(),
                         [&agentActionInvocations](jack::Agent&, jack::Message&, jack::Message&, jack::ActionHandle) {
                             agentActionInvocations++;
                             return jack::Event::SUCCESS;
                         })
           .commitAsAgent()
           .createAgentInstance("Agent A");

    auto* team = bdi.agent("Team Template")
                    .commitAsTeam()
                    .createTeamInstance("Team");
    /**************************************************************************
     * Engine
     **************************************************************************/
    /// Add the agent but the team can not delegate it as all agents will fail
    /// the plan precondition.
    ///
    /// \todo We have to poll once before pursuing to ensure that the team and
    /// agents are members of each in order to perform the goal and conduct
    /// delegation.
    team->start();
    agent->start();
    team->addMemberAgent(agent);
    bdi.poll(std::chrono::milliseconds(16));

    team->pursue(agentGoal, jack::GoalPersistent_No);
    for(int i = 0; i < 100; ++i) {
        bdi.poll(std::chrono::milliseconds(16));
    }
    EXPECT_EQ(agentActionInvocations, 0);
    EXPECT_EQ(team->desires().size(), 0);
    EXPECT_EQ(agent->desires().size(), 0);
}

TEST(TEAMS, StoppedTeamWithDelegatedIntention)
{
    /// Verify that a team that has delegated an intention, that is then stopped
    /// is able to stop and terminate.
    ///
    /// i.e. Is a team able to terminate execution even if it has an intention
    /// that is delegated (this code path used to block on the delegated
    /// intention which meant the team could hang as advancing the delegated
    /// intention can only occur by the team member returning a delegation
    /// result).
    jack::Engine bdi;
    bdi.exitWhenDone();

    /**************************************************************************
     * Agent Model
     **************************************************************************/
    auto agentAction = bdi.action("AgentAction").commit();
    auto agentGoal   = bdi.goal("AgentGoal").commit();
    auto agentPlan   = bdi.plan("AgentPlan")
                          .handles(agentGoal)
                          .body(bdi.coroutine()
                                   .action(agentAction.name()))
                          .commit();
    auto agentRole   = bdi.role("AgentRole")
                          .goal(agentGoal)
                          .commit();

    /**************************************************************************
     * Team Model
     **************************************************************************/
    auto teamGoal = bdi.goal("TeamGoal")
                        .commit();

    auto teamPlan = bdi.plan("TeamPlan")
                       .handles(teamGoal)
                       .body(bdi.coroutine()
                                .goal(agentGoal.name()))
                       .commit();


    /**************************************************************************
     * Create the team and agent
     **************************************************************************/
    jack::Team* team = bdi.agent("TeamTemplate")
                          .commitAsTeam()
                          .createTeamInstance("Team");

    jack::Agent* agent =
        bdi.agent("Agent Template")
            .plan(agentPlan)
            .role(agentRole)
            .handleAction(agentAction.name(),
                          [team](jack::Agent& agent, jack::Message&, jack::Message&, jack::ActionHandle) {
                              team->stop();
                              agent.stop();
                              return jack::Event::PENDING;
                          })
            .commitAsAgent()
            .createAgentInstance("Agent");

    /**************************************************************************
     * Test
     **************************************************************************/
    team->addMemberAgent(agent);
    agent->start();
    team->start();

    /// \todo There's an ordering bug here, if we create the team first then
    /// the team will process both the start event and the perform event.
    ///
    /// The agent has not started yet, so there will be no valid agents for the
    /// team to delegate to.
    ///
    /// See: AgentsTest.InitOrderMustHaveZeroSideEffects_BROKEN
    enginePollAndRecordSchedules(bdi, 1 /*pollCount*/);

    /// \note The test here is that the engine should eventually exit and not
    /// hang or timeout the tests because it got "live-locked".
    team->pursue(agentGoal, jack::GoalPersistent_No);
    enginePollAndRecordSchedules(bdi, -1 /*pollCount*/);
}

TEST(TEAMS, TeamRejectsGoalIfNoPlansOrMembersWithRoles)
{
    jack::Engine bdi;
    jack::GoalBuilder goal = bdi.goal("Goal")
                                .commit();

    jack::Agent *agent = bdi.agent("TeamTemplate")
                            .commitAsTeam()
                            .createTeamInstance("Team");
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

TEST(TEAMS, SlowAuctionNotInvalidated)
{
    jack::Engine bdi;
    bdi.exitWhenDone();

    /**************************************************************************
     * The team hierarchy in this test is as follows, a team has a sub-team
     * member and the sub-team has a member named an agent.
     *
     * Team
     * +- SubTeam
     *    +- Agent
     *
     **************************************************************************/

    /**************************************************************************
     * Message
     **************************************************************************/
    auto msg        = bdi.message("Message")
                         .field<bool>("dummy")
                         .commit();

    /**************************************************************************
     * Agent Model
     **************************************************************************/
    auto agentAction = bdi.action("Agent Action").commit();
    auto agentGoal   = bdi.goal("Agent Goal")
                          .satisfied([](const jack::BeliefContext &) {
                            // NOTE: This goal is never satisfied, on auction it
                            // will chain forever, this will slow down the
                            // auction and allow us to try an interrupt it.
                            return false;
                          })
                          .commit();

    auto agentPlan   = bdi.plan("Agent Plan")
                          .handles(agentGoal)
                          .effects([](const jack::BeliefContext&) {})
                          .body(bdi.coroutine()
                                   .action(agentAction.name()))
                          .commit();

    auto role       = bdi.role("Role")
                          .goal(agentGoal)
                          .commit();

    /**************************************************************************
     * Sub Team Model
     **************************************************************************/
    auto subTeamRole   = bdi.role("Sub Team Role")
                            .goal(agentGoal)
                            .commit();

    /**************************************************************************
     * Team Model
     **************************************************************************/
    auto teamGoal = bdi.goal("Team Goal")
                        .commit();

    auto teamPlan = bdi.plan("Team Plan")
                       .handles(teamGoal)
                       .body(bdi.coroutine()
                                .goal(agentGoal.name()))
                       .commit();

    /**************************************************************************
     * Create the agent(s)
     **************************************************************************/
    jack::Team* team = bdi.agent("Team Template")
                          .belief(msg)
                          .plan(teamPlan)
                          .commitAsTeam()
                          .createTeamInstance("Team");

    /// The 'sub-team' is a team member of 'team' who has an outgoing role that
    /// will proxy incoming goals and delegate to the agent.
    jack::Team* subTeam = reinterpret_cast<jack::Team*>(bdi.agent("Sub Team Template")
                                                           .role(role)
                                                           .commitAsTeam()
                                                           .createTeamInstance("Sub Team"));

    bool actionTriggered = false;
    jack::Agent* agent =
        bdi.agent("Agent Template")
           .plan(agentPlan)
           .role(role)
           .handleAction(agentAction.name(),
                         [team, subTeam, &actionTriggered](jack::Agent& agent, jack::Message&, jack::Message&, jack::ActionHandle) {
                             team->stop();
                             subTeam->stop();
                             agent.stop();
                             JACK_INFO_MSG("Action Triggered!");
                             actionTriggered = true;
                             return jack::Event::PENDING;
                         })
           .commitAsAgent()
           .createAgentInstance("Agent");

    /**************************************************************************
     * Test
     **************************************************************************/
    team->addMemberAgent(subTeam);
    subTeam->addMemberAgent(agent);

    team->start();
    subTeam->start();
    agent->start();

    /// \note Get agents started and running
    for (int i = 0; i < 4; i++) {
        bdi.poll();
    }

    EXPECT_TRUE(team->running());
    EXPECT_TRUE(subTeam->running());
    EXPECT_TRUE(agent->running());

    team->pursue(agentGoal, jack::GoalPersistent_No);
    for (int i = 0; i < 64; i++) {
        enginePollAndRecordSchedules(bdi, 1);

        /// \note Dirty the schedule by sending in a message every frame.
        jack::Message dirtyMsg = bdi.createMessage(msg.name());
        team->sendMessage(std::move(dirtyMsg), true /*broadcast to bus*/);
    }

    EXPECT_TRUE(actionTriggered);
}

TEST(TEAMS, TeamMemberDropsDelegatedGoal)
{
    jack::Engine bdi;
    /**************************************************************************
     * Agent Model
     **************************************************************************/
    auto agentAction = bdi.action("Agent Action").commit();
    auto agentGoal   = bdi.goal("Agent Goal")
                          .commit();

    auto agentPlan   = bdi.plan("Agent Plan")
                          .handles(agentGoal)
                          .effects([](const jack::BeliefContext&) {})
                          .body(bdi.coroutine()
                                   .action(agentAction.name()))
                          .commit();

    auto agentRole   = bdi.role("Role")
                          .goal(agentGoal)
                          .commit();

    /**************************************************************************
     * Create the agent(s)
     **************************************************************************/
    jack::Team* team = bdi.agent("Team Template")
                          .commitAsTeam()
                          .createTeamInstance("Team");

    bool actionTriggered = false;
    jack::Agent* agent =
        bdi.agent("Agent Template")
           .plan(agentPlan)
           .role(agentRole)
           .handleAction(agentAction.name(),
                         [](jack::Agent& agent, jack::Message&, jack::Message&, jack::ActionHandle) {
                             /// \note This action will never finish
                             return jack::Event::PENDING;
                         })
           .commitAsAgent()
           .createAgentInstance("Agent");

    /**************************************************************************
     * Test
     **************************************************************************/
    team->addMemberAgent(agent);
    team->start();
    agent->start();

    enginePollAndRecordSchedules(bdi, 16);

    jack::GoalPursue pursue = team->pursue(agentGoal.name(), jack::GoalPersistent_No);
    pursue.promise->then([&]() {
        agent->stop();
        team->stop();
        bdi.exit();
    });

    enginePollAndRecordSchedules(bdi, 16);

    /// \note Drop the goal on the agent, this should cause the delegation to
    /// fail and the team should trigger the team to drop the goal as the only
    /// team member has failed.
    agent->drop(pursue.handle);
    enginePollAndRecordSchedules(bdi, -1);

    /// @todo where is the test?
}

TEST(TEAMS, TeamMemberServiceUnavailableDropsDelegatedGoal)
{
    jack::Engine bdi;
    /**************************************************************************
     * Service Model
     **************************************************************************/
    auto serviceAction = bdi.action("Action").commit();

    /**************************************************************************
     * Agent Model
     **************************************************************************/
    auto agentGoal   = bdi.goal("Agent Goal")
                          .commit();

    auto agentPlan   = bdi.plan("Agent Plan")
                          .handles(agentGoal)
                          .effects([](const jack::BeliefContext&) {})
                          .body(bdi.coroutine()
                                   .action(serviceAction.name()))
                          .commit();

    auto agentRole   = bdi.role("Role")
                          .goal(agentGoal)
                          .commit();

    /**************************************************************************
     * Create the agent(s)
     **************************************************************************/
    jack::Team* team = bdi.agent("Team Template")
                          .commitAsTeam()
                          .createTeamInstance("Team");

    jack::Service* service =
        bdi.service("Service Template")
           .handleAction(serviceAction.name(),
                         [](jack::Service&, jack::Message&, jack::Message&, jack::ActionHandle) {
                             /// \note This action will never finish
                             return jack::Event::PENDING;
                         })
           .commit()
           .createInstance("Service", /*proxy*/ false);

    jack::Agent* agent =
        bdi.agent("Agent Template")
           .plan(agentPlan)
           .role(agentRole)
           .commitAsAgent()
           .createAgentInstance("Agent");

    /**************************************************************************
     * Test
     **************************************************************************/
    team->addMemberAgent(agent);
    team->start();
    agent->start();
    agent->attachService(service->handle(), /*force*/ false);

    
    enginePollAndRecordSchedules(bdi, 16);

    bool goalSuccess = true;
    jack::GoalPursue pursue = team->pursue(agentGoal.name(), jack::GoalPersistent_No);
    pursue.promise->then([&]() {
        agent->stop();
        team->stop();
        bdi.exit();
    }, [&]() {
        agent->stop();
        team->stop();
        bdi.exit();
        goalSuccess = false;
    });

    for (;;) {
        enginePollAndRecordSchedules(bdi, 1);

        /// \note Once the agent has received the delegated desire we exit.
        if (agent->desires().size()) {
            break;
        }
    }

    /// \note We now set the service unavailable. The agent *should* realise
    /// that the service is unavailable and return a failed cost. This should
    /// return back to the team causing it to fail the goal.
    service->setAvailability(false);

    enginePollAndRecordSchedules(bdi, -1);
    EXPECT_FALSE(goalSuccess);
}
