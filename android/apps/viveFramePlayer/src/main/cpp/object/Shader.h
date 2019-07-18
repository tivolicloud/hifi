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
#include <GLES3/gl31.h>
#include <GLES3/gl3ext.h>
#include <vector>
#include <memory>

class Shader {
private:
    const char * mName;
    const char * mVName;
    const char * mFName;
    const char * mVertexShader;
    const char * mFragmentShader;
    GLuint mProgramId;
    static std::vector<std::weak_ptr<Shader> > mShaderPool;

public:
    Shader(const char * name, const char * vname, const char * vertex, 
        const char * fname, const char * fragment);

    ~Shader();

    inline void useProgram() {
        glUseProgram(mProgramId);
    }

    inline void unuseProgram() {
        glUseProgram(0);
    }

    static void putShader(const std::shared_ptr<Shader>& shader);
    static std::shared_ptr<Shader> findShader(const char * vname, const char * fname);

private:
    bool hasShaderError(const char * type, int shaderId);

public:
    bool compile();
    int getUniformLocation(const char * name);
    int getAttributeLocation(const char * name);
};
