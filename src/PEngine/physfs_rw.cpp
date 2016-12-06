
// physfs_rw.cpp [pengine]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)


#include "pengine.h"

Sint64 physfs_size(SDL_RWops *context)
{
    PHYSFS_file *pfile = (PHYSFS_file *)context->hidden.unknown.data1;

    return PHYSFS_fileLength(pfile);
}

Sint64 physfs_seek(SDL_RWops *context, Sint64 offset, int whence)
{
  PHYSFS_file *pfile = (PHYSFS_file *)context->hidden.unknown.data1;
  
  Sint64 target;
  
  Sint64 curpos = PHYSFS_tell(pfile);
  
  switch (whence) {
  default:
  case SEEK_SET:
    target = offset;
    break;
  case SEEK_CUR:
    target = curpos + offset;
    break;
  case SEEK_END:
    target = PHYSFS_fileLength(pfile) + offset;
    break;
  }
  
    Sint64 result = PHYSFS_seek(pfile, target);
    if (! result) {
        throw MakePException("Error seeking: " + PHYSFS_getLastError());
    }
    
    return PHYSFS_tell(pfile);
  
    PHYSFS_seek(pfile, target);
  
  return curpos;
}


size_t physfs_read(SDL_RWops *context, void *ptr, size_t size, size_t maxnum)
{
  PHYSFS_file *pfile = (PHYSFS_file *)context->hidden.unknown.data1;
  
  const Sint64 r = PHYSFS_read(pfile, ptr, size, maxnum);
  
  // reading 0 bytes is considered an error now, thanks SDL2!
  return r == -1 ? 0 : r;
}


size_t physfs_write(SDL_RWops *context, const void *ptr, size_t size, size_t num)
{
  PHYSFS_file *pfile = (PHYSFS_file *)context->hidden.unknown.data1;
  
  return PHYSFS_write(pfile, ptr, size, num);
}


int physfs_close(SDL_RWops *context)
{
  PHYSFS_file *pfile = (PHYSFS_file *)context->hidden.unknown.data1;
  
  PHYSFS_close(pfile);
  
  SDL_FreeRW(context);
  
  return 0;
}


