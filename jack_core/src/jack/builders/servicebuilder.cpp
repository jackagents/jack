#include <jack/builders/servicebuilder.h>
#include <jack/engine.h>

namespace aos::jack
{
ServiceBuilder::ServiceBuilder(Engine& engine, std::string_view name)
: Builder(engine, name)
{
}

ServiceBuilder::ServiceBuilder(Engine& engine, const Service& service)
: Builder(engine, service.name())
, m_messages(service.beliefSetNames())
, m_actionHandlers(service.actionHandlers())
{
}

ServiceBuilder& ServiceBuilder::handleAction(const std::string& action, Service::ActionHandlerFunc func)
{
    m_actionHandlers[action] = func;
    return *this;
}

/**************************************************************************
 * Beliefs Functions
 **************************************************************************/

ServiceBuilder& ServiceBuilder::messageNames(const Span<std::string_view> items)
{
    m_messages.insert(m_messages.end(), items.begin(), items.end());
    return *this;
}

/******************************************************************************
 * Create Functions
 ******************************************************************************/
Service* ServiceBuilder::createInstance(const std::string& name, bool proxy, const UniqueId& uuid)
{
    Service* result = m_engine.createServiceInstance(m_name, name, proxy, uuid);
    if (!result) {
        JACK_WARNING("Service builder created service before the template was "
                     "committed. Commit the builder by calling commit() before "
                     "calling create(). [template={}, service={}]",
                     m_name, name);
        JACK_INVALID_CODE_PATH;
    }
    return result;
}

ServiceHandle ServiceBuilder::create(const std::string& name, bool proxy, const UniqueId& uuid)
{
    Service*      service = createInstance(name, proxy, uuid);
    ServiceHandle result  = {};
    if (service) {
        result = service->handle();
    }
    return result;
}

void ServiceBuilder::commitInternal(Service* service)
{
    service->m_actionHandlers = m_actionHandlers;
    service->m_beliefsetIds   = m_messages;
    m_engine.commitService(service);
}
} // namespace aos::jack
