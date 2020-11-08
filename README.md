# Datadog APM Module for the Apache HTTP Server

The `ddtrace` module can be used with the [Apache HTTP Server](https://httpd.apache.org/) to add distributed tracing for HTTP requests handled by the server.

This project is currently Early Access, suitable for development and pre-production environments.
Requests for support and new features should be submitted by first searching the existing [issues](https://github.com/DataDog/dd-trace-httpd/issues),
and opening a [new issue](https://github.com/DataDog/dd-trace-httpd/issues/new) if applicable.

## HTTPD compatibility

TBD. This module targets httpd v2.4.x, and is tested against
- apache2 v2.4.29 on Ubuntu 18.04
- httpd v2.4.34 on Centos 6 using the `centos-sclo-rh` repo

## Installation

Pre-compiled packages are not available at this time.

## Build From Source

Building this project from source requires the (dd-opentracing-cpp)[https://github.com/DataDog/dd-opentracing-cpp] library.
This requires a C++14 compiler. For older Linux distros, this may not be available from the primary repositories.

Module installation and configuration files also differ between linux distributions.
The docker-based setups for [Ubuntu 18.04](https://github.com/DataDog/dd-trace-httpd/tree/master/docker/ubuntu1804) and 
[Centos 6](https://github.com/DataDog/dd-trace-httpd/tree/master/docker/centos6) can be used as a reference for the build and installation steps required.
