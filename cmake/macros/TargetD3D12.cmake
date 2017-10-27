# 
#  Copyright 2017 High Fidelity, Inc.
#  Created by Tom Keresztes on 2017/10/10
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
# 
macro(TARGET_D3D12)
    if (APPLE)
			# Not applicable
    elseif(ANDROID)
		  # Not applicable
    else()
      find_package(OpenGL REQUIRED)
      if (${OPENGL_INCLUDE_DIR})
        include_directories(SYSTEM "${OPENGL_INCLUDE_DIR}")
      endif()
      target_link_libraries(${TARGET_NAME} "${OPENGL_LIBRARY}" d3dcompiler dxgi d3d12)
      target_include_directories(${TARGET_NAME} PUBLIC ${OPENGL_INCLUDE_DIR})
    endif()
    target_nsight()
endmacro()
