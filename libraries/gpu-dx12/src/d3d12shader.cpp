//
//  Created by Bradley Austin Davis on 2016/05/15
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "d3d12shader.h"
// #include <gl/GLShaders.h>

#include "d3d12backend.h"

using namespace gpu;
using namespace gpu::d3d12;

D3DShader::D3DShader(const std::weak_ptr<D3D12Backend>& backend) : _backend(backend) {
}

D3DShader::~D3DShader()
{
}
 
D3DShader* D3DShader::sync(D3D12Backend& backend, const Shader& shader) 
{
    D3DShader* object = Backend::getGPUObject<D3DShader>(shader);

    // If GPU object already created then good
    if (object) {
        return object;
    }
    // need to have a gpu object?
    if (shader.isProgram()) {
        D3DShader* tempObject = nullptr; // TODO: Fixme backend.compileBackendProgram(shader);
        if (tempObject) {
            object = tempObject;
            Backend::setGPUObject(shader, object);
        }
    } else if (shader.isDomain()) {
        D3DShader* tempObject = nullptr; // TODO: Fixme backend.compileBackendShader(shader);
        if (tempObject) {
            object = tempObject;
            Backend::setGPUObject(shader, object);
        }
    }

    return nullptr;
}

bool D3DShader::makeProgram(D3D12Backend& backend, Shader& shader, const Shader::BindingSet& slotBindings) 
{
    // First make sure the Shader has been compiled
    D3DShader* object = sync(backend, shader);
    if (!object) {
        return false;
    }

#if 0
    // Apply bindings to all program versions and generate list of slots from default version
    for (int version = 0; version < GLShader::NumVersions; version++) {
        auto& shaderObject = object->_shaderObjects[version];
        if (shaderObject.glprogram) {
            Shader::SlotSet buffers;
            // backend.makeUniformBlockSlots(shaderObject.glprogram, slotBindings, buffers);

            Shader::SlotSet uniforms;
            Shader::SlotSet textures;
            Shader::SlotSet samplers;
            // backend.makeUniformSlots(shaderObject.glprogram, slotBindings, uniforms, textures, samplers);

            Shader::SlotSet resourceBuffers;
            // backend.makeResourceBufferSlots(shaderObject.glprogram, slotBindings, resourceBuffers);

            Shader::SlotSet inputs;
            // backend.makeInputSlots(shaderObject.glprogram, slotBindings, inputs);

            Shader::SlotSet outputs;
            // backend.makeOutputSlots(shaderObject.glprogram, slotBindings, outputs);

            // Define the public slots only from the default version
            if (version == 0) {
                shader.defineSlots(uniforms, buffers, resourceBuffers, textures, samplers, inputs, outputs);
            } // else
            {
                D3DShader::UniformMapping mapping;
                for (auto srcUniform : shader.getUniforms()) {
                    mapping[srcUniform._location] = uniforms.findLocation(srcUniform._name);
                }
                object->_uniformMappings.push_back(mapping);
            }
        }
    }
#endif

    return true;
}



