#
# Addon makefile for the "TriggerRally.dev" Dev-C++ project;
# its purpose is to conveniently copy .DLL files to the binary folder.
#
# Written for Trigger Rally 0.6.3
# Updated for Trigger Rally 0.6.4, 2016-04-21
#

LIBWINPATH      = ..\..\libraries-win32\bin
LIBFMODPATH     = C:\Program Files\FMOD SoundSystem\FMOD Studio API Windows\api\lowlevel\lib
TR_BINDIR       = ..\bin
TR_DLLFILES     =                   \
    $(TR_BINDIR)\fmod.dll           \
    $(TR_BINDIR)\glew32.dll         \
    $(TR_BINDIR)\libjpeg-9.dll      \
    $(TR_BINDIR)\libphysfs.dll      \
    $(TR_BINDIR)\libpng16-16.dll    \
    $(TR_BINDIR)\SDL.dll            \
    $(TR_BINDIR)\SDL_image.dll

all-after: $(TR_DLLFILES)

$(TR_BINDIR)\fmod.dll:
	@xcopy "$(LIBFMODPATH)\fmod.dll" "$(TR_BINDIR)"

$(TR_BINDIR)\glew32.dll:
	@xcopy "$(LIBWINPATH)\glew32.dll" "$(TR_BINDIR)"

$(TR_BINDIR)\libjpeg-9.dll:
	@xcopy "$(LIBWINPATH)\libjpeg-9.dll" "$(TR_BINDIR)"

$(TR_BINDIR)\libphysfs.dll:
	@xcopy "$(LIBWINPATH)\libphysfs.dll" "$(TR_BINDIR)"

$(TR_BINDIR)\libpng16-16.dll:
	@xcopy "$(LIBWINPATH)\libpng16-16.dll" "$(TR_BINDIR)"

$(TR_BINDIR)\SDL.dll:
	@xcopy "$(LIBWINPATH)\SDL.dll" "$(TR_BINDIR)"

$(TR_BINDIR)\SDL_image.dll:
	@xcopy "$(LIBWINPATH)\SDL_image.dll" "$(TR_BINDIR)"
