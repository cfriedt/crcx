#!/bin/sh

set -e

# N.B. Older Ubuntu versions do not carry the "googletest" package
# (which includes Google Mock and Google Test), so better to just
# install from source / git.

cd /tmp
rm -Rf googletest

git clone https://github.com/google/googletest.git
cd googletest

# for some reason this doesn't work
#if [ "${TRAVIS_OS_NAME}" = "osx" ]; then
if [ "Darwin" = "$(uname)" ]; then
  # /usr is read-only on macos, even with sudo
  PREFIX="/usr/local"
else
  PREFIX="/usr"
fi

# installs headers to /usr/include/gtest /usr/include/gmock
# install static libraries to /usr/lib
cmake -DCMAKE_CXX_FLAGS="-fPIC" -DCMAKE_CXX_STANDARD=17  -DCMAKE_INSTALL_PREFIX:PATH=${PREFIX} . && make -j$(nproc --all) all install
# install shared libraries to /usr/lib
cmake -DCMAKE_CXX_FLAGS="-fPIC" -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX:PATH=${PREFIX} -DBUILD_SHARED_LIBS=ON && make -j$(nproc --all) all install

