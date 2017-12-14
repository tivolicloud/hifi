//
//  Created by Bradley Austin Davis on 2016/05/15
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_gl_GLShader_h
#define hifi_gpu_gl_GLShader_h

#include "d3d12shared.h"

namespace gpu { namespace d3d12 {

struct ShaderObject {
    int transformCameraSlot { -1 };
    int transformObjectSlot { -1 };
};

class D3DShader : public GPUObject {
public:
    static D3DShader* sync(D3D12Backend& backend, const Shader& shader);
    static bool makeProgram(D3D12Backend& backend, Shader& shader, const Shader::BindingSet& slotBindings);

    enum Version {
        Mono = 0,
        Stereo,

        NumVersions
    };

    // using ShaderObject = gpu::gl::ShaderObject;
    // using ShaderObjects = std::array< ShaderObject, NumVersions >;

    using UniformMapping = std::map<int, int>;
    using UniformMappingVersions = std::vector<UniformMapping>;

    D3DShader(const std::weak_ptr<D3D12Backend>& backend);
    ~D3DShader();

    // ShaderObjects _shaderObjects;
    UniformMappingVersions _uniformMappings;

#if 0
    unsigned int getProgram(Version version = Mono) const {
        return _shaderObjects[version].glprogram;
    }
#endif

    unsigned int getUniformLocation(int srcLoc, Version version = Mono) const {
        // This check protect against potential invalid src location for this shader, if unknown then return -1.
        const auto& mapping = _uniformMappings[version];
        auto found = mapping.find(srcLoc);
        if (found == mapping.end()) {
            return -1;
        }
        return found->second;
    }

    const std::weak_ptr<D3D12Backend> _backend;
};

} }


#endif
