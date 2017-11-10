//
//  D3D12Backend.cpp
//  libraries/gpu/src/gpu
//
//  Created by Sam Gateau on 10/27/2014.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "d3d12backend.h"

#include <mutex>
#include <queue>
#include <list>
#include <functional>
#include <glm/gtc/type_ptr.hpp>
#include <dxgi1_4.h>

#if defined(NSIGHT_FOUND)
#include "nvToolsExt.h"
#endif

#include <shared/GlobalAppProperties.h>
#include <GPUIdent.h>
#include <QtCore/QProcessEnvironment>

#include "d3d12texture.h"
#include "d3d12shader.h"

using namespace gpu;
using namespace gpu::d3d12;

static const QString DEBUG_FLAG("HIFI_DISABLE_OPENGL_45");
static bool disableOpenGL45 = QProcessEnvironment::systemEnvironment().contains(DEBUG_FLAG);

static D3D12Backend* INSTANCE { nullptr };

namespace 
{
    static void getHardwareAdapter(IDXGIFactory1 *factory, IDXGIAdapter1 **outAdapter, D3D_FEATURE_LEVEL featureLevel)
    {
        Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;

        for (int adapterIndex = 0; factory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND; ++adapterIndex) {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                continue;

            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), featureLevel, _uuidof(ID3D12Device), Q_NULLPTR))) {
                const QString name = QString::fromUtf16((char16_t *)desc.Description);
                qDebug("Using adapter '%s'", qPrintable(name));
                break;
            }
        }

        *outAdapter = adapter.Detach();
    }
}


BackendPointer D3D12Backend::createBackend() {
    // The ATI memory info extension only exposes 'free memory' so we want to force it to 
    // cache the value as early as possible
    getDedicatedMemory();

    // FIXME provide a mechanism to override the backend for testing
    // Where the gpuContext is initialized and where the TRUE Backend is created and assigned
    // TODO: Feature levels ? 
    std::shared_ptr<D3D12Backend> result = std::make_shared<gpu::d3d12::D3D12Backend>();
    result->init();
    result->initInput();
    result->initTransform();
    result->initTextureManagementStage();

    INSTANCE = result.get();
    void* voidInstance = &(*result);
    qApp->setProperty(hifi::properties::gl::BACKEND, QVariant::fromValue(voidInstance));
    return result;
}

D3D12Backend& getBackend() {
    if (!INSTANCE) {
        INSTANCE = static_cast<D3D12Backend*>(qApp->property(hifi::properties::gl::BACKEND).value<void*>());
    }
    return *INSTANCE;
}

bool D3D12Backend::makeProgram(Shader& shader, const Shader::BindingSet& slotBindings) {
    // FIXME: implement this
    // return GLShader::makeProgram(getBackend(), shader, slotBindings);
    return false;
}

