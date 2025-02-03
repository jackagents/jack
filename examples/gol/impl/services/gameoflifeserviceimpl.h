#ifndef GOL_GAME_OF_LIFE_SERVICE_IMPL_H
#define GOL_GAME_OF_LIFE_SERVICE_IMPL_H

// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <gol/meta/services/gameoflifeservicemeta.h>

#include <gol/gameoflife.h>


/******************************************************************************
 * \class  GameOfLifeService
 * \author jackmake
 ******************************************************************************/
class GameOfLifeService : public GameOfLifeServiceMeta
{
public:
    /**************************************************************************
     * Constructor/Destructors
     **************************************************************************/
    GameOfLifeService(aos::jack::Engine& bdi, std::string_view name);
    GameOfLifeService(const GameOfLifeService* other, std::string_view name);
    ~GameOfLifeService() override {}

    /**************************************************************************
     * Action Handlers
     **************************************************************************/
    aos::jack::Event::Status onLiveAction(const CellCommand& request, aos::jack::ActionHandle handle) override;
    aos::jack::Event::Status onDieAction(const CellCommand& request, aos::jack::ActionHandle handle) override;

    /// @todo stop jack_make creating this
    //void subscribe(entityx::Entity entity) {}

    void setEnvironment(GameOfLife* gol) {
        m_gol = gol; 
    }

    GameOfLife* m_gol = nullptr;
};


#endif /// GOL_GAME_OF_LIFE_SERVICE_IMPL_H