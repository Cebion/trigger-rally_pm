
// physfs_rw.cpp [pengine]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// Copyright 2018 Emanuele Sorce, emanuele.sorce@hotmail.com
// Copyright 2019 Andrei Bondor, ab396356@users.sourceforge.net
// License: GPL version 2 (see included gpl.txt)

//
// In this file there are functions that wrap up common PHYSFS related functions
//

#include <cassert>
#include "pengine.h"
#include "physfs_utils.h"

///
/// @brief Retrieves the size of the file.
/// @param [in] context     RWops context.
/// @return Size of the file.
/// @retval -1      Size unknown or error encountered.
///
Sint64 physfs_size(SDL_RWops *context)
{
    PHYSFS_File * const file = reinterpret_cast<PHYSFS_File *> (context->hidden.unknown.data1);

    return PHYSFS_fileLength(file);
}

///
/// @brief Seeks to an `offset` in the file, depending on the `whence` starting position.
/// @param [in] context     RWops context.
/// @param [in] offset      Offset to target.
/// @param [in] whence      Starting position.
/// @return The final offset achieved.
/// @retval -1      An error was encountered.
///
Sint64 physfs_seek(SDL_RWops *context, Sint64 offset, int whence)
{
    PHYSFS_File * const file = reinterpret_cast<PHYSFS_File *> (context->hidden.unknown.data1);
    PHYSFS_sint64 const curr = PHYSFS_tell(file);
    PHYSFS_sint64 const end = PHYSFS_fileLength(file);
    Sint64 pos;

    assert(curr != -1);
    assert(end != -1);

    switch (whence)
    {
        case RW_SEEK_SET:
        default:
            pos = 0 + offset;
            break;

        case RW_SEEK_CUR:
            pos = curr + offset;
            break;

        case RW_SEEK_END:
            pos = end + offset;
            break;
    }

    return PHYSFS_seek(file, pos) == 0 ? -1 : pos;
}

///
/// @brief Reads `maxnum` objects of size `size` from the file.
/// @param [in] context     RWops context.
/// @param [out] ptr        Destination of data to be read.
/// @param [in] size        Size of one object.
/// @param [in] maxnum      Maximum number of objects.
/// @return Number of objects read.
/// @retval 0   Error encountered and/or no objects read.
///
size_t physfs_read(SDL_RWops *context, void *ptr, size_t size, size_t maxnum)
{
    PHYSFS_File * const file = reinterpret_cast<PHYSFS_File *> (context->hidden.unknown.data1);
    #if PHYSFS_VER_MAJOR >= 3
    PHYSFS_sint64 const r = PHYSFS_readBytes(file, ptr, size * maxnum);

    return r == -1 ? 0 : r / size;
    #else
    PHYSFS_sint64 const r = PHYSFS_read(file, ptr, size, maxnum);

    return r == -1 ? 0 : r;
    #endif
}

///
/// @brief Writes `num` objects or size `size` to the file.
/// @param [in] context     RWops context.
/// @param [in] ptr         Source of data to be written.
/// @param [in] size        Size of one object.
/// @param [in] num         Number of objects.
/// @return Number of objects written.
/// @retval 0   Error encountered and/or no objects written.
///
size_t physfs_write(SDL_RWops *context, const void *ptr, size_t size, size_t num)
{
    PHYSFS_File * const file = reinterpret_cast<PHYSFS_File *> (context->hidden.unknown.data1);
    #if PHYSFS_VER_MAJOR >= 3
    PHYSFS_sint64 const r = PHYSFS_writeBytes(file, ptr, size * num);

    return r == -1 ? 0 : r / size;
    #else
    PHYSFS_sint64 const r = PHYSFS_write(file, ptr, size, num);

    return r == -1 ? 0 : r;
    #endif
}

///
/// @brief Closes the file and frees the `context`.
/// @param [in] context     RWops context.
/// @return Whether or not closing was successful.
/// @retval 0   Success.
/// @retval -1  Failure.
///
int physfs_close(SDL_RWops *context)
{
    PHYSFS_File * const file = reinterpret_cast<PHYSFS_File *> (context->hidden.unknown.data1);

    SDL_FreeRW(context);
    return PHYSFS_close(file) == 0 ? -1 : 0;
}

//
// @todo:
//
// PHYSFS 3 is pretty new, and not all the distro ship with it yet,
// But using the deprecated functions we have plenty of warnings at compile time on version 3,
// we change the code based on what version we build it. @todo: In the future, one
// day, maybe consider to remove the code for PHYSFS < 3
// Emanuele Sorce - 7/5/18
//
// Update: for version 0.6.6 remove the old code for PHYSFS < 3
// Emanuele Sorce - 8/14/18
//
std::string physfs_getErrorString()
{
	std::stringstream ss;
	#if PHYSFS_VER_MAJOR >= 3
		auto err = PHYSFS_getLastErrorCode();
		ss << err << " - " << PHYSFS_getErrorByCode(err);
	// version 2.x and downwards
	#else
		ss << PHYSFS_getLastError();
	#endif

	return ss.str();
}

PHYSFS_sint64 physfs_read
(
	PHYSFS_File *  	handle,
	void *  		buffer,
	PHYSFS_uint32  	objSize,
	PHYSFS_uint32  	objCount
)
{
	#if PHYSFS_VER_MAJOR >= 3
	return PHYSFS_readBytes(handle, buffer, objSize*objCount);
	#else
	return PHYSFS_read(handle, buffer, objSize, objCount);
	#endif
}

PHYSFS_sint64 physfs_write
(
	PHYSFS_File *	handle,
	const void *	buffer,
	PHYSFS_uint32	objSize,
	PHYSFS_uint32	objCount
)
{
	#if PHYSFS_VER_MAJOR >= 3
	return PHYSFS_writeBytes(handle, buffer, objSize*objCount);
	#else
	return PHYSFS_write(handle, buffer, objSize, objCount);
	#endif
}

std::string physfs_getDir()
{
	#if PHYSFS_VER_MAJOR >= 3
	return PHYSFS_getPrefDir("trigger-rally-team","trigger-rally");
	#else
	const std::string trdir = ".trigger-rally";
	PHYSFS_setWriteDir(PHYSFS_getUserDir());
	PHYSFS_mkdir(trdir.c_str());
	return PHYSFS_getUserDir() + trdir;
	#endif
}

bool physfs_isDirectory(const std::string& file)
{
	#if PHYSFS_VER_MAJOR >= 3
	PHYSFS_Stat stat;
	PHYSFS_stat(file.c_str(), &stat);

	return stat.filetype == PHYSFS_FILETYPE_DIRECTORY;

	#else
	return PHYSFS_isDirectory(file.c_str());
	#endif
}