D3D12Backend::CommandCall D3D12Backend::_commandCalls[Batch::NUM_COMMANDS] =
{
    (&::gpu::d3d12::D3D12Backend::do_draw),
    (&::gpu::d3d12::D3D12Backend::do_drawIndexed),
    (&::gpu::d3d12::D3D12Backend::do_drawInstanced),
    (&::gpu::d3d12::D3D12Backend::do_drawIndexedInstanced),
    (&::gpu::d3d12::D3D12Backend::do_multiDrawIndirect),
    (&::gpu::d3d12::D3D12Backend::do_multiDrawIndexedIndirect),

    (&::gpu::d3d12::D3D12Backend::do_setInputFormat),
    (&::gpu::d3d12::D3D12Backend::do_setInputBuffer),
    (&::gpu::d3d12::D3D12Backend::do_setIndexBuffer),
    (&::gpu::d3d12::D3D12Backend::do_setIndirectBuffer),

    (&::gpu::d3d12::D3D12Backend::do_setModelTransform),
    (&::gpu::d3d12::D3D12Backend::do_setViewTransform),
    (&::gpu::d3d12::D3D12Backend::do_setProjectionTransform),
    (&::gpu::d3d12::D3D12Backend::do_setViewportTransform),
    (&::gpu::d3d12::D3D12Backend::do_setDepthRangeTransform),

    (&::gpu::d3d12::D3D12Backend::do_setPipeline),
    (&::gpu::d3d12::D3D12Backend::do_setStateBlendFactor),
    (&::gpu::d3d12::D3D12Backend::do_setStateScissorRect),

    (&::gpu::d3d12::D3D12Backend::do_setUniformBuffer),
    (&::gpu::d3d12::D3D12Backend::do_setResourceBuffer),
    (&::gpu::d3d12::D3D12Backend::do_setResourceTexture),

    (&::gpu::d3d12::D3D12Backend::do_setFramebuffer),
    (&::gpu::d3d12::D3D12Backend::do_clearFramebuffer),
    (&::gpu::d3d12::D3D12Backend::do_blit),
    (&::gpu::d3d12::D3D12Backend::do_generateTextureMips),

    (&::gpu::d3d12::D3D12Backend::do_beginQuery),
    (&::gpu::d3d12::D3D12Backend::do_endQuery),
    (&::gpu::d3d12::D3D12Backend::do_getQuery),

    (&::gpu::d3d12::D3D12Backend::do_resetStages),
    
    (&::gpu::d3d12::D3D12Backend::do_disableContextViewCorrection),
    (&::gpu::d3d12::D3D12Backend::do_restoreContextViewCorrection),
    (&::gpu::d3d12::D3D12Backend::do_disableContextStereo),
    (&::gpu::d3d12::D3D12Backend::do_restoreContextStereo),

    (&::gpu::d3d12::D3D12Backend::do_runLambda),

    (&::gpu::d3d12::D3D12Backend::do_startNamedCall),
    (&::gpu::d3d12::D3D12Backend::do_stopNamedCall),

    (&::gpu::d3d12::D3D12Backend::do_glUniform1i),
    (&::gpu::d3d12::D3D12Backend::do_glUniform1f),
    (&::gpu::d3d12::D3D12Backend::do_glUniform2f),
    (&::gpu::d3d12::D3D12Backend::do_glUniform3f),
    (&::gpu::d3d12::D3D12Backend::do_glUniform4f),
    (&::gpu::d3d12::D3D12Backend::do_glUniform3fv),
    (&::gpu::d3d12::D3D12Backend::do_glUniform4fv),
    (&::gpu::d3d12::D3D12Backend::do_glUniform4iv),
    (&::gpu::d3d12::D3D12Backend::do_glUniformMatrix3fv),
    (&::gpu::d3d12::D3D12Backend::do_glUniformMatrix4fv),

    (&::gpu::d3d12::D3D12Backend::do_glColor4f),

    (&::gpu::d3d12::D3D12Backend::do_pushProfileRange),
    (&::gpu::d3d12::D3D12Backend::do_popProfileRange),
};

void D3D12Backend::init() 
{
    // TODO: Fix this. 
    // Removed the init_once as it's possible to have more than adapter in the system.
    QString vendor;
    QString renderer;
    GPUIdent* gpu = GPUIdent::getInstance(vendor, renderer);
    // From here on, GPUIdent::getInstance()->getMumble() should efficiently give the same answers.
    // qCDebug(gpugllogging) << "GPU:";
    // qCDebug(gpugllogging) << "\tcard:" << gpu->getName();
    // qCDebug(gpugllogging) << "\tdriver:" << gpu->getDriver();
    // qCDebug(gpugllogging) << "\tdedicated memory:" << gpu->getMemory() << "MB";

    Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();
    }

    Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
    if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)))) {
        qWarning("Failed to create DXGI");
        return;
    }

    D3D_FEATURE_LEVEL desiredFeatureLevel = D3D_FEATURE_LEVEL_12_1;
    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
    getHardwareAdapter(factory.Get(), &adapter, desiredFeatureLevel);

    bool warp = true;
    if (adapter) {
        HRESULT hr = D3D12CreateDevice(adapter.Get(), desiredFeatureLevel, IID_PPV_ARGS(&_device));
        if (SUCCEEDED(hr))
            warp = false;
        else
            qWarning("Failed to create device: 0x%x", hr);
    }
    else {
        qWarning("No usable hardware adapters found");
    }

    if (warp) {
        qDebug("Using WARP");
        Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
        factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
        HRESULT hr = D3D12CreateDevice(warpAdapter.Get(), desiredFeatureLevel, IID_PPV_ARGS(&_device));
        if (FAILED(hr)) {
            qWarning("Failed to create WARP device: 0x%x", hr);
            return;
        }
    }

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    if (FAILED(_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_commandQueue)))) {
        qWarning("Failed to create command queue");
        return;
    }


