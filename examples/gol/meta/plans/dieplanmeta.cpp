#include <gol/meta/plans/dieplanmeta.h>

/// Project
#include <gol/impl/plans/dieplanimpl.h>

/// JACK
#include <jack/corelib.h>
#include <jack/beliefquery.h>
#include <jack/beliefcontext.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
DiePlanMeta::DiePlanMeta()
: aos::jack::Plan("gol.Die Plan")
{
}

DiePlanMeta::DiePlanMeta(std::string_view name)
: aos::jack::Plan(name)
{
}

DiePlanMeta::DiePlanMeta(const DiePlanMeta *other)
: aos::jack::Plan(other)
{
}

/******************************************************************************
 * Functions
 ******************************************************************************/
aos::jack::Plan* DiePlanMeta::clone(JACK_CALL_SITE_ARGS_NO_TAIL_COMMA) const
{
    DiePlan* result = aos::jack::globalHeapAllocator.newInternal<DiePlan>(JACK_CALL_SITE_INPUT static_cast<const DiePlan*>(this));

    aos::jack::BeliefQuery query([result](const aos::jack::BeliefContext& context) {
        return result->pre(context);
    });
    result->setPrecondition(query);

    return result;
}
