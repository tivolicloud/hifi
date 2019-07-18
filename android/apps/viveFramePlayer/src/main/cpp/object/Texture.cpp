// "WaveVR SDK 
// © 2017 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#define LOG_TAG "Texture"
#include <Texture.h>
#include <Context.h>
#include <log.h>
#include <android/bitmap.h>
#include <GLES2/gl2.h>

Texture::Texture() :
    mTexture(0),
    mBitmap(NULL),
    mWidth(0),
    mHeight(0),
    mStride(0),
    mSize(0) {
}

Texture::~Texture() {
    clear();
}

void Texture::clear() {
    if (mTexture != 0) {
        glDeleteTextures(1, &mTexture);
        mTexture = 0;
    }
    cleanBitmap();
}

void Texture::setTexture(int texture) {
    clear();
    mTexture = texture;
}

Texture * Texture::genTexture() {
    GLuint texture = 0;
    glGenTextures(1, &texture);

    Texture * m = new Texture();
    m->setTexture(texture);
    return m;
}

Texture * Texture::loadTexture(const char * assetFile) {
    Context * context = Context::getInstance();
    EnvWrapper ew = context->getEnv();
    JNIEnv * env = ew.get();

    AssetFile textureFile(context->getAssetManager(), assetFile);
    if (!textureFile.open())
        return NULL;

    const void * data = textureFile.getBuffer();
    size_t length = textureFile.getLength();
    BitmapFactory * bf = context->getBitmapFactory();

    AndroidBitmapInfo info;
    uint8_t * bmp = bf->decodeByteArray(env, data, length, info);
    if (bmp == NULL)
        return NULL;

    Texture * texture = genTexture();
    texture->mBitmap = bmp;
    texture->mWidth = info.width;
    texture->mHeight = info.height;
    texture->mStride = info.stride;
    texture->mSize = info.stride * info.height;
    switch (info.format) {
    case ANDROID_BITMAP_FORMAT_RGB_565:
        texture->mType = GL_UNSIGNED_SHORT_5_6_5;
        texture->mFormat = GL_RGB565;
        break;
    case ANDROID_BITMAP_FORMAT_RGBA_4444:
        texture->mType = GL_UNSIGNED_SHORT_4_4_4_4;
        texture->mFormat = GL_RGBA4;
        break;
    case ANDROID_BITMAP_FORMAT_RGBA_8888:
        texture->mType = GL_UNSIGNED_BYTE;
        texture->mFormat = GL_RGBA;
        break;
    case ANDROID_BITMAP_FORMAT_A_8:
        texture->mType = GL_UNSIGNED_BYTE;
        texture->mFormat = GL_RED;
        break;
    default:
        texture->mType = GL_UNSIGNED_BYTE;
        texture->mFormat = GL_RGBA;
        break;
    }

    return texture;
}

uint8_t * Texture::cropBitmap(const uint8_t * origBitmap, const size_t origW, const size_t origH, const size_t x, const size_t y, const size_t w, const size_t h) {
    if (x >= origW || y >= origH) {
        LOGE("Croped image is fully out of bound.");
        return NULL;
    }
    const size_t x2 = x + w;
    const size_t y2 = y + h;
    size_t xpadding = 0;
    if (x2 > origW || y2 > origH)
        LOGW("Pixel filled of croped image will be less than expected because out of bound.");
    if (x2 > origW)
        xpadding = x2 - origW;

    uint8_t * croped = new uint8_t [w * h];
    uint8_t * ptr = croped;
    for (int j = y; j < origH && j < y2; j++) {
        const uint8_t * origPtr = origBitmap + x + origW * j;
        for (int i = x; i < origW && i < x2; i++)
            *ptr++ = *origPtr++;
        ptr += xpadding;
    }
    return croped;
}

Texture * Texture::loadSkyboxTexture(const char * assetFile) {
    Texture * texture = loadTexture(assetFile);
    if (texture == NULL)
        return NULL;

    size_t stride = texture->mStride / 4;
    size_t width = texture->mWidth / 4;
    size_t height = texture->mHeight / 3;
    if (((height * 3) != texture->mHeight) ||
        ((stride * 4) != texture->mStride)) {
        LOGW("May not a Skybox image.  Stop process");
        delete texture;
        return NULL;
    }

    /**
     *    1
     *  4 5 6 7
     *    9
     *    GL_TEXTURE_CUBE_MAP_POSITIVE_X 0	Right
     *    GL_TEXTURE_CUBE_MAP_NEGATIVE_X 1	Left
     *    GL_TEXTURE_CUBE_MAP_POSITIVE_Y 2	Top
     *    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 3	Bottom
     *    GL_TEXTURE_CUBE_MAP_POSITIVE_Z 4	Back
     *    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 5	Front
     */
    const int index[] = {6, 4, 1, 9, 5, 7};
    const int faces[] = {
                GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
            };

    texture->bindTextureCubeMap();

    for (int i = 0; i < 6; i++) {
        int x = stride * (index[i] % 4);
        int y = height * (index[i] / 4);
        uint8_t * bitmap = cropBitmap(texture->mBitmap, texture->mStride, texture->mHeight, x, y, stride, height);

        // Always output as GL_RGB5_A1 because the skybox don't need quality
    	glTexImage2D(faces[i], 0, GL_RGB5_A1, width, height, 0, texture->mFormat, texture->mType, bitmap);

    	delete [] bitmap;
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    texture->unbindTextureCubeMap();
    texture->cleanBitmap();

    return texture;
}
