#ifndef SIMPLE_MAINTAIN_COMFORT_IMPL_H
#define SIMPLE_MAINTAIN_COMFORT_IMPL_H

#include <simple/meta/goals/maintaincomfortmeta.h>


/******************************************************************************
 * \class  MaintainComfort
 * \author jackmake
 ******************************************************************************/
class MaintainComfort : public MaintainComfortMeta
{
public:
    /**************************************************************************
     * Constructor/Destructors
     **************************************************************************/
    MaintainComfort() = default;
    MaintainComfort(std::string_view name);
    MaintainComfort(const MaintainComfort* other);
};

#endif /// SIMPLE_MAINTAIN_COMFORT_IMPL_H