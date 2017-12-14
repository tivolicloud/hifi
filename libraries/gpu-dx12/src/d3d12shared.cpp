//
//  Created by Bradley Austin Davis on 2016/05/14
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "d3d12shared.h"

#include <mutex>

#include <QtCore/QThread>

#include <GPUIdent.h>
#include <NumericalConstants.h>
#include <fstream>

Q_LOGGING_CATEGORY(gpugllogging, "hifi.gpu.gl")
Q_LOGGING_CATEGORY(trace_render_gpu_gl, "trace.render.gpu.gl")
Q_LOGGING_CATEGORY(trace_render_gpu_gl_detail, "trace.render.gpu.gl.detail")

namespace gpu 
{
    namespace gl {

        gpu::Size getFreeDedicatedMemory() {
            Size result{ 0 };
            return result;
        }

        gpu::Size getDedicatedMemory() {
            static Size dedicatedMemory{ 0 };
            static std::once_flag once;
            std::call_once(once, [&] {

                if (!dedicatedMemory) {
                    auto gpuIdent = GPUIdent::getInstance();
                    if (gpuIdent && gpuIdent->isValid()) {
                        dedicatedMemory = MB_TO_BYTES(gpuIdent->getMemory());
                    }
                }
            });

            return dedicatedMemory;
        }

        ComparisonFunction comparisonFuncFromGL(D3D12_COMPARISON_FUNC func) {
            if (func == D3D12_COMPARISON_FUNC_NEVER) {
                return NEVER;
            }
            else if (func == D3D12_COMPARISON_FUNC_LESS) {
                return LESS;
            }
            else if (func == D3D12_COMPARISON_FUNC_EQUAL) {
                return EQUAL;
            }
            else if (func == D3D12_COMPARISON_FUNC_LESS_EQUAL) {
                return LESS_EQUAL;
            }
            else if (func == D3D12_COMPARISON_FUNC_GREATER) {
                return GREATER;
            }
            else if (func == D3D12_COMPARISON_FUNC_NOT_EQUAL) {
                return NOT_EQUAL;
            }
            else if (func == D3D12_COMPARISON_FUNC_GREATER_EQUAL) {
                return GREATER_EQUAL;
            }
            else if (func == D3D12_COMPARISON_FUNC_ALWAYS) {
                return ALWAYS;
            }

            return ALWAYS;
        }

        State::StencilOp stencilOpFromGL(D3D12_STENCIL_OP stencilOp) {
            if (stencilOp == D3D12_STENCIL_OP_KEEP) {
                return State::STENCIL_OP_KEEP;
            }
            else if (stencilOp == D3D12_STENCIL_OP_ZERO) {
                return State::STENCIL_OP_ZERO;
            }
            else if (stencilOp == D3D12_STENCIL_OP_REPLACE) {
                return State::STENCIL_OP_REPLACE;
            }
            else if (stencilOp == D3D12_STENCIL_OP_INCR_SAT) {
                return State::STENCIL_OP_INCR_SAT;
            }
            else if (stencilOp == D3D12_STENCIL_OP_DECR_SAT) {
                return State::STENCIL_OP_DECR_SAT;
            }
            else if (stencilOp == D3D12_STENCIL_OP_INVERT) {
                return State::STENCIL_OP_INVERT;
            }
            else if (stencilOp == D3D12_STENCIL_OP_INCR) {
                return State::STENCIL_OP_INCR;
            }
            else if (stencilOp == D3D12_STENCIL_OP_DECR) {
                return State::STENCIL_OP_DECR;
            }

            return State::STENCIL_OP_KEEP;
        }

        State::BlendOp blendOpFromGL(D3D12_BLEND_OP blendOp) {
            if (blendOp == D3D12_BLEND_OP_ADD) {
                return State::BLEND_OP_ADD;
            }
            else if (blendOp == D3D12_BLEND_OP_SUBTRACT) {
                return State::BLEND_OP_SUBTRACT;
            }
            else if (blendOp == D3D12_BLEND_OP_REV_SUBTRACT) {
                return State::BLEND_OP_REV_SUBTRACT;
            }
            else if (blendOp == D3D12_BLEND_OP_MIN) {
                return State::BLEND_OP_MIN;
            }
            else if (blendOp == D3D12_BLEND_OP_MAX) {
                return State::BLEND_OP_MAX;
            }

            return State::BLEND_OP_ADD;
        }

        State::BlendArg blendArgFromGL(D3D12_BLEND blendArg) {
            if (blendArg == D3D12_BLEND_ZERO) {
                return State::ZERO;
            }
            else if (blendArg == D3D12_BLEND_ONE) {
                return State::ONE;
            }
            else if (blendArg == D3D12_BLEND_SRC_COLOR) {
                return State::SRC_COLOR;
            }
            else if (blendArg == D3D12_BLEND_INV_SRC_COLOR) {
                return State::INV_SRC_COLOR;
            }
            else if (blendArg == D3D12_BLEND_DEST_COLOR) {
                return State::DEST_COLOR;
            }
            else if (blendArg == D3D12_BLEND_INV_DEST_COLOR) {
                return State::INV_DEST_COLOR;
            }
            else if (blendArg == D3D12_BLEND_SRC_ALPHA) {
                return State::SRC_ALPHA;
            }
            else if (blendArg == D3D12_BLEND_INV_SRC_ALPHA) {
                return State::INV_SRC_ALPHA;
            }
            else if (blendArg == D3D12_BLEND_DEST_ALPHA) {
                return State::DEST_ALPHA;
            }
            else if (blendArg == D3D12_BLEND_INV_DEST_ALPHA) {
                return State::INV_DEST_ALPHA;
            }
            else if (blendArg == D3D12_BLEND_BLEND_FACTOR) {
                return State::FACTOR_COLOR;
            }
            else if (blendArg == D3D12_BLEND_INV_BLEND_FACTOR) {
                return State::INV_FACTOR_COLOR;
            }
            else if (blendArg == D3D12_BLEND_BLEND_FACTOR) {
                return State::FACTOR_ALPHA;
            }
            else if (blendArg == D3D12_BLEND_INV_BLEND_FACTOR) {
                return State::INV_FACTOR_ALPHA;
            }

            return State::ONE;
        }

        void serverWait()
        {
        }

        void clientWait()
        {
        }

    }
}

using namespace gpu;


