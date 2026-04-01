cmake_minimum_required(VERSION 3.19)
project(PromptLab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Sql)

qt_standard_project_setup()

set(SOURCES
    src/main.cpp
    src/model/prompt.cpp
    src/model/category.cpp
    src/model/tag.cpp
    src/model/model_score.cpp
    src/database/database_manager.cpp
    src/repository/prompt_repository.cpp
    src/repository/category_repository.cpp
    src/repository/tag_repository.cpp
    src/repository/model_score_repository.cpp
    src/service/prompt_service.cpp
    src/service/category_service.cpp
    src/service/tag_service.cpp
    src/ui/main_window.cpp
    src/ui/dashboard_widget.cpp
    src/ui/prompt_list_widget.cpp
    src/ui/prompt_editor_dialog.cpp
    src/ui/prompt_detail_widget.cpp
    src/ui/category_manager_dialog.cpp
    src/ui/tag_manager_dialog.cpp
    src/ui/star_rating.cpp
    src/ui/model_score_dialog.cpp
    src/utils/translation_manager.cpp
)

set(HEADERS
    src/model/prompt.h
    src/model/category.h
    src/model/tag.h
    src/model/model_score.h
    src/database/database_manager.h
    src/repository/prompt_repository.h
    src/repository/category_repository.h
    src/repository/tag_repository.h
    src/repository/model_score_repository.h
    src/service/prompt_service.h
    src/service/category_service.h
    src/service/tag_service.h
    src/ui/main_window.h
    src/ui/dashboard_widget.h
    src/ui/prompt_list_widget.h
    src/ui/prompt_editor_dialog.h
    src/ui/prompt_detail_widget.h
    src/ui/category_manager_dialog.h
    src/ui/tag_manager_dialog.h
    src/ui/star_rating.h
    src/ui/model_score_dialog.h
    src/utils/translation_manager.h
)

set(RESOURCES
    resources/resources.qrc
)

qt_add_executable(PromptLab
    WIN32 MACOSX_BUNDLE
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

target_include_directories(PromptLab PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model
    ${CMAKE_CURRENT_SOURCE_DIR}/src/database
    ${CMAKE_CURRENT_SOURCE_DIR}/src/repository
    ${CMAKE_CURRENT_SOURCE_DIR}/src/service
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
)

target_link_libraries(PromptLab
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Sql
)

include(GNUInstallDirs)

install(TARGETS PromptLab
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET PromptLab
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
