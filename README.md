# get
[![Build Status](https://travis-ci.org/vgmoose/get.svg?branch=master)](https://travis-ci.org/vgmoose/get)

get is a WIP mini package manager. It's my intent to use this as the backend to [Homebrew App Store](http://github.com/vgmoose/hbas) to allow more formal package management.

## Why?
**get** is intended for use in smaller systems, such as homebrew'd video game consoles, where there is code execution but not a full stack of tools that package managers usually utilize, such as shell scripts or external libraries.

For early documentation on the proposed metadata syntax, see [this gist](https://gist.github.com/vgmoose/90f48949c95927c8e92c990bd6985b38).

## Usage
### Setting up repos
A "repos.json" file should be present on the local machine, in the same directory as the `get` binary. See [repos.json](https://github.com/vgmoose/get/blob/master/repos.json) for what this file should look like. If this file does not exist, depending on the use case (such as on the Wii U) a default repos.json should automatically be generated.

When `get` runs, it will go through the enabled repos in that config file, and try to make a GET request to `/repo.json`, which should contain (on the remote server) a listing of all of the packages and descriptions. Here is an example of what the remote's repo.json with one package looks like:
```
{
  "packages": [
    {
      "name": "space",
      "title": "Space Game",
      "author": "vgmoose",
      "desc": "Shoot rocks in outer space, and stuff",
      "version": "1.0.0"
    }
  ]
}
```

### Installing a package
Installing a package requires the desired package name to exist in one of the repos, and for a GET to `/zips/$PKG_NAME.zip` to resolve. For example, to download the package `space` from above, the following command would be used:
```
./get space
```

It will try to fetch `/zips/space.zip` from the repo that contains the `space` package, and save it in `sdroot`.

### Listing all available packages
```
./get -l
```

## Building for PC
First clone the repo recursively
```
git clone --recursive https://github.com/vgmoose/get.git
```

and cd into the folder and run make:
```
cd get
make
```

The `minizip` library is used to provide zip.h and zip.c files.
