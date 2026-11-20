#include "vortex_mouse.hpp"

GLFWwindow* VortexMouse::s_window = nullptr;
std::unordered_map<std::string, int> VortexMouse::s_button_map;
std::unordered_map<std::string, bool> VortexMouse::s_previous_buttons;
double VortexMouse::s_mouse_x = 0.0f;
double VortexMouse::s_mouse_y = 0.0f;

double VortexMouse::s_previous_mouse_x = 0.0f;
double VortexMouse::s_previous_mouse_y = 0.0f;

double VortexMouse::s_scroll_accum_x = 0.0;
double VortexMouse::s_scroll_accum_y = 0.0;
double VortexMouse::s_scroll_x = 0.0;
double VortexMouse::s_scroll_y = 0.0;
GLFWscrollfun VortexMouse::s_previous_scroll_callback = nullptr;

void VortexMouse::init(GLFWwindow *window)
{
    s_window = window;

    s_previous_scroll_callback = glfwSetScrollCallback(window, scroll_callback);

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

    s_scroll_x = s_scroll_accum_x;
    s_scroll_y = s_scroll_accum_y;
    s_scroll_accum_x = 0.0;
    s_scroll_accum_y = 0.0;

    for (const auto &pair : s_button_map)
    {
        s_previous_buttons[pair.first] = (glfwGetMouseButton(s_window, pair.second) == GLFW_PRESS);
    }

    s_previous_mouse_x = s_mouse_x;
    s_previous_mouse_y = s_mouse_y;

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

double VortexMouse::get_delta_x()
{
    return s_mouse_x - s_previous_mouse_x;
}

double VortexMouse::get_y()
{
    return s_mouse_y;
}

double VortexMouse::get_delta_y()
{
    return s_previous_mouse_y - s_mouse_y;
}

glm::vec2 VortexMouse::get_position()
{
    return glm::vec2(s_mouse_x, s_mouse_y);
}

void VortexMouse::scroll_callback(GLFWwindow* window, double x_offset, double y_offset)
{
    s_scroll_accum_x += x_offset;
    s_scroll_accum_y += y_offset;

    if (s_previous_scroll_callback)
    {
        s_previous_scroll_callback(window, x_offset, y_offset);
    }
}

double VortexMouse::get_scroll()
{
    return s_scroll_y;
}

double VortexMouse::get_scroll_x()
{
    return s_scroll_x;
}
