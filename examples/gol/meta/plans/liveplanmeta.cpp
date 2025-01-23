#include <gol/meta/plans/liveplanmeta.h>

/// Project
#include <gol/impl/plans/liveplanimpl.h>

/// JACK
#include <jack/corelib.h>
#include <jack/beliefquery.h>
#include <jack/beliefcontext.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
LivePlanMeta::LivePlanMeta()
: aos::jack::Plan("gol.Live Plan")
{
}

LivePlanMeta::LivePlanMeta(std::string_view name)
: aos::jack::Plan(name)
{
}

LivePlanMeta::LivePlanMeta(const LivePlanMeta *other)
: aos::jack::Plan(other)
{
}

/******************************************************************************
 * Functions
 ******************************************************************************/
aos::jack::Plan* LivePlanMeta::clone(JACK_CALL_SITE_ARGS_NO_TAIL_COMMA) const
{
    LivePlan* result = aos::jack::globalHeapAllocator.newInternal<LivePlan>(JACK_CALL_SITE_INPUT static_cast<const LivePlan*>(this));

    aos::jack::BeliefQuery query([result](const aos::jack::BeliefContext& context) {
        return result->pre(context);
    });
    result->setPrecondition(query);

    return result;
}
