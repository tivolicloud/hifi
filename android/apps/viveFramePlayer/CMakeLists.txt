set(TARGET_NAME viveFramePlayer)
setup_hifi_library()
#link_hifi_libraries(shared ktx shaders gpu gl ${PLATFORM_GL_BACKEND})

target_include_directories(${TARGET_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/object"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/scene"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/shared"
)
target_link_libraries(${TARGET_NAME} android log m)
target_opengl()
target_vive_focus()


