# Threading

## Introduction

We need to switch to a threaded architecture in order to scale our embedded system. The primary purpose of this project is to put RaceCapture message reception, sensor update polling, and log file writes on separate threads. This will allow us to refresh more rapidly and deal with data as we need it, rather than blocking every time we want to update.

Do not worry about fully porting the existing functionality to this branch - we are primarily concerned with creating a template for the threaded redesign that we can guarantee works correctly. Until we have verified that concurrency works as expected, we shouldn't be working with hardware. This will at some point require building out a software-based testing suite.

This repo is empty, but feel free to borrow header files as needed to properly implement things like sensor objects. We will worry about properly merging new code once the project is complete.

## Specifications

Central to this redesign is the idea of a car model. We would like to model the "state" of the car at any time in software. This model of the car should be updated asynchronously by our threads so that it is always the most current. Access to the model should be mediated by mutexes, and ideally through an opaque interface of getters and setters. Consider the following structure as a starting point:
```c
typedef struct {
	sensor *sensor_list;
	gyro rc_gyro;
	accel rc_accel;
	gps rc_gps;
	...
} car;
```
Only the `sensor` type is currently implemented, while the other fields are just examples. You will have to determine the best way to associate (ie, structures) and store information associated with sensors on the car, which will require some understanding of the hardware involved.

Ideally, a reference to this car model will be shared between all threads that need access to it. As such, there should also be a related structure that stores mutexes for each field of the model. This will allow for thread-safe reading and writing.

The specific behavior of each thread is a concern for after the basic data and threading models have been created.
