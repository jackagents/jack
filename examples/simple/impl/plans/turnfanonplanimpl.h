#ifndef SIMPLE_TURN_FAN_ON_PLAN_IMPL_H
#define SIMPLE_TURN_FAN_ON_PLAN_IMPL_H

#include <simple/meta/plans/turnfanonplanmeta.h>

namespace aos::jack
{
class BeliefContext;
}


/******************************************************************************
 * \class  TurnFanOnPlan
 * \author jackmake
 ******************************************************************************/
class TurnFanOnPlan : public TurnFanOnPlanMeta
{
public:
    /**************************************************************************
     * Constructor/Destructors
     **************************************************************************/
    TurnFanOnPlan() = default;
    TurnFanOnPlan(std::string_view name);
    TurnFanOnPlan(const TurnFanOnPlan* other);

    /**************************************************************************
     * Functions
     **************************************************************************/
    bool pre(const aos::jack::BeliefContext& context) override;
};

#endif /// SIMPLE_TURN_FAN_ON_PLAN_IMPL_H