#if THREADED_TEXTURE_BUFFERING
        // This has to happen on the main thread in order to give the thread 
        // pool a reasonable parent object
    GLVariableAllocationSupport::TransferJob::startBufferingThread();
#endif
}

D3D12Backend::D3D12Backend(bool syncCache) {
    // TODO: Fixme _pipeline._cameraCorrectionBuffer._buffer->flush();
    // glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &_uboAlignment);
}

D3D12Backend::D3D12Backend() {
}


D3D12Backend::~D3D12Backend() {
    killInput();
    killTransform();
}

void D3D12Backend::renderPassTransfer(const Batch& batch) {
    const size_t numCommands = batch.getCommands().size();
    const Batch::Commands::value_type* command = batch.getCommands().data();
    const Batch::CommandOffsets::value_type* offset = batch.getCommandOffsets().data();

    _inRenderTransferPass = true;
    { // Sync all the buffers
        PROFILE_RANGE(render_gpu_gl_detail, "syncGPUBuffer");

        for (auto& cached : batch._buffers._items) {
            if (cached._data) {
                syncGPUObject(*cached._data);
            }
        }
    }

    { // Sync all the transform states
        PROFILE_RANGE(render_gpu_gl_detail, "syncCPUTransform");
        _transform._cameras.clear();
        _transform._cameraOffsets.clear();

        for (_commandIndex = 0; _commandIndex < numCommands; ++_commandIndex) {
            switch (*command) {
                case Batch::COMMAND_draw:
                case Batch::COMMAND_drawIndexed:
                case Batch::COMMAND_drawInstanced:
                case Batch::COMMAND_drawIndexedInstanced:
                case Batch::COMMAND_multiDrawIndirect:
                case Batch::COMMAND_multiDrawIndexedIndirect:
                    _transform.preUpdate(_commandIndex, _stereo);
                    break;

                case Batch::COMMAND_disableContextStereo:
                    _stereo._contextDisable = true;
                    break;

                case Batch::COMMAND_restoreContextStereo:
                    _stereo._contextDisable = false;
                    break;

                case Batch::COMMAND_setViewportTransform:
                case Batch::COMMAND_setViewTransform:
                case Batch::COMMAND_setProjectionTransform: {
                    CommandCall call = _commandCalls[(*command)];
                    (this->*(call))(batch, *offset);
                    break;
                }

                default:
                    break;
            }
            command++;
            offset++;
        }
    }

    { // Sync the transform buffers
        PROFILE_RANGE(render_gpu_gl_detail, "syncGPUTransform");
        transferTransformState(batch);
    }

    _inRenderTransferPass = false;
}

