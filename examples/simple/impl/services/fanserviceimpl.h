#ifndef SIMPLE_FAN_SERVICE_IMPL_H
#define SIMPLE_FAN_SERVICE_IMPL_H

#include <simple/meta/services/fanservicemeta.h>


/******************************************************************************
 * \class  FanService
 * \author jackmake
 ******************************************************************************/
class FanService : public FanServiceMeta
{
public:
    /**************************************************************************
     * Constructor/Destructors
     **************************************************************************/
    FanService(aos::jack::Engine& bdi, std::string_view name);
    FanService(const FanService* other, std::string_view name);
    ~FanService() override {}

    /**************************************************************************
     * Action Handlers
     **************************************************************************/
    aos::jack::Event::Status onTurnFanOn(const FanControl& request, aos::jack::ActionHandle handle) override;
    aos::jack::Event::Status onTurnFanOff(const FanControl& request, aos::jack::ActionHandle handle) override;

    //void subscribe(entityx::Entity entity) {}

    /**************************************************************************
     * Accessors
     **************************************************************************/
    bool isFanOn() const { return m_fanOn; }

private:
    bool m_fanOn = false;
};


#endif /// SIMPLE_FAN_SERVICE_IMPL_H