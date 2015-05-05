//
// Copyright (C) 2004-2006 Jasmine Langridge, ja-reiko@users.sourceforge.net
// Copyright (C) 2015 Andrei Bondor, ab396356@users.sourceforge.net
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

// Available:
//
//  USE_NULL
//  USE_OPENAL
//  USE_FMOD
//  USE_SDL_MIXER
//
// ... and they're all squashed into this one file!

#include "../../config.h"
#include "../unrefp.h"

#ifdef UNIX
//#define USE_NULL
#define USE_OPENAL
#endif

#ifdef WIN32
//#define USE_NULL
//#define USE_OPENAL
#define USE_FMOD
#endif

#include "pengine.h"

// I was half way through implementing SDL_mixer support when
// I realised it has no ability to change pitch / playback
// speed. Don't enable USE_SDL_MIXER.

//#define USE_SDL_MIXER

// This function is common to the various implementations

PAudioSample *PSSAudio::loadSample(const std::string &name, bool positional3D)
{
    PAudioSample *samp = samplist.find(name);

    if (!samp)
    {
        try
        {
            samp = new PAudioSample(name, positional3D);
        }
        catch (PException e)
        {
            if (PUtil::isDebugLevel(DEBUGLEVEL_ENDUSER))
                PUtil::outLog() << "Failed to load " << name << ": " << e.what() << std::endl;

            return nullptr;
        }
        samplist.add(samp);
    }
    return samp;
}

#ifdef USE_NULL

PSSAudio::PSSAudio(PApp &parentApp) : PSubsystem(parentApp)
{
    PUtil::outLog() << "Initialising audio subsystem [NULL]" << std::endl;
}

PSSAudio::~PSSAudio()
{
    PUtil::outLog() << "Shutting down audio subsystem" << std::endl;
    samplist.clear();
}

PAudioSample::PAudioSample(const std::string &filename, bool positional3D)
{
    buffer = 0;

    if (PUtil::isDebugLevel(DEBUGLEVEL_TEST))
        PUtil::outLog() << "Loading sample \"" << filename << "\"" << std::endl;

    unload();
    name = filename;
}

void PAudioSample::unload()
{
}

PAudioInstance::PAudioInstance(PAudioSample *_samp, bool looping) :
    samp(_samp)
{
}

PAudioInstance::~PAudioInstance()
{
}


void PAudioInstance::update(const vec3f &pos, const vec3f &vel)
{
}

void PAudioInstance::setGain(float gain)
{
}

void PAudioInstance::setHalfDistance(float lambda)
{
}

void PAudioInstance::setPitch(float pitch)
{
}


void PAudioInstance::play()
{
}

void PAudioInstance::stop()
{
}

bool PAudioInstance::isPlaying()
{
    return false;
}

#endif // USE_NULL


#ifdef USE_OPENAL

#include <AL/al.h>
//#include <AL/alu.h> // not available in newest OpenAL
#include <AL/alut.h>

PSSAudio::PSSAudio(PApp &parentApp) : PSubsystem(parentApp)
{
    PUtil::outLog() << "Initialising audio subsystem [OpenAL]" << std::endl;

    if (alutInit(0, nullptr) != AL_TRUE)
        throw MakePException("ALUT:alutInit() error: " + alutGetErrorString(alutGetError()));
}

PSSAudio::~PSSAudio()
{
    PUtil::outLog() << "Shutting down audio subsystem" << std::endl;
    samplist.clear();
    alutExit();
}

PAudioSample::PAudioSample(const std::string &filename, bool positional3D)
{
    buffer = 0;
    positional3D = positional3D; // unused (atm)

    if (PUtil::isDebugLevel(DEBUGLEVEL_TEST))
        PUtil::outLog() << "Loading sample \"" << filename << "\"" << std::endl;

    unload();
    /* load contents from file into memory using physfs functions */
    name = filename;
    PHYSFS_file *pfile = PHYSFS_openRead(filename.c_str());

    if (!pfile)
    {
        throw MakePException ("Load failed: PhysFS: " + PHYSFS_getLastError());
    }

    int filesize = PHYSFS_fileLength(pfile);

    char *wavbuffer = new char[filesize];

    PHYSFS_read(pfile, wavbuffer, filesize, 1);
    PHYSFS_close(pfile);

    /* create the alut buffer from memory contents */
    this->buffer = alutCreateBufferFromFileImage(
                       reinterpret_cast<const ALvoid *>(wavbuffer),
                       filesize);

    /* clean up */
    delete [] wavbuffer;

    /* check if loading was successful */
    if (AL_NONE == this->buffer)
    {
        throw MakePException("Sample load failed:"
                             + alutGetErrorString(alutGetError()));
    }
}

