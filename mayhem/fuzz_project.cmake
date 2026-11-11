# Included from the end of upstream's top-level CMakeLists.txt via CMAKE_PROJECT_INCLUDE (see build.sh),
# so the source lists stay upstream's own. Builds the headless game core (the same file set as the
# upstream `autopilot` test, with its stubs) as one static library for the fuzz harnesses.
function(mayhem_except_file var excluded_file)
    set(list_var "")
    foreach(f ${ARGN})
        if(NOT ${f} MATCHES ".*${excluded_file}")
            list(APPEND list_var ${f})
        endif()
    endforeach()
    set(${var} "${list_var}" PARENT_SCOPE)
endfunction()

mayhem_except_file(MAYHEM_CORE_FILES "core/image.c" ${CORE_FILES})
mayhem_except_file(MAYHEM_CORE_FILES "core/lang.c" ${MAYHEM_CORE_FILES})
mayhem_except_file(MAYHEM_CORE_FILES "core/speed.c" ${MAYHEM_CORE_FILES})
mayhem_except_file(MAYHEM_BUILDING_FILES "building/model.c" ${BUILDING_FILES})

set(T ${PROJECT_SOURCE_DIR}/test/stub)
add_library(julius_fuzzcore STATIC EXCLUDE_FROM_ALL
    ${T}/image.c
    ${T}/input.c
    ${T}/lang.c
    ${PROJECT_SOURCE_DIR}/mayhem/stub_log.c
    ${T}/model.c
    ${T}/sound_device.c
    ${T}/ui.c
    ${T}/video.c
    ${PROJECT_SOURCE_DIR}/src/platform/file_manager.c
    ${MAYHEM_CORE_FILES}
    ${MAYHEM_BUILDING_FILES}
    ${CITY_FILES}
    ${EMPIRE_FILES}
    ${FIGURE_FILES}
    ${FIGURETYPE_FILES}
    ${GAME_FILES}
    ${MAP_FILES}
    ${SCENARIO_FILES}
    ${SOUND_FILES}
    ${EDITOR_FILES}
)
