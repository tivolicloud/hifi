//package io.highfidelity.frameplayer;


package com.htc.vr.sdk;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.content.res.Configuration;
import android.hardware.display.DisplayManager;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.Display;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceView;
import android.view.View;
import com.htc.vr.sdk.VRMiracastConnector.Callback;
import com.htc.vr.sdk.VR;

import org.qtproject.qt5.android.bindings.QtActivity;

public class QtVRActivity extends QtActivity {
    private static final String TAG = "VRCORE_VRActivity";
    private VR mDelegate = new VR(this);
    private Handler mFadeOutHandler;

    public QtVRActivity() {
        Log.i("VRCORE_VRActivity", "VRActivity: start");
        Log.i("VRCORE_VRActivity", "VRActivity: end");
    }

    private void closeActivityInternal() {
        this.mDelegate.mVRPlatform.closeVrActivity();
    }

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i("VRCORE_VRActivity", "[CheckPoint_A_1] onCreate() start (" + this.getApplicationContext().getPackageName() + ") " + "3.0.104");
        this.mDelegate.onCreate(savedInstanceState);
        Log.i("VRCORE_VRActivity", "onCreate: end");
    }

    public void setRequestedOrientation(int requestedOrientation) {
        Log.d("VRCORE_VRActivity", "setRequestedOrientation(" + requestedOrientation + ")");
        if (!this.mDelegate.IsSVRPlatform()) {
            super.setRequestedOrientation(requestedOrientation);
        }
    }

    protected void onStart() {
        Log.i("VRCORE_VRActivity", "onStart() start (" + this.getApplicationContext().getPackageName() + ") " + "3.0.104");
        super.onStart();
        this.mDelegate.onStart();
        Log.i("VRCORE_VRActivity", "onStart() end");
    }

    protected void onResume() {
        Log.i("VRCORE_VRActivity", "onResume() start (" + this.getApplicationContext().getPackageName() + ") " + "3.0.104");
        super.onResume();
        this.mDelegate.onResume();
        Log.i("VRCORE_VRActivity", "onResume() end");
    }

    protected void onPause() {
        Log.i("VRCORE_VRActivity", "onPause() start (" + this.getApplicationContext().getPackageName() + ") " + "3.0.104");
        this.mDelegate.onPause();
        super.onPause();
        Log.i("VRCORE_VRActivity", "onPause() end");
    }

    protected void onStop() {
        Log.i("VRCORE_VRActivity", "onStop() start (" + this.getApplicationContext().getPackageName() + ") " + "3.0.104");
        this.mDelegate.onStop();
        super.onStop();
        Log.i("VRCORE_VRActivity", "onStop() end");
    }

    public void onConfigurationChanged(Configuration newConfig) {
        Log.i("VRCORE_VRActivity", "onConfigurationChanged() start (" + this.getApplicationContext().getPackageName() + ") " + "3.0.104");
        this.mDelegate.onConfigurationChanged(newConfig);
        super.onConfigurationChanged(newConfig);
        Log.i("VRCORE_VRActivity", "onConfigurationChanged() end");
    }

    protected void onDestroy() {
        Log.i("VRCORE_VRActivity", "onDestroy() (" + this.getApplicationContext().getPackageName() + ") " + "3.0.104");
        this.mDelegate.onDestroy();
        this.mDelegate.mVRPlatform.onOverlayDestroy();
        super.onDestroy();
    }

    public void onWindowFocusChanged(boolean hasFocus) {
        Log.d("VRCORE_VRActivity", "onWindowFocusChanged " + hasFocus);
        this.mDelegate.mVRPlatform.onWindowFocusChanged(hasFocus);
    }

    public void onLowMemory() {
        super.onLowMemory();
        Log.v("VRCORE_VRActivity", "onLowMemory");
        this.mDelegate.mVRPlatform.onLowMemory();
    }

    protected void informStop() {
        Log.v("VRCORE_VRActivity", "informStop() start (" + this.getApplicationContext().getPackageName() + ")");
        this.mDelegate.informStop();
        Log.v("VRCORE_VRActivity", "informStop() end");
    }

    public boolean onKeyUp(int keyCode, KeyEvent event) {
        return this.mDelegate.onKeyUp(keyCode, event);
    }

    public boolean onKeyDown(int keyCode, KeyEvent event) {
        return this.mDelegate.onKeyDown(keyCode, event);
    }

    public boolean dispatchKeyEvent(KeyEvent event) {
        this.mDelegate.dispatchKeyEvent(event);
        return super.dispatchKeyEvent(event);
    }

    private Display getTargetDisplay() {
        DisplayManager manager = (DisplayManager)this.getSystemService("display");
        Display[] presentationDisplays = manager.getDisplays("android.hardware.display.category.PRESENTATION");
        if (presentationDisplays.length > 0) {
            Display[] var3 = presentationDisplays;
            int var4 = presentationDisplays.length;

            for(int var5 = 0; var5 < var4; ++var5) {
                Display display = var3[var5];
                if (display.getName().contains("Overlay") || display.getName().contains("HDMI")) {
                    return display;
                }
            }
        }

        return this.getWindowManager().getDefaultDisplay();
    }

    public Context getContext() {
        return this;
    }

    public void closeVrActivity() {
        this.mDelegate.mVRPlatform.setFadeOutEnable(1);
        QtVRActivity.FadeOutCloseActivity runnable = new QtVRActivity.FadeOutCloseActivity();
        Handler handler = new Handler();
        int delayInterval = VRPlatform.getFadeOutDuration();
        handler.postDelayed(runnable, (long)delayInterval);
    }

    public void onVrEnvironmentVerifyCompleted(boolean hasExternalDisplay) {
        Log.d("VRCORE_VRActivity", "onVrEnvironmentVerifyCompleted");
        this.mDelegate.mVRPlatform.onVrEnvironmentVerifyCompleted(hasExternalDisplay);
    }

    public void onConfigChanged(Bundle data) {
        this.mDelegate.onConfigChanged(data);
    }

    public boolean dispatchTouchEvent(MotionEvent ev) {
        return this.mDelegate.dispatchTouchEvent(ev) || super.dispatchTouchEvent(ev);
    }

    public boolean onTouchEvent(MotionEvent event) {
        return this.mDelegate.onTouchEvent(event) || super.onTouchEvent(event);
    }

    public void WVR_startActivity(Runnable runnable) {
        int delayInterval = VRPlatform.getFadeOutDuration();
        if (delayInterval > 1) {
            this.mDelegate.mVRPlatform.setFadeOutEnable(1);
        } else {
            Log.d("VRCORE_VRActivity", "[FadeOut] have NOT to show fade out animation");
        }

        this.mFadeOutHandler = new Handler();
        this.mFadeOutHandler.postDelayed(runnable, (long)delayInterval);
    }

    public void setPresentView(View view) {
        this.mDelegate.mVRPlatform.setPresentView(view);
    }

    public void setUsingRenderBaseActivity(boolean usingRenderBaseActivity) {
        this.mDelegate.mVRPlatform.setUsingRenderBaseActivity(usingRenderBaseActivity);
    }

    public void setCustomContextSurfaceType(int customContextSurfaceType) {
        this.mDelegate.mVRPlatform.setCustomContextSurfaceType(customContextSurfaceType);
    }

    public boolean IsSVRPlatform() {
        return this.mDelegate.IsSVRPlatform();
    }

    public void finishOnUiThread() {
        Log.i("VRCORE_VRActivity", "finishOnUiThread");
        this.runOnUiThread(new Runnable() {
            public void run() {
                QtVRActivity.this.finish();
            }
        });
    }

    public void registerMirrorScreenSettingCallback(Callback callback) {
        this.mDelegate.mVRPlatform.registerMirrorScreenSettingCallback(callback);
    }

    public void hookUnitySurface(SurfaceView UnityPlayer) {
        this.mDelegate.mVRPlatform.hookUnitySurface(UnityPlayer);
    }

    public AssetFileDescriptor getControllerModelFileDescriptor(int deviceIndex) {
        return this.mDelegate.getControllerModelFileDescriptor(deviceIndex);
    }

    public String getConfigData(String key) {
        return this.mDelegate.getConfigData(key);
    }

    public Bundle getConfigBundle(String key) {
        return this.mDelegate.getConfigBundle(key);
    }

    public void onNativeReady() {
        Log.d("VRCORE_VRActivity", "onNativeReady");
    }

    private class FadeOutCloseActivity implements Runnable {
        public FadeOutCloseActivity() {
        }

        public void run() {
            QtVRActivity.this.closeActivityInternal();
        }
    }
}