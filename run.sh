#!bin/zsh

# mkdir build > ./message.txt
cd build
cmake ..
echo 'make'
make -j 4