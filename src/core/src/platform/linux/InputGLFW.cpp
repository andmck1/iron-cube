#include "platform/linux/InputGLFW.h"
#include <GLFW/glfw3.h>
#include <unordered_map>

InputGLFW *InputGLFW::instance_ = nullptr;

void InputGLFW::init()
{
    instance_ = this;
    key_states_.clear();
}

void InputGLFW::shutdown()
{
    key_states_.clear();
}

bool InputGLFW::isKeyPressed(KeyCode key) const
{
    auto it = key_states_.find(static_cast<int32_t>(key));
    if (it != key_states_.end())
        return it->second;
    return false;
}

void InputGLFW::keyCallback(GLFWwindow *, int key, int, int action, int)
{
    if (!instance_)
        return;

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        instance_->key_states_[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        instance_->key_states_[key] = false;
    }
}
