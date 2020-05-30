#!/bin/sh

set -e
set -x

# N.B. Older Ubuntu versions do not carry the "googletest" package
# (which includes Google Mock and Google Test), so better to just
# install from source / git.

cd /tmp
rm -Rf googletest

git clone https://github.com/google/googletest.git
cd googletest

# for some reason this doesn't work
case "$(uname)" in
  Darwin)
    # /usr is read-only on macos, even with sudo
    PREFIX="/usr/local"
  ;;
  *)
    PREFIX="/usr"
  ;;
esac

# for some reason this doesn't work
case "$(uname)" in
  Darwin|Linux)
    # installs headers to /usr/include/gtest /usr/include/gmock
    # install static libraries to /usr/lib
    cmake -DCMAKE_CXX_FLAGS="-fPIC" -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX:PATH=${PREFIX} .
    make -j$(nproc --all) all install
    # install shared libraries to /usr/lib
    cmake -DCMAKE_CXX_FLAGS="-fPIC" -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX:PATH=${PREFIX} -DBUILD_SHARED_LIBS=ON .
    make -j$(nproc --all) all install
  ;;
  *)
    # installs headers to /usr/include/gtest /usr/include/gmock
    # install static libraries to /usr/lib
    cmake -G"MSYS Makefiles" -DCMAKE_CXX_FLAGS="-fPIC" -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX:PATH=${PREFIX} .
    mingw32-make -j$(nproc --all) all install
    # install shared libraries to /usr/lib
    cmake -G"MSYS Makefiles" -DCMAKE_CXX_FLAGS="-fPIC" -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX:PATH=${PREFIX} -DBUILD_SHARED_LIBS=ON .
    mingw32-make -j$(nproc --all) all install
  ;;
esac
