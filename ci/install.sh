#!/bin/bash
set -e # Enables cheking of return values from each command
set -x # Prints every command

# This file is only used from Travis CI, where the only Linux distro used is Ubuntu

if [[ "$(uname -s)" == 'Linux' ]]; then
    sudo apt-get update
    sudo apt-get install cmake
    sudo apt-get install zlib1g-dev libssh-dev python3-pip libxml2-utils

    if [ -n "$WITH_VALGRIND" ]; then
        sudo apt-get install valgrind
    fi
    sudo pip install virtualenv
    virtualenv conan
    source conan/bin/activate
    pip install conan==1.30.2
    pip install codecov
    pip install lxml
fi

python3 --version

sudo pip3 install virtualenv
virtualenv conan
source conan/bin/activate
pip3 install conan==1.30.2
pip3 install codecov
pip3 install lxml

conan --version
conan config set storage.path=~/conanData
conan profile new default --detect

if [[ "$(uname -s)" == 'Linux' ]]; then
    conan profile update settings.compiler.libcxx=libstdc++11 default
fi
