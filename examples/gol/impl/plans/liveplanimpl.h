#ifndef GOL_LIVE_PLAN_IMPL_H
#define GOL_LIVE_PLAN_IMPL_H

#include <gol/meta/plans/liveplanmeta.h>

namespace aos::jack
{
class BeliefContext;
}


/******************************************************************************
 * \class  LivePlan
 * \author jackmake
 ******************************************************************************/
class LivePlan : public LivePlanMeta
{
public:
    /**************************************************************************
     * Constructor/Destructors
     **************************************************************************/
    LivePlan() = default;
    LivePlan(std::string_view name);
    LivePlan(const LivePlan* other);

    /**************************************************************************
     * Functions
     **************************************************************************/
    bool pre(const aos::jack::BeliefContext& context) override;
};

#endif /// GOL_LIVE_PLAN_IMPL_H