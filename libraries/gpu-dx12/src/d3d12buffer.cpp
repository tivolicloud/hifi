//
//  Created by Bradley Austin Davis on 2016/05/15
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "d3d12buffer.h"
#include "d3d12backend.h"

using namespace gpu;
using namespace gpu::d3d12;

D3DBuffer::~D3DBuffer() {
    Backend::bufferCount.decrement();
    Backend::bufferGPUMemSize.update(_size, 0);

    if (_id) {
        auto backend = _backend.lock();
        if (backend) {
            backend->releaseBuffer(_id, _size);
        }
    }
}

D3DBuffer::D3DBuffer(const std::weak_ptr<D3D12Backend>& backend, const Buffer& buffer, unsigned int id) :
    DxObject(backend, buffer, id),
    _size((unsigned int)buffer._renderSysmem.getSize()),
    _stamp(buffer._renderSysmem.getStamp())
{
    Backend::bufferCount.increment();
    Backend::bufferGPUMemSize.update(0, _size);
}

