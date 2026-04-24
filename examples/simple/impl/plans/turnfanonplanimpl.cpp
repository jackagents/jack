#include <simple/impl/plans/turnfanonplanimpl.h>

#include <simple/meta/messages/temperaturereadingmeta.h>

/// JACK
#include <jack/beliefcontext.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
TurnFanOnPlan::TurnFanOnPlan(std::string_view name)
: TurnFanOnPlanMeta(name)
{
}

TurnFanOnPlan::TurnFanOnPlan(const TurnFanOnPlan* other)
: TurnFanOnPlanMeta(other)
{
}

/******************************************************************************
 * Functions
 ******************************************************************************/

bool TurnFanOnPlan::pre([[ maybe_unused ]] const aos::jack::BeliefContext& context)
{
    const auto tempReading = context.getMessageAsPtr<const TemperatureReading>();
    if (!tempReading) {
        return false;
    }
    return tempReading->temp >= 25.0f;
}
