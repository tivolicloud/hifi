#include "qd3d12window.h"

static void getHardwareAdapter(IDXGIFactory1 *factory, IDXGIAdapter1 **outAdapter)
{
    ComPtr<IDXGIAdapter1> adapter;

    for (int adapterIndex = 0; factory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND; ++adapterIndex) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            continue;

        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), Q_NULLPTR))) {
            const QString name = QString::fromUtf16((char16_t *) desc.Description);
            qDebug("Using adapter '%s'", qPrintable(name));
            break;
        }
    }

    *outAdapter = adapter.Detach();
}

void QD3D12Window::initialize()
{
    if (initialized)
        return;

    swapChainBufferCount = 2;
    HWND hwnd = reinterpret_cast<HWND>(winId());

    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();
    }

    ComPtr<IDXGIFactory4> factory;
    if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)))) {
        qWarning("Failed to create DXGI");
        return;
    }

    ComPtr<IDXGIAdapter1> adapter;
    getHardwareAdapter(factory.Get(), &adapter);

    bool warp = true;
    if (adapter) {
        HRESULT hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
        if (SUCCEEDED(hr))
            warp = false;
        else
            qWarning("Failed to create device: 0x%x", hr);
    } else {
        qWarning("No usable hardware adapters found");
    }

    if (warp) {
        qDebug("Using WARP");
        ComPtr<IDXGIAdapter> warpAdapter;
        factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
        HRESULT hr = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
        if (FAILED(hr)) {
            qWarning("Failed to create WARP device: 0x%x", hr);
            return;
        }
    }

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    if (FAILED(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)))) {
        qWarning("Failed to create command queue");
        return;
    }

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = swapChainBufferCount;
    swapChainDesc.BufferDesc.Width = width() * devicePixelRatio();
    swapChainDesc.BufferDesc.Height = height() * devicePixelRatio();
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // D3D12 requires the flip model
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SampleDesc.Count = 1; // Flip does not support MSAA so no choice here
    swapChainDesc.Windowed = TRUE;

    ComPtr<IDXGISwapChain> baseSwapChain;
    HRESULT hr = factory->CreateSwapChain(m_commandQueue.Get(), &swapChainDesc, &baseSwapChain);
    if (FAILED(hr)) {
        qWarning("Failed to create swap chain: 0x%x", hr);
        return;
    }
    if (FAILED(baseSwapChain.As(&m_swapChain))) {
        qWarning("Failed to cast swap chain");
        return;
    }

    factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

    if (FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)))) {
        qWarning("Failed to create command allocator");
        return;
    }
    
    if (FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(&m_bundleAllocator)))) {
        qWarning("Failed to create command bundle allocator");
        return;
    }

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = swapChainBufferCount + extraRenderTargetCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    if (FAILED(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)))) {
        qWarning("Failed to create render target view descriptor heap");
        return;
    }
    rtvStride = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1 + extraRenderTargetCount;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    if (FAILED(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)))) {
        qWarning("Failed to create depth stencil heap");
        return;
    }
    dsvStride = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    setupRenderTargets();

    initialized = true;
    initializeD3D();
}

DXGI_SAMPLE_DESC QD3D12Window::makeSampleDesc(DXGI_FORMAT format, int samples)
{
    DXGI_SAMPLE_DESC sampleDesc;
    sampleDesc.Count = 1;
    sampleDesc.Quality = 0;

    if (samples > 1) {
        D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaInfo = {};
        msaaInfo.Format = format;
        msaaInfo.SampleCount = samples;
        if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaInfo, sizeof(msaaInfo)))) {
            if (msaaInfo.NumQualityLevels > 0) {
                sampleDesc.Count = samples;
                sampleDesc.Quality = msaaInfo.NumQualityLevels - 1;
            } else {
                qWarning("No quality levels for multisampling?");
            }
        } else {
            qWarning("Failed to query multisample quality levels");
        }
    }

    return sampleDesc;
}

