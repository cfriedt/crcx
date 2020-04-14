[![Build Status](https://travis-ci.com/cfriedt/crcx.svg?branch=master)](https://travis-ci.com/cfriedt/crcx)
[![Codecov Status](https://codecov.io/gh/cfriedt/crcx/branch/master/graph/badge.svg)](https://codecov.io/gh/cfriedt/crcx)
[![Code Quality](https://api.codacy.com/project/badge/Grade/2591b5d32ac84f1897b4a7e8d45d1544)](https://www.codacy.com/app/cfriedt/crcx?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=cfriedt/crcx&amp;utm_campaign=Badge_Grade)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17 Compliant](https://img.shields.io/badge/c%2B%2B17-compliant-blue)](https://en.wikipedia.org/wiki/C%2B%2B17)

# LibCRCx

A simple, straightforward [CRC](https://en.wikipedia.org/wiki/Cyclic_redundancy_check) library that uses [lookup-tables](https://en.wikipedia.org/wiki/Lookup_table).

# Build Status

| (OS, Compiler) | Arch1            | Arch2              | Arch3            | Arch4            |
|----------------|------------------|--------------------|------------------|------------------|
| (Linux, gcc)   | [![amd64][2]][1] | [![ppc64le][3]][1] | [![s390x][4]][1] | [![arm64][5]][1] |
| (macOS, clang) | [![amd64][6]][1] |                    |                  |                  |
| (Windows, gcc) | [![amd64][7]][1] |                    |                  |                  |

[1]: https://travis-ci.com/cfriedt/crcx
[2]: https://travis-matrix-badges.herokuapp.com/repos/cfriedt/crcx/branches/master/1
[3]: https://travis-matrix-badges.herokuapp.com/repos/cfriedt/crcx/branches/master/2
[4]: https://travis-matrix-badges.herokuapp.com/repos/cfriedt/crcx/branches/master/3
[5]: https://travis-matrix-badges.herokuapp.com/repos/cfriedt/crcx/branches/master/4
[6]: https://travis-matrix-badges.herokuapp.com/repos/cfriedt/crcx/branches/master/5
[7]: https://travis-matrix-badges.herokuapp.com/repos/cfriedt/crcx/branches/master/6

Other architectures coming soon.

# Documentation

The Doxygen API docs are availble [here](https://cfriedt.github.io/crcx/), or for individual language bindings

* [C](https://cfriedt.github.io/crcx/crcx_8h.html)
* [C++](https://cfriedt.github.io/crcx/crc3x_8h.html)

# License

See the file [LICENSE](https://github.com/cfriedt/crcx/blob/master/LICENSE) for licensing terms and copyright.
