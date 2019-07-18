// "WaveVR SDK 
// © 2017 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include <GLES3/gl31.h>
#include <GLES3/gl3ext.h>

class VertexArrayObject {
private:
    GLuint mVAO;
    GLuint mAB;
    GLuint mEAB;

public:
    VertexArrayObject(bool hasAB, bool hasEAB);
    ~VertexArrayObject();

    inline GLuint getVertexArrayObject() {return mVAO;}

    inline GLuint getArrayBuffer() {return mAB;}

    inline GLuint getElementArrayBuffer() {return mEAB;}

    inline void bindVAO() {
        glBindVertexArray(mVAO);
    }

    inline void unbindVAO() {
        glBindVertexArray(0);
    }

    inline void bindArrayBuffer() {
        glBindBuffer(GL_ARRAY_BUFFER, mAB);
    }

    inline void unbindArrayBuffer() {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    inline void bindElementArrayBuffer() {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEAB);
    }

    inline void unbindElementArrayBuffer() {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
};
