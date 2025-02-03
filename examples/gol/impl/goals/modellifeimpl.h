#ifndef GOL_MODEL_LIFE_IMPL_H
#define GOL_MODEL_LIFE_IMPL_H

// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <gol/meta/goals/modellifemeta.h>


/******************************************************************************
 * \class  ModelLife
 * \author jackmake
 ******************************************************************************/
class ModelLife : public ModelLifeMeta
{
public:
    /**************************************************************************
     * Constructor/Destructors
     **************************************************************************/
    ModelLife() = default;
    ModelLife(std::string_view name);
    ModelLife(const ModelLife* other);
};

#endif /// GOL_MODEL_LIFE_IMPL_H