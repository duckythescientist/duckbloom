

# Duckbloom
A fast Bloomfilter implementation in C with a Python wrapper.

> A Bloom filter is a space-efficient probabilistic data structure, conceived by Burton Howard Bloom in 1970, that is used to test whether an element is a member of a set. False positive matches are possible, but false negatives are not – in other words, a query returns either "possibly in set" or "definitely not in set". 
> 
> - https://en.wikipedia.org/wiki/Bloom_filter

Works with memory mapped files or pure memory filters thanks to a slightly modifieid `mmapf` from the great [Brainflayer](https://github.com/ryancdotorg/brainflayer) project.

Super duper fast hashing is provided by [xxHash](https://github.com/Cyan4973/xxHash)





## Build

This has been tested on Ubuntu 18.04, but it should work most places. Windows will need some work for the Python module.

```bash
git clone https://github.com/duckythescientist/duckbloom
cd duckbloom
git submodule init  &&  git submodule update
make
```

## Install
This is optional. Installs `libduckbloom.so` to `/usr/local/lib` and the header to `/usr/local/include`.

```bash
make install
```

## Basic Usage Example
```c
// C/C++
bloom_ctx * ctx = bloom_malloc();
bloom_open(ctx, "myfile", 4096, 10);
char * mystr = "hello";
bloom_add(ctx, mystr, strlen(mystr));
bool ret = bloom_check(ctx, mystr, strlen(mystr));
```

```python
# Python
bloom = Bloom("myfile.blm", n=1000, p=1E-6)
bloom.add("hello, world")
bloom.add(b"works with bytes too")
assert bloom.check("hello, world")
```

The header file is reasonably well documented.


## Bloom Size Equations
Calculator: https://hur.st/bloomfilter

```
n = ceil(m / (-k / log(1 - exp(log(p) / k))))
p = pow(1 - exp(-k / (m / n)), k)
m = ceil((n * log(p)) / log(1 / pow(2, log(2))));
k = round((m / n) * log(2));
```

