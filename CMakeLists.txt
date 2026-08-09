cmake_minimum_required(VERSION 3.21)

project(ProceduralTextureMaker VERSION 0.1.0 LANGUAGES CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(PROCEDURAL_TEXTURE_MAKER_STATIC_BUILD
    "Require static Qt libraries and link a self-contained release"
    OFF
)
option(PROCEDURAL_TEXTURE_MAKER_UI_TESTS
    "Build the offscreen UI smoke test"
    ON
)
find_program(CLANG_TIDY_EXE NAMES clang-tidy)
find_program(CLANG_FORMAT_EXE NAMES clang-format)
find_program(NPM_EXECUTABLE NAMES npm.cmd npm)
if(CLANG_TIDY_EXE)
    find_package(Python3 COMPONENTS Interpreter QUIET)
    find_program(RUN_CLANG_TIDY_EXE NAMES run-clang-tidy run-clang-tidy.py)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    OpenGL
    OpenGLWidgets
    Qml
    Widgets
    Xml
)
if(BUILD_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)
endif()
find_package(Threads REQUIRED)

if(PROCEDURAL_TEXTURE_MAKER_STATIC_BUILD)
    get_target_property(PROCEDURAL_TEXTURE_MAKER_QT_CORE_TYPE Qt6::Core TYPE)
    if(NOT PROCEDURAL_TEXTURE_MAKER_QT_CORE_TYPE STREQUAL "STATIC_LIBRARY")
        message(FATAL_ERROR
            "PROCEDURAL_TEXTURE_MAKER_STATIC_BUILD requires a statically built Qt. "
            "Set CMAKE_PREFIX_PATH to the installation prefix of a static Qt build."
        )
    endif()
endif()

set(PROCEDURAL_TEXTURE_MAKER_SOURCES
    main.cpp
    cli/commandline.cpp
    cli/commandline.h
    texgenapplication.cpp
    texgenapplication.h
    global.h

    gui/applicationtheme.cpp
    gui/applicationtheme.h
    gui/clipboardoperations.cpp
    gui/clipboardoperations.h

    base/backgroundbrushstyles.h
    base/settingsmanager.cpp
    base/settingsmanager.h
    base/projectfileservice.cpp
    base/projectfileservice.h
    base/textureimage.cpp
    base/textureimage.h
    base/textureexporter.cpp
    base/textureexporter.h
    base/texturenode.cpp
    base/texturenode.h
    base/textureproject.cpp
    base/textureproject.h
    base/texturerendermanager.cpp
    base/texturerendermanager.h
    base/editmanager.cpp
    base/editmanager.h
    base/texturegenerator.cpp
    base/texturegenerator.h
    base/jstexgen.cpp
    base/jstexgen.h
    base/jstexgenmanager.cpp
    base/jstexgenmanager.h

    gui/addnodepanel.cpp
    gui/addnodepanel.h
    gui/connectionwidget.cpp
    gui/connectionwidget.h
    gui/generatorinfowidget.cpp
    gui/generatorinfowidget.h
    gui/cubewidget.cpp
    gui/cubewidget.h
    gui/iteminfopanel.cpp
    gui/iteminfopanel.h
    gui/mainwindow.cpp
    gui/mainwindow.h
    gui/menuactions.cpp
    gui/menuactions.h
    gui/nodesettingswidget.cpp
    gui/nodesettingswidget.h
    gui/previewimagepanel.cpp
    gui/previewimagepanel.h
    gui/texturebackground.h
    gui/qdoubleslider.h
    gui/sceneinfowidget.cpp
    gui/sceneinfowidget.h
    gui/settingspanel.cpp
    gui/settingspanel.h

    sceneview/viewnodeitem.cpp
    sceneview/viewnodeitem.h
    sceneview/viewnodeline.cpp
    sceneview/viewnodeline.h
    sceneview/viewnodescene.cpp
    sceneview/viewnodescene.h
    sceneview/viewnodeview.cpp
    sceneview/viewnodeview.h

    generators/builtinregistry.cpp
    generators/builtinregistry.h
    generators/boxblur.cpp
    generators/boxblur.h
    generators/cutout.cpp
    generators/cutout.h
    generators/displacementmap.cpp
    generators/displacementmap.h
    generators/empty.cpp
    generators/empty.h
    generators/gaussianblur.cpp
    generators/gaussianblur.h
    generators/gradient.cpp
    generators/gradient.h
    generators/greyscale.cpp
    generators/greyscale.h
    generators/invert.cpp
    generators/invert.h
    generators/lens.cpp
    generators/lens.h
    generators/merge.cpp
    generators/merge.h
    generators/mirror.cpp
    generators/mirror.h
    generators/modifylevels.cpp
    generators/modifylevels.h
    generators/normalmap.cpp
    generators/normalmap.h
    generators/pointillism.cpp
    generators/pointillism.h
    generators/setchannels.cpp
    generators/setchannels.h
    generators/sinetransform.cpp
    generators/sinetransform.h
    generators/stackblur.cpp
    generators/stackblur.h
    generators/star.cpp
    generators/star.h
    generators/text.cpp
    generators/text.h

    texgen.qrc
    generators.qrc
    shaders.qrc
)