ID3D12Resource *QD3D12Window::createOffscreenRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE viewHandle,
                                                          const QSize &size, const float *clearColor, int samples)
{
    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    if (clearColor)
        memcpy(clearValue.Color, clearColor, 4 * sizeof(float));

    D3D12_HEAP_PROPERTIES heapProp = {};
    heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC rtDesc = {};
    rtDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    rtDesc.Width = size.width();
    rtDesc.Height = size.height();
    rtDesc.DepthOrArraySize = 1;
    rtDesc.MipLevels = 1;
    rtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtDesc.SampleDesc = makeSampleDesc(rtDesc.Format, samples); // MSAA works here, unlike the backbuffer
    rtDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    ID3D12Resource *resource = Q_NULLPTR;
    if (FAILED(m_device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &rtDesc,
                                               D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, IID_PPV_ARGS(&resource)))) {
        qWarning("Failed to create offscreen render target of size %dx%d", size.width(), size.height());
        return Q_NULLPTR;
    }

    m_device->CreateRenderTargetView(resource, Q_NULLPTR, viewHandle);

    return resource;
}

ID3D12Resource *QD3D12Window::createDepthStencil(D3D12_CPU_DESCRIPTOR_HANDLE viewHandle, const QSize &size, int samples)
{
    D3D12_CLEAR_VALUE depthClearValue = {};
    depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    depthClearValue.DepthStencil.Depth = 1.0f;
    depthClearValue.DepthStencil.Stencil = 0;

    D3D12_HEAP_PROPERTIES heapProp = {};
    heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC bufDesc = {};
    bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    bufDesc.Width = size.width();
    bufDesc.Height = size.height();
    bufDesc.DepthOrArraySize = 1;
    bufDesc.MipLevels = 1;
    bufDesc.Format = DXGI_FORMAT_D32_FLOAT;
    bufDesc.SampleDesc = makeSampleDesc(bufDesc.Format, samples);
    bufDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    bufDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    ID3D12Resource *resource = Q_NULLPTR;
    if (FAILED(m_device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &bufDesc,
                                               D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&resource)))) {
        qWarning("Failed to create depth-stencil buffer of size %dx%d", size.width(), size.height());
        return Q_NULLPTR;
    }

    D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
    depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilDesc.ViewDimension = bufDesc.SampleDesc.Count <= 1 ? D3D12_DSV_DIMENSION_TEXTURE2D : D3D12_DSV_DIMENSION_TEXTURE2DMS;

    m_device->CreateDepthStencilView(resource, &depthStencilDesc, viewHandle);

    return resource;
}

void QD3D12Window::setupRenderTargets()
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (int i = 0; i < swapChainBufferCount; ++i) {
        if (FAILED(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])))) {
            qWarning("Failed to get buffer %d from swap chain", i);
            return;
        }
        m_device->CreateRenderTargetView(m_renderTargets[i].Get(), Q_NULLPTR, rtvHandle);
        rtvHandle.ptr += rtvStride;
    }

    ID3D12Resource *ds = createDepthStencil(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), size(), 0);
    if (ds)
    {
        m_depthStencil.Attach(ds);
    }
}

void QD3D12Window::handleResize()
{
    if (initialized)
    {
        // Clear these, otherwise resizing will fail.
        
        m_depthStencil = Q_NULLPTR;
        for (int i = 0; i < swapChainBufferCount; ++i)
        {
            m_renderTargets[i] = Q_NULLPTR;
        }

        HRESULT hr = m_swapChain->ResizeBuffers(swapChainBufferCount, width(), height(), DXGI_FORMAT_R8G8B8A8_UNORM, 0);
        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) 
        {
            deviceLost();
            return;
        }
        else if (FAILED(hr)) 
        {
            qWarning("Failed to resize buffers: 0x%x", hr);
            return;
        }

        setupRenderTargets();
    }
}

void QD3D12Window::deviceLost()
{
    qWarning("D3D device lost");

    releaseD3D();
    m_bundleAllocator.Reset();
    m_commandAllocator.Reset();
    rtvStride = dsvStride = 0;
    m_depthStencil.Reset();
    for (int i = 0; i < swapChainBufferCount; ++i)
    {
        m_renderTargets[i].Reset();
    }
    m_dsvHeap.Reset();
    m_rtvHeap.Reset();
    m_commandQueue.Reset();
    m_swapChain.Reset();
    m_device.Reset();

    initialized = false;
    initialize();
}

