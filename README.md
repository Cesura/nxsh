# nxsh
BusyBox-like remote shell for Nintendo Switch over telnet

![screenshot1](https://github.com/Cesura/nxsh/blob/master/screenshots/nxsh-switch.jpg)


## Installation

### Prebuilt

For a basic installation, grab the latest nxsh.nro version from the "Releases" tab above. You *can* build it from source if you so choose, in which case you should check out the section below.

To make the application available at the root of your homebrew menu, simply copy the nxsh.nro file to the /switch directory of your SD card. You can do this directly, but I recommend using one of the many ftpd applications that are available on the homebrew app store.

### Building from source (Unix)

If you choose to compile the application from source, clone the latest code from GitHub and build:
```
$ git clone https://github.com/Cesura/nxsh.git
$ cd nxsh
$ make
```

I've included a Makefile for simplifying the build process, but you *will* need to have an aarch64 cross compiler installed, as well as the latest version of libnx and devkitpro. More information on how to set up a proper build environment can be found [here](https://switchbrew.org/wiki/Setting_up_Development_Environment).

## Connecting

Once started, the application will listen on port 5050 for incoming connections. If the socket initializes correctly, the listen address will be displayed on your Switch's screen. Unfortunately, until multithreading is fully supported for Switch homebrew apps, the shell cannot accept more than 1 simultaneous connection.

### Linux/BSD/Mac OS

First ensure that you have the "telnet" client installed on your local machine (it may or may not be bundled with your base distribution). For Mac users, I recommend installing the client via the Homebrew package manager 
```
$ brew install telnet
```

From there, connecting to nxsh is as simple as:
```
$ telnet [address] 5050
```
where address is your Switch's IP.

### Windows

The built-in Windows telnet client does not play nicely with nxsh, so I recommend installing [PuTTY](https://www.putty.org/).

## Usage

nxsh bundles its own implementations of basic coreutils, such as (but not limited to):
* cat
* ls
* rm

etc. etc.

For a full list of commands supported by your current version of nxsh, type "help" in the prompt.

## Bugs

As this is an alpha release, there are bound to be many unforeseen bugs in the software. If it segfaults or something, I take no responsibility, but I *would* be very appreciative of a new GitHub issue explaining out your problem. ;)

## License

This project is licensed under the BSD 3-clause license.
