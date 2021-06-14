function(append_list_defines_system DEFINES_LIST)
    if(MSVC)
        message(STATUS "Add define: CRT_SECURE_NO_WARNINGS")
        list(APPEND DEFINES "-D_CRT_SECURE_NO_WARNINGS")
    endif()
    if(WIN32)
        message(STATUS "Add define: OS_WINDOWS")
        list(APPEND DEFINES "-DOS_WINDOWS")
    elseif(UNIX)
        message(STATUS "Add define: OS_UNIX")
        list(APPEND DEFINES "-DOS_UNIX")
    else()
        message(SEND_ERROR "Undefined system, please check ${CMAKE_CURRENT_LIST_FILE}")
    endif()
    set(${DEFINES_LIST} ${DEFINES} PARENT_SCOPE)
endfunction()

function(copy_compile_commands TARGET)
    if(UNIX)
        message(STATUS "\t\tGenerate compile_commands.json for ${TARGET}")
        set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
        add_custom_target(
                copy-compile-commands-${TARGET} ALL
                ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_CURRENT_SOURCE_DIR})
    endif(UNIX)
endfunction(post_build)