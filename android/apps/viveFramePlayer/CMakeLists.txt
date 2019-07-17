set(TARGET_NAME viveFramePlayer)
setup_hifi_library(AndroidExtras)
link_hifi_libraries(shared ktx shaders gpu gl ${PLATFORM_GL_BACKEND})

target_link_libraries(${TARGET_NAME} android log m)
target_opengl()
target_vive_focus()


