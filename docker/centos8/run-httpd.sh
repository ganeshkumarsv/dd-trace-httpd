#!/bin/bash

# Based on the script for centos 7 at
# https://raw.githubusercontent.com/CentOS/CentOS-Dockerfiles/master/httpd/centos7/run-httpd.sh

# Make sure we're not confused by old, incompletely-shutdown httpd
# context after restarting the container.  httpd won't start correctly
# if it thinks it is already running.
rm -rf /run/httpd/* /tmp/httpd*

# Set environment variables for the datadog tracer
export DD_SERVICE=traced-httpd-centos8
export DD_AGENT_HOST=dd-agent

exec /usr/sbin/httpd -DFOREGROUND
