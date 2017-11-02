set(TARGET_NAME gpu-dx12)
setup_hifi_library(Concurrent)
link_hifi_libraries(shared gpu)
if (UNIX)
    target_link_libraries(${TARGET_NAME} pthread)
endif(UNIX)
GroupSources("src")
target_d3d12()

