# nxsh
BusyBox-like remote shell for the Nintendo Switch over telnet

Switch view:

![screenshot_1](https://i.imgur.com/qBBy9M4.jpg)

Remote view:

![screenshot_2](https://i.imgur.com/SaL7JNQ.png)


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

I've included a Makefile for simplifying the build process, but you *will* need to have an aarch64 cross compiler installed, as well as the latest version of libnx and devkitpro. As of version 0.1.5 alpha, you'll need to link it against libcurl and zlib as well. More information on how to set up a proper build environment can be found [here](https://switchbrew.org/wiki/Setting_up_Development_Environment).

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

The built-in Windows telnet client does not play nicely with nxsh, so I recommend installing [PuTTY](https://www.putty.org/). Note that connecting via a "raw" connection instead of a "telnet" one will avoid a pesky "command not found" message upon initialization of the shell. The telnet protocol sends some garbage text as it's negotiating the connection, so if you want a clean-looking shell, ditch that and go for a raw socket.

## Usage

nxsh bundles its own implementations of basic coreutils, such as (but not necessarily limited to):
* cat
* ls
* rm
* cp
* mv

etc. etc.

For a full list of commands supported by your current version of nxsh, type "help" in the prompt.

## Scripting

As of version 0.1.7 beta, nxsh supports scripting via an ECMAScript-compliant engine named duktape. You can execute scripts in the following way:
```
$ ./script_name.js
```

See examples/quadratic.js for an basic example script that works with the shell. As many of the more desirable JS features are non-standard and machine-dependent (such as file I/O), I am slowly but surely writing my own implementations of them.

## In Progress

Right now, I'm working on:
* Passing command line arguments to JS scripts
* Smoothing out general stability of the shell
* Executing other NROs from within the shell
* Any other requests you'd like to throw my way

## Bugs

As this is an alpha release, there are bound to be many unforeseen bugs in the software. If it segfaults or something, I take no responsibility, but I *would* be very appreciative of a little nudge in the way of a new GitHub issue pointing out your problem. ;)

## License

This project is licensed under the BSD 3-clause license.
