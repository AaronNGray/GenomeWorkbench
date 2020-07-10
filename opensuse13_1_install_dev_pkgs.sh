#! /bin/sh
# $Id: opensuse13_1_install_dev_pkgs.sh 43921 2019-09-19 14:44:17Z katargir $

# Install development packages neccessary to build GBench from sources on Fedora 18
# Run this script under root priveleges

zypper install make automake gcc gcc-c++ kernel-devel wget rpm-build \
    gtk2-devel Mesa-libGL-devel \
    libdb-4_8-devel libxml2-devel libxslt-devel sqlite3-devel ftgl-devel glew-devel \
    boost-devel lzo-devel git-core libgnutls-devel flex
