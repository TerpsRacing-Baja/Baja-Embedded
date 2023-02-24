# Baja-Embedded

This repository contains all of the code used for Terps Racing Baja Crew embedded systems.
It is maintained by the Electronics and Testing subteam.

Currently, the repo contains in-development code for the car's on-board computer and data
collection system. The client code "blast" is designed to interface with a webserver
configured to display a live feed of car diagnostics.

## Code Organization

The sensors installed on the Baja car are subject to change at any time, so our embedded system is designed to be configured dynamically. It deals with `sensor` structures that store the data associated with an individual, physical sensor on the car. At startup, a configuration file specifying the sensors on the car and their hardware port assignments is used to construct a list of sensors that can be polled continuously for data acquisition. The format for this configuration file is described later in this document.

The main looping structure of the embedded system lives in [src/blast.c](src/blast.c). This file contains boilerplate for setting up our hardware interfaces (we use Intel's [LibMRAA](https://iotdk.intel.com/docs/master/mraa/)) and calls to helper functions that construct an array of `sensor` objects from the configuration, in addition to the looping data acquisition logic. Helper functions for data structure creation, configuration parsing, and data formatting can be found in [src/blast_data.c](src/blast_data.c).

Individual update functions must be created for every sensor we put on the car. These use LibMRAA to interface with analog, digital, or I2C sensors and retrieve an updated sensor value. They may also perform scaling or mathematical operations as appropriate in order to prepare data to be processed later. Every update function has its own file in [src/sensors/](src/sensors/). We use function pointers to associate a given update function with a sensor object - see [src/sensor_table.c](src/sensor_table.c) for our jump table format and [src/include/blast_data.h](src/include/blast_data.h) for type and structure definitions.

All C files have in-line comments describing the granular behavior of functions. For overall explanations of functoin purposes and behavior, please reference the [include files](src/include/).

## Configuration File Format

A sensor configuration file has the following format:
```
SENSOR MODEL (as in jump table)|USER-DEFINED SENSOR NAME|SENSOR DIGITAL OR ANALOG PIN (or mux pin if I2C)
...
...
```
It should contain as many lines as there are sensors on the car.

## Compilation

We use an Intel Edison for our embedded system, so our compilation target is Intel x86, 64-bit. Typically we will compile the binary directly on the board, so cross-compilation is not an issue. We use Make as our build system (see [the Makefile](Makefile)).

There are two compilation flags that a user may be interested in. We use `TESTING` when compiling for some target other than the Edison, for which accessing certain features of LibMRAA may cause undefined behavior. We also use `DEBUG` for comprehensive printed data at runtime. Examples of how these flags are used to guard compilation can be seen throughout the source, and we compile with them by doing something like `make all CFLAGS="-DTESTING -DDEBUG"`.

Make sure to delete the build directory before pushing a commit. Or, write an ignore file so that it isn't an issue and merge it in.
