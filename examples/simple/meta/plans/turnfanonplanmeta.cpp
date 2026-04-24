#include <simple/meta/plans/turnfanonplanmeta.h>

/// Project
#include <simple/impl/plans/turnfanonplanimpl.h>

/// JACK
#include <jack/corelib.h>
#include <jack/beliefquery.h>
#include <jack/beliefcontext.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
TurnFanOnPlanMeta::TurnFanOnPlanMeta()
: aos::jack::Plan("simple.Turn Fan On Plan")
{
}

TurnFanOnPlanMeta::TurnFanOnPlanMeta(std::string_view name)
: aos::jack::Plan(name)
{
}

TurnFanOnPlanMeta::TurnFanOnPlanMeta(const TurnFanOnPlanMeta *other)
: aos::jack::Plan(other)
{
}

/******************************************************************************
 * Functions
 ******************************************************************************/
aos::jack::Plan* TurnFanOnPlanMeta::clone(JACK_CALL_SITE_ARGS_NO_TAIL_COMMA) const
{
    TurnFanOnPlan* result = aos::jack::globalHeapAllocator.newInternal<TurnFanOnPlan>(JACK_CALL_SITE_INPUT static_cast<const TurnFanOnPlan*>(this));

    aos::jack::BeliefQuery query([result](const aos::jack::BeliefContext& context) {
        return result->pre(context);
    });
    result->setPrecondition(query);

    return result;
}
