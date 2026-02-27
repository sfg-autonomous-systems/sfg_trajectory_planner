#pragma once

#include "sfg_trajectory_planner/engine/core/generic_factory.hpp"
#include "sfg_trajectory_planner/engine/editor/editor_context.hpp"
#include "sfg_trajectory_planner/engine/editor/scene_object_editor.hpp"

namespace sfg_trajectory_planner::engine::editor
{
    class SceneObjectEditorFactory : public core::GenericFactory<SceneObjectEditor<void>, EditorContext &>
    {
    public:
        template <typename EditorType>
        void register_editor(const std::string &display_name = "");
        template <typename EditorType>
        void register_editor(CreatorFunction creator);
    };
}

#include "sfg_trajectory_planner/engine/editor/scene_object_editor_factory.tpp"