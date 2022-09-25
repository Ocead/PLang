function(compile_resource TARGET RESOURCE_COMPILER PATH)
    file(GLOB_RECURSE RESOURCES "${PATH}/*")
    foreach (INPUT_FILE ${RESOURCES})
        if (NOT ${INPUT_FILE} MATCHES ".*.cpp" AND NOT IS_DIRECTORY ${INPUT_FILE})
            get_filename_component(RES_PATH ${INPUT_FILE} DIRECTORY)
            file(RELATIVE_PATH INPUT_FILE_NAME "${PATH}" ${INPUT_FILE})
            string(REPLACE "/" "_" OUTPUT_FILE_NAME "${INPUT_FILE_NAME}")
            string(REPLACE "\\" "_" OUTPUT_FILE_NAME "${OUTPUT_FILE_NAME}")
            set(OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/binary/${OUTPUT_FILE_NAME}.cpp)
            add_custom_command(
                    OUTPUT ${OUTPUT_FILE}
                    COMMAND ${RESOURCE_COMPILER} -i ${INPUT_FILE_NAME} ${OUTPUT_FILE}
                    WORKING_DIRECTORY ${PATH}
                    COMMENT "Creating CXX source file ${OUTPUT_FILE_NAME}.cpp from ${INPUT_FILE_NAME}"
                    DEPENDS ${INPUT_FILE})
            list(APPEND RESOURCES ${OUTPUT_FILE})
        endif ()
    endforeach ()
    set(${TARGET} ${RESOURCES} PARENT_SCOPE)
endfunction()
