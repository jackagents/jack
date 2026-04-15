#include <simple/impl/plans/turnfanoffplanimpl.h>

#include <simple/meta/messages/temperaturereadingmeta.h>

/// JACK
#include <jack/beliefcontext.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
TurnFanOffPlan::TurnFanOffPlan(std::string_view name)
: TurnFanOffPlanMeta(name)
{
}

TurnFanOffPlan::TurnFanOffPlan(const TurnFanOffPlan* other)
: TurnFanOffPlanMeta(other)
{
}

/******************************************************************************
 * Functions
 ******************************************************************************/

bool TurnFanOffPlan::pre([[ maybe_unused ]] const aos::jack::BeliefContext& context)
{
    const auto tempReading = context.getMessageAsPtr<const TemperatureReading>();
    if (!tempReading) {
        return false;
    }
    return tempReading->temp <= 23.0f;
}
