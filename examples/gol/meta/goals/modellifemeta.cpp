#include <gol/meta/goals/modellifemeta.h>

/// Project
#include <gol/impl/goals/modellifeimpl.h>

/// JACK
#include <jack/beliefquery.h>
#include <jack/beliefcontext.h>
#include <jack/corelib.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
ModelLifeMeta::ModelLifeMeta()
: aos::jack::Goal("gol.Model Life")
{
}

ModelLifeMeta::ModelLifeMeta(std::string_view name)
: aos::jack::Goal(name)
{
}

ModelLifeMeta::ModelLifeMeta(const ModelLifeMeta *other)
: aos::jack::Goal(other)
{
}

/******************************************************************************
 * Functions
 ******************************************************************************/
aos::jack::Goal* ModelLifeMeta::clone(JACK_CALL_SITE_ARGS_NO_TAIL_COMMA) const
{
    /// Ensures all the functions are copied over to the cloned goal
    ModelLife* result = aos::jack::globalHeapAllocator.newInternal<ModelLife>(JACK_CALL_SITE_INPUT static_cast<const ModelLife*>(this));

    return result;
}

/******************************************************************************
 * Static Functions
 ******************************************************************************/
