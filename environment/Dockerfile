FROM ubuntu:20.04

# Good practice
RUN apt-get update && apt-get -y upgrade

# Install libmraa
# https://github.com/up-board/up-community/wiki/MRAA#setup
# provides add-apt-repository
RUN apt-get install -y software-properties-common
RUN add-apt-repository ppa:mraa/mraa
RUN apt-get update
RUN apt-get install -y mraa-tools mraa-examples libmraa2 libmraa-dev libupm-dev libupm2 upm-examples
RUN apt-get install -y python-mraa python3-mraa libmraa-java

# Add development conveniences below
RUN apt-get install -y gcc