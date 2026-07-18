cmake_minimum_required(VERSION 3.21)

project(ProceduralTextureMaker VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(PROCEDURAL_TEXTURE_MAKER_STATIC_BUILD
    "Require static Qt libraries and link a self-contained release"
    OFF
)

find_program(CLANG_TIDY_EXE NAMES clang-tidy)
find_program(CLANG_FORMAT_EXE NAMES clang-format)
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
    texgenapplication.cpp
    texgenapplication.h
    global.h

    gui/applicationtheme.cpp
    gui/applicationtheme.h

    base/settingsmanager.cpp
    base/settingsmanager.h
    base/textureimage.cpp
    base/textureimage.h
    base/texturenode.cpp
    base/texturenode.h
    base/textureproject.cpp
    base/textureproject.h
    base/texturerenderthread.cpp
    base/texturerenderthread.h

    gui/addnodepanel.cpp
    gui/addnodepanel.h
    gui/connectionwidget.cpp
    gui/connectionwidget.h
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

    generators/blending.cpp
    generators/blending.h
    generators/boxblur.cpp
    generators/boxblur.h
    generators/bricks.cpp
    generators/bricks.h
    generators/checkboard.cpp
    generators/checkboard.h
    generators/circle.cpp
    generators/circle.h
    generators/cutout.cpp
    generators/cutout.h
    generators/displacementmap.cpp
    generators/displacementmap.h
    generators/empty.cpp
    generators/empty.h
    generators/fill.cpp
    generators/fill.h
    generators/fire.cpp
    generators/fire.h
    generators/gaussianblur.cpp
    generators/gaussianblur.h
    generators/glow.cpp
    generators/glow.h
    generators/gradient.cpp
    generators/gradient.h
    generators/greyscale.cpp
    generators/greyscale.h
    generators/invert.cpp
    generators/invert.h
    generators/javascript.cpp
    generators/javascript.h
    generators/lens.cpp
    generators/lens.h
    generators/lines.cpp
    generators/lines.h
    generators/merge.cpp
    generators/merge.h
    generators/mirror.cpp
    generators/mirror.h
    generators/modifylevels.cpp
    generators/modifylevels.h
    generators/noise.cpp
    generators/noise.h
    generators/normalmap.cpp
    generators/normalmap.h
    generators/perlinnoise.cpp
    generators/perlinnoise.h
    generators/pixelate.cpp
    generators/pixelate.h
    generators/pointillism.cpp
    generators/pointillism.h
    generators/setchannels.cpp
    generators/setchannels.h
    generators/shadow.cpp
    generators/shadow.h
    generators/sineplasma.cpp
    generators/sineplasma.h
    generators/sinetransform.cpp
    generators/sinetransform.h
    generators/square.cpp
    generators/square.h
    generators/stackblur.cpp
    generators/stackblur.h
    generators/star.cpp
    generators/star.h
    generators/text.cpp
    generators/text.h
    generators/texturegenerator.cpp
    generators/texturegenerator.h
    generators/transform.cpp
    generators/transform.h
    generators/whirl.cpp
    generators/whirl.h

    texgen.qrc
    shaders.qrc
)

set(PROCEDURAL_TEXTURE_MAKER_CLANG_TIDY_SOURCES ${PROCEDURAL_TEXTURE_MAKER_SOURCES})
list(FILTER PROCEDURAL_TEXTURE_MAKER_CLANG_TIDY_SOURCES INCLUDE REGEX "\\.cpp$")
set(PROCEDURAL_TEXTURE_MAKER_CLANG_TIDY_FILTER
    ".*(base|generators|gui|sceneview)[/\\\\].*\\.cpp$|.*(main|texgenapplication)\\.cpp$"
)
set(PROCEDURAL_TEXTURE_MAKER_FORMAT_SOURCES ${PROCEDURAL_TEXTURE_MAKER_SOURCES})
list(FILTER PROCEDURAL_TEXTURE_MAKER_FORMAT_SOURCES INCLUDE REGEX "\\.(cpp|h)$")

qt_add_executable(ProceduralTextureMaker
    MANUAL_FINALIZATION
    WIN32
    MACOSX_BUNDLE
    ${PROCEDURAL_TEXTURE_MAKER_SOURCES}
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
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::Qml
    Qt6::Widgets
    Qt6::Xml
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
    add_dependencies(clang-tidy ProceduralTextureMaker_autogen)
else()
    add_custom_target(clang-tidy
        COMMAND ${CMAKE_COMMAND} -E echo "clang-tidy was not found. Install clang-tidy and rerun CMake."
        COMMAND ${CMAKE_COMMAND} -E false
        COMMENT "Running clang-tidy"
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

qt_finalize_executable(ProceduralTextureMaker)
