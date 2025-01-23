#ifndef GOL_DIE_PLAN_IMPL_H
#define GOL_DIE_PLAN_IMPL_H

#include <gol/meta/plans/dieplanmeta.h>

namespace aos::jack
{
class BeliefContext;
}


/******************************************************************************
 * \class  DiePlan
 * \author jackmake
 ******************************************************************************/
class DiePlan : public DiePlanMeta
{
public:
    /**************************************************************************
     * Constructor/Destructors
     **************************************************************************/
    DiePlan() = default;
    DiePlan(std::string_view name);
    DiePlan(const DiePlan* other);

    /**************************************************************************
     * Functions
     **************************************************************************/
    bool pre(const aos::jack::BeliefContext& context) override;
};

#endif /// GOL_DIE_PLAN_IMPL_H