#pragma once

#include <string>
#include <vector>
#include <memory>

// ty Vulkan!
class Entity;

class Component
{
    public:
    virtual ~Component() = default;

    void setOwner(Entity *entity)
    {
        owner_ = entity;
    }
    Entity *getOwner() const
    {
        return owner_;
    }

    protected:
    Entity *owner_ = nullptr;
};

class Entity
{
    public:
    explicit Entity(const std::string &entityName) : name_(entityName)
    {
    }

    Entity(Entity &&) = default;
    Entity &operator=(Entity &&) = default;
    Entity(const Entity &) = delete;
    Entity &operator=(const Entity &) = delete;

    const std::string &getName() const
    {
        return name_;
    }
    bool isActive() const
    {
        return active_;
    }
    void setActive(bool isActive)
    {
        active_ = isActive;
    }

    template <typename T, typename... Args> T *addComponent(Args &&...args)
    {
        static_assert(
            std::is_base_of<Component, T>::value, "T must derive from Component"
        );

        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T *componentPtr = component.get();
        componentPtr->setOwner(this);
        components_.push_back(std::move(component));
        return componentPtr;
    }

    template <typename T> T *getComponent() const
    {
        for (const std::unique_ptr<Component> &component : components_)
        {
            if (T *result = dynamic_cast<T *>(component.get()))
            {
                return result;
            }
        }
        return nullptr;
    }

    template <typename T> bool removeComponent()
    {
        for (auto it = components_.begin(); it != components_.end(); ++it)
        {
            if (dynamic_cast<T *>(it->get()))
            {
                components_.erase(it);
                return true;
            }
        }
        return false;
    }

    private:
    std::string name_;
    bool active_ = true;
    std::vector<std::unique_ptr<Component>> components_;
};