void PAudioSample::unload()
{
    if (buffer)
    {
        alDeleteBuffers(1, &buffer);
        buffer = 0;
    }
}



PAudioInstance::PAudioInstance(PAudioSample *_samp, bool looping)
{
    samp = _samp;

    alGenSources(1, &source);

    alSourcei(source, AL_BUFFER, samp->buffer);
    alSourcei(source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);

    alSourcePlay(source);
}

PAudioInstance::~PAudioInstance()
{
    if (isPlaying()) stop();
    alDeleteSources(1, &source);
}


void PAudioInstance::update(const vec3f &pos, const vec3f &vel)
{
    alSourcefv(source, AL_POSITION, (vec3f)pos);
    alSourcefv(source, AL_VELOCITY, (vec3f)vel);
}

void PAudioInstance::setGain(float gain)
{
    //alSourcef(source, AL_MIN_GAIN, gain);
    //alSourcef(source, AL_MAX_GAIN, gain);
    alSourcef(source, AL_GAIN, gain);
}

void PAudioInstance::setHalfDistance(float lambda)
{
    alSourcef(source, AL_REFERENCE_DISTANCE, lambda);
}

void PAudioInstance::setPitch(float pitch)
{
    alSourcef(source, AL_PITCH, pitch);
}


void PAudioInstance::play()
{
    alSourceRewind(source);
    alSourcePlay(source);
}

void PAudioInstance::stop()
{
    alSourceStop(source);
}

bool PAudioInstance::isPlaying()
{
    int state = AL_STOPPED;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    return (state == AL_PLAYING);
}

#endif // USE_OPENAL

#ifdef USE_FMOD

#include <fmod.h>
#include <fmod_errors.h>

