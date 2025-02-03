// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include "bitmaskops.h"
#include "testhelpers.h"

#include <jack/jack.h>
#include <gtest/gtest.h>

using namespace aos;

/**
 * @brief Hanoi puzzle
 *
 * The purpose of this test is to exercise the core planning algorithm of JACK using
 * a simple 3 disk implementation of the tower of hanoi puzzle
 *
 * https://en.wikipedia.org/wiki/Tower_of_Hanoi
 */

// the possible states of each peg in the hanoi puzzle
// there are 3 disks in there version

enum PegState {
    EmptyPeg   = 0x0,
    BigDisk    = 0x1,
    MediumDisk = 0x2,
    SmallDisk  = 0x4
};
ENABLE_BITMASK_OPERATORS(PegState);

// return the top peg (the smallest assuming the model is correct)
PegState top(const PegState &peg)
{
    if ((peg & SmallDisk) == SmallDisk) return SmallDisk;
    if ((peg & MediumDisk) == MediumDisk) return MediumDisk;
    if ((peg & BigDisk) == BigDisk) return BigDisk;
    return EmptyPeg;
}

// remove the top peg
PegState pop(PegState &peg)
{
    PegState t = top(peg);
    peg &= ~t;
    return t;
}

// add a peg
void push(PegState &peg, PegState disk)
{
    peg |= disk;
}

// can the top disk move 'from' peg 'to' peg
bool canMove(PegState from, PegState to)
{
    return top(from) > top(to);
}

// move disk 'from' peg 'to' peg
// assume we can move
void moveDisk(PegState &from, PegState &dest)
{
    PegState disk = pop(from);
    push(dest, disk);
}

// render the buffer using colours
void printColourBar(char* buffer)
{
    std::string_view onwhite = "\033[47m";
    std::string_view onred   = "\033[41m";
    std::string_view ongreen = "\033[42m";
    std::string_view onblue  = "\033[44m";
    std::string_view off     = "\033[0m";

    static const std::map<char, std::string_view> colourMap = {
        { 'A', onred },
        { 'B', ongreen },
        { 'C', onblue }
    };

    jack::ThreadScratchAllocator scratch = jack::getThreadScratchAllocator(nullptr);
    auto                         builder = jack::StringBuilder(scratch.arena);
    while (*buffer != '\0') {
        auto it = colourMap.find(*buffer);
        builder.append(FMT_STRING("{} "), (it != colourMap.end() ? it->second : onwhite));
        buffer++;
    }
    fmt::println(stdout, "{} {}", builder.toStringArena(scratch.arena), off);
}


// render the peg into the text buffers
void writePeg(PegState peg, char *top, char *mid, char *bot)
{
    if ((peg & (BigDisk | MediumDisk)) == (BigDisk | MediumDisk)) {
        if ((peg & SmallDisk) == SmallDisk) {
            top[2] = 'A';
        }
    }

    if ((peg & BigDisk) == BigDisk) {
        if ((peg & MediumDisk) == MediumDisk) {
            mid[1] = 'B';
            mid[2] = 'B';
            mid[3] = 'B';
        } else if ((peg & SmallDisk) == SmallDisk) {
            mid[2] = 'A';
        }
    } else if ((peg & MediumDisk) == MediumDisk) {
        if ((peg & SmallDisk) == SmallDisk) {
            mid[2] = 'A';
        }
    }

    if ((peg & BigDisk) == BigDisk) {
        bot[0] = 'C';
        bot[1] = 'C';
        bot[2] = 'C';
        bot[3] = 'C';
        bot[4] = 'C';
    } else if ((peg & MediumDisk) == MediumDisk) {
        bot[1] = 'B';
        bot[2] = 'B';
        bot[3] = 'B';
    } else if ((peg & SmallDisk) == SmallDisk) {
        bot[2] = 'A';
    }
}

/// print the game to the terminal
void printPuzzle(PegState a, PegState b, PegState c)
{
    char top[18] = {};
    char middle[18] = {};
    char bottom[18] = {};

    std::fill_n(top, 17, ' ');
    std::fill_n(middle, 17, ' ');
    std::fill_n(bottom, 17, ' ');

    writePeg(a, top, middle, bottom);
    writePeg(b, top+6, middle+6, bottom+6);
    writePeg(c, top+12, middle+12, bottom+12);

    printColourBar(top);
    printColourBar(middle);
    printColourBar(bottom);
}

