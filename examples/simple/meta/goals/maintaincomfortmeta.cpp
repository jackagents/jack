#include <simple/meta/goals/maintaincomfortmeta.h>

/// Project
#include <simple/impl/goals/maintaincomfortimpl.h>

/// JACK
#include <jack/beliefquery.h>
#include <jack/beliefcontext.h>
#include <jack/corelib.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
MaintainComfortMeta::MaintainComfortMeta()
: aos::jack::Goal("simple.Maintain Comfort")
{
}

MaintainComfortMeta::MaintainComfortMeta(std::string_view name)
: aos::jack::Goal(name)
{
}

MaintainComfortMeta::MaintainComfortMeta(const MaintainComfortMeta *other)
: aos::jack::Goal(other)
{
}

/******************************************************************************
 * Functions
 ******************************************************************************/
aos::jack::Goal* MaintainComfortMeta::clone(JACK_CALL_SITE_ARGS_NO_TAIL_COMMA) const
{
    /// Ensures all the functions are copied over to the cloned goal
    MaintainComfort* result = aos::jack::globalHeapAllocator.newInternal<MaintainComfort>(JACK_CALL_SITE_INPUT static_cast<const MaintainComfort*>(this));

    return result;
}

/******************************************************************************
 * Static Functions
 ******************************************************************************/
