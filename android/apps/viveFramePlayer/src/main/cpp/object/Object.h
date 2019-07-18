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
#include <shared/Matrices.h>
#include <shared/Vectors.h>
#include <Shader.h>
#include <string>

class VertexArrayObject;
class Texture;

class Object {
protected:
    const char * mName;
    Object * mParent;
    Matrix4 mTransform;
    std::shared_ptr<Shader> mShader;
    VertexArrayObject * mVAO;
    Texture * mTexture;
    bool mEnable;
    bool mHasError;

public:
    Object();
    virtual ~Object();

    static inline bool hasGlExtension(const char * ext_name) {
        // It's not effencient but only use at initial.
        const char *v = (const char *) glGetString(GL_EXTENSIONS);
        std::string extensions = v;
        return !extensions.empty() &&
            extensions.find(ext_name) != std::string::npos;
    }

    void loadShaderFromAsset(const char * vfile, const char * ffile);

    virtual void setEnable(bool enable);
    
    inline bool isEnabled() const {
        return mEnable;
    }

    bool areEnabled() const;

    inline bool hasError() const {
        return mHasError;
    }

    bool hasGLError() const;

    inline void setParent(Object * parent) {
        mParent = parent;
    }

    inline Object * getParent() {
        return mParent;
    }

    inline Object * getParentRoot() {
        if (mParent != NULL)
            return getParentRoot();
        return this;
    }

    Object * move(float x, float y, float z);

    Object * move(const Vector4& direction, float distance);

    inline Matrix4& getTransform() {
        return mTransform;
    }

    Matrix4 getTransforms() const;
    
    Matrix3 makeNormalMatrix(const Matrix4& view) const;

    virtual void draw(const Matrix4& projection, const Matrix4& eye, const Matrix4& view, const Vector4& lightDir);
};
