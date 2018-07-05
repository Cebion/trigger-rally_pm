// Copyright 2018      Emanuele Sorce,    emanuele.sorce@hotmail.com
// License: GPL version 2 (see included gpl.txt)

#ifndef PHYSFS_UTILSH
#define PHYSFS_UTILSH

#include <string>

//
// Functions protoypes for functions that wrap common PHYSFS functionalities
//
Sint64 physfs_size(SDL_RWops *context);
Sint64 physfs_seek(SDL_RWops *context, Sint64 offset, int whence);
size_t physfs_read(SDL_RWops *context, void *ptr, size_t size, size_t maxnum);
size_t physfs_write(SDL_RWops *context, const void *ptr, size_t size, size_t num);
int physfs_close(SDL_RWops *context);

// Get a string explaining the last error happened to physfs
std::string physfs_getErrorString();

#endif
