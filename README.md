# AnyOpt

AnyOpt (Any Optional) is a C++11 compatible library

this header-only library was primarily developed due to the requirements of my resource manager:
    being able to store any type
    being able to automatically free the resource stored upon object death
    being able to determine if anything is actually stored or not, since nullptr is a valid storage

# building and testing

```shell script
make test_debug
```