#
# Addon makefile for the "TriggerRally.dev" Dev-C++ project;
# its purpose is to conveniently copy DLL files to the binary folder.
#
# Written for Trigger Rally 0.6.3
#
# TODO: make sure backslashes won't cause problems
#

LIBWINPATH  = ..\..\libraries-win32\bin
LIBFMODPATH = C:\Program Files\FMOD SoundSystem\FMOD Studio API Windows\api\lowlevel\lib
TRBINDIR    = ..\bin
DLLFILES    = \
    $(TRBINDIR)\fmod.dll \
    $(TRBINDIR)\glew32.dll \
    $(TRBINDIR)\libjpeg-9.dll \
    $(TRBINDIR)\libphysfs.dll \
    $(TRBINDIR)\SDL.dll \
    $(TRBINDIR)\SDL_image.dll

all-after: $(DLLFILES)

$(TRBINDIR)\fmod.dll:
	@xcopy "$(LIBFMODPATH)\fmod.dll" "$(TRBINDIR)"

$(TRBINDIR)\glew32.dll:
	@xcopy "$(LIBWINPATH)\glew32.dll" "$(TRBINDIR)"

$(TRBINDIR)\libjpeg-9.dll:
	@xcopy "$(LIBWINPATH)\libjpeg-9.dll" "$(TRBINDIR)"

$(TRBINDIR)\libphysfs.dll:
	@xcopy "$(LIBWINPATH)\libphysfs.dll" "$(TRBINDIR)"

$(TRBINDIR)\SDL.dll:
	@xcopy "$(LIBWINPATH)\SDL.dll" "$(TRBINDIR)"

$(TRBINDIR)\SDL_image.dll:
	@xcopy "$(LIBWINPATH)\SDL_image.dll" "$(TRBINDIR)"
