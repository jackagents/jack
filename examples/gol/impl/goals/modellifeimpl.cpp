// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <gol/impl/goals/modellifeimpl.h>

/// JACK
#include <jack/beliefcontext.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
ModelLife::ModelLife(std::string_view name)
: ModelLifeMeta(name)
{
}

ModelLife::ModelLife(const ModelLife* other)
: ModelLifeMeta(other)
{
}
