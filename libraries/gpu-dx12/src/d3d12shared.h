//
//  Created by Bradley Austin Davis on 2016/05/15
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_GLShared_h
#define hifi_gpu_GLShared_h

// #include <gl/Config.h>
#include <d3d12.h>
#include <wrl/client.h>
#include <dxgiformat.h>
#include <gpu/Forward.h>
#include <gpu/Format.h>
#include <gpu/Context.h>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(gpugllogging)
Q_DECLARE_LOGGING_CATEGORY(trace_render_gpu_gl)
Q_DECLARE_LOGGING_CATEGORY(trace_render_gpu_gl_detail)

#define BUFFER_OFFSET(bytes) ((GLubyte*) nullptr + (bytes))

namespace gpu
{
    namespace d3d12
    {
        // Create a fence and inject a GPU wait on the fence
        
        void serverWait();

        // Create a fence and synchronously wait on the fence
        void clientWait();

        gpu::Size getDedicatedMemory();
        gpu::Size getFreeDedicatedMemory();
        ComparisonFunction comparisonFuncFromGL(D3D12_COMPARISON_FUNC func);
        State::StencilOp stencilOpFromGL(D3D12_STENCIL_OP stencilOp);
        State::BlendOp blendOpFromGL(D3D12_BLEND_OP blendOp);
        State::BlendArg blendArgFromGL(D3D12_BLEND blendArg);
        void getCurrentGLState(State::Data& state);

        enum GLSyncState {
            // The object is currently undergoing no processing, although it's content
            // may be out of date, or it's storage may be invalid relative to the 
            // owning GPU object
            Idle,
            // The object has been queued for transfer to the GPU
            Pending,
            // The object has been transferred to the GPU, but is awaiting
            // any post transfer operations that may need to occur on the 
            // primary rendering thread
            Transferred,
        };

        static const D3D12_BLEND_OP BLEND_OPS_TO_GL[State::NUM_BLEND_OPS] = {
            D3D12_BLEND_OP_ADD,
            D3D12_BLEND_OP_SUBTRACT,
            D3D12_BLEND_OP_REV_SUBTRACT,
            D3D12_BLEND_OP_MIN,
            D3D12_BLEND_OP_MAX
        };

        static const D3D12_BLEND BLEND_ARGS_TO_GL[State::NUM_BLEND_ARGS] = {
            D3D12_BLEND_ZERO,
            D3D12_BLEND_ONE,
            D3D12_BLEND_SRC_COLOR,
            D3D12_BLEND_INV_SRC_COLOR,
            D3D12_BLEND_SRC_ALPHA,
            D3D12_BLEND_INV_SRC_ALPHA,
            D3D12_BLEND_DEST_ALPHA,
            D3D12_BLEND_INV_DEST_ALPHA,
            D3D12_BLEND_DEST_COLOR,
            D3D12_BLEND_INV_DEST_COLOR,
            D3D12_BLEND_SRC_ALPHA_SAT,
            D3D12_BLEND_BLEND_FACTOR,
            D3D12_BLEND_INV_BLEND_FACTOR,
            D3D12_BLEND_BLEND_FACTOR,
            D3D12_BLEND_INV_BLEND_FACTOR,
        };

        static const D3D12_COMPARISON_FUNC COMPARISON_TO_GL[gpu::NUM_COMPARISON_FUNCS] = {
            D3D12_COMPARISON_FUNC_NEVER,
            D3D12_COMPARISON_FUNC_LESS,
            D3D12_COMPARISON_FUNC_EQUAL,
            D3D12_COMPARISON_FUNC_LESS_EQUAL,
            D3D12_COMPARISON_FUNC_GREATER,
            D3D12_COMPARISON_FUNC_NOT_EQUAL,
            D3D12_COMPARISON_FUNC_GREATER_EQUAL,
            D3D12_COMPARISON_FUNC_ALWAYS
        };

        // TODO: D3D12 does not support fans 
        static const D3D12_PRIMITIVE_TOPOLOGY PRIMITIVE_TO_GL[gpu::NUM_PRIMITIVES] = {
            D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
            D3D_PRIMITIVE_TOPOLOGY_LINELIST,
            D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
        };

        static const DXGI_FORMAT ELEMENT_TYPE_TO_GL[gpu::NUM_TYPES] = {
            DXGI_FORMAT_R32_FLOAT,
            DXGI_FORMAT_R32_SINT,
            DXGI_FORMAT_R32_UINT,
            DXGI_FORMAT_R16_FLOAT,
            DXGI_FORMAT_R16_SINT,
            DXGI_FORMAT_R16_UINT,
            DXGI_FORMAT_R8_SINT,
            DXGI_FORMAT_R8_UINT,

            // Normalized values
            DXGI_FORMAT_R32_SINT,
            DXGI_FORMAT_R32_UINT,
            DXGI_FORMAT_R16_SINT,
            DXGI_FORMAT_R16_UINT,
            DXGI_FORMAT_R8_SINT,
            DXGI_FORMAT_R8_UINT
        };

        bool checkGLError(const char* name = nullptr);
        bool checkGLErrorDebug(const char* name = nullptr);

        class D3D12Backend;

        template <typename GPUType>
        struct DxObject : public GPUObject {
        public:
            DxObject(const std::weak_ptr<D3D12Backend>& backend, const GPUType& gpuObject, unsigned int object_id) : _gpuObject(gpuObject), _id(object_id), _backend(backend) {}

            virtual ~DxObject() { }

            const GPUType& _gpuObject;
            const unsigned int _id;
        protected:
            const std::weak_ptr<D3D12Backend> _backend;
        };

        class d3d12Buffer;
        class d3d12Framebuffer;
        class GLPipeline;
        class GLQuery;
        class GLState;
        class D3DShader;
        class D3D12Texture;
        struct ShaderObject;

    }
}

#endif



