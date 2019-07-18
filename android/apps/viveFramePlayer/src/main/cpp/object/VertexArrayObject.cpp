// "WaveVR SDK 
// © 2017 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include <VertexArrayObject.h>

VertexArrayObject::VertexArrayObject(bool hasAB, bool hasEAB) :
    mVAO(0), mAB(0), mEAB(0) {
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    if (hasAB)
        glGenBuffers(1, &mAB);

    if (hasEAB)
        glGenBuffers(1, &mEAB);
}

VertexArrayObject::~VertexArrayObject() {
    glDeleteVertexArrays(1, &mVAO);
    glDeleteBuffers(1, &mAB);
    glDeleteBuffers(1, &mEAB);
}
