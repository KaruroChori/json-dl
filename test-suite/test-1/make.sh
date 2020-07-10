#!/bin/bash
rm ./host
rm ./libmodule.so
rm ./host.cpp
rm ./module.cpp

cat ../../../test-suite/test-1/model.json | ../../apps/gen-host/gen-host | clang-format > host.cpp
cat ../../../test-suite/test-1/impl.json  | ../../apps/gen-dl/gen-dl     | clang-format > module.cpp

clang++-10 -c host.cpp -std=c++20
clang++-10  -o host host.o -ldl

#clang++-10  -c -Wall -Werror -Wno-return-type-c-linkage -Wno-unused-variable -fpic module.cpp -std=c++20
clang++-10  -c -fpic module.cpp -std=c++20
clang++-10  -shared -o libmodule.so module.o

./host ./libmodule.so
