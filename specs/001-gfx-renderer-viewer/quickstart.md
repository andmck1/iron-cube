# specs/001-gfx-renderer-viewer/quickstart.md

# Quick Start Guide for the GFX Renderer Viewer

This guide assumes you have successfully built the core renderer engine and are integrating a new feature module (a Plugin).

## Prerequisites
1.  **Core Libraries**: Must link against `vulkan_renderer` (core library), `central_manager` (state management), and `rpc_bus` (communication layer).
2.  **API Access**: The plugin must implement the `IRenderConfigurator` interface defined in the core contracts.

## Step 1: Define Plugin Goal & Scope
Determine what functionality your module adds or modifies (e.g., "Adding a particle emitter visualizer").

## Step 2: Implement Core Contract Adherence
1.  Implement the `IRenderConfigurator` interface (`getPluginID`, `getVersion`, etc.).
2.  Use the plugin's unique ID when registering itself with the Central Core Manager during its initialization phase.

## Step 3: Establish Communication Flows
Determine if your module needs to read settings (use `CentralCoreManager::getParameters()`) or write settings/send commands (use `RpcBus::sendMessage(CONFIG_UPDATE, ...)`). All communication MUST flow through the RPC layer defined in our contracts.

## Step 4: Integrate Rendering Logic
1.  Initialize Vulkan resources specific to your feature within the main render loop callback provided by the core renderer.
2.  Use the `Render Target` entity coordinates and input parameters passed via the Message Bus payload.

**Remember**: Every interaction, whether reading a setting or drawing geometry, must be treated as a contract call governed by our defined interfaces (IRenderConfigurator and RendererMessageBus).