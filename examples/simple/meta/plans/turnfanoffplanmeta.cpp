#include <simple/meta/plans/turnfanoffplanmeta.h>

/// Project
#include <simple/impl/plans/turnfanoffplanimpl.h>

/// JACK
#include <jack/corelib.h>
#include <jack/beliefquery.h>
#include <jack/beliefcontext.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
TurnFanOffPlanMeta::TurnFanOffPlanMeta()
: aos::jack::Plan("simple.Turn Fan Off Plan")
{
}

TurnFanOffPlanMeta::TurnFanOffPlanMeta(std::string_view name)
: aos::jack::Plan(name)
{
}

TurnFanOffPlanMeta::TurnFanOffPlanMeta(const TurnFanOffPlanMeta *other)
: aos::jack::Plan(other)
{
}

/******************************************************************************
 * Functions
 ******************************************************************************/
aos::jack::Plan* TurnFanOffPlanMeta::clone(JACK_CALL_SITE_ARGS_NO_TAIL_COMMA) const
{
    TurnFanOffPlan* result = aos::jack::globalHeapAllocator.newInternal<TurnFanOffPlan>(JACK_CALL_SITE_INPUT static_cast<const TurnFanOffPlan*>(this));

    aos::jack::BeliefQuery query([result](const aos::jack::BeliefContext& context) {
        return result->pre(context);
    });
    result->setPrecondition(query);

    return result;
}
