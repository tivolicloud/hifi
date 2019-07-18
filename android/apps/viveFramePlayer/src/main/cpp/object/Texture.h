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

class Texture {
private:
    GLuint mTexture;
    uint8_t * mBitmap;
    size_t mWidth;
    size_t mHeight;
    size_t mStride;
    size_t mSize;
    size_t mType;  // Used in 
    size_t mFormat;

private:
    Texture();

public:
    ~Texture();

private:
    void setTexture(int texture);
    void clear();

public:
    static Texture * genTexture();
    static Texture * loadTexture(const char * assetFile);
    // loadSkyboxTexture will do glTexImage2D() inside.  Don't need do the bindBitmap().
    static Texture * loadSkyboxTexture(const char * assetFile);

    static uint8_t * cropBitmap(const uint8_t * origBitmap, const size_t origW, const size_t origH, const size_t x, const size_t y, const size_t w, const size_t h);

    inline void bindBitmap(int internalFormat = -1) {
        if (internalFormat != -1)
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, mWidth, mHeight, 0, mFormat, mType, mBitmap);
        else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, mFormat, mType, mBitmap);
    }

    inline void cleanBitmap() {
        if (mBitmap != NULL)
            delete [] mBitmap;
        mBitmap = NULL;
    }

    inline void bindTexture() {
        glBindTexture(GL_TEXTURE_2D, mTexture);
    }

    inline void unbindTexture() {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    inline void bindTextureCubeMap() {
        glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture);
    }

    inline void unbindTextureCubeMap() {
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
    
    inline size_t getWidth() {
        return mWidth;
    }

    inline size_t getHeight() {
        return mHeight;
    }

    inline GLuint getTextureId() {
        return mTexture;
    }

    inline size_t getFormat() {
        return mFormat;
    }

    inline size_t getType() {
        return mType;
    }

    inline uint8_t* getBitmap() {
        return mBitmap;
    }
};
