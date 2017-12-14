//
//  Created by Bradley Austin Davis on 2016/05/15
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_d3d12_texture_h
#define hifi_gpu_d3d12_texture_h

#include <QtCore/QThreadPool>
#include <QtConcurrent>

#include "d3d12shared.h"
#include "d3d12Backend.h"
#include <thread>

#define THREADED_TEXTURE_BUFFERING 1

namespace gpu { namespace d3d12 {

class GLVariableAllocationSupport {
    friend class D3D12Backend;

public:
    GLVariableAllocationSupport();
    virtual ~GLVariableAllocationSupport();

    enum class MemoryPressureState {
        Idle,
        Transfer,
        Oversubscribed,
        Undersubscribed,
    };

    using QueuePair = std::pair<TextureWeakPointer, float>;
    struct QueuePairLess {
        bool operator()(const QueuePair& a, const QueuePair& b) {
            return a.second < b.second;
        }
    };
    using WorkQueue = std::priority_queue<QueuePair, std::vector<QueuePair>, QueuePairLess>;

    class TransferJob {
        using VoidLambda = std::function<void()>;
        using VoidLambdaQueue = std::queue<VoidLambda>;
        const D3D12Texture& _parent;
        Texture::PixelsPointer _mipData;
        size_t _transferOffset { 0 };
        size_t _transferSize { 0 };

        bool _bufferingRequired { true };
        VoidLambda _transferLambda;
        VoidLambda _bufferingLambda;

#if THREADED_TEXTURE_BUFFERING
        // Indicates if a transfer from backing storage to internal storage has started
        QFuture<void> _bufferingStatus;
        static QThreadPool* _bufferThreadPool;
#endif

    public:
        TransferJob(const TransferJob& other) = delete;
        TransferJob(const D3D12Texture& parent, std::function<void()> transferLambda);
        TransferJob(const D3D12Texture& parent, uint16_t sourceMip, uint16_t targetMip, uint8_t face, uint32_t lines = 0, uint32_t lineOffset = 0);
        ~TransferJob();
        bool tryTransfer();

#if THREADED_TEXTURE_BUFFERING
        void startBuffering();
        bool bufferingRequired() const;
        bool bufferingCompleted() const;
        static void startBufferingThread();
#endif

    private:
        void transfer();
    };

    using TransferJobPointer = std::shared_ptr<TransferJob>;
    using TransferQueue = std::queue<TransferJobPointer>;
    static MemoryPressureState _memoryPressureState;

public:
    static void addMemoryManagedTexture(const TexturePointer& texturePointer);

protected:
    static size_t _frameTexturesCreated;
    static std::atomic<bool> _memoryPressureStateStale;
    static std::list<TextureWeakPointer> _memoryManagedTextures;
    static WorkQueue _transferQueue;
    static WorkQueue _promoteQueue;
    static WorkQueue _demoteQueue;
#if THREADED_TEXTURE_BUFFERING
    static TexturePointer _currentTransferTexture;
    static TransferJobPointer _currentTransferJob;
#endif
    static const uvec3 INITIAL_MIP_TRANSFER_DIMENSIONS;
    static const uvec3 MAX_TRANSFER_DIMENSIONS;
    static const size_t MAX_TRANSFER_SIZE;


    static void updateMemoryPressure();
    static void processWorkQueues();
    static void processWorkQueue(WorkQueue& workQueue);
    static TexturePointer getNextWorkQueueItem(WorkQueue& workQueue);
    static void addToWorkQueue(const TexturePointer& texture);
    static WorkQueue& getActiveWorkQueue();

    static void manageMemory();

    //bool canPromoteNoAllocate() const { return _allocatedMip < _populatedMip; }
    bool canPromote() const { return _allocatedMip > _minAllocatedMip; }
    bool canDemote() const { return _allocatedMip < _maxAllocatedMip; }
    bool hasPendingTransfers() const { return _populatedMip > _allocatedMip; }
#if THREADED_TEXTURE_BUFFERING
    void executeNextBuffer(const TexturePointer& currentTexture);
#endif
    bool executeNextTransfer(const TexturePointer& currentTexture);
    virtual void populateTransferQueue() = 0;
    virtual void promote() = 0;
    virtual void demote() = 0;

