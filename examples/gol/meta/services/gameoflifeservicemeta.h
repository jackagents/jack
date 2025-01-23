#ifndef GOL_GAME_OF_LIFE_SERVICE_META_H
#define GOL_GAME_OF_LIFE_SERVICE_META_H

/// JACK
#include <jack/service.h>

/// Third Party
#include <string_view>

namespace aos::jack { class Engine; }


class CellCommand;
class CellInfo;


class GameOfLifeService;
/******************************************************************************
 * \class  GameOfLifeServiceMeta
 * \author jackmake
 * \source gol.jack
 ******************************************************************************/
class GameOfLifeServiceMeta : public aos::jack::Service
{
public:
    /**************************************************************************
     * Constructor/Destructors
     **************************************************************************/
    GameOfLifeServiceMeta(aos::jack::Engine& bdi, std::string_view name);
    GameOfLifeServiceMeta(const GameOfLifeServiceMeta* other, std::string_view name);
    virtual ~GameOfLifeServiceMeta() { }

    GameOfLifeServiceMeta(const GameOfLifeServiceMeta& other) = delete;
    GameOfLifeServiceMeta& operator=(const GameOfLifeServiceMeta& other) = delete;

    /**************************************************************************
     * Functions
     **************************************************************************/
    aos::jack::Service* clone(std::string_view name) const override;

    /// \note Actions
    virtual aos::jack::Event::Status onLiveAction(const CellCommand& request, aos::jack::ActionHandle handle) = 0;
    virtual aos::jack::Event::Status onDieAction(const CellCommand& request, aos::jack::ActionHandle handle) = 0;

protected:
    /// Link the action handler to this service's derived action handler.
    void setupHandlers();

public:
    /**************************************************************************
     * Static Functions
     **************************************************************************/
    /// Create an instance of the service and return the service handle
    /// @param proxy Create a proxy service that will forward events onto the bus to the real instance
    static aos::jack::ServiceHandle create(aos::jack::Engine& bdi, std::string_view name, bool proxy, const aos::jack::UniqueId& uuid = aos::jack::UniqueId::random());

    /// Create an instance of the service and return a pointer to the service
    /// @param proxy Create a proxy service that will forward events onto the bus to the real instance
    static GameOfLifeService* createInstance(aos::jack::Engine& bdi, std::string_view name, bool proxy, const aos::jack::UniqueId& uuid = aos::jack::UniqueId::random());

    /**************************************************************************
     * Static Fields
     **************************************************************************/
    /// The name of the C++ class
    static constexpr inline std::string_view CLASS_NAME = "GameOfLifeService";

    /// The name of this object in the JACK model
    static constexpr inline std::string_view MODEL_NAME = "gol.Game Of Life Service Template";

    /// \todo: This should use the qualified name so that the editor can
    /// use nice names. But its been @harcoded to the bumpy case.
    /// The name of this object in the simulator's ECS
    static constexpr inline std::string_view COMPONENT_NAME = "gol.Game Of Life Service Component";
};

#endif /// GOL_GAME_OF_LIFE_SERVICE_META_H