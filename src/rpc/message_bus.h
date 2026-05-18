// src/rpc/message_bus.h
#pragma once

#include <string>
#include <map>
#include <iostream>

/**
 * @brief Abstract interface for the Remote Procedure Call message bus
 * (gRPC/ZeroMQ replacement). All inter-module communication MUST use this layer
 * to enforce contracts and structure.
 */
class MessageBus
{
    public:
    // Defines the mandatory message format.
    struct MessagePayload
    {
        std::string sender_id;
        std::string message_type; // e.g., "CONFIG_UPDATE", "RENDER_REQUEST"
        std::map<std::string, std::string>
            payload_data; // Simple string map for generic data passing
    };

    static MessageBus &getInstance()
    {
        static MessageBus instance;
        return instance;
    }

    // Broadcasts a message to all relevant subscribers.
    void publish(const MessagePayload &message) const
    {
        std::cout << "[RPC BUS] Publishing message: " << message.message_type
                  << " from " << message.sender_id << std::endl;
        // In reality, this sends over the network/IPC pipes.
    }

    // Client call to request a specific response asynchronously.
    std::optional<MessagePayload> subscribe(const std::string &topic) const
    {
        std::cout << "[RPC BUS] Subscribing client to topic: " << topic
                  << std::endl;
        return {}; // Mock return
    }

    private:
    MessageBus() = default;
};