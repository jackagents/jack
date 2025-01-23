#ifndef JACK_AUCTIONEVENT_H
#define JACK_AUCTIONEVENT_H

#include <jack/events/event.h>
#include <jack/goal.h>
#include <jack/handles.h>

namespace aos { namespace jack {

/*! ***********************************************************************************************
 * \struct  AuctionEvent
 *
 * An event to request and track a delegation auction
 * ************************************************************************************************/
struct AuctionEventBid
{
    AgentHandle bidder;
    float       score;
};

struct AuctionEvent : public Event
{
    AuctionEvent(const GoalHandle &goal, size_t scheduleId)
        : Event(Event::AUCTION)
        , m_goal(goal)
        , m_scheduleId(scheduleId)
    {
    }

    GoalHandle                   m_goal;        /// Identifies the goal instance this auction was initated for.
    std::vector<AuctionEventBid> m_bids;        /// All the valid bids for the auction.
    uint16_t                     m_missingBids; /// Indicates how many bids did not arrive back to the schedule in time
    size_t                       m_scheduleId;  /// The schedule ID to which this auction was commenced by.
};

}} // namespace aos::jack

#endif // JACK_AUCTIONEVENT_H