namespace
{

FMOD_SYSTEM *fs;

// callbacks to integrate PhysFS with FMOD
FMOD_RESULT F_CALLBACK fmod_file_open(const char *name, unsigned int *filesize, void **handle, void *userdata);
FMOD_RESULT F_CALLBACK fmod_file_close(void *handle, void *userdata);
FMOD_RESULT F_CALLBACK fmod_file_read(void *handle, void *buffer, unsigned int sizebytes, unsigned int *bytesread, void *userdata);
FMOD_RESULT F_CALLBACK fmod_file_seek(void *handle, unsigned int pos, void *userdata);

FMOD_RESULT F_CALLBACK fmod_file_open(const char *name, unsigned int *filesize, void **handle, void *userdata)
{
    UNREFERENCED_PARAMETER(userdata);

    if (PHYSFS_exists(name) == 0)
    {
        PUtil::outLog() << "PhysFS: file \"" << name << "\" was not found." << std::endl;
        return FMOD_ERR_FILE_NOTFOUND;
    }

    *handle = PHYSFS_openRead(name);

    if (*handle == nullptr)
    {
        PUtil::outLog() << "PhysFS: " << PHYSFS_getLastError() << std::endl;
        return FMOD_ERR_FILE_BAD;
    }

    *filesize = PHYSFS_fileLength(*reinterpret_cast<PHYSFS_File **> (handle));
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fmod_file_close(void *handle, void *userdata)
{
    UNREFERENCED_PARAMETER(userdata);

    if (PHYSFS_close(reinterpret_cast<PHYSFS_File *> (handle)) == 0)
        PUtil::outLog() << "PhysFS: could not close a file." << std::endl;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fmod_file_read(void *handle, void *buffer, unsigned int sizebytes, unsigned int *bytesread, void *userdata)
{
    UNREFERENCED_PARAMETER(userdata);

    if (PHYSFS_read(reinterpret_cast<PHYSFS_File *> (handle), buffer, sizebytes, 1) == -1)
    {
        PUtil::outLog() << "PhysFS: " << PHYSFS_getLastError() << std::endl;
        return FMOD_ERR_FILE_ENDOFDATA;
    }

    *bytesread = sizebytes;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fmod_file_seek(void *handle, unsigned int pos, void *userdata)
{
    UNREFERENCED_PARAMETER(userdata);

    if (PHYSFS_seek(reinterpret_cast<PHYSFS_File *> (handle), pos) == 0)
    {
        PUtil::outLog() << "PhysFS: " << PHYSFS_getLastError() << std::endl;
        return FMOD_ERR_FILE_COULDNOTSEEK;
    }

    return FMOD_OK;
}

}

///
/// @brief Initializes the FMOD audio subsystem.
/// @param [in,out] parentApp
///
PSSAudio::PSSAudio(PApp &parentApp):
    PSubsystem(parentApp)
{
    PUtil::outLog() << "Initialising audio subsystem [FMOD]" << std::endl;

    FMOD_RESULT fr = FMOD_System_Create(&fs);

    if (fr != FMOD_OK)
        throw MakePException("FMOD initialisation failed: " + FMOD_ErrorString(fr));

    fr = FMOD_System_Init(fs, 512, FMOD_INIT_NORMAL, nullptr);

    if (fr != FMOD_OK)
        throw MakePException("FMOD initialisation failed: " + FMOD_ErrorString(fr));

    fr = FMOD_System_SetFileSystem(
        fs,
        fmod_file_open,
        fmod_file_close,
        fmod_file_read,
        fmod_file_seek,
        nullptr,
        nullptr,
        -1
        );

    if (fr != FMOD_OK)
        throw MakePException("FMOD initialisation failed: " + FMOD_ErrorString(fr));
}

///
/// @brief Shuts down the FMOD audio subsystem.
///
PSSAudio::~PSSAudio()
{
    PUtil::outLog() << "Shutting down audio subsystem" << std::endl;
    samplist.clear();
    FMOD_System_Release(fs);
}

///
/// @brief Loads an audio sample within the FMOD audio subsystem.
/// @param [in] filename    The filename of the audio sample.
/// @param positional3D     Flag to load file as 3D or 2D.
///
PAudioSample::PAudioSample(const std::string &filename, bool positional3D):
    buffer(nullptr)
{
    if (PUtil::isDebugLevel(DEBUGLEVEL_TEST))
        PUtil::outLog() << "Loading sample \"" << filename << "\"" << std::endl;

    name = filename;

    FMOD_RESULT fr = FMOD_System_CreateSound(fs, filename.c_str(),
                     FMOD_UNIQUE | (positional3D ? FMOD_3D : FMOD_2D), nullptr, &buffer);

    if (fr != FMOD_OK)
        throw MakePException("Sample load failed: " + FMOD_ErrorString(fr));
}

///
/// @brief Unloads the audio sample within the FMOD audio subsystem.
///
void PAudioSample::unload()
{
    if (buffer != nullptr)
    {
        FMOD_Sound_Release(buffer);
        buffer = nullptr;
    }
}

///
/// @brief Sets up an audio sample to be played within the FMOD audio subsystem.
/// @todo IN params should be pointer-to-const.
/// @param [in] _samp       Sample to be played.
/// @param looping          Flag to enable looping.
///
PAudioInstance::PAudioInstance(PAudioSample *_samp, bool looping):
    samp(_samp)
{
    FMOD_System_PlaySound(fs, samp->buffer, nullptr, true, &source);
    FMOD_Channel_GetFrequency(source, &reserved1);
    FMOD_Channel_SetMode(source, looping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
}

PAudioInstance::~PAudioInstance()
{
    if (isPlaying())
        stop();
}

void PAudioInstance::update(const vec3f &pos, const vec3f &vel)
{
    // TODO
    UNREFERENCED_PARAMETER(pos);
    UNREFERENCED_PARAMETER(vel);
}

///
/// @brief Sets the gain of the channel.
/// @param gain     The desired gain.
/// @pre `gain` must be a value within the [0.0, 0.1] interval.
///
void PAudioInstance::setGain(float gain)
{
    CLAMP(gain, 0.0f, 1.0f);
    FMOD_Channel_SetVolume(source, gain);
}

void PAudioInstance::setHalfDistance(float lambda)
{
    // TODO
    UNREFERENCED_PARAMETER(lambda);
}

///
/// @brief Sets the pitch of the channel.
/// @param pitch    The desired pitch.
///
void PAudioInstance::setPitch(float pitch)
{
    FMOD_Channel_SetFrequency(source, pitch * reserved1);
}

///
/// @brief Unpauses the channel.
///
void PAudioInstance::play()
{
    FMOD_Channel_SetPaused(source, false);
}

///
/// @brief Stops the channel.
///
void PAudioInstance::stop()
{
    FMOD_Channel_Stop(source);
}

///
/// @brief Checks if the channel is playing.
/// @returns Whether or not the channel is playing.
/// @retval true    If the channel is playing.
/// @retval false   If the channel isn't playing.
/// @todo Stop fooling around and remove the redundant comments above.
///
bool PAudioInstance::isPlaying()
{
    FMOD_BOOL fb;

    FMOD_Channel_IsPlaying(source, &fb);
    return static_cast<bool> (fb);
}

#endif // USE_FMOD

#ifdef USE_SDL_MIXER

#include <SDL_mixer.h>

PSSAudio::PSSAudio(PApp &parentApp) : PSubsystem(parentApp)
{
    PUtil::outLog() << "Initialising audio subsystem [SDL_mixer]" << std::endl;

    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 1, 2048) != 0)
    {
        PUtil::outLog() << "SDL_mixer failed to initialise" << std::endl;
        PUtil::outLog() << "SDL_mixer: " << Mix_GetError() << std::endl;
        return PException ();
    }
}

PSSAudio::~PSSAudio()
{
    PUtil::outLog() << "Shutting down audio subsystem" << std::endl;

    samplist.clear();

    Mix_CloseAudio();
}


PAudioSample::PAudioSample(const std::string &filename, bool positional3D)
{
    buffer = 0;

    if (PUtil::isDebugLevel(DEBUGLEVEL_TEST))
        PUtil::outLog() << "Loading sample \"" << filename << "\"" << std::endl;

    unload();

    name = filename;

    PHYSFS_file *pfile = PHYSFS_openRead(filename.c_str());

    if (!pfile)
    {
        PUtil::outLog() << "Load failed: PhysFS: " << PHYSFS_getLastError() << std::endl;
        throw PFileException ();
    }

    buffer = (uint32) Mix_LoadWAV_RW(PUtil::allocPhysFSops(pfile), 1);

    PHYSFS_close(pfile);

    if (!buffer)
    {
        PUtil::outLog() << "Sample load failed" << std::endl;
        PUtil::outLog() << "SDL_mixer: " << Mix_GetError() << std::endl;
        throw PFileException ();
    }
}

void PAudioSample::unload()
{
    if (buffer)
        Mix_FreeChunk((Mix_Chunk *) buffer);
    buffer = 0;
}
}


PAudioInstance::PAudioInstance(PAudioSample *_samp, bool looping) :
    samp(_samp)
{
    source = (uint32) Mix_PlayChannel(-1
                                      (Mix_Chunk *)samp->buffer, looping ? -1 : 0);

    *((float*)&reserved1) = (float)FSOUND_GetFrequency((int)source);

    FSOUND_SetLoopMode((int)source,
                       looping ? FSOUND_LOOP_NORMAL : FSOUND_LOOP_OFF);
}

PAudioInstance::~PAudioInstance()
{
    if (source != -1)
    {
    }
}


void PAudioInstance::update(const vec3f &pos, const vec3f &vel)
{
    // TODO
}

void PAudioInstance::setGain(float gain)
{
}

void PAudioInstance::setHalfDistance(float lambda)
{
    // TODO
}

void PAudioInstance::setPitch(float pitch)
{
}


void PAudioInstance::play()
{
}

void PAudioInstance::stop()
{
}

bool PAudioInstance::isPlaying()
{
    return false;
}

#endif // USE_SDL_MIXER




