#! /bin/sh
# $Id: fedora25_install_dev_pkgs.sh 43921 2019-09-19 14:44:17Z katargir $

# Install development packages neccessary to build GBench from sources on Fedora 25
# Run this script under root priveleges

dnf -y install git make automake gcc gcc-c++ kernel-devel wget rpm-build rpm-sign pinentry \
    gtk2-devel gtk+-devel mesa-libGL-devel mesa-libGLU-devel \
    libdb-devel libxml2-devel libxslt-devel libsq3-devel ftgl-devel glew-devel \
    boost-devel lzo-devel gnutls-devel libidn2-devel libunistring-devel trousers-devel \
    flex
