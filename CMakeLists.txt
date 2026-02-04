cmake_minimum_required(VERSION 3.8)
project(sfg_trajectory_planner)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

cmake_policy(SET CMP0072 NEW)

# ########################################################################################################
# Find dependencies.                                                                                     #
# ########################################################################################################
find_package(ament_cmake REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(sfg_imgui_vendor REQUIRED)
find_package(sfg_utils REQUIRED)

include(FetchContent)

FetchContent_Declare(
  glm
  GIT_REPOSITORY "https://github.com/g-truc/glm.git"
  GIT_TAG 1.0.3
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glm)

# ########################################################################################################
# Add nodes and libraries.                                                                               #
# ########################################################################################################
sfg_utils__add_component("trajectory_planner" "${PROJECT_NAME}::TrajectoryPlanner"
  SOURCES
  "src/trajectory_planner_component.cpp"
  "src/trajectory_planner_gui.cpp"
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
)

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