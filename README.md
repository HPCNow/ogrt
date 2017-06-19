# OGRT

## Ominous Glibc Runtime Tracker

OGRT is a tool designed to track user processes on a HPC cluster.
It extends on the concepts introduced with [XALT](https://github.com/Fahey-McLay/xalt).

### Features

* Transparent tracking of user processes
* Transparent tracking of shared objects a process loaded
* Watermarking of applications at link-time
* Ultra-fast reading of watermarks
* Capturing of process environment (whole environment or single
  variables)
* Capturing of loaded environment modules
* Filtering of monitored binaries (eg. skip monitoring of /bin)
* Zero runtime dependencies - runs in any environment
* Configurable outputs (Elasticsearch, Splunk, File)
* Painless deployment

### Limitiations

* Only works with dynamic executables
* Only works on GLIBC systems (depends on LD_PRELOAD and GLIBC
  functions)


### Presentations

#### [[Into the Job: Gaining Insight into Your Workloads Using OGRT]](http://hpckp.org/index.php/conference/2016/159-into-the-job-gaining-insight-into-your-workloads-using-ogrt)

Introduction of OGRT on the [HPCKP16](http://hpckp.org/index.php/conference/2016)

#### How to stalk the users of your cluster using OGRT: [[slides]](http://goo.gl/zbvChr) [[recording]](https://www.youtube.com/watch?v=3l0eJq0nrOU)

An introduction to OGRT on the [1st EasyBuild User
Meeting](https://github.com/hpcugent/easybuild/wiki/1st-EasyBuild-User-Meeting), includes a demo of tracking functionality and
getting the data into Elasticsearch/Kibana. Also some history on
how OGRT came to be.


### Quick Start

Get going with OGRT on your local machine in under 10 minutes!

#### Server

Open a terminal and run:

    wget -q https://github.com/IMPIMBA/ogrt/releases/download/v0.4.1/ogrt-server-v0.4.1.tar.bz2
    tar xf ogrt-server-v0.4.1.tar.bz2
    cd ogrt-server-v0.4.1
    ./ogrt-server


#### Client

In another terminal:

    git clone https://github.com/IMPIMBA/ogrt.git
    cd ogrt/client
    ./vendorize
    ./configure --prefix=/tmp/ogrt
    make install
    LD_PRELOAD=$(find /tmp/ogrt/ -name libogrt.so) OGRT_ACTIVE=1 bash
    # every command you run in the spawned bash gets sent to the server
    ls

## Architecture

### client

Preload library written in C. It needs to be preloaded into the process
that needs to be tracked.

This library uses GNU libc facilities to query the loaded shared objects
of the process it was loaded into. It also checks these shared objects
for a signature. This signature is not used at the moment, but it is
intended for tagging programs at link time and then reading them at
runtime. Reading of this signature happens in memory and is quite fast
(preloading into an interactive shell is not noticable).

All information gathered by this library is packed into a protobuf message
and sent (via UDP) to the server. Failure in the preload library
does not interrupt program execution.


### server

Daemon written in Go. The purpose of this daemon is receive and
preprocess data from the preload library, before persisting it.

It supports the following outputs with a configurable number of
simultaneous workers:

* JSON over TCP (for e.g. Splunk)
* Elasticsearch
* JSON to local filesystem (for debugging only)

It is configured using a config file (ogrt.conf).

### protocol

Contains the protobuf protocol definition. After modifying this file you
need to run 'generate-protocol' and recompile the preload library and
the daemon. You can also use the protobuf definition to implement your
version of the client/server.

## Building

### client

Requirements:

* [Google Protocol Buffers](https://github.com/google/protobuf)
* [Protocol Buffers for C](https://github.com/protobuf-c/protobuf-c) (as static library)
* the uuid library from util-linux (also static)
* libelf development headers

Compilation:

1. Change to the client directory.
2. Make sure your machine fulfills the requirements. If you do not care
   about specifics use the vendorize script.
3. Run ./configure --server-host=[ogrt-server] --env-jobid="JOBID"
   --prefix=[installdir]
4. Run 'make install'
5. You now have libogrt.so in [installdir], which talks to
   [ogrt-server] on port 7971 and uses the environment variable JOBID to
   figure out the ID of the currently running job

Configuration:

The client is very flexible in what it sends to the server.
Run "./configure --help" for a full list of options.

### server

The server uses a Makefile that automatically sets the
GOPATH and uses vendored dependecies.

Compilation:

1. Make sure you have a working installation of Golang
5. Run 'make setup' in the 'server' directory
5. Run 'make' in the 'server' directory
6. Your server binary is 'server/ogrt-server'
7. For guidance on how to configure the outputs check ogrt.conf in the
   server directory.

## Running

Execute the server binary. The config file should be in the same directory as
the server and the name must be named ogrt.conf. The default ogrt.conf
should be enough to get started.

For the client to be preloaded you need to set LD_PRELOAD to the absolute path
of libogrt.so. By default the client does not transmit data.

### Client Environment Variables

- **OGRT_ACTIVE**: activate OGRT (values: 0/1)
- **OGRT_SILENT**: supresses all output (values: 0/1)
- **OGRT_DEBUG_INFO**: print the settings OGRT was compiled with (values: 0/1)

## Example JSON Output

This is an example of the data provided by OGRT for the job "TESTJOB", which
only ran "gcc --help". Of the shared libraries only libogrt.so was watermarked.

    {
        "binpath": "/usr/bin/gcc-4.8",
        "cmdline": "/usr/bin/gcc --help",
        "hostname": "ogrtest",
        "job_id": "TESTJOB",
        "loaded_modules": [
            {
                "name": "gcc/4.6.2"
            },
            {
                "name": "dev"
            }
        ],
        "parent_pid": 3342,
        "pid": 12089,
        "shared_objects": [
            {
                "path": "/tmp/install/lib/libogrt.so",
                "signature": "708e1ffd-4ced-45d3-81f3-52e059ea3128"
            },
            {
                "path": "/lib/x86_64-linux-gnu/libc-2.19.so"
            },
            {
                "path": "/lib/x86_64-linux-gnu/ld-2.19.so"
            },
            {
                "path": "/lib/x86_64-linux-gnu/libnss_files-2.19.so"
            }
        ],
        "time": 1475062870,
        "username": "georg.rath"
    }

## License

All of this code is GPL3 licensed.

