//
//  Created by Bradley Austin Davis on 2016/05/15
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_gl_GLFramebuffer_h
#define hifi_gpu_gl_GLFramebuffer_h

#include "d3d12shared.h"
#include "d3d12backend.h"

namespace gpu { namespace d3d12 {

class d3d12Framebuffer : public DxObject<Framebuffer> {
public:
    template <typename d3d12FramebufferType>
    static d3d12FramebufferType* sync(D3D12Backend& backend, const Framebuffer& framebuffer) {
        d3d12FramebufferType* object = Backend::getGPUObject<d3d12FramebufferType>(framebuffer);

        bool needsUpate { false };
        if (!object ||
            framebuffer.getDepthStamp() != object->_depthStamp ||
            framebuffer.getColorStamps() != object->_colorStamps) {
            needsUpate = true;
        }

        // If GPU object already created and in sync
        if (!needsUpate) {
            return object;
        } else if (framebuffer.isEmpty()) {
            // NO framebuffer definition yet so let's avoid thinking
            return nullptr;
        }

        // need to have a gpu object?
        if (!object) {
            // All is green, assign the gpuobject to the Framebuffer
            object = new d3d12FramebufferType(backend.shared_from_this(), framebuffer);
            Backend::setGPUObject(framebuffer, object);
            (void)CHECK_GL_ERROR();
        }

        object->update();
        return object;
    }

    template <typename d3d12FramebufferType>
    static unsigned int getId(D3D12Backend& backend, const Framebuffer& framebuffer) {
        d3d12FramebufferType* fbo = sync<d3d12FramebufferType>(backend, framebuffer);
        if (fbo) {
            return fbo->_id;
        } else {
            return 0;
        }
    }

    const unsigned int& _fbo { _id };
    std::vector<int> _colorBuffers;
    Stamp _depthStamp { 0 };
    std::vector<Stamp> _colorStamps;

protected:
    virtual void update() = 0;
    bool checkStatus() const;

    d3d12Framebuffer(const std::weak_ptr<D3D12Backend>& backend, const Framebuffer& framebuffer, unsigned int id) : DxObject(backend, framebuffer, id) {}
    ~d3d12Framebuffer();

};

} }


#endif
