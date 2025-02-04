#ifndef TESTS_PROJECT_META_H
#define TESTS_PROJECT_META_H

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

/******************************************************************************
 * \class  tests
 * \author jackmake
 ******************************************************************************/
class tests : public aos::jack::Engine
{
public:
    /**************************************************************************
     * Constructor/Destructors
     **************************************************************************/
    tests();
    virtual ~tests();

    /**************************************************************************
     * Functions
     **************************************************************************/

    /**************************************************************************
     * Static Functions
     **************************************************************************/
    #if defined(JACK_WITH_SIM)
    static void initSimModel(aos::sim::SimulationBase* sim);
    static bool addComponentToEntity(aos::jack::Engine& engine, aos::sim::EntityWrapper entity, std::string_view componentName, const aos::sim::JsonParsedComponent *config);
    #endif /// defined(JACK_WITH_SIM)

    /// The name of the class
    static constexpr inline std::string_view CLASS_NAME = "tests";
};

#endif /// TESTS_PROJECT_META_H