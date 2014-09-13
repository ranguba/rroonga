#!/bin/bash

export DEBIAN_FRONTEND=noninteractive
sudo apt-get update
sudo apt-get -y install libssl-dev libreadline-dev
sudo apt-get -y install git build-essential ruby mingw-w64

/vagrant/user-build.sh
