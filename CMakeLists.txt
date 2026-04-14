cmake_minimum_required(VERSION 3.21)

project(logos_node_inspector VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(NODE_INSPECTOR_PACKAGE_NAME "node_inspector_qml")
set(NODE_INSPECTOR_VARIANT "linux-amd64")
set(NODE_INSPECTOR_PACKAGE_ROOT "${CMAKE_CURRENT_BINARY_DIR}/package-root")
set(NODE_INSPECTOR_VARIANT_ROOT "${NODE_INSPECTOR_PACKAGE_ROOT}/variants/${NODE_INSPECTOR_VARIANT}")
set(NODE_INSPECTOR_MODULE_ROOT "${NODE_INSPECTOR_VARIANT_ROOT}/Logos/NodeInspector")

include(CTest)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

qt_standard_project_setup(REQUIRES 6.5)

set_source_files_properties(
  qml/DesignSystem.qml
  PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

file(MAKE_DIRECTORY "${NODE_INSPECTOR_VARIANT_ROOT}")
configure_file(package/Main.qml.in "${NODE_INSPECTOR_VARIANT_ROOT}/Main.qml" @ONLY)
configure_file(package/manifest.json.in "${NODE_INSPECTOR_PACKAGE_ROOT}/manifest.json" @ONLY)
configure_file(package/metadata.json.in "${NODE_INSPECTOR_VARIANT_ROOT}/metadata.json" @ONLY)

add_library(node_inspector_model STATIC
  src/remote_output_parser.cpp
  src/status_classifier.cpp
)

target_include_directories(node_inspector_model PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

qt_add_qml_module(node_inspector_plugin
  SHARED
  PLUGIN_TARGET node_inspector_plugin
  OUTPUT_DIRECTORY "${NODE_INSPECTOR_MODULE_ROOT}"
  URI Logos.NodeInspector
  VERSION 1.0
  SOURCES
    src/node_inspector_controller.cpp
    src/node_inspector_controller.h
    src/ssh_collector.cpp
    src/ssh_collector.h
  QML_FILES
    qml/DesignSystem.qml
    qml/Main.qml
    qml/components/InfoRow.qml
    qml/components/MetricTile.qml
    qml/components/NavRailItem.qml
    qml/components/SeverityChip.qml
    qml/panes/ConnectionPane.qml
    qml/panes/HostPane.qml
    qml/panes/LogsPane.qml
    qml/panes/LogosPane.qml
    qml/panes/OverviewPane.qml
    qml/panes/RecommendationsPane.qml
    qml/panes/ServicesPane.qml
)

target_link_libraries(node_inspector_plugin PRIVATE
  Qt6::Core
  Qt6::Gui
  Qt6::Qml
  Qt6::Quick
  Qt6::QuickControls2
  node_inspector_model
)

install(TARGETS
  node_inspector_plugin
  LIBRARY DESTINATION "variants/${NODE_INSPECTOR_VARIANT}/Logos/NodeInspector"
  RUNTIME DESTINATION "variants/${NODE_INSPECTOR_VARIANT}/Logos/NodeInspector"
  BUNDLE DESTINATION "variants/${NODE_INSPECTOR_VARIANT}/Logos/NodeInspector"
)

install(DIRECTORY
  "${NODE_INSPECTOR_MODULE_ROOT}/"
  DESTINATION "variants/${NODE_INSPECTOR_VARIANT}/Logos/NodeInspector"
)

install(FILES
  "${NODE_INSPECTOR_PACKAGE_ROOT}/manifest.json"
  DESTINATION "."
)

install(FILES
  "${NODE_INSPECTOR_VARIANT_ROOT}/Main.qml"
  "${NODE_INSPECTOR_VARIANT_ROOT}/metadata.json"
  DESTINATION "variants/${NODE_INSPECTOR_VARIANT}"
)

install(CODE [[
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E tar cfvz "${CMAKE_INSTALL_PREFIX}/node_inspector_qml.lgx" --format=gnutar manifest.json variants
    WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}"
    RESULT_VARIABLE node_inspector_lgx_result
  )
  if (NOT node_inspector_lgx_result EQUAL 0)
    message(FATAL_ERROR "Failed to create node_inspector_qml.lgx")
  endif()
]])

if(BUILD_TESTING)
  add_executable(test_remote_output_parser
    tests/test_remote_output_parser.cpp
  )
  target_link_libraries(test_remote_output_parser PRIVATE node_inspector_model)
  target_compile_definitions(test_remote_output_parser PRIVATE
    NODE_INSPECTOR_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures"
  )
  add_test(NAME test_remote_output_parser COMMAND test_remote_output_parser)

  add_executable(test_status_classifier
    tests/test_status_classifier.cpp
  )
  target_link_libraries(test_status_classifier PRIVATE node_inspector_model)
  target_compile_definitions(test_status_classifier PRIVATE
    NODE_INSPECTOR_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures"
  )
  add_test(NAME test_status_classifier COMMAND test_status_classifier)
endif()
