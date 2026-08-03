function(add_ptm_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE
        ptm_engine
        Qt6::Test
    )
    target_include_directories(${target} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}
    )
    add_test(NAME ${target} COMMAND ${target})
    set_tests_properties(${target} PROPERTIES
        TIMEOUT 30
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endfunction()

add_ptm_test(textureimage_test
    base/textureimage_test.cpp
)
set_tests_properties(textureimage_test PROPERTIES LABELS "base")

add_ptm_test(textureproject_test
    base/textureproject_test.cpp
    support/testgenerators.cpp
    support/testgenerators.h
)
set_tests_properties(textureproject_test PROPERTIES LABELS "base")

add_ptm_test(editmanager_test
    base/editmanager_test.cpp
    support/testgenerators.cpp
    support/testgenerators.h
)
set_tests_properties(editmanager_test PROPERTIES LABELS "base")

add_ptm_test(projectfileservice_test
    io/projectfileservice_test.cpp
)
target_compile_definitions(projectfileservice_test PRIVATE
    PTM_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
)
set_tests_properties(projectfileservice_test PROPERTIES LABELS "base;persistence")

add_ptm_test(texturerendermanager_test
    base/texturerendermanager_test.cpp
    support/testgenerators.cpp
    support/testgenerators.h
)
set_tests_properties(texturerendermanager_test PROPERTIES LABELS "base;render")

add_ptm_test(settingsmanager_test
    base/settingsmanager_test.cpp
)
set_tests_properties(settingsmanager_test PROPERTIES LABELS "base")

add_ptm_test(builtin_generators_test
    generators/builtin_generators_test.cpp
)
set_tests_properties(builtin_generators_test PROPERTIES LABELS "generators")
set_tests_properties(builtin_generators_test PROPERTIES TIMEOUT 60)

add_ptm_test(javascript_generators_test
    generators/javascript_generators_test.cpp
)
set_tests_properties(javascript_generators_test PROPERTIES LABELS "generators")

add_executable(javascript_generators_benchmark
    generators/javascript_generators_benchmark.cpp
)
target_link_libraries(javascript_generators_benchmark PRIVATE ptm_engine)
target_include_directories(javascript_generators_benchmark PRIVATE ${PROJECT_SOURCE_DIR})

add_ptm_test(cli_export_test
    cli/cli_export_test.cpp
)
target_compile_definitions(cli_export_test PRIVATE
    PTM_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
    PTM_APPLICATION_EXECUTABLE="$<TARGET_FILE:ProceduralTextureMaker>"
)
add_dependencies(cli_export_test ProceduralTextureMaker)
set_tests_properties(cli_export_test PROPERTIES LABELS "cli")
set_tests_properties(cli_export_test PROPERTIES TIMEOUT 90)

if(PROCEDURAL_TEXTURE_MAKER_UI_TESTS)
    add_executable(ui_smoke_test ui/ui_smoke_test.cpp)
    target_link_libraries(ui_smoke_test PRIVATE ptm_gui Qt6::Test)
    target_compile_definitions(ui_smoke_test PRIVATE PTM_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
    add_test(NAME ui_smoke_test COMMAND ui_smoke_test)
    set_tests_properties(ui_smoke_test PROPERTIES
        LABELS "ui"
        TIMEOUT 30
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endif()
