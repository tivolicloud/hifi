#pragma once

#include <QAtomicInt>
#include <QPaintDeviceWindow>
#include <QImage>
#include <QMainWindow.h>
#include <QCloseEvent>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

using namespace Microsoft::WRL;

class QD3D12Window : public QWindow
{
    Q_OBJECT

public:
    struct Fence 
    {
        Fence() : event(INVALID_HANDLE_VALUE) 
        { 
        }

        ~Fence()
        {
            if (event != INVALID_HANDLE_VALUE)
            {
                CloseHandle(event);
            }
        }

        ComPtr<ID3D12Fence> fence;
        HANDLE              event;
        QAtomicInt          value;

    private:
        Q_DISABLE_COPY(Fence)
    };

    QD3D12Window(QWindow* parent = Q_NULLPTR);


    bool event(QEvent* event) override;
    void setExtraRenderTargetCount(int count);

    virtual void initializeD3D() = 0;
    virtual void releaseD3D() = 0;
    virtual void resizeD3D(const QSize &size) = 0;
    virtual void paintD3D() = 0;
    virtual void afterPresent()  = 0;

    void flush();
    void initialize();
    void setupRenderTargets();
    void handleResize();
    void deviceLost();

    ID3D12Device *dxDevice() const;
    ID3D12CommandQueue *dxCommandQueue() const;
    ID3D12CommandAllocator *dxCommandAllocator() const;
    ID3D12CommandAllocator *dxBundleAllocator() const;

    Fence *createFence() const;
    void waitForGPU(Fence *f) const;

    void transitionResource(ID3D12Resource *resource, ID3D12GraphicsCommandList *commandList,
                            D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) const;
    void uavBarrier(ID3D12Resource *resource, ID3D12GraphicsCommandList *commandList) const;

    ID3D12Resource *backBufferRenderTarget() const;
    D3D12_CPU_DESCRIPTOR_HANDLE backBufferRenderTargetCPUHandle() const;

    D3D12_CPU_DESCRIPTOR_HANDLE depthStencilCPUHandle() const;

    D3D12_CPU_DESCRIPTOR_HANDLE extraRenderTargetCPUHandle(int idx) const;
    D3D12_CPU_DESCRIPTOR_HANDLE extraDepthStencilCPUHandle(int idx) const;

    ID3D12Resource *createExtraRenderTargetAndView(D3D12_CPU_DESCRIPTOR_HANDLE viewHandle,
                                                   const QSize &size,
                                                   const float *clearColor = Q_NULLPTR,
                                                   int samples = 0);
    ID3D12Resource *createExtraDepthStencilAndView(D3D12_CPU_DESCRIPTOR_HANDLE viewHandle,
                                                   const QSize &size,
                                                   int samples = 0);

    quint32 alignedCBSize(quint32 size) const;
    quint32 alignedTexturePitch(quint32 rowPitch) const;
    quint32 alignedTextureOffset(quint32 offset) const;

    QImage readbackRGBA8888(ID3D12Resource *rt, D3D12_RESOURCE_STATES rtState, ID3D12GraphicsCommandList *commandList);

    DXGI_SAMPLE_DESC makeSampleDesc(DXGI_FORMAT format, int samples);
    ID3D12Resource *createOffscreenRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE viewHandle, const QSize &size, const float *clearColor, int samples);
    ID3D12Resource *createDepthStencil(D3D12_CPU_DESCRIPTOR_HANDLE viewHandle, const QSize &size, int samples);


signals:
    void HandleChanged(WId handle);

protected:

    void resizeEvent(QResizeEvent *) override;

    bool initialized;
    int swapChainBufferCount;
    int extraRenderTargetCount;
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    ComPtr<ID3D12Resource> m_renderTargets[2];
    ComPtr<ID3D12Resource> m_depthStencil;
    UINT rtvStride;
    UINT dsvStride;
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12CommandAllocator> m_bundleAllocator;
};

