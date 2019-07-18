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
#include <stdio.h>
#include <string>
#include <vector>

#include <GLES3/gl31.h>
#include "shared/Matrices.h"

#include <wvr/wvr_device.h>
#include <wvr/wvr_events.h>
#include <wvr/wvr_types.h>
#include <Sphere.h>
#include <Floor.h>
class Context;
class Texture;
class SkyBox;
class SeaOfCubes;
class ControllerAxes;
class ControllerCube;
class ReticlePointer;
class FrameBufferObject;
class Picture;
class Clock;
class Object;

class MainApplication
{
public:
    MainApplication();
    virtual ~MainApplication();

    bool initVR();
    bool initGL();
    void shutdownVR();
    void shutdownGL();

    void setupCameras();
    void setupControllerCubes();
    void setupControllerCubeForDevice(WVR_DeviceType deviceType);
    ControllerCube * findOrLoadControllerCube(WVR_DeviceType deviceType);

    bool handleInput();
    void processVREvent(const WVR_Event_t & event);
    bool renderFrame();

    void renderStereoTargets();
    void drawControllers();
    void renderScene(WVR_Eye nEye);

    void updateTime();
    void updateHMDMatrixPose();
    void updateEyeToHeadMatrix(bool is6DoF);

    inline Matrix4 wvrmatrixConverter(const WVR_Matrix4f_t& mat) const {
        return Matrix4(
            mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
            mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
            mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
            mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
        );
    }

protected:
    void moveSphereHandler();
    WVR_DevicePosePair_t mVRDevicePairs[WVR_DEVICE_COUNT_LEVEL_1];

    Matrix4 mDevicePoseArray[WVR_DEVICE_COUNT_LEVEL_1];
    bool mShowDeviceArray[WVR_DEVICE_COUNT_LEVEL_1];

    int mControllerCount;
    int mControllerCount_Last;
    int mValidPoseCount;
    int mValidPoseCount_Last;

    std::string mPoseClasses;                            // what classes we saw poses for this frame
    char mDevClassChar[WVR_DEVICE_COUNT_LEVEL_1];        // for each device, a character representing its class

    float mNearClip;
    float mFarClip;

    Matrix4 mHMDPose;
    Matrix4 mEyePosLeft;
    Matrix4 mEyePosRight;

    Matrix4 mProjectionLeft;
    Matrix4 mProjectionRight;

    uint32_t mRenderWidth;
    uint32_t mRenderHeight;

    std::vector<ControllerCube *> mControllerCubes;
    ControllerCube * mControllerCubeTableById[WVR_DEVICE_COUNT_LEVEL_1];

    uint32_t mIndexLeft;
    uint32_t mIndexRight;

    void* mLeftEyeQ;
    void* mRightEyeQ;

    std::vector<FrameBufferObject*> mLeftEyeFBO;
    std::vector<FrameBufferObject*> mRightEyeFBO;

    std::vector<FrameBufferObject*> mLeftEyeFBOMSAA;
    std::vector<FrameBufferObject*> mRightEyeFBOMSAA;

    SkyBox * mSkyBox;
    ControllerAxes * mControllerAxes;
    Picture * mGridPicture;
    ReticlePointer * mReticlePointer;

    Matrix4 mWorldTranslation;  // a little backward and upper to avoid been in a cube.
    float mWorldRotation;  // a little backward and upper to avoid been in a cube.
    float mDriveAngle;
    float mDriveSpeed;

    float mTimeDiff;
    uint32_t mTimeAccumulator2S;  // add in micro second.
    struct timeval mRtcTime;
    int mFrameCount = 0;
    float mFPS = 0;

    uint32_t mClockCount = 0;

    bool m3DOF;
    bool mMove;

    bool mLight;
    bool mIs6DoFPose = true;
    Vector4 mLightDir;  // The Light direction and ambient light intensity.

    uint32_t mScene;
    bool mPointToSphere=false;
    bool mPointToSphere_L= false;
    bool mPointToSphere_R=false;
    Sphere::Color currColor=Sphere::Color::green;
    Sphere *mSphere;
    Floor *mFloor;
    Vector3 oriSpherePos;
    WVR_DeviceType mCurFocusController;

    void switchInteractionMode();
    void switchGazeTriggerType();
    std::vector<float> mLUV { 0,0 };
    std::vector<float> mUUV { 0,0 };

    WVR_InteractionMode mInteractionMode;
    WVR_GazeTriggerType mGazeTriggerType;

    void drawReticlePointer();
    void switchResolution();
};