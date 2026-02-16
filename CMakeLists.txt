cmake_minimum_required(VERSION 3.8)
project(sfg_trajectory_planner)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

cmake_policy(SET CMP0072 NEW)
cmake_policy(SET CMP0079 NEW)

# ########################################################################################################
# Find dependencies.                                                                                     #
# ########################################################################################################
find_package(ament_cmake REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(sfg_imgui_vendor REQUIRED)
find_package(sfg_utils REQUIRED)
find_package(yaml-cpp REQUIRED)

pkg_check_modules(UUID REQUIRED uuid)

include(FetchContent)

FetchContent_Declare(
  glm
  GIT_REPOSITORY "https://github.com/g-truc/glm.git"
  GIT_TAG 1.0.3
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
  stduuid
  GIT_REPOSITORY "https://github.com/mariusbancila/stduuid.git"
  GIT_TAG v1.2.3
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(stduuid)
target_compile_definitions(stduuid INTERFACE UUID_SYSTEM_GENERATOR)
target_link_libraries(stduuid INTERFACE ${UUID_LIBRARIES})

# ########################################################################################################
# Add nodes and libraries.                                                                               #
# ########################################################################################################
sfg_utils__add_component("trajectory_planner" "${PROJECT_NAME}::TrajectoryPlanner"
  SOURCES
  "src/app/core/grid.cpp"
  "src/app/core/trajectory.cpp"
  "src/app/editor/grid_editor.cpp"
  "src/app/editor/trajectory_editor.cpp"
  "src/app/trajectory_planner_gui.cpp"

  "src/engine/core/gfx/camera.cpp"
  "src/engine/core/gfx/renderer.cpp"
  "src/engine/core/gfx/utils.cpp"
  "src/engine/core/serialization/yaml_serializer.cpp"
  "src/engine/core/scene.cpp"
  "src/engine/core/transform.cpp"
  "src/engine/core/scene_object.cpp"
  "src/engine/editor/inspector.cpp"
  "src/engine/editor/scene_hierarchy.cpp"
  "src/engine/editor/scene_object_editor.cpp"
  "src/engine/editor/selection_context.cpp"
  "src/engine/editor/viewport.cpp"

  "src/trajectory_planner_component.cpp"
  "src/trajectory_planner.cpp"
  AMENT_DEPENDENCIES
  rclcpp
  rclcpp_components
  sfg_imgui_vendor
  sfg_utils
  SYSTEM_DEPENDENCIES
  PRIVATE
  glm::glm
  magic_enum::magic_enum
  stduuid
  yaml-cpp
)

# Add GLM_SWIZZLE to the compile definitions of the trajectory_planner library to enable swizzling for glm vector types.
target_compile_definitions(${target_name} PRIVATE GLM_FORCE_SWIZZLE GLM_ENABLE_EXPERIMENTAL)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)

  # The following line skips the linter which checks for copyrights.
  # Comment the line when a copyright and license is added to all source files.
  set(ament_cmake_copyright_FOUND TRUE)

  # The following line skips cpplint (only works in a git repo).
  # Comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files.
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()