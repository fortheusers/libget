# get
get is a WIP mini package manager. It's my intent to use this as the backend to [Homebrew App Store](http://github.com/vgmoose/hbas) to allow more formal package management.

## Why?
**get** is intended for use in smaller systems, such as homebrew'd video game consoles, where there is code execution but not a full stack of tools that package managers usually utilize, such as shell scripts or external libraries.

## Dependencies
The binary does not have any dependencies. To build the source zip.h and unzip.h are required. It's my impression that these are common libraries, as devkitpro provides them for arm and ppc.
