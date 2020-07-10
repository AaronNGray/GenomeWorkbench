#! /bin/sh
# $Id: ubuntu18_install_dev_pkgs.sh 43921 2019-09-19 14:44:17Z katargir $

# Install development packages neccessary to build GBench from sources on Ubuntu 10
# Run this script under root priveleges

# Compiler, make etc.
apt-get -y install build-essential git

# To build wxWidgets
apt-get -y install libgtk2.0-dev libgl1-mesa-dev libglu1-mesa-dev

# To build GBench
apt-get -y install libbz2-dev liblzo2-dev libxml2-dev libxslt1-dev libdb5.3-dev \
                   libsqlite3-dev libglew-dev libftgl-dev libboost-dev \
                   libgnutls28-dev
