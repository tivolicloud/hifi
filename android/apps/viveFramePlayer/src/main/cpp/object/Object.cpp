// "WaveVR SDK 
// © 2017 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#define LOG_TAG "Object"
#include <Object.h>
#include <Context.h>
#include <Shader.h>
#include <Texture.h>
#include <VertexArrayObject.h>
#include <log.h>

Object::Object() :
        mParent(NULL), mVAO(NULL),
        mTexture(NULL), mEnable(true), mHasError(false) {
}

Object::~Object() {
    if (mTexture != NULL)
        delete mTexture;
    mTexture = NULL;
    if (mVAO != NULL)
        delete mVAO;
    mVAO = NULL;
}

void Object::setEnable(bool enable) {
    mEnable = enable;
}

bool Object::areEnabled() const {
    const bool enable = mEnable;
    if (mParent != NULL)
        return enable & mParent->areEnabled();
    return enable;
}

bool Object::hasGLError() const {
    GLenum err = glGetError();
    if (err == GL_OUT_OF_MEMORY) {
        LogE(mName, "glGetError(): GL_OUT_OF_MEMORY");
        return true;
    } else if (err != GL_NO_ERROR) {
        LogE(mName, "glGetError(): %d", err);
        return true;
    } else {
        return false;
    }
}

void Object::loadShaderFromAsset(const char * vpath, const char * fpath) {
    if (strcmp(mName, "SeaOfCubes") == 0) {
        LOGI("loadShaderFromAsset for SeaOfCubes, ignore Shader::findShader");
    } else {
        mShader = Shader::findShader(vpath, fpath);
    }
    if (mShader != NULL)
        return;

    Context * context = Context::getInstance();
    EnvWrapper ew = context->getEnv();
    JNIEnv * env = ew.get();

    AssetFile vfile(context->getAssetManager(), vpath);
    AssetFile ffile(context->getAssetManager(), fpath);
    if (!vfile.open() || !ffile.open()) {
        LogE(mName, "Unable to read shader files");
        mHasError = true;
        return;
    }

    char * vstr = vfile.toString();
    char * fstr = ffile.toString();

    //LogD(mName, "%s\n%s\n", vpath, vstr);
    //LogD(mName, "%s\n%s\n", fpath, fstr);

    mShader = std::make_shared<Shader>(mName, vpath, vstr, fpath, fstr);
    bool ret = mShader->compile();

    delete [] vstr;
    delete [] fstr;
    if (!ret)
        mHasError = true;

    if (strcmp(mName, "SeaOfCubes") == 0) {
        LOGI("loadShaderFromAsset for SeaOfCubes, ignore Shader::putShader");
    } else {
        Shader::putShader(mShader);
    }
}

Object * Object::move(float x, float y, float z) {
    const float * m = mTransform.get();
    const float col [] = {m[12] + x, m[13] + y, m[14] + z, m[15]};
    mTransform.setColumn(3, col);
    return this;
}

Object * Object::move(const Vector4& direction, float distance) {
    // TODO
    return this;
}

Matrix4 Object::getTransforms() const {
    if (mParent != NULL) {
        return mParent->getTransforms() * mTransform;
    } else {
        return mTransform;
    }
}

Matrix3 Object::makeNormalMatrix(const Matrix4& view) const {
    const float * m = view.get();
    Matrix3 out(m[0], m[1], m[2], m[4], m[5], m[6], m[8], m[9], m[10]);
    return out.invert().transpose();
}

void Object::draw(const Matrix4& projection, const Matrix4& eye, const Matrix4& view, const Vector4& lightDir) {
}
