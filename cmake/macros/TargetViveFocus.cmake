macro(target_vive_focus)
    set(INSTALL_DIR ${HIFI_ANDROID_PRECOMPILED}/vive_3.0.104)

    # Mobile SDK
    set(VIVE_FOCUS_INCLUDE_DIRS ${INSTALL_DIR}/include)
    target_include_directories(${TARGET_NAME} PRIVATE ${VIVE_FOCUS_INCLUDE_DIRS})
    set(VIVE_FOCUS_LIBRARY_DIR  ${INSTALL_DIR}/jni/arm64-v8a)
    
    # ????  which of these are required?
    set(VIVE_FOCUS_LIB_NAMES dummy overlay_api svrapi vrcore vr_jni wvr_api wvr_ctf_internal_common wvr_internal_using_assimp wvr_monitor wvr_runtime)
    foreach(LIB ${VIVE_FOCUS_LIB_NAMES})
        list(APPEND VIVE_FOCUS_LIBRARIES ${VIVE_FOCUS_LIBRARY_DIR}/lib${LIB}.so)
    endforeach()
    target_link_libraries(${TARGET_NAME} ${VIVE_FOCUS_LIBRARIES})
endmacro()
