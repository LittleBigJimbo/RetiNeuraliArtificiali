g++ -c clWrapper.cpp -o libclw.o -std=c++11
rm libclw.a
ar -cvq libclw.a libclw.o
