#include <vortex_actions.hpp>

std::deque<EditorAction*> ActionManager::undo_stack = {};
std::deque<EditorAction*> ActionManager::redo_stack = {};

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