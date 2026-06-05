#include "core_platform/linux/inputglfw.h"
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <unordered_map>

InputGLFW *InputGLFW::instance_ = nullptr;

void InputGLFW::init()
{
    instance_ = this;
    key_states_.clear();
    mouse_button_states_.clear();
    mouseDelta_ = {0.0f, 0.0f};
    cursorInitialised_ = false;
    spdlog::debug("Input system initialised");
}

void InputGLFW::shutdown()
{
    key_states_.clear();
    mouse_button_states_.clear();
}

bool InputGLFW::isKeyPressed(KeyCode key) const
{
    auto key_state_iterator = key_states_.find(static_cast<int32_t>(key));
    if (key_state_iterator != key_states_.end())
        return key_state_iterator->second;
    return false;
}

bool InputGLFW::isMouseButtonPressed(MouseButton button) const
{
    auto button_state_iterator =
        mouse_button_states_.find(static_cast<int32_t>(button));
    if (button_state_iterator != mouse_button_states_.end())
        return button_state_iterator->second;
    return false;
}

MouseDelta InputGLFW::getMouseDelta() const
{
    return mouseDelta_;
}

void InputGLFW::resetMouseDelta()
{
    mouseDelta_ = {0.0f, 0.0f};
}

void InputGLFW::setWindow(GLFWwindow *window)
{
    window_ = window;
}

void InputGLFW::setCursorMode(bool disabled)
{
    if (!window_)
        return;
    if (disabled)
    {
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else
    {
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
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

void InputGLFW::mouseButtonCallback(GLFWwindow *, int button, int action, int)
{
    if (!instance_)
        return;
    
    // TODO - generalise
    if (action == GLFW_PRESS)
    {
        instance_->mouse_button_states_[button] = true;
        if (button == static_cast<int>(MouseButton::Right))
        {
            instance_->setCursorMode(true);
        }
    }
    else if (action == GLFW_RELEASE)
    {
        instance_->mouse_button_states_[button] = false;
        if (button == static_cast<int>(MouseButton::Right))
        {
            instance_->setCursorMode(false);
        }
    }
}

void InputGLFW::cursorPositionCallback(GLFWwindow *, double xpos, double ypos)
{
    if (!instance_)
        return;

    if (!instance_->cursorInitialised_)
    {
        instance_->lastCursorX_ = xpos;
        instance_->lastCursorY_ = ypos;
        instance_->cursorInitialised_ = true;
        return;
    }

    instance_->mouseDelta_.x +=
        static_cast<float>(xpos - instance_->lastCursorX_);
    instance_->mouseDelta_.y +=
        static_cast<float>(ypos - instance_->lastCursorY_);
    instance_->lastCursorX_ = xpos;
    instance_->lastCursorY_ = ypos;
}
