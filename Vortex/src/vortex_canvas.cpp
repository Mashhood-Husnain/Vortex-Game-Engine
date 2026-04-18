#include "vortex_canvas.hpp"

void VortexCanvas::add_element(const VortexCanvas_UIElement& element)
{
    elements.push_back(element);
}

VortexCanvas_UIElement* VortexCanvas::get_element(const std::string& element_id)
{
    for (auto& el : elements)
    {
        if (el.id == element_id)
        {
            return &el;
        }
    }
    return nullptr;
}

void VortexCanvas::draw()
{
    if (!is_active) return;

    VortexHUD::Begin();

    for (const VortexCanvas_UIElement& el : elements)
    {
        if (!el.is_active) continue;

        if (el.type == VortexCanvas_UIType::TEXT)
        {
            VortexHUD::Text(el.text_data.c_str(), el.position);
        }
        else if (el.type == VortexCanvas_UIType::BAR)
        {
            VortexHUD::Bar(el.text_data.c_str(), el.current_value, el.max_value, el.position, el.size, el.color);
        }
        else if (el.type == VortexCanvas_UIType::COUNTER)
        {
            VortexHUD::Counter(el.text_data.c_str(), el.current_value, el.position, el.color);
        }
    }

    VortexHUD::End();
}