void D3D12Backend::renderPassDraw(const Batch& batch) {
    _currentDraw = -1;
    _transform._camerasItr = _transform._cameraOffsets.begin();
    const size_t numCommands = batch.getCommands().size();
    const Batch::Commands::value_type* command = batch.getCommands().data();
    const Batch::CommandOffsets::value_type* offset = batch.getCommandOffsets().data();
    for (_commandIndex = 0; _commandIndex < numCommands; ++_commandIndex) {
        switch (*command) {
            // Ignore these commands on this pass, taken care of in the transfer pass
            // Note we allow COMMAND_setViewportTransform to occur in both passes
            // as it both updates the transform object (and thus the uniforms in the 
            // UBO) as well as executes the actual viewport call
            case Batch::COMMAND_setModelTransform:
            case Batch::COMMAND_setViewTransform:
            case Batch::COMMAND_setProjectionTransform:
                break;

            case Batch::COMMAND_draw:
            case Batch::COMMAND_drawIndexed:
            case Batch::COMMAND_drawInstanced:
            case Batch::COMMAND_drawIndexedInstanced:
            case Batch::COMMAND_multiDrawIndirect:
            case Batch::COMMAND_multiDrawIndexedIndirect: {
                // updates for draw calls
                ++_currentDraw;
                updateInput();
                updateTransform(batch);
                updatePipeline();

                CommandCall call = _commandCalls[(*command)];
                (this->*(call))(batch, *offset);
                break;
            }
            default: {
                CommandCall call = _commandCalls[(*command)];
                (this->*(call))(batch, *offset);
                break;
            }
        }

        command++;
        offset++;
    }
}

void D3D12Backend::render(const Batch& batch) {
    _transform._skybox = _stereo._skybox = batch.isSkyboxEnabled();
    // Allow the batch to override the rendering stereo settings
    // for things like full framebuffer copy operations (deferred lighting passes)
    bool savedStereo = _stereo._enable;
    if (!batch.isStereoEnabled()) {
        _stereo._enable = false;
    }
    
    {
        PROFILE_RANGE(render_gpu_gl_detail, "Transfer");
        renderPassTransfer(batch);
    }

#ifdef GPU_STEREO_DRAWCALL_INSTANCED
    if (_stereo.isStereo()) {
        // TODO: Fix this.
        // glEnable(GL_CLIP_DISTANCE0);
    }
#endif
    {
        PROFILE_RANGE(render_gpu_gl_detail, _stereo.isStereo() ? "Render Stereo" : "Render");
        renderPassDraw(batch);
    }
#ifdef GPU_STEREO_DRAWCALL_INSTANCED
    if (_stereo.isStereo()) {
        // TODO: Fix this
        // glDisable(GL_CLIP_DISTANCE0);
    }
#endif
    // Restore the saved stereo state for the next batch
    _stereo._enable = savedStereo;
}


void D3D12Backend::syncCache() {
    PROFILE_RANGE(render_gpu_gl_detail, __FUNCTION__);

    syncTransformStateCache();
    syncPipelineStateCache();
    syncInputStateCache();
    syncOutputStateCache();
}

#ifdef GPU_STEREO_DRAWCALL_DOUBLED
void D3D12Backend::setupStereoSide(int side) {
    ivec4 vp = _transform._viewport;
    vp.z /= 2;
    glViewport(vp.x + side * vp.z, vp.y, vp.z, vp.w);


#ifdef GPU_STEREO_CAMERA_BUFFER
#ifdef GPU_STEREO_DRAWCALL_DOUBLED
    glVertexAttribI1i(14, side);
#endif
#else
    _transform.bindCurrentCamera(side);
#endif

}
#else
#endif

void D3D12Backend::do_resetStages(const Batch& batch, size_t paramOffset) {
    resetStages();
}

void D3D12Backend::do_disableContextViewCorrection(const Batch& batch, size_t paramOffset) {
    _transform._viewCorrectionEnabled = false;
}

void D3D12Backend::do_restoreContextViewCorrection(const Batch& batch, size_t paramOffset) {
    _transform._viewCorrectionEnabled = true;
}

void D3D12Backend::do_disableContextStereo(const Batch& batch, size_t paramOffset) {

}

void D3D12Backend::do_restoreContextStereo(const Batch& batch, size_t paramOffset) {

}

void D3D12Backend::do_runLambda(const Batch& batch, size_t paramOffset) {
    std::function<void()> f = batch._lambdas.get(batch._params[paramOffset]._uint);
    f();
}

