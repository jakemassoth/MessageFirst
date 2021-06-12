# MessageFirst: a reliable user API for message-oriented network workloads
This repository contains code for my thesis project where I designed and implemented MessageFirst, which is a user API
for message-oriented network workloads. It is a blocking API that supports the notion of a "message", a unit of transmission
that is not considered done until a message has been sent and a respective response has been received.
## Building and running tests
Before running any benchmarks, first build and run the tests to make sure your environment is set up correctly.

Requires CMake.

```
$ cmake .
$ make
$ make test
```

If these all run successfully, you're good to go!

## Running Examples
TODO

## Running Benchmarks
TODO