std::string printState(PegState state)
{
    if (state == EmptyPeg) return "";
    if (state == SmallDisk) return "1";
    if (state == MediumDisk) return "2";
    if (state == BigDisk) return "3";
    if (state == (BigDisk | MediumDisk)) return "23";
    if (state == (BigDisk | SmallDisk)) return "13";
    if (state == (MediumDisk | SmallDisk)) return "12";
    if (state == (BigDisk | MediumDisk | SmallDisk)) return "123";
    return "<Unhandled PegState Enum>";
}

TEST(HANOI, HanoiTest1)
{
    /*! ***************************************************************************************
     * Demonstration of performing a simple puzzle
     * The aim is to transistion the disks from peg 1 to peg 3
     * The rules:
     * a larger disk cannot be stacked ontop of a
     * ****************************************************************************************/

    // create the jack engine
    jack::Engine bdi("AgentNode");
    jack::FieldRegistry& registry = jack::FieldRegistry::getInstance();
    registry.registerType<PegState>("PegState");

    //bdi.exitWhenDone();

    // the three pegs of the hanoi puzzle
    std::string_view peg3Name = "peg3";
    auto bs = bdi.message("Hanoi")
                 .fieldWithValue<PegState>("peg1",   BigDisk | MediumDisk | SmallDisk)
                 .fieldWithValue<PegState>("peg2",   EmptyPeg)
                 .fieldWithValue<PegState>(peg3Name, EmptyPeg)
                 .fieldWithValue<std::string>("peg1_str", "123")
                 .fieldWithValue<std::string>("peg2_str", "")
                 .fieldWithValue<std::string>("peg3_str", "")
                 .fieldWithValue<float>("cost", 0.0f)
                 .commit();

    auto moveMsg = bdi.message("Move Message")
                      .field<std::string>("from")
                      .field<std::string>("to")
                      .commit();

    /// possible actions to perform
    bdi.action("MoveDisk")
       .request(moveMsg.name())
       .reply(moveMsg.name())
       .commit();

    bdi.action("RecordResult").commit();

    /// An achievement goal to perform all the tasks
    auto solvePuzzleGoal = bdi.goal("SolvePuzzleGoal")
                               .pre([](const jack::BeliefContext& context) {
                                   // this goal requires that we haven't completed 10 tasks
                                   auto bs    = context.message("Hanoi");
                                   auto state = *bs->getPtr<PegState>("peg3");
                                   return (state != (BigDisk | MediumDisk | SmallDisk));
                               })
                               .satisfied([&](const jack::BeliefContext& context) {
                                   auto bs    = context.message("Hanoi");
                                   auto state = *bs->getPtr<PegState>("peg3");
                                   return (state == (BigDisk | MediumDisk | SmallDisk));
                               })
                               .heuristic([](const jack::BeliefContext& context) {
                                   // until we have proper resource costing and utility this unit
                                   // test does really work correctly.

                                   // Return an estimate of the number of actions remaining to
                                   // complete the puzzle. This is an admissable estimate; it will
                                   // always under estimate It may take more actions

                                   // no guide
                                   /*    auto bs = context.beliefSet("Hanoi");
                                       float cost;
                                       bs->get("cost", cost);
                                       return cost;*/

                                   auto bs = context.message("Hanoi");
                                   auto statePeg1 = *bs->getPtr<PegState>("peg1");
                                   auto statePeg2 = *bs->getPtr<PegState>("peg2");
                                   auto statePeg3 = *bs->getPtr<PegState>("peg3");

                                   if (statePeg1 == (BigDisk | MediumDisk | SmallDisk))
                                       return 7.f;  // start

                                   // cost to complete from the state of peg 3 and peg 2
                                   if (statePeg3 == EmptyPeg) {
                                       if (statePeg2 == EmptyPeg) {
                                           return 7.f;
                                       } else if (statePeg2 == SmallDisk) {
                                           return 8.f;
                                       } else {
                                           return 4.f;
                                       }
                                   }
                                   if (statePeg3 == SmallDisk) return 5.f;  // empty then 4
                                   if (statePeg3 == MediumDisk) {
                                       if (statePeg2 == SmallDisk) {
                                           return 9.f;
                                       } else {
                                           return 8.f;
                                       }
                                   }
                                   if (statePeg3 == BigDisk) return 2.f;                 // easy
                                   if (statePeg3 == (BigDisk | MediumDisk)) return 1.f;  // easy
                                   if (statePeg3 == (BigDisk | SmallDisk))
                                       return 5.f;  // empty 1 then 4
                                   if (statePeg3 == (MediumDisk | SmallDisk))
                                       return 6.f;  // empty 2 then 4
                                   if (statePeg3 == (BigDisk | MediumDisk | SmallDisk))
                                       return 0.f;  // already solved*/
                                   return 0.f;
                               })
                               .commit();

    auto createMovePlan = [&](const std::string &pegA, const std::string &pegB) {
        std::string label = JACK_FMT("{}-to-{}-plan", pegA, pegB);
        return bdi.plan(label)
            .handles(solvePuzzleGoal)
            .pre([pegA, pegB](const jack::BeliefContext& context) {
                auto bs        = context.message("Hanoi");
                auto pegAState = *bs->getPtr<PegState>(pegA);
                auto pegBState = *bs->getPtr<PegState>(pegB);
                return canMove(pegAState, pegBState);
            })
            .effects([pegA, pegB](jack::BeliefContext& context) {
                std::shared_ptr<jack::Message> hanoi     = context.message("Hanoi");
                auto                           pegAState = *hanoi->getPtr<PegState>(pegA);
                auto                           pegBState = *hanoi->getPtr<PegState>(pegB);
                moveDisk(pegAState, pegBState);
                hanoi->setFieldValue(pegA, pegAState);
                hanoi->setFieldValue(pegB, pegBState);
                hanoi->setFieldValue(pegA + "_str", printState(pegAState));
                hanoi->setFieldValue(pegB + "_str", printState(pegBState));

                // add a cost
                auto cost = *hanoi->getPtr<float>("cost");
                hanoi->setFieldValue("cost", cost + 1.0f);
            })
            .body(bdi.coroutine()
                     .action("MoveDisk")
                     .param<std::string>("from", pegA)
                     .param<std::string>("to", pegB))
            .commit();
    };

    jack::TacticHandle tactic = bdi.tactic("Tactic")
       .goal(solvePuzzleGoal.name())
       .loopPlansInfinitely()
       .planOrder(jack::Tactic::PlanOrder::ChooseBestPlan)
       .commit();

    // create plans to move the disks between the pegs
    std::vector<jack::PlanBuilder> movePlans;
    movePlans.emplace_back(createMovePlan("peg1", "peg2"));
    movePlans.emplace_back(createMovePlan("peg1", "peg3"));
    movePlans.emplace_back(createMovePlan("peg2", "peg1"));
    movePlans.emplace_back(createMovePlan("peg2", "peg3"));
    movePlans.emplace_back(createMovePlan("peg3", "peg1"));
    movePlans.emplace_back(createMovePlan("peg3", "peg2"));

    jack::AgentHandle agentHandle = bdi.agent("TestAgent1")
                                       .belief(bs)
                                       .plans(movePlans)
                                       .handleAction("MoveDisk", [&](jack::Agent &agent, jack::Message &msg, jack::Message &out, jack::ActionHandle) {

                                           std::string from = *msg.getPtr<std::string>("from");
                                           std::string to = *msg.getPtr<std::string>("to");

                                           // print the before state of the puzzle
                                           jack::BeliefContext& context = agent.context();
                                           jack::Message        hanoi = *context.message("Hanoi");
                                           auto peg1State = *hanoi.getPtr<PegState>("peg1");
                                           auto peg2State = *hanoi.getPtr<PegState>("peg2");
                                           auto peg3State = *hanoi.getPtr<PegState>("peg3");
                                           printPuzzle(peg1State, peg2State, peg3State);
                                           JACK_DEBUG("Performing action, moving disk [from={}, to={}]", from, to);

                                           // move the disk if we can
                                           auto pegStateA = *hanoi.getPtr<PegState>(from);
                                           auto pegStateB = *hanoi.getPtr<PegState>(to);
                                           if (canMove(pegStateA, pegStateB)) {
                                               moveDisk(pegStateA, pegStateB);
                                               hanoi.setFieldValue(from, pegStateA);
                                               hanoi.setFieldValue(to, pegStateB);

                                               hanoi.setFieldValue(from + "_str", printState(pegStateA));
                                               hanoi.setFieldValue(to + "_str", printState(pegStateB));
                                               agent.sendMessage(std::move(hanoi), false /*broadcastToBus*/);
                                           }

                                           return jack::Event::SUCCESS;
                                       })
                                       .commitAsAgent()
                                       .createAgent("agent1");

    jack::Agent *agent = bdi.getAgent(agentHandle);
    agent->start();
    agent->selectTactic(tactic.m_name);

    jack::GoalPursue pursue = agent->pursue(solvePuzzleGoal, jack::GoalPersistent_Yes);
    pursue.promise->then([&] () {
        agent->stop();
        bdi.exit();
    });

    enginePollAndRecordSchedules(bdi);

    std::shared_ptr<jack::Message> msg = agent->context().message(bs.name());
    PegState peg3 = *msg->getPtr<PegState>(peg3Name);
    EXPECT_EQ(peg3, (BigDisk | MediumDisk | SmallDisk));

}