void D3D12Backend::do_startNamedCall(const Batch& batch, size_t paramOffset) {
    batch._currentNamedCall = batch._names.get(batch._params[paramOffset]._uint);
    _currentDraw = -1;
}

void D3D12Backend::do_stopNamedCall(const Batch& batch, size_t paramOffset) {
    batch._currentNamedCall.clear();
}

void D3D12Backend::resetStages() {
    resetInputStage();
    resetPipelineStage();
    resetTransformStage();
    resetUniformStage();
    resetResourceStage();
    resetOutputStage();
    resetQueryStage();
}


void D3D12Backend::do_pushProfileRange(const Batch& batch, size_t paramOffset) {
    if (trace_render_gpu_gl_detail().isDebugEnabled()) {
        auto name = batch._profileRanges.get(batch._params[paramOffset]._uint);
        profileRanges.push_back(name);
#if defined(NSIGHT_FOUND)
        nvtxRangePush(name.c_str());
#endif
    }
}

void D3D12Backend::do_popProfileRange(const Batch& batch, size_t paramOffset) {
    if (trace_render_gpu_gl_detail().isDebugEnabled()) {
        profileRanges.pop_back();
#if defined(NSIGHT_FOUND)
        nvtxRangePop();
#endif
    }
}

// TODO: As long as we have gl calls explicitely issued from interface
// code, we need to be able to record and batch these calls. THe long 
// term strategy is to get rid of any GL calls in favor of the HIFI GPU API

// As long as we don;t use several versions of shaders we can avoid this more complex code path
#ifdef GPU_STEREO_CAMERA_BUFFER
#define GET_UNIFORM_LOCATION(shaderUniformLoc) ((_pipeline._programShader) ? _pipeline._programShader->getUniformLocation(shaderUniformLoc, (GLShader::Version) isStereo()) : -1)
#else
#define GET_UNIFORM_LOCATION(shaderUniformLoc) shaderUniformLoc
#endif

void D3D12Backend::do_glUniform1i(const Batch& batch, size_t paramOffset) {
    if (_pipeline._program == 0) {
        // We should call updatePipeline() to bind the program but we are not doing that
        // because these uniform setters are deprecated and we don;t want to create side effect
        return;
    }
    updatePipeline();

#if 0
    // TODO: Fix uniforms.
    glUniform1i(
        GET_UNIFORM_LOCATION(batch._params[paramOffset + 1]._int),
        batch._params[paramOffset + 0]._int);
    (void)CHECK_GL_ERROR();
#endif
}

void D3D12Backend::do_glUniform1f(const Batch& batch, size_t paramOffset) {
    if (_pipeline._program == 0) {
        // We should call updatePipeline() to bind the program but we are not doing that
        // because these uniform setters are deprecated and we don;t want to create side effect
        return;
    }
    updatePipeline();

#if 0
    // TODO: Fix uniforms.
    glUniform1f(
        GET_UNIFORM_LOCATION(batch._params[paramOffset + 1]._int),
        batch._params[paramOffset + 0]._float);
    (void)CHECK_GL_ERROR();
#endif
}

void D3D12Backend::do_glUniform2f(const Batch& batch, size_t paramOffset) {
    if (_pipeline._program == 0) {
        // We should call updatePipeline() to bind the program but we are not doing that
        // because these uniform setters are deprecated and we don;t want to create side effect
        return;
    }
    updatePipeline();
#if 0
    // TODO: Fix uniforms.
    glUniform2f(
        GET_UNIFORM_LOCATION(batch._params[paramOffset + 2]._int),
        batch._params[paramOffset + 1]._float,
        batch._params[paramOffset + 0]._float);
    (void)CHECK_GL_ERROR();
#endif
}

