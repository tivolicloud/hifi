set(TARGET_NAME directx12)
setup_hifi_library(Concurrent)
link_hifi_libraries(shared)
GroupSources("src")
target_d3d12()


