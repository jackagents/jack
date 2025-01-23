#include "bitmaskops.h"

#include <jack/jack.h>
#include <gtest/gtest.h>

#include <vector>
#include <thread>
#include <chrono>
#include <cmath>

using namespace aos;

/*! ***********************************************************************************************
 * \class   FanServiceMeta
 * \author  jackmake
 * ************************************************************************************************/
template<typename ImplType>
class FanServiceMeta : public jack::Service
{
public:
    /* ****************************************************************************************
     * Public Ctor & Dtor
     * ****************************************************************************************/
    FanServiceMeta(jack::Engine& bdi, std::string_view name)
    : jack::Service(bdi, name)
    {
        setupHandlers();
    }

    FanServiceMeta(const ImplType* other, std::string_view name)
    : jack::Service(other, name)
    {
        setupHandlers();
    }

    FanServiceMeta(const FanServiceMeta &other) = delete;

    virtual ~FanServiceMeta() { }

    /* ****************************************************************************************
     * Operators Override
     * ****************************************************************************************/
    FanServiceMeta& operator=(const FanServiceMeta &other) = delete;

    /* ****************************************************************************************
     * Public Accessors
     * ****************************************************************************************/
    //! Maps entity name for the logger
    static std::string objectName() { static std::string const &name = "MyService"; return name; }

    /* ****************************************************************************************
     * Action handlers
     * ****************************************************************************************/
    virtual jack::Event::Status onToggleOn(jack::Message &msg, jack::Message &out, jack::ActionHandle handle) = 0;
    virtual jack::Event::Status onToggleOff(jack::Message &msg, jack::Message &out, jack::ActionHandle handle) = 0;

    /* ****************************************************************************************
     * Public Functions
     * ****************************************************************************************/
    void setupHandlers()
    {
        addActionHandler("ToggleOn", [](jack::Service &service, jack::Message &msg, jack::Message &out, jack::ActionHandle handle) {
            return static_cast<FanServiceMeta&>(service).onToggleOn(msg, out, handle);
        });
        addActionHandler("ToggleOff", [](jack::Service &service, jack::Message &msg, jack::Message &out, jack::ActionHandle handle) {
            return static_cast<FanServiceMeta&>(service).onToggleOff(msg, out, handle);
        });
    }

    /* ****************************************************************************************
     * Public Static Functions
     * ****************************************************************************************/
    static jack::ServiceBuilder init(jack::Engine &bdi)
    {
        /// \todo where are the actions defined? in the agent?
        /// Configure the service
        return bdi.service("FanServiceTemplate")
                  .messageName("Fan")
                  .commit<ImplType>();
    }

protected:

    /* ****************************************************************************************
     * Protected Functions
     * ****************************************************************************************/

    jack::Service* clone(std::string_view name) const override {
        return JACK_NEW(ImplType, static_cast<const ImplType*>(this), name);
    }
};

/*! ***********************************************************************************************
 * \class   FanService
 * \author  jackmake
 *
 * The implementaion of the Fan Service
 * ************************************************************************************************/
class FanService : public FanServiceMeta<FanService>
{
public:
    /* ****************************************************************************************
     * Public Ctor & Dtor
     * ****************************************************************************************/
    FanService(jack::Engine &bdi, std::string_view name);
    FanService(const FanService* other, std::string_view name);
    ~FanService() override;

    /* ****************************************************************************************
     * BDI Action handlers
     * ****************************************************************************************/
    jack::Event::Status onToggleOn(jack::Message &msg, jack::Message &out, jack::ActionHandle handle) override final;
    jack::Event::Status onToggleOff(jack::Message &msg, jack::Message &out, jack::ActionHandle handle) override final;

    bool isFanOn() const { return m_fanOn; }

protected:

    // private state of the service
    bool m_fanOn = false;
};

FanService::FanService(jack::Engine &bdi, std::string_view name)
: FanServiceMeta<FanService>(bdi, name) {
}

FanService::FanService(const FanService* other, std::string_view name)
: FanServiceMeta<FanService>(other, name) {
}

FanService::~FanService() { }

jack::Event::Status FanService::onToggleOn(jack::Message& msg, jack::Message &out, jack::ActionHandle handle) {

    /*! ********************************************************************************
     * Switch the fan on
     * ********************************************************************************/

    m_fanOn = true;
    std::cout << "Switching fan on" << std::endl;

    // send a percept to switch the fan on
    percept("Fan", "fan", true);

    /// \todo ?? add code gen setter? - setFan(true)

    return jack::Event::Status::SUCCESS;
}

jack::Event::Status FanService::onToggleOff(jack::Message &msg, jack::Message &out, jack::ActionHandle handle) {

    /*! ********************************************************************************
     * Switch the fan on
     * ********************************************************************************/

    m_fanOn = false;
    std::cout << "Switching fan off" << std::endl;

    // send a percept to switch the fan off
    percept("Fan", "fan", false);

    /// \todo ?? add code gen setter? - setFan(false)

    return jack::Event::Status::SUCCESS;
}

