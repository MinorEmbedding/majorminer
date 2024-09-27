# graph-embedder
[![Documentation Status](https://readthedocs.org/projects/majorminer/badge/?version=latest)](https://majorminer.readthedocs.io/en/latest/?badge=latest)


Testing a bunch of graph minor embedding heuristics and related techniques to improve the embedding of QUBOs.

Our final results for the project can be found [here](doc/Studienarbeit.pdf).



| :warning:    | This is a research repo, so APIs and algorithms might change frequently as we are trying out different ideas. Moreover, this means that the code is not production-ready. |
|---------------|:------------------------|

| :point_up:    | We currently do not implement the same approaches in C++ and Python, but try out different ideas in different programming languages. Make sure to check out both the C++ and Python implementation. |
|---------------|:------------------------|

# Python
All Python-related code and documentation can be found [in the Python folder](python/).

# C++
## Build C++ library
Note that in order to build, you have to clone the submodules as well. That is,
if you have already cloned this repository, you should run ```git submodule update --init --recursive``` and find the submodules in ```external/```.
In the case, you are about to clone the repository, just run ```git clone --recursive https://github.com/MinorEmbedding/majorminer.git```.

In order to build, you must then run the following commands
```
bash prepare.sh
mkdir build
cd build/
cmake .. -DMAJORMINER_BUILD_TESTS=[ON|OFF] # depending on whether you want to run tests
make
```

## use the C++ libary
In the [examples](examples/) folder, there is an example project that solves embedding problem and creates debug images.
There is a readme containing the output in the same folder as well.

## Libraries used in the C++-Project
#### [oneTBB](https://github.com/oneapi-src/oneTBB) (License: [Apache 2.0](https://choosealicense.com/licenses/apache-2.0/))
#### [GoogleTest](https://github.com/google/googletest) (License: [BSD 3-Clause "New" or "Revised"](https://choosealicense.com/licenses/bsd-3-clause/))
#### [LEMON](https://lemon.cs.elte.hu/trac/lemon) (License: [Boost Software License 1.0](https://choosealicense.com/licenses/bsl-1.0/))
#### [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page) (License: [MPL2](https://choosealicense.com/licenses/mpl-2.0/), disabled LGPL features.)
#### [thread-pool](https://github.com/bshoshany/thread-pool) (License: [MIT](https://choosealicense.com/licenses/mit/))

