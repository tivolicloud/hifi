// "WaveVR SDK 
// © 2017 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#pragma once
#include <Object.h>
#include <Vectors.h>

class Texture;
class VertexArrayObject;
class Shader;

class SkyBox : public Object {
private:
    int mMatrixLocation = 0;
    int mTextureLocation = 0;
    Vector4 mLightDir;
    const int mVertices = 36;

public:
    SkyBox(bool debug);
    ~SkyBox();
    
    inline const Vector4& getLightDir() {
        return mLightDir;
    }

    void setDebug(bool debug);

private:
    void initVertices();
    void loadRandomTexture();

public:
    virtual void draw(const Matrix4& projection, const Matrix4& eye, const Matrix4& view, const Vector4& lightDir);
};
