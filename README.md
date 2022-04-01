## Quake For PS2

A proper port of Quake for the PS2, written to actually take advantage of the Playstation 2 architechure.

(Planned) features:
 - Native Pad support
 - VU0-accelerated mathlib (in macro mode. It might be faster to maybe key in microprograms sometimes in hot loops)
 - VU1/GS proper hardware rendering refresh
 - (Maybe?) Redbook/audio streaming?

## Building

Should be like any old cmake project, just set the toolchain to cmake/Toolchain/ps2.cmake and off you go

You need PS2SDK obviously.
