#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <deque>

#include "vortex_model.hpp"
#include "vortex_objectmanager.hpp"
#include "vortex_application.hpp"

class EditorAction
{
public:
    virtual ~EditorAction() = default;
    virtual void undo() = 0;
    virtual void redo() = 0;

    virtual std::string get_name() const = 0;
};

class ActionTransform : public EditorAction
{
    VortexModel *m_target;
    glm::vec3 m_old_pos;
    glm::vec3 m_new_pos;

    glm::quat m_old_rot;
    glm::quat m_new_rot;

    glm::vec3 m_old_scale;
    glm::vec3 m_new_scale;
public:
    ActionTransform(
        VortexModel *model,
        glm::vec3 old_pos, glm::vec3 new_pos,
        glm::quat old_rot, glm::quat new_rot,
        glm::vec3 old_scale, glm::vec3 new_scale
    );

    void undo() override;
    void redo() override;

    std::string get_name() const override;
};

class ActionCreate : public EditorAction
{
    VortexModel *m_target;
    bool m_is_in_scene;
public:
    ActionCreate(VortexModel *model);
    ~ActionCreate() override;

    void undo() override;
    void redo() override;

    std::string get_name() const override;
};

class ActionDelete : public EditorAction
{
    VortexModel *m_target;
    bool m_is_deleted;
public:
    ActionDelete(VortexModel *model);
    ~ActionDelete() override;

    void undo() override;
    void redo() override;

    std::string get_name() const override;
};

class ActionManager
{
    static std::deque<EditorAction*> undo_stack;
    static std::deque<EditorAction*> redo_stack;
    static const int MAX_HISTORY = 50;
public:

    static void push_action(EditorAction *action);
    static void undo();
    static void redo();

    static const std::deque<EditorAction*> &get_undo_stack();
    static const std::deque<EditorAction*> &get_redo_stack();
};