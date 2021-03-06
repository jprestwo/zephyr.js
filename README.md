# The Zephyr.js Project

This purpose of this project is to provide a JavaScript API and environment
for IoT development on low-memory systems, using JerryScript and the Zephyr
operating system.

This code requires a local copy of JerryScript and Zephyr source, and we
will upstream code to those projects as appropriate, but this repo is for
everything else.

## File Descriptions
```zjs-env.sh``` - Source this file to be able to use tools from ```scripts/```
  anywhere.

## Subdirectories
- ```arc/``` - Contains sensor subsystem code for ARC side of the Arduino 101.
- ```build/``` - Directory generated by jsrunner build, can be safely removed.
- ```deps/``` - Contains dependency repos and scripts for working with them.
- ```samples/``` - Sample JavaScript files that can be build and executed with ```jsrunner```.
- ```scripts/``` - Subdirectory containing tools useful during development.
- ```src/``` - JS API bindings for JerryScript written directly on top of Zephyr.
- ```test/``` - Zephyr applications that test various components.

## Setting up your environment
To check out the other repos Zephyr.js is dependent on:

```
$ cd deps
$ ./getrepos
```

Later, to update them to the latest compatible code:
```
$ cd deps
$ ./update
```

Download the [latest Zephyr SDK] (https://nexus.zephyrproject.org/content/repositories/releases/org/zephyrproject/zephyr-sdk/), then:
```
$ chmod +x zephyr-sdk-0.*.*-i686-setup.run
$ sudo ./zephyr-sdk-0.*.*-i686-setup.run
```

Next, set up the Zephyr OS environment variables:
```
$ source deps/zephyr/zephyr-env.sh
```

If you wish to use a Zephyr tree elsewhere on your system, source its
zephyr-env.sh file instead.

Then, set up the Zephyr SDK environment variables. It is recommended that you
add the next two lines to your ```.bashrc``` so that you don't have to type
them again. If you installed your Zephyr SDK elsewhere, adjust as needed.
```
$ export ZEPHYR_GCC_VARIANT=zephyr
$ export ZEPHYR_SDK_INSTALL_DIR=/opt/zephyr-sdk
```

Next, set up the Zephyr.js environment variables:
```
$ source zjs-env.sh
```

Note: If you are using Zephyr.js with an Arduino101 that you have converted to
a 256KB X86 partition (see below), you should pass 256 to this command:
```
$ source zjs-env.sh 256
```

The command will display the current expected partition size to remind you of
the expected target.

## Getting more space on your Arduino 101
By default, Arduino 101 comes with a **144K** X86 partition, but we're able to
pretty safely increase it to **256K**. You should only use the ```dfu-util```
method of flashing after this.

The easiest way to do this is to use a flashpack.zip file that I can provide
you (geoff@linux.intel.com). I need to make sure of the license details before
I add it to the repo.

The instructions below are still refering to the old JTAG method.

## Build the *Hello World* sample
```
$ jsrunner samples/HelloWorld.js
```

```jsrunner``` creates a fresh ```build/``` directory, copies in some config
files from ```scripts/template```, generates a ```script.h``` include file from
the ```HelloWorld.js``` script, and builds using the ```src/``` files. Then it
tells you it's ready to flash your Arduino 101.

To use the DFU flashing method, you should have your A101 connected via a USB
cable. You reset the device with the Master Reset button and then hit Enter to
attempt the flashing about three seconds later. There is a window of about five
seconds where the DFU server is available, starting a second or two after the
device resets.

(You can also flash with JTAG and a Flyswatter 2 device connected, using the -j
flag on jsrunner. But this is no longer recommended because it's possible to
brick your device if you try to get it back to working with the original
firmware and DFU again.)

## Build other samples
The other samples may require some hardware to be set up and connected; read
the top of each JS file, but then simply pass in the path to the JS file to
```jsrunner``` as with ```HelloWorld.js``` above.

## Running JerryScript Unit Tests
```
$ cd apps/jerryscript_test/tests/
$ ./rununittest.sh
```

This will iterate through the unit test one by one. After each test completes, press Ctrl-x
to exit QEMU and continue with the next test.
