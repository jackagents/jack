// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <gol/impl/plans/liveplanimpl.h>

#include <gol/meta/messages/cellinfometa.h>

/// JACK
#include <jack/beliefcontext.h>

#include <iostream>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
LivePlan::LivePlan(std::string_view name)
: LivePlanMeta(name)
{
}

LivePlan::LivePlan(const LivePlan* other)
: LivePlanMeta(other)
{
}

/******************************************************************************
 * Functions
 ******************************************************************************/

bool LivePlan::pre([[ maybe_unused ]] const aos::jack::BeliefContext& context)
{
    const auto bs = context.getMessageAsPtr<const CellInfo>();
    assert(bs && "Agent missing CellInfo");
    return !bs->is_alive && (bs->population == 3);
}