set(PROCEDURAL_TEXTURE_MAKER_CLANG_TIDY_SOURCES ${PROCEDURAL_TEXTURE_MAKER_SOURCES})
list(FILTER PROCEDURAL_TEXTURE_MAKER_CLANG_TIDY_SOURCES INCLUDE REGEX "\\.cpp$")
set(PROCEDURAL_TEXTURE_MAKER_CLANG_TIDY_FILTER
    ".*(base|cli|generators|gui|sceneview)[/\\\\].*\\.cpp$|.*(main|texgenapplication)\\.cpp$"
)
set(PROCEDURAL_TEXTURE_MAKER_FORMAT_SOURCES ${PROCEDURAL_TEXTURE_MAKER_SOURCES})
list(FILTER PROCEDURAL_TEXTURE_MAKER_FORMAT_SOURCES INCLUDE REGEX "\\.(cpp|h)$")
if(BUILD_TESTING)
    file(GLOB_RECURSE PROCEDURAL_TEXTURE_MAKER_TEST_FORMAT_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.h"
    )
    list(APPEND PROCEDURAL_TEXTURE_MAKER_FORMAT_SOURCES
        ${PROCEDURAL_TEXTURE_MAKER_TEST_FORMAT_SOURCES}
    )
endif()

set(PROCEDURAL_TEXTURE_MAKER_ENGINE_SOURCES ${PROCEDURAL_TEXTURE_MAKER_SOURCES})
list(FILTER PROCEDURAL_TEXTURE_MAKER_ENGINE_SOURCES INCLUDE REGEX
    "^(base|generators)/.*|^global\\.h$"
)

set(PROCEDURAL_TEXTURE_MAKER_GUI_SOURCES ${PROCEDURAL_TEXTURE_MAKER_SOURCES})
list(FILTER PROCEDURAL_TEXTURE_MAKER_GUI_SOURCES INCLUDE REGEX
    "^(gui|sceneview)/.*|^texgenapplication\\.(cpp|h)$"
)

add_library(ptm_engine STATIC
    ${PROCEDURAL_TEXTURE_MAKER_ENGINE_SOURCES}
    generators.qrc
)

target_include_directories(ptm_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ptm_engine PUBLIC
    Threads::Threads
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Xml
)

add_library(ptm_gui STATIC
    ${PROCEDURAL_TEXTURE_MAKER_GUI_SOURCES}
)

target_include_directories(ptm_gui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ptm_gui PUBLIC
    ptm_engine
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::Widgets
)

qt_add_executable(ProceduralTextureMaker
    MANUAL_FINALIZATION
    WIN32
    MACOSX_BUNDLE
    main.cpp
    cli/commandline.cpp
    cli/commandline.h
    texgen.qrc
    shaders.qrc
)

