// Copyright 2018      Emanuele Sorce,    emanuele.sorce@hotmail.com
// License: GPL version 2 (see included gpl.txt)
//
// Functions protoypes for functions that wrap common PHYSFS functionalities
//

#ifndef PHYSFS_UTILSH
#define PHYSFS_UTILSH

#include <string>


Sint64 physfs_size(SDL_RWops *context);
Sint64 physfs_seek(SDL_RWops *context, Sint64 offset, int whence);
size_t physfs_read(SDL_RWops *context, void *ptr, size_t size, size_t maxnum);
size_t physfs_write(SDL_RWops *context, const void *ptr, size_t size, size_t num);
int physfs_close(SDL_RWops *context);

// Get a string explaining the last error happened to physfs
std::string physfs_getErrorString();

// Read a buffer from an handle
PHYSFS_sint64 physfs_read
(
	PHYSFS_File *	handle,
	void *			buffer,
	PHYSFS_uint32	objSize,
	PHYSFS_uint32	objCount
);

// Write a buffer to an handle
PHYSFS_sint64 physfs_write
(
	PHYSFS_File *	handle,
	const void *	buffer,
	PHYSFS_uint32	objSize,
	PHYSFS_uint32	objCount
);

// get the preferred config directory
std::string physfs_getDir();

// return if the file is a directory
bool physfs_isDirectory(const std::string& file);

#endif
