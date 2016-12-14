/*
 * https://github.com/kripken/emscripten/blob/master/tests/openal_playback.cpp
 */


#include <stdio.h>
#include <stdlib.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif
#ifdef ANDROID
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

AAssetManager*mgr;

#define  FILE AAsset
#define fopen(path,mode) AAssetManager_open(mgr,path,AASSET_MODE_BUFFER)
#define fseek(file,offset,whence) AAsset_seek(file,offset,whence)
#define fread(buf,size,n,file)  AAsset_read(file, buf, size);
#define ftell(file)  AAsset_getLength(file)
#define fclose(file)  AAsset_close(file)

#endif

ALCdevice* device;
ALCcontext* context;
ALuint buffers[1];
ALuint sources[1];

void playSource(void* arg) {

    ALuint source = static_cast<ALuint> (reinterpret_cast<intptr_t> (arg));


    ALint state;

    alSourceStop(source);
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    assert(state == AL_STOPPED);

    alDeleteSources(1, sources);
    alDeleteBuffers(1, buffers);
    device = alcGetContextsDevice(context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

int main() {


    int major, minor;
    alcGetIntegerv(NULL, ALC_MAJOR_VERSION, 1, &major);
    alcGetIntegerv(NULL, ALC_MAJOR_VERSION, 1, &minor);

    assert(major == 1);

    printf("ALC version: %i.%i\n", major, minor);
    printf("Default device: %s\n", alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER));

    device = alcOpenDevice(NULL);
    context = alcCreateContext(device, NULL);
    alcMakeContextCurrent(context);

    assert(alGetString(AL_VERSION));

    printf("OpenAL version: %s\n", alGetString(AL_VERSION));
    printf("OpenAL vendor: %s\n", alGetString(AL_VENDOR));
    printf("OpenAL renderer: %s\n", alGetString(AL_RENDERER));

    ALfloat listenerPos[] = {0.0, 0.0, 0.0};
    ALfloat listenerVel[] = {0.0, 0.0, 0.0};
    ALfloat listenerOri[] = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0};

    alListenerfv(AL_POSITION, listenerPos);
    alListenerfv(AL_VELOCITY, listenerVel);
    alListenerfv(AL_ORIENTATION, listenerOri);

    // check getting and setting global gain
    ALfloat volume;
    alGetListenerf(AL_GAIN, &volume);
    assert(volume == 1.0);
    alListenerf(AL_GAIN, 0.0);
    alGetListenerf(AL_GAIN, &volume);
    assert(volume == 0.0);

    alListenerf(AL_GAIN, 1.0); // reset gain to default



    alGenBuffers(1, buffers);

    FILE* source = fopen("sounds/h.wav", "rb");

    if (source == NULL)
        printf("\nfopen error\n");


    fseek(source, 0, SEEK_END);
    int size = ftell(source);
    fseek(source, 0, SEEK_SET);

    unsigned char* buffer = (unsigned char*) malloc(size);
    fread(buffer, size, 1, source);

    unsigned offset = 12; // ignore the RIFF header
    offset += 8; // ignore the fmt header
    offset += 2; // ignore the format type

    unsigned channels = buffer[offset + 1] << 8;
    channels |= buffer[offset];
    offset += 2;
    printf("Channels: %u\n", channels);

    unsigned frequency = buffer[offset + 3] << 24;
    frequency |= buffer[offset + 2] << 16;
    frequency |= buffer[offset + 1] << 8;
    frequency |= buffer[offset];
    offset += 4;
    printf("Frequency: %u\n", frequency);

    offset += 6; // ignore block size and bps

    unsigned bits = buffer[offset + 1] << 8;
    bits |= buffer[offset];
    offset += 2;
    printf("Bits: %u\n", bits);

    ALenum format = 0;
    if (bits == 8) {
        if (channels == 1)
            format = AL_FORMAT_MONO8;
        else if (channels == 2)
            format = AL_FORMAT_STEREO8;
    } else if (bits == 16) {
        if (channels == 1)
            format = AL_FORMAT_MONO16;
        else if (channels == 2)
            format = AL_FORMAT_STEREO16;
    }

    offset += 8; // ignore the data chunk

    printf("Start offset: %d\n", offset);

    alBufferData(buffers[0], format, &buffer[offset], size - offset, frequency);

    ALint val;
    alGetBufferi(buffers[0], AL_FREQUENCY, &val);
    assert(val == frequency);
    alGetBufferi(buffers[0], AL_SIZE, &val);
    assert(val == size - offset);
    alGetBufferi(buffers[0], AL_BITS, &val);
    assert(val == bits);
    alGetBufferi(buffers[0], AL_CHANNELS, &val);
    assert(val == channels);

    alGenSources(1, sources);

    assert(alIsSource(sources[0]));

    alSourcei(sources[0], AL_BUFFER, buffers[0]);

    ALint state;
    alGetSourcei(sources[0], AL_SOURCE_STATE, &state);
    assert(state == AL_INITIAL);

    alSourcePlay(sources[0]);

    alGetSourcei(sources[0], AL_SOURCE_STATE, &state);
    assert(state == AL_PLAYING);

#ifdef EMSCRIPTEN
    emscripten_async_call(playSource, reinterpret_cast<void*> (sources[0]), 1200);
#else
    usleep(1200000);
    playSource(reinterpret_cast<void*> (sources[0]));
#endif


    return 0;
}

#ifdef ANDROID
extern "C" {

    JNIEXPORT int JNICALL Java_android_platform_audio_MainActivity_play(JNIEnv *env, jobject obj, jobject assetManager) {

        mgr = AAssetManager_fromJava(env, assetManager);

        return main();
    }
}
#endif

