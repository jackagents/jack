#include <simple/impl/services/fanserviceimpl.h>

#include <iostream>

/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
FanService::FanService(aos::jack::Engine& bdi, std::string_view name)
: FanServiceMeta(bdi, name), m_fanOn(false) {
}

FanService::FanService(const FanService* other, std::string_view name)
: FanServiceMeta(other, name), m_fanOn(other->m_fanOn) {
}

/******************************************************************************
 * Action Handlers
 ******************************************************************************/

aos::jack::Event::Status FanService::onTurnFanOn(const FanControl& request, aos::jack::ActionHandle handle)
{
    if (!m_fanOn) {
        m_fanOn = true;
        std::cout << "[FanService] Turning fan ON" << std::endl;
    }
    return aos::jack::Event::Status::SUCCESS;
}


aos::jack::Event::Status FanService::onTurnFanOff(const FanControl& request, aos::jack::ActionHandle handle)
{
    if (m_fanOn) {
        m_fanOn = false;
        std::cout << "[FanService] Turning fan OFF" << std::endl;
    }
    return aos::jack::Event::Status::SUCCESS;
}
