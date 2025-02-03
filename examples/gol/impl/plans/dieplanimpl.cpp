// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <gol/impl/plans/dieplanimpl.h>

#include <gol/meta/messages/cellinfometa.h>

/// JACK
#include <jack/beliefcontext.h>

#include <iostream>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
DiePlan::DiePlan(std::string_view name)
: DiePlanMeta(name)
{
}

DiePlan::DiePlan(const DiePlan* other)
: DiePlanMeta(other)
{
}

/******************************************************************************
 * Functions
 ******************************************************************************/

bool DiePlan::pre([[ maybe_unused ]] const aos::jack::BeliefContext& context)
{
    const auto bs = context.getMessageAsPtr<const CellInfo>();
    assert(bs && "Agent missing CellInfo");
    return bs->is_alive && ((bs->population < 2) || (bs->population > 3));
}
