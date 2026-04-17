#include "vortex_mouse.hpp"

GLFWwindow* VortexMouse::s_window = nullptr;
std::unordered_map<std::string, int> VortexMouse::s_button_map;
std::unordered_map<std::string, bool> VortexMouse::s_previous_buttons;
double VortexMouse::s_mouse_x = 0.0;
double VortexMouse::s_mouse_y = 0.0;

void VortexMouse::init(GLFWwindow *window)
{
    s_window = window;

    if (s_button_map.empty())
    {
        s_button_map = {
            {"LEFT", GLFW_MOUSE_BUTTON_LEFT},
            {"RIGHT", GLFW_MOUSE_BUTTON_RIGHT},
            {"MIDDLE", GLFW_MOUSE_BUTTON_MIDDLE}
        };

        for (const auto &pair : s_button_map)
        {
            s_previous_buttons[pair.first] = false;
        }
    }
}

void VortexMouse::update()
{
    if (!s_window) return;

    for (const auto &pair : s_button_map)
    {
        s_previous_buttons[pair.first] = (glfwGetMouseButton(s_window, pair.second) == GLFW_PRESS);
    }

    glfwGetCursorPos(s_window, &s_mouse_x, &s_mouse_y);
}

bool VortexMouse::get_button_down(const std::string &button)
{
    if (!s_window)
    {
        VORTEX_WARN("[INPUT WARNING] Mouse not initialized! Call VortexMouse::init(window) first.");
        return false;
    }

    auto it = s_button_map.find(button);
    if (it != s_button_map.end())
    {
        bool is_down_now = (glfwGetMouseButton(s_window, it->second) == GLFW_PRESS);
        bool was_down_last_frame = s_previous_buttons[button];

        return is_down_now && !was_down_last_frame;
    }

    VORTEX_WARN("[INPUT WARNING] Mouse Button '", button, "' is not recognized!");
    return false;
}

bool VortexMouse::get_button(const std::string &button)
{
    if (!s_window)
    {
        VORTEX_WARN("[INPUT WARNING] Mouse not initialized! Call VortexMouse::init(window) first.");
        return false;
    }

    auto it = s_button_map.find(button);
    if (it != s_button_map.end())
    {
        return (glfwGetMouseButton(s_window, it->second));
    }

    VORTEX_WARN("[INPUT WARNING] Mouse Button '", button, "' is not recognized!");
    return false;
}

double VortexMouse::get_x()
{
    return s_mouse_x;
}

double VortexMouse::get_y()
{
    return s_mouse_y;
}
