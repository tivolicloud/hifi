//
//  main.cpp
//  tests/render-utils/src
//
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <iostream>
#include <string>
#include <vector>

#include <gpu/gl/GLBackend.h>

#include <gl/QOpenGLContextWrapper.h>
#include <gl/GLHelpers.h>

#include <QDir>
#include <QElapsedTimer>
#include <QApplication>
#include <QLoggingCategory>
#include <QResizeEvent>
#include <QTimer>
#include <QWindow>
#include <qd3d12window.h>

class RateCounter {
    std::vector<float> times;
    QElapsedTimer timer;
public:
    RateCounter() {
        timer.start();
    }

    void reset() {
        times.clear();
    }

    unsigned int count() const {
        return (unsigned int)times.size() - 1;
    }

    float elapsed() const {
        if (times.size() < 1) {
            return 0.0f;
        }
        float elapsed = *times.rbegin() - *times.begin();
        return elapsed;
    }

    void increment() {
        times.push_back(timer.elapsed() / 1000.0f);
    }

    float rate() const {
        if (elapsed() == 0.0f) {
            return 0.0f;
        }
        return (float) count() / elapsed();
    }
};


const QString& getQmlDir() {
    static QString dir;
    if (dir.isEmpty()) {
        QDir path(__FILE__);
        path.cdUp();
        dir = path.cleanPath(path.absoluteFilePath("../../../interface/resources/qml/")) + "/";
        qDebug() << "Qml Path: " << dir;
    }
    return dir;
}

// Create a simple OpenGL window that renders text in various ways
class QTestWindow : public QD3D12Window 
{
public:
    RateCounter fps;

    void initializeD3D() override
    {
        f = createFence();
        ID3D12Device *dev = dxDevice();
        if (FAILED(dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, dxCommandAllocator(), Q_NULLPTR, IID_PPV_ARGS(&commandList)))) {
            qWarning("Failed to create command list");
            return;
        }
        commandList->Close();
    }

    void resizeD3D(const QSize &size) override
    {
        qDebug("resize %d %d", size.width(), size.height());
    }

    void paintD3D() override
    {
        // TODO: This is check is needed as this example does not handle shutdown properly, 
        //       and the QT timer could trigger a render after the window is closed. 
        if (isVisible())
        {
            dxCommandAllocator()->Reset();
            commandList->Reset(dxCommandAllocator(), Q_NULLPTR);

            transitionResource(backBufferRenderTarget(), commandList.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

            green += 0.01f;
            if (green > 1.0f)
                green = 0.0f;
            const float clearColor[] = { 0.0f, green, 0.0f, 1.0f };
            commandList->ClearRenderTargetView(backBufferRenderTargetCPUHandle(), clearColor, 0, Q_NULLPTR);

            transitionResource(backBufferRenderTarget(), commandList.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            commandList->Close();

            ID3D12CommandList *commandLists[] = { commandList.Get() };
            dxCommandQueue()->ExecuteCommandLists(_countof(commandLists), commandLists);

            waitForGPU(f);
        }
    }

    void releaseD3D() override
    {
    }

    void afterPresent()
    {
        waitForGPU(f);
    }

public:
    QTestWindow() : f(Q_NULLPTR), green(0)
    {
    }

    ~QTestWindow() 
    {
        delete f;
    }

private:
    Fence *f;
    ComPtr<ID3D12GraphicsCommandList> commandList;
    float green;
};

#ifndef SERIF_FONT_FAMILY
#define SERIF_FONT_FAMILY "Times New Roman"
#endif

//static const wchar_t* EXAMPLE_TEXT = L"Hello";
//static const wchar_t* EXAMPLE_TEXT = L"\xC1y Hello 1.0\ny\xC1 line 2\n\xC1y";

void testShaderBuild(const char* vs_src, const char * fs_src) {
    auto vs = gpu::Shader::createVertex(std::string(vs_src));
    auto fs = gpu::Shader::createPixel(std::string(fs_src));
    auto pr = gpu::Shader::createProgram(vs, fs);
    gpu::Shader::makeProgram(*pr);
}

void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message) {
    if (!message.isEmpty()) {
#ifdef Q_OS_WIN
        OutputDebugStringA(message.toLocal8Bit().constData());
        OutputDebugStringA("\n");
#else 
        std::cout << message.toLocal8Bit().constData() << std::endl;
#endif
    }
}


const char * LOG_FILTER_RULES = R"V0G0N(
hifi.gpu=true
)V0G0N";

int main(int argc, char** argv) {    
    QGuiApplication app(argc, argv);
    // qInstallMessageHandler(messageHandler);
    // QLoggingCategory::setFilterRules(LOG_FILTER_RULES);
    
    QTestWindow window;
    window.initialize();
    window.resize(1280, 720);
    window.showNormal();

    QTimer timer;
    timer.setInterval(1); // Qt::CoarseTimer acceptable
    app.connect(&timer, &QTimer::timeout, &app, [&] {
        window.paintD3D();
        window.flush();
    });
    timer.start();
    app.exec();
    return 0;
}

