cmake_minimum_required(VERSION 3.11.0)
include(FetchContent)

function(ExternalComponent_Get name git_repository git_ref) 
    FetchContent_Declare(${name}
                        GIT_REPOSITORY ${git_repository}
                        GIT_TAG ${git_ref}
                        SOURCE_DIR ${CMAKE_BINARY_DIR}/_components/${name})
    FetchContent_GetProperties(${name})
    string(TOLOWER ${name} nameLower)
    if(NOT ${name}_POPULATED)
        FetchContent_Populate(${name})
    endif()

    list(APPEND EXTRA_COMPONENT_DIRS ${CMAKE_BINARY_DIR}/_components)
    list(REMOVE_DUPLICATES EXTRA_COMPONENT_DIRS)
    set(EXTRA_COMPONENT_DIRS ${EXTRA_COMPONENT_DIRS} PARENT_SCOPE)
endfunction()