void D3D12Backend::do_glUniform3f(const Batch& batch, size_t paramOffset) {
    if (_pipeline._program == 0) {
        // We should call updatePipeline() to bind the program but we are not doing that
        // because these uniform setters are deprecated and we don;t want to create side effect
        return;
    }
    updatePipeline();
#if 0
    // TODO: Fix uniforms.
    glUniform3f(
        GET_UNIFORM_LOCATION(batch._params[paramOffset + 3]._int),
        batch._params[paramOffset + 2]._float,
        batch._params[paramOffset + 1]._float,
        batch._params[paramOffset + 0]._float);
    (void)CHECK_GL_ERROR();
#endif
}

void D3D12Backend::do_glUniform4f(const Batch& batch, size_t paramOffset) {
    if (_pipeline._program == 0) {
        // We should call updatePipeline() to bind the program but we are not doing that
        // because these uniform setters are deprecated and we don;t want to create side effect
        return;
    }
    updatePipeline();
#if 0
    // TODO: Fix uniforms.
    glUniform4f(
        GET_UNIFORM_LOCATION(batch._params[paramOffset + 4]._int),
        batch._params[paramOffset + 3]._float,
        batch._params[paramOffset + 2]._float,
        batch._params[paramOffset + 1]._float,
        batch._params[paramOffset + 0]._float);
    (void)CHECK_GL_ERROR();
#endif
}

void D3D12Backend::do_glUniform3fv(const Batch& batch, size_t paramOffset) {
    if (_pipeline._program == 0) {
        // We should call updatePipeline() to bind the program but we are not doing that
        // because these uniform setters are deprecated and we don;t want to create side effect
        return;
    }
    updatePipeline();
#if 0
    // TODO: Fix uniforms.
    glUniform3fv(
        GET_UNIFORM_LOCATION(batch._params[paramOffset + 2]._int),
        batch._params[paramOffset + 1]._uint,
        (const GLfloat*)batch.readData(batch._params[paramOffset + 0]._uint));

    (void)CHECK_GL_ERROR();
#endif
}

void D3D12Backend::do_glUniform4fv(const Batch& batch, size_t paramOffset) {
    if (_pipeline._program == 0) {
        // We should call updatePipeline() to bind the program but we are not doing that
        // because these uniform setters are deprecated and we don;t want to create side effect
        return;
    }
    updatePipeline();

#if 0
    // TODO: Fix uniforms.
    GLint location = GET_UNIFORM_LOCATION(batch._params[paramOffset + 2]._int);
    GLsizei count = batch._params[paramOffset + 1]._uint;
    const GLfloat* value = (const GLfloat*)batch.readData(batch._params[paramOffset + 0]._uint);
    glUniform4fv(location, count, value);

    (void)CHECK_GL_ERROR();
#endif
}

void D3D12Backend::do_glUniform4iv(const Batch& batch, size_t paramOffset) {
    if (_pipeline._program == 0) {
        // We should call updatePipeline() to bind the program but we are not doing that
        // because these uniform setters are deprecated and we don;t want to create side effect
        return;
    }
    updatePipeline();

#if 0
    // TODO: Fix uniforms.
    glUniform4iv(
        GET_UNIFORM_LOCATION(batch._params[paramOffset + 2]._int),
        batch._params[paramOffset + 1]._uint,
        (const GLint*)batch.readData(batch._params[paramOffset + 0]._uint));

    (void)CHECK_GL_ERROR();
#endif
}

void D3D12Backend::do_glUniformMatrix3fv(const Batch& batch, size_t paramOffset) {
    if (_pipeline._program == 0) {
        // We should call updatePipeline() to bind the program but we are not doing that
        // because these uniform setters are deprecated and we don;t want to create side effect
        return;
    }
    updatePipeline();

#if 0
    // TODO: Fix uniforms.
    glUniformMatrix3fv(
        GET_UNIFORM_LOCATION(batch._params[paramOffset + 3]._int),
        batch._params[paramOffset + 2]._uint,
        batch._params[paramOffset + 1]._uint,
        (const GLfloat*)batch.readData(batch._params[paramOffset + 0]._uint));
    (void)CHECK_GL_ERROR();
#endif
}