/*! ***********************************************************************************************
 * \class   ServicesTest
 *
 * This google test fixture aims to provide a context for Service specific testing
 * \todo See "X.X.X Services" section of Use Cases & Requirements s
 * ************************************************************************************************/
class ServicesTest : public ::testing::Test {
    protected:

    ServicesTest()
        : temperatureBS(bdi, "Temperature")
        , fanBS(bdi, "Fan")
        , fanServiceBuilder(FanService::init(bdi))
        , temperatureServiceBuilder(bdi, "TempService")
        , currentTemp(15.0f)
    {
        bdi.exitWhenDone();
    }

    void SetUp() override
    {
        // A Beliefset for the TemperatureSensor
        temperatureBS
            .fieldWithValue<float>("temp", 0.0f)
            .commit();

        // a bs for the fan
        fanBS
            .fieldWithValue<bool>("fan", false)
            .commit();

        // A Temperature Sensor
        temperatureServiceBuilder
            .message(temperatureBS)
            .commit();

        // action messages
        bdi.action("ToggleOn").commit();
        bdi.action("ToggleOff").commit();
    }

    // Objects declared here can be used by all tests in the test case.
    jack::Engine bdi;

    jack::MessageBuilder temperatureBS;
    jack::MessageBuilder fanBS;
    jack::ServiceBuilder fanServiceBuilder;
    jack::ServiceBuilder temperatureServiceBuilder;

    //bool fanOn;
    float currentTemp;
};

// a simple test for services
TEST_F(ServicesTest, FanTest)
{
    float minTemp = 15.0f, maxTemp = 15.0f;

    // a goal that will attempt to reduce the temperature if it is too hot
    auto maintainCoolGoal = bdi.goal("MaintainCoolGoal")
                                .pre([&](const jack::BeliefContext& context) {
                                    auto bs   = context.message("Temperature");
                                    auto temp = *bs->getPtr<float>("temp");
                                    // too far from the ideal temperature 18
                                    return (temp >= 19 || temp <= 17);
                                })
                                .commit();

    // plan to switch the fan on
    auto coolPlan = bdi.plan("CoolPlan")
                        .pre([&](const jack::BeliefContext& context) {
                            auto bs   = context.message("Temperature");
                            auto temp = *bs->getPtr<float>("temp");
                            return (temp >= 19);
                        })
                        .handles(maintainCoolGoal)
                        .body(bdi.coroutine()
                                  .print("starting cool plan\n")
                                  .action("ToggleOn")
                                  .print("ending cool plan\n"))
                        .commit();

    auto unCoolPlan = bdi.plan("UnCoolPlan")
                          .pre([&](const jack::BeliefContext& context) {
                              auto bs   = context.message("Temperature");
                              auto temp = *bs->getPtr<float>("temp");
                              return (temp <= 18);
                          })
                          .handles(maintainCoolGoal)
                          .body(bdi.coroutine()
                                    .print("starting uncool plan\n")
                                    .action("ToggleOff")
                                    .print("ending uncool plan\n"))
                          .commit();

    // plan to do a task
    // This plan will drop if the battery level drops below 25%
    // or all 10 tasks are completed
    // uses a recursion for iteration

    auto* tempService = temperatureServiceBuilder.createInstance("temps1", false /*proxy*/);
    auto* fanService  = dynamic_cast<FanService*>(fanServiceBuilder.createInstance("fan1", false /*proxy*/));
    JACK_ASSERT(tempService);
    JACK_ASSERT(fanService);

    tempService->start();
    fanService->start();
    std::cout << "starting services" << std::endl;

    jack::Agent* agent = bdi.agent("TestAgent")
                            .beliefs(std::array{fanBS, temperatureBS})
                            .services(std::array{fanServiceBuilder, temperatureServiceBuilder})
                            .plans(std::array{coolPlan, unCoolPlan})
                            .commitAsAgent()
                            .createAgentInstance("myagent");
    agent->attachService(fanService->handle(), false /*force*/);
    agent->attachService(tempService->handle(), false /*force*/);
    agent->start();

    // this agent will have a desire to keep cool
    agent->pursue(maintainCoolGoal, jack::GoalPersistent_Yes);

    // loop for a while
    for(int i=0; i < 1000; ++i) {
        bdi.poll();

        if (fanService->isFanOn()) {
            // cool
            currentTemp -= 0.1f;
        } else {
            // warm
            currentTemp += 0.1f;
        }

        // update the services
        tempService->percept("Temperature", "temp", currentTemp);
        std::cout << "temp = " << currentTemp << std::endl;

        // record min, max
        minTemp = std::min(minTemp, currentTemp);
        maxTemp = std::max(maxTemp, currentTemp);
    }

   EXPECT_GE(minTemp, 15.0f);
   EXPECT_LE(maxTemp, 20.0f);
}
