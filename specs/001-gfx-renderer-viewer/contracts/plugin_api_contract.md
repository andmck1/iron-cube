# /contracts/plugin_api_contract.md

**Contract Name**: IRenderConfigurator
**Purpose**: Defines the public interface for all third-party plugins to interact with and extend the core renderer's state (e.g., debug settings).
**Versioning**: v1.0.0 (Future updates MUST increment this version.)

## Core Interface Definition

```cpp
// C++20 Interface
class IRenderConfigurator {
public:
    virtual ~IRenderConfigurator() = default;

    // Mandatory method for plugin identification and contract validation
    virtual std::string getPluginID() const = 0;
    virtual int getVersion() const = 0;

    // Called once during initialization to validate required resources.
    virtual bool initialize(CentralCoreManager& core_manager) = 0;

    // Method for plugin-specific configuration update (e.g., setting a custom debug value).
    // The Core Manager handles persistence and thread safety.
    virtual std::optional<std::string> setParameter(const std::string& key, const std::variant<float, bool>& value) = 0;

    // Method to retrieve the current state of the component's controlled parameters.
    virtual std::map<std::string, std::variant<float, bool>> getParameters() const = 0;
};
```
**Usage**: All plugins MUST implement this interface and register an instance with the Central Core Manager during application startup.

# /contracts/communication_protocol_contract.md

**Contract Name**: RendererMessageBus (RPC)
**Purpose**: Standardized, asynchronous protocol for communication between disparate modules (Plugins, App Shell, Core). This replaces direct function calls or shared memory access for high modularity.
**Versioning**: v1.0.0

## Message Structure
All messages MUST follow this schema to ensure routing and validation:

| Field | Type | Description | Notes |
| :--- | :--- | :--- | :--- |
| `sender_id` | String | Unique ID of the originating module/plugin (e.g., "PluginA", "Core"). | Required for auditability. |
| `message_type` | Enum | Defines the purpose of the message (e.g., `CONFIG_UPDATE`, `RENDER_REQUEST`, `STAT_QUERY`). | Must be explicitly defined. |
| `payload` | JSON object | The actual data payload, schema-validated against the specified `message_type`. | Data must not exceed 1MB. |

## Key Message Types (Examples)
*   **CONFIG\_UPDATE**: Used by plugins to propose changes to debug settings. Payload contains `{key: string, value: any}`.
*   **RENDER\_REQUEST**: Used by the App Shell to signal a mandatory frame update or resource loading event. Payload may include `target_resolution` and `update_flags`.

**Implementation Note**: Use an established RPC framework (gRPC is recommended) that handles serialization/deserialization and transport reliability automatically.