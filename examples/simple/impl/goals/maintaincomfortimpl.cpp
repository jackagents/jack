#include <simple/impl/goals/maintaincomfortimpl.h>

/// JACK
#include <jack/beliefcontext.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
MaintainComfort::MaintainComfort(std::string_view name)
: MaintainComfortMeta(name)
{
}

MaintainComfort::MaintainComfort(const MaintainComfort* other)
: MaintainComfortMeta(other)
{
}
