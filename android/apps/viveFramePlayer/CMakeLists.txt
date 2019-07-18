set(TARGET_NAME viveFramePlayer)
# don't use the setup_hifi_library macro, we don't want ANY qt dependencies
file(GLOB_RECURSE LIB_SRCS "src/main/cpp/*.h" "src/main/cpp/*.cpp" "src/main/cpp/*.c")
add_library(${TARGET_NAME} SHARED ${LIB_SRCS})
#setup_hifi_library()
#link_hifi_libraries(shared ktx shaders gpu gl ${PLATFORM_GL_BACKEND})

target_include_directories(${TARGET_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/object"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/scene"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/shared"
)
target_link_libraries(${TARGET_NAME} android log m jnigraphics)
target_opengl()
target_vive_focus()


