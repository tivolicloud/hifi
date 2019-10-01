# If we're running under the gradle build, HIFI_ANDROID will be set here, but
# ANDROID will not be set until after the `project` statement.  This is the *ONLY*
# place you need to use `HIFI_ANDROID` instead of `ANDROID`
if (WIN32 AND NOT HIFI_ANDROID)
  cmake_minimum_required(VERSION 3.7)
else()
  message(FATAL_ERROR "Don't build")
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/macros/TargetPython.cmake")
target_python()

execute_process(
    COMMAND ${HIFI_PYTHON_EXEC} ${CMAKE_CURRENT_SOURCE_DIR}/prebuild.py --release-type ${RELEASE_TYPE} --build-root ${CMAKE_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

message(FATAL_ERROR "Shhh")