void QD3D12Window::flush()
{
    HRESULT hr = m_swapChain->Present(1, 0);
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
        deviceLost();
        return;
    } else if (FAILED(hr)) {
        qWarning("Present failed: 0x%x", hr);
        return;
    }

    afterPresent();
}

QD3D12Window::QD3D12Window(QWindow* parent) :   QWindow(parent), 
                                                initialized(false),
                                                extraRenderTargetCount(0)
{
}

bool QD3D12Window::event(QEvent* event)
{
    switch (event->type())
    {
        case QEvent::WinIdChange:
        {
            emit HandleChanged(winId());
            break;
        }

        case QEvent::FocusIn:
        {
            break;
        }

        case QEvent::FocusOut:
        {
            break;
        }
        
        case QEvent::Close:
        {
            break;
        }
            

        default:
            break;
    }

    return QWindow::event(event);
}

void QD3D12Window::setExtraRenderTargetCount(int count)
{
    if (initialized) {
        qWarning("setExtraRenderTargetCount: Already initialized, request ignored.");
        return;
    }
    extraRenderTargetCount = qMax(0, count);
}

void QD3D12Window::resizeEvent(QResizeEvent *event)
{
    // TODO: Why is QT resizing the window after close ? 
    //       if thats normal, it needs to hook into the close event properly.
    Q_UNUSED(event);

    if (size().isEmpty())
    {
        return;
    }

    handleResize();
    if (m_depthStencil)
    {
        resizeD3D(size());
        paintD3D();
    }
    afterPresent();
}

ID3D12Device *QD3D12Window::dxDevice() const
{
    return m_device.Get();
}

ID3D12CommandQueue *QD3D12Window::dxCommandQueue() const
{
    return m_commandQueue.Get();
}

ID3D12CommandAllocator *QD3D12Window::dxCommandAllocator() const
{
    return m_commandAllocator.Get();
}

ID3D12CommandAllocator *QD3D12Window::dxBundleAllocator() const
{
    return m_bundleAllocator.Get();
}

QD3D12Window::Fence *QD3D12Window::createFence() const
{
    Fence *f = new Fence;
    if (FAILED(m_device->CreateFence(f->value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&f->fence)))) {
        qWarning("Failed to create fence");
        return f;
    }
    f->event = CreateEvent(Q_NULLPTR, FALSE, FALSE, Q_NULLPTR);
    return f;
}

void QD3D12Window::waitForGPU(Fence *f) const
{
    const UINT64 newValue = f->value.fetchAndAddAcquire(1) + 1;
    m_commandQueue->Signal(f->fence.Get(), newValue);
    if (f->fence->GetCompletedValue() < newValue) {
        if (FAILED(f->fence->SetEventOnCompletion(newValue, f->event))) {
            qWarning("SetEventOnCompletion failed");
            return;
        }
        WaitForSingleObject(f->event, INFINITE);
    }
}

void QD3D12Window::transitionResource(ID3D12Resource *resource, ID3D12GraphicsCommandList *commandList,
                                      D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) const
{
    D3D12_RESOURCE_BARRIER barrier;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = resource;
    barrier.Transition.StateBefore = before;
    barrier.Transition.StateAfter = after;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    commandList->ResourceBarrier(1, &barrier);
}

void QD3D12Window::uavBarrier(ID3D12Resource *resource, ID3D12GraphicsCommandList *commandList) const
{
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.UAV.pResource = resource;

    commandList->ResourceBarrier(1, &barrier);
}