void D3D12Backend::do_glUniformMatrix4fv(const Batch& batch, size_t paramOffset) {
    if (_pipeline._program == 0) {
        // We should call updatePipeline() to bind the program but we are not doing that
        // because these uniform setters are deprecated and we don;t want to create side effect
        return;
    }
    updatePipeline();

#if 0
    // TODO: Fix uniforms.
    glUniformMatrix4fv(
        GET_UNIFORM_LOCATION(batch._params[paramOffset + 3]._int),
        batch._params[paramOffset + 2]._uint,
        batch._params[paramOffset + 1]._uint,
        (const GLfloat*)batch.readData(batch._params[paramOffset + 0]._uint));
    (void)CHECK_GL_ERROR();
#endif
}

void D3D12Backend::do_glColor4f(const Batch& batch, size_t paramOffset) {

    glm::vec4 newColor(
        batch._params[paramOffset + 3]._float,
        batch._params[paramOffset + 2]._float,
        batch._params[paramOffset + 1]._float,
        batch._params[paramOffset + 0]._float);

#if 0
    // TODO: Fix uniforms.
    if (_input._colorAttribute != newColor) {
        _input._colorAttribute = newColor;
        glVertexAttrib4fv(gpu::Stream::COLOR, &_input._colorAttribute.r);
    }
    (void)CHECK_GL_ERROR();
#endif
}

void D3D12Backend::releaseBuffer(unsigned int id, Size size) const {
    Lock lock(_trashMutex);
    _buffersTrash.push_back({ id, size });
}

void D3D12Backend::releaseExternalTexture(unsigned int id, const Texture::ExternalRecycler& recycler) const {
    Lock lock(_trashMutex);
    _externalTexturesTrash.push_back({ id, recycler });
}

void D3D12Backend::releaseTexture(unsigned int id, Size size) const {
    Lock lock(_trashMutex);
    _texturesTrash.push_back({ id, size });
}

void D3D12Backend::releaseFramebuffer(unsigned int id) const {
    Lock lock(_trashMutex);
    _framebuffersTrash.push_back(id);
}

void D3D12Backend::releaseShader(unsigned int id) const {
    Lock lock(_trashMutex);
    _shadersTrash.push_back(id);
}

void D3D12Backend::releaseProgram(unsigned int id) const {
    Lock lock(_trashMutex);
    _programsTrash.push_back(id);
}

void D3D12Backend::releaseQuery(unsigned int id) const {
    Lock lock(_trashMutex);
    _queriesTrash.push_back(id);
}

void D3D12Backend::queueLambda(const std::function<void()> lambda) const {
    Lock lock(_trashMutex);
    _lambdaQueue.push_back(lambda);
}