if(APPLE)
    set(PROCEDURAL_TEXTURE_MAKER_MACOS_ICON
        "${CMAKE_CURRENT_SOURCE_DIR}/images/mainicon.icns"
    )
    set(PROCEDURAL_TEXTURE_MAKER_MACOS_PLIST
        "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
    )
    set_source_files_properties(
        ${PROCEDURAL_TEXTURE_MAKER_MACOS_ICON}
        PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
    )
    set_source_files_properties(
        ${PROCEDURAL_TEXTURE_MAKER_MACOS_PLIST}
        PROPERTIES MACOSX_PACKAGE_LOCATION "."
    )
    target_sources(ProceduralTextureMaker PRIVATE
        ${PROCEDURAL_TEXTURE_MAKER_MACOS_ICON}
        ${PROCEDURAL_TEXTURE_MAKER_MACOS_PLIST}
    )
endif()

target_include_directories(ProceduralTextureMaker PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ProceduralTextureMaker PRIVATE
    ptm_gui
)

if(PROCEDURAL_TEXTURE_MAKER_STATIC_BUILD AND WIN32)
    if(MSVC)
        set_property(TARGET ProceduralTextureMaker PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
        )
    elseif(MINGW)
        target_link_options(ProceduralTextureMaker PRIVATE
            -static
            -static-libgcc
            -static-libstdc++
        )
    endif()
endif()

set_target_properties(ProceduralTextureMaker PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
    MACOSX_BUNDLE_ICON_FILE "mainicon.icns"
)

if(CLANG_TIDY_EXE)
    if(RUN_CLANG_TIDY_EXE AND Python3_Interpreter_FOUND)
        add_custom_target(clang-tidy
            COMMAND ${Python3_EXECUTABLE}
                    ${RUN_CLANG_TIDY_EXE}
                    -p ${CMAKE_BINARY_DIR}
                    -clang-tidy-binary ${CLANG_TIDY_EXE}
                    -source-filter ${PROCEDURAL_TEXTURE_MAKER_CLANG_TIDY_FILTER}
                    -quiet
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Running clang-tidy"
            VERBATIM
        )
    else()
        add_custom_target(clang-tidy
            COMMAND ${CLANG_TIDY_EXE}
                    ${PROCEDURAL_TEXTURE_MAKER_CLANG_TIDY_SOURCES}
                    -p ${CMAKE_BINARY_DIR}
                    --quiet
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Running clang-tidy"
            VERBATIM
        )
    endif()
    add_dependencies(clang-tidy
        ptm_engine_autogen
        ptm_gui_autogen
        ProceduralTextureMaker_autogen
    )
else()
    add_custom_target(clang-tidy
        COMMAND ${CMAKE_COMMAND} -E echo "clang-tidy was not found. Install clang-tidy and rerun CMake."
        COMMAND ${CMAKE_COMMAND} -E false
        COMMENT "Running clang-tidy"
        VERBATIM
    )
endif()

if(NPM_EXECUTABLE)
    add_custom_target(eslint
        COMMAND ${NPM_EXECUTABLE} run lint:js
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running ESLint"
        VERBATIM
    )
else()
    add_custom_target(eslint
        COMMAND ${CMAKE_COMMAND} -E echo "npm was not found. Install Node.js and rerun CMake."
        COMMAND ${CMAKE_COMMAND} -E false
        COMMENT "Running ESLint"
        VERBATIM
    )
endif()

if(CLANG_FORMAT_EXE)
    add_custom_target(clang-format-check
        COMMAND ${CLANG_FORMAT_EXE}
                --dry-run
                --Werror
                ${PROCEDURAL_TEXTURE_MAKER_FORMAT_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Checking code formatting"
        VERBATIM
    )

    add_custom_target(clang-format
        COMMAND ${CLANG_FORMAT_EXE}
                -i
                ${PROCEDURAL_TEXTURE_MAKER_FORMAT_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running clang-format"
        VERBATIM
    )
else()
    add_custom_target(clang-format-check
        COMMAND ${CMAKE_COMMAND} -E echo "clang-format was not found. Install clang-format and rerun CMake."
        COMMAND ${CMAKE_COMMAND} -E false
        COMMENT "Checking code formatting"
        VERBATIM
    )

    add_custom_target(clang-format
        COMMAND ${CMAKE_COMMAND} -E echo "clang-format was not found. Install clang-format and rerun CMake."
        COMMAND ${CMAKE_COMMAND} -E false
        COMMENT "Running clang-format"
        VERBATIM
    )
endif()

install(TARGETS ProceduralTextureMaker
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

qt_finalize_executable(ProceduralTextureMaker)
