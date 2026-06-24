#include <vortex_actions.hpp>

std::deque<EditorAction*> ActionManager::undo_stack = {};
std::deque<EditorAction*> ActionManager::redo_stack = {};

ActionDecalTransform::ActionDecalTransform(
    VortexDecal *decal,
    glm::vec3 old_pos, glm::vec3 new_pos,
    glm::quat old_rot, glm::quat new_rot,
    glm::vec3 old_scale, glm::vec3 new_scale
)
{
    m_target = decal;

    m_old_pos = old_pos;
    m_new_pos = new_pos;

    m_old_rot = old_rot;
    m_new_rot = new_rot;

    m_old_scale = old_scale;
    m_new_scale = new_scale;
}

ActionDecalTransform::~ActionDecalTransform()
{
    if (m_target)
    {
        m_target = nullptr;
    }
}

void ActionDecalTransform::undo()
{
    if (!m_target) return;
    m_target->position = m_old_pos;
    m_target->orientation = m_old_rot;
    m_target->scale = m_old_scale;
}

void ActionDecalTransform::redo()
{
    if (!m_target) return;
    m_target->position = m_new_pos;
    m_target->orientation = m_new_rot;
    m_target->scale = m_new_scale;
}

std::string ActionDecalTransform::get_name() const
{
    return "Transform Decal";
}

std::string ActionDecalTransform::get_details() const
{
    VortexModelDetailsDict model_details = {
        .name = m_target->decal_texture_path,
        .position = m_new_pos,
        .orientation = m_new_rot,
        .scale = m_new_scale
    };

    return vortex_ModelDetailsDictToString(model_details);
}

ActionCreateDecal::ActionCreateDecal(VortexModel *model, VortexDecal *decal)
{
    m_target_model = model;
    m_decal = decal;
    m_is_in_scene = false;
}

ActionCreateDecal::~ActionCreateDecal()
{
    if (!m_is_in_scene && m_decal)
    {
        delete m_decal;
        m_decal = nullptr;
    }
}

void ActionCreateDecal::undo()
{
    if (!m_target_model || !m_decal) return;

    auto& decals = m_target_model->decals;
    decals.erase(std::remove(decals.begin(), decals.end(), m_decal), decals.end());
    m_is_in_scene = false;

    if (VortexGUI::g_active_decal_for_gizmo == m_decal) VortexGUI::g_active_decal_for_gizmo = nullptr;
}

void ActionCreateDecal::redo()
{
    if (!m_target_model || !m_decal) return;

    m_target_model->decals.push_back(m_decal);
    m_is_in_scene = true;
}

std::string ActionCreateDecal::get_name() const
{
    return "Add Decal Component";
}

std::string ActionCreateDecal::get_details() const
{
    return "{Parent: " + (m_target_model ? m_target_model->model_name : "Unknown") + "}";
}

ActionDeleteDecal::ActionDeleteDecal(VortexModel *model, VortexDecal *decal)
{
    m_target_model = model;
    m_decal = decal;
    m_is_deleted = false;
}

ActionDeleteDecal::~ActionDeleteDecal()
{
    if (m_is_deleted && m_decal)
    {
        delete m_decal;
        m_decal = nullptr;
    }
}

void ActionDeleteDecal::undo()
{
    if (!m_target_model || !m_decal) return;

    m_target_model->decals.push_back(m_decal);
    m_is_deleted = false;
}

void ActionDeleteDecal::redo()
{
    if (!m_target_model || !m_decal) return;

    auto& decals = m_target_model->decals;
    decals.erase(std::remove(decals.begin(), decals.end(), m_decal), decals.end());
    m_is_deleted = true;

    if (VortexGUI::g_active_decal_for_gizmo == m_decal) VortexGUI::g_active_decal_for_gizmo = nullptr;
}

std::string ActionDeleteDecal::get_name() const
{
    return "Delete Decal Component";
}

std::string ActionDeleteDecal::get_details() const
{
    return "{Parent: " + (m_target_model ? m_target_model->model_name : "Unknown") + "}";
}

