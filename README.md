# MessageFirst: a reliable user API for message-oriented network workloads
This repository contains code for my thesis project where I designed and implemented MessageFirst, which is a user API
for message-oriented network workloads. It is a blocking API that supports the notion of a "message", a unit of transmission
that is not considered done until a message has been sent and a respective response has been received.
## Building examples
Requires CMake.

```
$ cmake .
$ make
```