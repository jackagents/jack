#ifndef GOL_PROJECT_META_H
#define GOL_PROJECT_META_H

/// JACK
#include <jack/engine.h>

/// Sim
#if defined(JACK_WITH_SIM)
#include <sim/entity.h>
namespace aos::sim { class SimulationBase; class JsonParsedComponent; }
#endif


/******************************************************************************
 * Forward Declarations
 ******************************************************************************/
class GameOfLifeAgent;
class GameOfLifeService;

/******************************************************************************
 * \class  gol
 * \author jackmake
 ******************************************************************************/
class gol : public aos::jack::Engine
{
public:
    /**************************************************************************
     * Constructor/Destructors
     **************************************************************************/
    gol();
    virtual ~gol();

    /**************************************************************************
     * Functions
     **************************************************************************/
    /// Create an agent and return a handle to the instance.
    aos::jack::AgentHandle createGameOfLifeAgent(std::string_view name, const aos::jack::UniqueId& uuid = aos::jack::UniqueId::random());

    /// Create an agent and return the pointer to the instance.
    GameOfLifeAgent* createGameOfLifeAgentInstance(std::string_view name, const aos::jack::UniqueId& uuid = aos::jack::UniqueId::random());
    /// Create a service and return a handle to the instance.
    aos::jack::ServiceHandle createGameOfLifeService(std::string_view name, bool proxy, const aos::jack::UniqueId& uuid = aos::jack::UniqueId::random());

    /// Create a service and return a pointer to the instance.
    GameOfLifeService* createGameOfLifeServiceInstance(std::string_view name, bool proxy, const aos::jack::UniqueId& uuid = aos::jack::UniqueId::random());

    /**************************************************************************
     * Static Functions
     **************************************************************************/
    #if defined(JACK_WITH_SIM)
    static void initSimModel(aos::sim::SimulationBase* sim);
    static bool addComponentToEntity(aos::jack::Engine& engine, aos::sim::EntityWrapper entity, std::string_view componentName, const aos::sim::JsonParsedComponent *config);
    #endif /// defined(JACK_WITH_SIM)

    /// The name of the class
    static constexpr inline std::string_view CLASS_NAME = "gol";
};

#endif /// GOL_PROJECT_META_H