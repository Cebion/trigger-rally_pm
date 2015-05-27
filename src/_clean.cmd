@echo off
del Makefile.win
del TriggerRally.layout
for /R %%f in (*.o) do del %%f
