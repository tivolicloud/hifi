#pragma once

#include <QAtomicInt>
#include <QPaintDeviceWindow>
#include <QImage>
#include <QMainWindow.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

using namespace Microsoft::WRL;

class QD3D12WindowPrivate;

class QD3D12Window : public QWindow
{
    Q_OBJECT

public:
    struct Fence {
        Fence() : event(Q_NULLPTR) { }
        ~Fence();
        ComPtr<ID3D12Fence> fence;
        HANDLE event;
        QAtomicInt value;
    private:
        Q_DISABLE_COPY(Fence)
    };

    QD3D12Window(QWindow* parent = Q_NULLPTR);

    void beginPaint(const QRegion &region);
    void flush(const QRegion &region);


    bool event(QEvent* event) override;
    void setExtraRenderTargetCount(int count);

    virtual void initializeD3D();
    virtual void releaseD3D();
    virtual void resizeD3D(const QSize &size);
    virtual void paintD3D();
    virtual void afterPresent();

    ID3D12Device *device() const;
    ID3D12CommandQueue *commandQueue() const;
    ID3D12CommandAllocator *commandAllocator() const;
    ID3D12CommandAllocator *bundleAllocator() const;

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

signals:
    void HandleChanged(WId handle);

protected:

    // FIXME: paintevent
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *) override;

    QD3D12WindowPrivate* m_private;
};

