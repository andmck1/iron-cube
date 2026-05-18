// src/util/serializer.cpp
#include "serializer.hpp"
#include <nlohmann/json.hpp> // Assuming external JSON library

using json = nlohmann::json;

/**
 * @brief Serializes a map of arbitrary data into a validated JSON string.
 * @param data The key-value pairs to serialize.
 * @return A standardized JSON string representation.
 */
std::string Serializer::serialize(const std::map<std::string, std::any> &data)
{
    json j = json::object();
    for (const auto &pair : data)
    {
        // NOTE: This requires proper type checking/conversion for 'std::any' in
        // a real scenario.
        if (pair.second.type() == typeid(float))
        {
            j[pair.first] = std::any_cast<float>(pair.second);
        }
        else if (pair.second.type() == typeid(bool))
        {
            j[pair.first] = std::any_cast<bool>(pair.second);
        }
        // Add handlers for other types (int, string) as needed
    }
    return j.dump(4); // Dump with 4 spaces indentation for readability
}

/**
 * @brief Deserializes a JSON string into the expected internal map structure.
 * @param json_str The incoming JSON payload.
 * @return A populated map of data. Throws exception on failure.
 */
std::map<std::string, std::any>
Serializer::deserialize(const std::string &json_str)
{
    try
    {
        json j = json::parse(json_str);
        std::map<std::string, std::any> data;
        for (auto it = j.begin(); it != j.end(); ++it)
        {
            // Basic type inference needed here: e.g., if value is a number,
            // cast to float/double.
            if (j.is_number())
            {
                data[it.key()] =
                    j.get<float>(); // Assuming most configs are floats
            }
            else if (j.is_boolean())
            {
                data[it.key()] = j.get<bool>();
            }
            // ... handle other types
        }
        return data;
    }
    catch (const nlohmann::json::exception &e)
    {
        throw std::runtime_error(
            "JSON Deserialization Error: " + std::string(e.what())
        );
    }
}