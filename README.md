# get
[![Build Status](https://travis-ci.org/vgmoose/get.svg?branch=master)](https://travis-ci.org/vgmoose/get)

get is a WIP mini package manager. It's my intent to use this as the backend to [Homebrew App Store](http://github.com/vgmoose/hbas) to allow more formal package management.

## Why?
**get** is intended for use in smaller systems, such as homebrew'd video game consoles, where there is code execution but not a full stack of tools that package managers usually utilize, such as shell scripts or external libraries.

For early documentation on the proposed metadata syntax, see [this gist](https://gist.github.com/vgmoose/90f48949c95927c8e92c990bd6985b38).

## Dependencies
The binary does not have any dependencies. To build the source zip.h and unzip.h are required. It's my impression that these are common libraries, as devkitpro provides them for arm and ppc.

If building on PC, `minizip` provides the above two files.
