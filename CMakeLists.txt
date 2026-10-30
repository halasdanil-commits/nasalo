cmake_minimum_required(VERSION 3.10.2)
project("CloudMenu")

include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com
    GIT_TAG        v1.91.5
)
FetchContent_MakeAvailable(imgui)

add_library(CloudMenu SHARED
    main.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_android.cpp
)

target_include_directories(CloudMenu PRIVATE ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(CloudMenu log android GLESv3)