ID3D12Resource *QD3D12Window::backBufferRenderTarget() const
{
    return m_renderTargets[m_swapChain->GetCurrentBackBufferIndex()].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE QD3D12Window::backBufferRenderTargetCPUHandle() const
{
    const int frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.ptr += frameIndex * rtvStride;
    return rtvHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE QD3D12Window::depthStencilCPUHandle() const
{
    return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE QD3D12Window::extraRenderTargetCPUHandle(int idx) const
{
    Q_ASSERT(idx >= 0 && idx < extraRenderTargetCount);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.ptr += (swapChainBufferCount + idx) * rtvStride;
    return rtvHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE QD3D12Window::extraDepthStencilCPUHandle(int idx) const
{
    Q_ASSERT(idx >= 0 && idx < extraRenderTargetCount);
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
    dsvHandle.ptr += (1 + idx) * dsvStride;
    return dsvHandle;
}

ID3D12Resource *QD3D12Window::createExtraRenderTargetAndView(D3D12_CPU_DESCRIPTOR_HANDLE viewHandle,
                                                             const QSize &size,
                                                             const float *clearColor,
                                                             int samples)
{
    return createOffscreenRenderTarget(viewHandle, size, clearColor, samples);
}

ID3D12Resource *QD3D12Window::createExtraDepthStencilAndView(D3D12_CPU_DESCRIPTOR_HANDLE viewHandle,
                                                             const QSize &size,
                                                             int samples)
{
    return createDepthStencil(viewHandle, size, samples);
}

quint32 QD3D12Window::alignedCBSize(quint32 size) const
{
    return (size + D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
}

quint32 QD3D12Window::alignedTexturePitch(quint32 rowPitch) const
{
    return (rowPitch + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);
}

quint32 QD3D12Window::alignedTextureOffset(quint32 offset) const
{
    return (offset + D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT - 1);
}

QImage QD3D12Window::readbackRGBA8888(ID3D12Resource *rt, D3D12_RESOURCE_STATES rtState, ID3D12GraphicsCommandList *commandList)
{
    ComPtr<ID3D12Resource> readbackBuf;

    D3D12_RESOURCE_DESC rtDesc = rt->GetDesc();
    UINT64 textureByteSize = 0;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT textureLayout = {};
    m_device->GetCopyableFootprints(&rtDesc, 0, 1, 0, &textureLayout, Q_NULLPTR, Q_NULLPTR, &textureByteSize);

    D3D12_HEAP_PROPERTIES heapProp = {};
    heapProp.Type = D3D12_HEAP_TYPE_READBACK;

    D3D12_RESOURCE_DESC bufDesc = {};
    bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufDesc.Width = textureByteSize;
    bufDesc.Height = 1;
    bufDesc.DepthOrArraySize = 1;
    bufDesc.MipLevels = 1;
    bufDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufDesc.SampleDesc.Count = 1;
    bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    if (FAILED(m_device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &bufDesc,
                                            D3D12_RESOURCE_STATE_COPY_DEST, Q_NULLPTR, IID_PPV_ARGS(&readbackBuf)))) {
        qWarning("Failed to create committed resource (readback buffer)");
        return QImage();
    }

    D3D12_TEXTURE_COPY_LOCATION dstLoc;
    dstLoc.pResource = readbackBuf.Get();
    dstLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    dstLoc.PlacedFootprint = textureLayout;
    D3D12_TEXTURE_COPY_LOCATION srcLoc;
    srcLoc.pResource = rt;
    srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    srcLoc.SubresourceIndex = 0;

    transitionResource(rt, commandList, rtState, D3D12_RESOURCE_STATE_COPY_SOURCE);
    commandList->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, Q_NULLPTR);
    transitionResource(rt, commandList, D3D12_RESOURCE_STATE_COPY_SOURCE, rtState);
    commandList->Close();

    ID3D12CommandList *commandLists[] = { commandList };
    m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
    QScopedPointer<Fence> f(createFence());
    waitForGPU(f.data());

    QImage img(rtDesc.Width, rtDesc.Height, QImage::Format_RGBA8888);
    quint8 *p = Q_NULLPTR;
    D3D12_RANGE readRange = { 0, 0 };
    if (FAILED(readbackBuf->Map(0, &readRange, reinterpret_cast<void **>(&p)))) {
        qWarning("Mapping the readback buffer failed");
        return QImage();
    }
    for (UINT y = 0; y < rtDesc.Height; ++y) {
        quint8 *dst = img.scanLine(y);
        memcpy(dst, p, rtDesc.Width * 4);
        p += textureLayout.Footprint.RowPitch;
    }
    readbackBuf->Unmap(0, Q_NULLPTR);

    return img;
}

