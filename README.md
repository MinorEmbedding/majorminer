# majorminer
Testing a bunch of graph minor embedding heuristics and related techniques to improve the embedding of QUBOs.


# Build C++ library
Note that in order to build, you have to clone the submodules as well. That is,
if you have already cloned this repository, you should run ```git submodule update --init --recursive``` and find the submodules in ```external/```.
In the case, you are about to clone the repository, just run ```git clone --recursive https://github.com/MinorEmbedding/majorminer.git```.

In order to build, you must then run the following commands
```
bash prepare.sh
mkdir build
cd build/
cmake ..
make
```




# Libraries used in the C++-Project
### [oneTBB](https://github.com/oneapi-src/oneTBB) (License: [Apache 2.0](https://choosealicense.com/licenses/apache-2.0/))
### [GoogleTest](https://github.com/google/googletest) (License: [BSD 3-Clause "New" or "Revised"](https://choosealicense.com/licenses/bsd-3-clause/))
### [LEMON](https://lemon.cs.elte.hu/trac/lemon) (License: [Boost Software License 1.0](https://choosealicense.com/licenses/bsl-1.0/))
### [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page) (License: [MPL2](https://choosealicense.com/licenses/mpl-2.0/), disabled LGPL features.)