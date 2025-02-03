// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#ifndef JACK_SERVICE_BUILDER_H
#define JACK_SERVICE_BUILDER_H

#include <jack/builders/builder.h>  // for Builder
#include <jack/service.h>           // for Service

#include <functional>
#include <vector>
#include <string>
#include <string_view>

namespace aos::jack
{
class Engine;

/*******************************************************************************
 * @brief A builder pattern object used to configure a service template,
 * submitting it to the engine and creating a runtime instance from it.
 *
 * @tparam T The Service class or a custom class that derives from Service.
 *
 * A service builder is requested from the engine by calling the service member
 * function with the desired name for the template.
 *
 * @code
 * 
 * 
 * bdi.service("Temperature Service Template");
 * @endcode
 *
 * Once constructed the builder can be used to configure the Service template
 * 
 * by chaining calls on the returned object.
 *
 * @code
 * bdi.service("Service")
 *    .beliefs(beliefs)
 *    .handleAction("PerformAction", actionHandler)
 *    .commit();
 * @endcode
 *
 * @see Engine
 ******************************************************************************/
class ServiceBuilder : public Builder
{
public:
    /**************************************************************************
     * Constructor & Destructor
     **************************************************************************/
    /// Initialise a ServiceBuilder and the name of the Service template.
    /// @param[in] engine The BDI instance to commit the template to.
    /// @param[in] name The name of this Service template.
    ServiceBuilder(Engine& engine, std::string_view name);

    /// Constructs a ServiceBuilder for an existing Service.
    /// @param[in] engine The JACK engine instance.
    /// @param[in] service The name of the Service.
    /// \todo add builder unit test to make sure we can re-confige an Service
    /// template.
    ServiceBuilder(Engine& engine, const Service& service);

    /// Add a callback that is triggered when the Service receives a request to
    /// execute the action.
    /// @param[in] action The name of the action that will be handled.
    /// @param[in] func The function that will be invoked when the action is
    /// triggered on the Service.
    /// @return This ServiceBuilder.
    ServiceBuilder& handleAction(const std::string& action, Service::ActionHandlerFunc func);

    /**************************************************************************
     * Beliefs Functions
     **************************************************************************/
    /// Add an array of message names to this service
    ServiceBuilder& messageNames(const Span<std::string_view> items);

    /// Add a message by name to this service
    ServiceBuilder& messageName(std::string_view item) { return messageNames(Span<std::string_view>(&item, 1)); }

    /**************************************************************************
     * Create Functions
     **************************************************************************/
    /// Create an instance of this Service. The service must be committed before
    /// creation.
    /// @param[in] name The name of the new Service.
    /// @param[in] proxy Set the instance to be a proxy Service.
    /// @param[in] uuid The UniqueId to assign to the new Service.
    /// @return A pointer to the new created Service. Null pointer if creation
    /// failed.
    Service* createInstance(const std::string& name, bool proxy, const UniqueId& uuid = UniqueId::random());

    /// Create an instance of this Service. The service must be committed before
    /// creation.
    /// @param[in] name The name of the new Service.
    /// @param[in] proxy Set the instance to be a proxy Service.
    /// @param[in] uuid The UniqueId to assign to the new Service.
    /// @return The handle to the newly created Service. Invalid handle if
    /// creation failed.
    ServiceHandle create(const std::string& name, bool proxy, const UniqueId& uuid = UniqueId::random());

    /// Commit the Service template into the engine's BDI model. Once commited
    /// this template can be used to create a runtime Service instance.
    /// @return The ServiceBuilder.
    template <typename ServiceT = Service>
    ServiceBuilder& commit()
    {
        ServiceT service(m_engine, m_name);
        commitInternal(&service);
        return *this;
    }

private:
    /// Assign the properties to the service passed in and commit the service to
    /// the engine.
    void commitInternal(Service *service);

protected:
    /**************************************************************************
     * Fields
     **************************************************************************/
    std::vector<std::string>    m_messages;        ///< The messages supported by this service
    Service::ActionHandlerTable m_actionHandlers; ///< The actions the service can handle
};
} // namespace aos::jack
#endif