void D3D12Backend::recycle() const {
    PROFILE_RANGE(render_gpu_gl, __FUNCTION__)
    {
        std::list<std::function<void()>> lamdbasTrash;
        {
            Lock lock(_trashMutex);
            std::swap(_lambdaQueue, lamdbasTrash);
        }
        for (auto lambda : lamdbasTrash) {
            lambda();
        }
    }

    {
        std::vector<unsigned int> ids;
        std::list<std::pair<unsigned int, Size>> buffersTrash;
        {
            Lock lock(_trashMutex);
            std::swap(_buffersTrash, buffersTrash);
        }
        ids.reserve(buffersTrash.size());
        for (auto pair : buffersTrash) {
            ids.push_back(pair.first);
        }

        // TODO: Fix this.
#if 0
        if (!ids.empty()) {
            glDeleteBuffers((GLsizei)ids.size(), ids.data());
        }
#endif
    }

    {
        std::vector<unsigned int> ids;
        std::list<unsigned int> framebuffersTrash;
        {
            Lock lock(_trashMutex);
            std::swap(_framebuffersTrash, framebuffersTrash);
        }
        ids.reserve(framebuffersTrash.size());
        for (auto id : framebuffersTrash) {
            ids.push_back(id);
        }
        // TODO: Fix this.
#if 0
        if (!ids.empty()) {
            glDeleteFramebuffers((GLsizei)ids.size(), ids.data());
        }
#endif
    }

    {
        std::vector<unsigned int> ids;
        std::list<std::pair<unsigned int, Size>> texturesTrash;
        {
            Lock lock(_trashMutex);
            std::swap(_texturesTrash, texturesTrash);
        }
        ids.reserve(texturesTrash.size());
        for (auto pair : texturesTrash) {
            ids.push_back(pair.first);
        }
        // TODO: Fix this.
#if 0
        if (!ids.empty()) {
            glDeleteTextures((GLsizei)ids.size(), ids.data());
        }
#endif
    }

    {
        std::list<std::pair<unsigned int, Texture::ExternalRecycler>> externalTexturesTrash;
        {
            Lock lock(_trashMutex);
            std::swap(_externalTexturesTrash, externalTexturesTrash);
        }
        if (!externalTexturesTrash.empty()) {
            // TODO: Fix this.
#if 0
            std::vector<GLsync> fences;
            fences.resize(externalTexturesTrash.size());
            for (size_t i = 0; i < externalTexturesTrash.size(); ++i) {
                fences[i] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            }
#endif
            // External texture fences will be read in another thread/context, so we need a flush

            // TODO: Fix this.
#if 0
            // glFlush();
            size_t index = 0;
            for (auto pair : externalTexturesTrash) {
                auto fence = fences[index++];
                pair.second(pair.first, fence);
            }
#endif
    }
    }

    {
        std::list<unsigned int> programsTrash;
        {
            Lock lock(_trashMutex);
            std::swap(_programsTrash, programsTrash);
        }
        // TODO: Fix this.
#if 0
        for (auto id : programsTrash) {
            glDeleteProgram(id);
        }
#endif
    }

    {
        std::list<unsigned int> shadersTrash;
        {
            Lock lock(_trashMutex);
            std::swap(_shadersTrash, shadersTrash);
        }
        // TODO: Fix this.
#if 0
        for (auto id : shadersTrash) {
            glDeleteShader(id);
        }
#endif

    }

    {
        std::vector<unsigned int> ids;
        std::list<unsigned int> queriesTrash;
        {
            Lock lock(_trashMutex);
            std::swap(_queriesTrash, queriesTrash);
        }
        ids.reserve(queriesTrash.size());
        for (auto id : queriesTrash) {
            ids.push_back(id);
        }
        // TODO: Fix this.
#if 0
        if (!ids.empty()) {
            glDeleteQueries((GLsizei)ids.size(), ids.data());
        }
#endif
    }

    GLVariableAllocationSupport::manageMemory();
    GLVariableAllocationSupport::_frameTexturesCreated = 0;

}

void D3D12Backend::setCameraCorrection(const Mat4& correction) {
    _transform._correction.correction = correction;
    _transform._correction.correctionInverse = glm::inverse(correction);
    _pipeline._cameraCorrectionBuffer._buffer->setSubData(0, _transform._correction);
    _pipeline._cameraCorrectionBuffer._buffer->flush();
}

void D3D12Backend::do_draw(const Batch& batch, size_t paramOffset) 
{
}

void D3D12Backend::do_drawIndexed(const Batch& batch, size_t paramOffset)
{
}

void D3D12Backend::do_drawInstanced(const Batch& batch, size_t paramOffset)
{
}

void D3D12Backend::do_drawIndexedInstanced(const Batch& batch, size_t paramOffset)
{
}

void D3D12Backend::do_multiDrawIndirect(const Batch& batch, size_t paramOffset)
{
}

void D3D12Backend::do_multiDrawIndexedIndirect(const Batch& batch, size_t paramOffset)
{
}

void D3D12Backend::do_blit(const Batch& batch, size_t paramOffset)
{
}
