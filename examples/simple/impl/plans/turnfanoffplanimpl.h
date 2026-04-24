#ifndef SIMPLE_TURN_FAN_OFF_PLAN_IMPL_H
#define SIMPLE_TURN_FAN_OFF_PLAN_IMPL_H

#include <simple/meta/plans/turnfanoffplanmeta.h>

namespace aos::jack
{
class BeliefContext;
}


/******************************************************************************
 * \class  TurnFanOffPlan
 * \author jackmake
 ******************************************************************************/
class TurnFanOffPlan : public TurnFanOffPlanMeta
{
public:
    /**************************************************************************
     * Constructor/Destructors
     **************************************************************************/
    TurnFanOffPlan() = default;
    TurnFanOffPlan(std::string_view name);
    TurnFanOffPlan(const TurnFanOffPlan* other);

    /**************************************************************************
     * Functions
     **************************************************************************/
    bool pre(const aos::jack::BeliefContext& context) override;
};

#endif /// SIMPLE_TURN_FAN_OFF_PLAN_IMPL_H