    // THe amount of memory currently allocated
    Size _size { 0 };

    // The amount of memory currnently populated
    void incrementPopulatedSize(Size delta) const;
    void decrementPopulatedSize(Size delta) const;
    mutable Size _populatedSize { 0 };

    // The allocated mip level, relative to the number of mips in the gpu::Texture object 
    // The relationship between a given glMip to the original gpu::Texture mip is always 
    // glMip + _allocatedMip
    uint16 _allocatedMip { 0 };
    // The populated mip level, relative to the number of mips in the gpu::Texture object
    // This must always be >= the allocated mip
    uint16 _populatedMip { 0 };
    // The highest (lowest resolution) mip that we will support, relative to the number 
    // of mips in the gpu::Texture object
    uint16 _maxAllocatedMip { 0 };
    // The lowest (highest resolution) mip that we will support, relative to the number
    // of mips in the gpu::Texture object
    uint16 _minAllocatedMip { 0 };
    // Contains a series of lambdas that when executed will transfer data to the GPU, modify 
    // the _populatedMip and update the sampler in order to fully populate the allocated texture 
    // until _populatedMip == _allocatedMip
    TransferQueue _pendingTransfers;
};

class D3D12Texture : public DxObject<Texture> {
    using Parent = DxObject<Texture>;
    friend class D3D12Backend;
    friend class GLVariableAllocationSupport;

public:
    static const uint16_t INVALID_MIP { (uint16_t)-1 };
    static const uint8_t INVALID_FACE { (uint8_t)-1 };

    ~D3D12Texture();

    const unsigned int& _texture { _id };
    const std::string _source;
    // const unsigned int _target;
    DXGI_FORMAT _texelFormat;

    D3D12_SHADER_RESOURCE_VIEW_DESC         _shaderResourceView;
    Microsoft::WRL::ComPtr<ID3D12Resource>  _textureResource;

    // static const std::vector<unsigned int>& getFaceTargets(unsigned int textureType);
    // static uint8_t getFaceCount(int textureType);
    // static GLenum getGLTextureType(const Texture& texture);

    static const uint8_t TEXTURE_2D_NUM_FACES = 1;
    static const uint8_t TEXTURE_CUBE_NUM_FACES = 6;
#if 0
    static const GLenum CUBE_FACE_LAYOUT[TEXTURE_CUBE_NUM_FACES];
#endif
    static const D3D12_FILTER FILTER_MODES[Sampler::NUM_FILTERS];
    static const D3D12_TEXTURE_ADDRESS_MODE WRAP_MODES[Sampler::NUM_WRAP_MODES];

protected:
    virtual Size size() const = 0;
    virtual void generateMips() const = 0;
    virtual void syncSampler() const = 0;

    // virtual Size copyMipFaceLinesFromTexture(uint16_t mip, uint8_t face, const uvec3& size, uint32_t yOffset, DXGI_FORMAT format, D3D12_RESOURCE_DIMENSION type, Size sourceSize, const void* sourcePointer) const = 0;
    // virtual Size copyMipFaceFromTexture(uint16_t sourceMip, uint16_t targetMip, uint8_t face) const final;
    // virtual void copyTextureMipsInGPUMem(GLuint srcId, GLuint destId, uint16_t srcMipOffset, uint16_t destMipOffset, uint16_t populatedMips) {} // Only relevant for Variable Allocation textures

    D3D12Texture(const std::weak_ptr<d3d12::D3D12Backend>& backend, const Texture& texture, unsigned int id);
};

class D3D12ExternalTexture : public D3D12Texture {
    using Parent = D3D12Texture;
    friend class D3D12Backend;
public:
    ~D3D12ExternalTexture();
protected:
    D3D12ExternalTexture(const std::weak_ptr<d3d12::D3D12Backend>& backend, const Texture& texture, unsigned int id);
    void generateMips() const override {}
    void syncSampler() const override {}
    // Size copyMipFaceLinesFromTexture(uint16_t mip, uint8_t face, const uvec3& size, uint32_t yOffset, GLenum internalFormat, GLenum format, GLenum type, Size sourceSize, const void* sourcePointer) const override { return 0;}

    Size size() const override { return 0; }
};


} }

#endif
