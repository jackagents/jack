#include <simple/meta/services/fanservicemeta.h>

/// Project
#include <simple/impl/services/fanserviceimpl.h>
#include <simple/meta/messages/fancontrolmeta.h>
#include <simple/meta/messages/fanstatusmeta.h>

/// JACK
#include <jack/corelib.h>
#include <jack/engine.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
FanServiceMeta::FanServiceMeta(aos::jack::Engine& bdi, std::string_view name)
: aos::jack::Service(bdi, name)
{
    setupHandlers();
}

FanServiceMeta::FanServiceMeta(const FanServiceMeta* other, std::string_view name)
: aos::jack::Service(other, name)
{
    setupHandlers();
}

/******************************************************************************
 * Action Handlers
 ******************************************************************************/
aos::jack::Service* FanServiceMeta::clone(std::string_view name) const
{
    return JACK_NEW(FanService, static_cast<const FanService*>(this), name);
}

void FanServiceMeta::setupHandlers()
{
    addActionHandler("simple.Turn Fan On", [](aos::jack::Service& service, [[maybe_unused]] aos::jack::Message& in, [[maybe_unused]] aos::jack::Message& out, aos::jack::ActionHandle handle)
    {
        const FanControl& request = static_cast<FanControl&>(in);
        aos::jack::Event::Status result = static_cast<FanServiceMeta&>(service).onTurnFanOn(request, handle);
        return result;
    });
    addActionHandler("simple.Turn Fan Off", [](aos::jack::Service& service, [[maybe_unused]] aos::jack::Message& in, [[maybe_unused]] aos::jack::Message& out, aos::jack::ActionHandle handle)
    {
        const FanControl& request = static_cast<FanControl&>(in);
        aos::jack::Event::Status result = static_cast<FanServiceMeta&>(service).onTurnFanOff(request, handle);
        return result;
    });
}

/**************************************************************************
 * Static Functions
 **************************************************************************/
aos::jack::ServiceHandle FanServiceMeta::create(aos::jack::Engine& bdi, std::string_view name, bool proxy, const aos::jack::UniqueId& uuid)
{
    auto result = bdi.createService(MODEL_NAME, name, proxy, uuid);
    JACK_ASSERT(result.valid());
    return result;
}

FanService* FanServiceMeta::createInstance(aos::jack::Engine& bdi, std::string_view name, bool proxy, const aos::jack::UniqueId& uuid)
{
    auto* result = dynamic_cast<FanService*>(bdi.createServiceInstance(MODEL_NAME, name, proxy, uuid));
    JACK_ASSERT(result);
    return result;
}
