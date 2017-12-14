//
//  Created by Bradley Austin Davis on 2016/05/15
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "d3d12framebuffer.h"
#include "d3d12Backend.h"

using namespace gpu;
using namespace gpu::d3d12;

d3d12Framebuffer::~d3d12Framebuffer() {
    if (_id) {
        auto backend = _backend.lock();
        if (backend) {
            backend->releaseFramebuffer(_id);
        }
    } 
}

bool d3d12Framebuffer::checkStatus() const {
    // TODO: Implement transitions
    return false;
}