ActionTransform::ActionTransform(
    VortexModel *model,
    glm::vec3 old_pos, glm::vec3 new_pos,
    glm::quat old_rot, glm::quat new_rot,
    glm::vec3 old_scale, glm::vec3 new_scale
)
{
    m_target = model;

    m_old_pos = old_pos;
    m_new_pos = new_pos;

    m_old_rot = old_rot;
    m_new_rot = new_rot;

    m_old_scale = old_scale;
    m_new_scale = new_scale;
}

ActionTransform::~ActionTransform()
{
    if (m_target)
    {
        m_target = nullptr;
    }
}

void ActionTransform::undo()
{
    m_target->transform.position = m_old_pos;
    m_target->transform.orientation = m_old_rot;
    m_target->transform.scale = m_old_scale;
    m_target->set_model_matrix(m_target->get_model_matrix());
}

void ActionTransform::redo()
{
    m_target->transform.position = m_new_pos;
    m_target->transform.orientation = m_new_rot;
    m_target->transform.scale = m_new_scale;
    m_target->set_model_matrix(m_target->get_model_matrix());
}

std::string ActionTransform::get_name() const
{
    return "Transform Model";
}

std::string ActionTransform::get_details() const
{
    VortexModelDetailsDict model_details = {
        .name = m_target ? m_target->model_name : "Destroyed Model",
        .position = m_new_pos,
        .orientation = m_new_rot,
        .scale = m_new_scale
    };

    return vortex_ModelDetailsDictToString(model_details);
}

ActionCreate::ActionCreate(VortexModel *model)
{
    m_target = model;
    m_is_in_scene = true;
}

ActionCreate::~ActionCreate()
{
    if (!m_is_in_scene && m_target)
    {
        delete m_target;
        m_target = nullptr;
    }
}

void ActionCreate::undo()
{
    auto &models = VortexObjectManager::active_models;
    models.erase(
        std::remove(
            models.begin(),
            models.end(),
            m_target
        ),
        models.end()
    );

    m_is_in_scene = false;
}

void ActionCreate::redo()
{
    VortexObjectManager::active_models.push_back(m_target);
    m_is_in_scene = true;
}

std::string ActionCreate::get_name() const
{
    return "Create Model";
}

std::string ActionCreate::get_details() const
{
    return std::string("{") + "Name: " + m_target->model_name + "}";
}

ActionDelete::ActionDelete(VortexModel *model)
{
    m_target = model;
    m_is_deleted = true;
}

ActionDelete::~ActionDelete()
{
    if (m_is_deleted && m_target)
    {
        delete m_target;
        m_target = nullptr;
    }
}

void ActionDelete::undo()
{
    VortexObjectManager::active_models.push_back(m_target);
    m_is_deleted = false;
}

void ActionDelete::redo()
{
    auto &models = VortexObjectManager::active_models;
    models.erase(
        std::remove(
            models.begin(),
            models.end(),
            m_target
        ),
        models.end()
    );

    m_is_deleted = true;
}

std::string ActionDelete::get_name() const
{
    return "Delete Model";
}

std::string ActionDelete::get_details() const
{
    return std::string("{") + "Name: " + m_target->model_name + "}";
}

void ActionManager::push_action(EditorAction *action)
{
    undo_stack.push_back(action);
    if (undo_stack.size() > MAX_HISTORY)
    {
        delete undo_stack.front();
        undo_stack.pop_front();
    }

    for (auto *redo : redo_stack)
    {
        delete redo;
    }
    redo_stack.clear();

    VortexApplication::mark_unsaved_changes();
}

void ActionManager::undo()
{
    if (undo_stack.empty()) return;

    EditorAction *action = undo_stack.back();
    action->undo();

    undo_stack.pop_back();
    redo_stack.push_back(action);
}

void ActionManager::redo()
{
    if (redo_stack.empty()) return;

    EditorAction *action = redo_stack.back();
    action->redo();

    redo_stack.pop_back();
    undo_stack.push_back(action);
}

const std::deque<EditorAction*> &ActionManager::get_undo_stack()
{
    return undo_stack;
}

const std::deque<EditorAction*> &ActionManager::get_redo_stack()
{
    return redo_stack;
}

void ActionManager::clear_stack_history()
{
    for (EditorAction *action : undo_stack)
    {
        delete action;
    }
    undo_stack.clear();

    for (EditorAction *action : redo_stack)
    {
        delete action;
    }
    redo_stack.clear();

    VORTEX_INFO("[Action Manager] Deleting Action Stack History...");
}