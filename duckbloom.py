#!/usr/bin/env python3

import ctypes
import os
from math import log, log2, exp, ceil
from datetime import datetime


class Bloom():
    try:
        bloom = ctypes.cdll.LoadLibrary('libduckbloom.so')
    except OSError:
        bloom = ctypes.cdll.LoadLibrary('./libduckbloom.so')

    def __init__(self, fname, size=None, hashcount=None, n=None, p=None):
        """Bloom Filter wrapper

        Checks to see if an object has been added to the filter.
        Definitively says if object doesn't exist
        Probably says if object does exist
        Works with huge files

        Args:
            fname: name of the file (or None for memory only)
            size: size of the file in Bytes (power of 2)
            hashcount: number of hashes to run for each element
            n: maximum theoretical number of added elements
            p: probability of a false positive

        Not all of size, hashcount, n, and p must be specified.
        Will try to calculate size and hashcount from n and p.

        Strings get converted to utf-8, so "spam" == b"spam"

        From https://hur.st/bloomfilter/
        n = ceil(m / (-k / log(1 - exp(log(p) / k))))
        p = pow(1 - exp(-k / (m / n)), k)
        m = ceil((n * log(p)) / log(1 / pow(2, log(2))));
        k = round((m / n) * log(2));     
        """
        try:
            fsize = os.path.getsize(fname)
            if size and fsize and fsize != size:
                raise ValueError("Size of bloom file doesn't match supplied size argument")
        except (OSError, TypeError):
            pass

        if size is None:
            try:
                size = os.path.getsize(fname)
            except (OSError, TypeError):
                if n is None or p is None:
                    raise TypeError(
                        "Must specify n and p if not specifying size")
                m = -n * log(p) / log(2)**2
                size = 2 ** int(ceil(log2(m/8.0)))

        if size & (size - 1):
            raise ValueError("Size must be power of 2")
        self.size = size
        self.m = size*8
        if hashcount is None:
            if n is None:
                k = -log2(p)
            else:
                k = self.m/n * log(2)
            hashcount = round(k)
        self.hashcount = hashcount

        self.p_target = p
        self.p_estimated = None
        self.n = n
        if n:
            self.p_estimated = (
                1 - exp(-self.hashcount * self.n / self.m)) ** self.hashcount

        if isinstance(fname, str):
            fname = fname.encode("utf-8")

        self.fname = fname
        self.ctx = self.bloom.bloom_malloc()
        self.bloom.bloom_open(self.ctx, self.fname, self.size, self.hashcount)

    def __repr__(self):
        return "Bloom(fname=%s, size=%d-Bytes, hashcount=%d, p_estimated=%e)" % (self.fname or "None", self.size, self.hashcount, self.p_estimated or 0)
        # print(self.fname or "None", self.size, self.hashcount, self.p_estimated or 0)
        # return "blah"

    # Context manager for `with` statement
    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.sync()

    def sync(self):
        self.bloom.bloom_sync(self.ctx)

    def close(self):
        self.bloom.bloom_close(self.ctx)
        self.bloom.bloom_free(self.ctx)
        self.ctx = None

    @staticmethod
    def _to_arb(thing):
        if isinstance(thing, bytes):
            b = thing
        elif isinstance(thing, bytearray):
            b = bytes(thing)
        # Hash is weird about string, bytes, and datetime
        # https://docs.python.org/3/reference/datamodel.html#object.__hash__
        elif isinstance(thing, str):
            b = thing.encode("utf-8")
        elif isinstance(thing, datetime):
            # Todo: turn datetimes into unique bytes
            raise NotImplementedError
        else:
            h = hash(thing)
            b = h.to_bytes((h.bit_length() + 7) // 8,
                           byteorder="big", signed=True)
        return (b, len(b))

    def check(self, thing):
        """Is thing in the bloom filter -> boolean"""
        return self.bloom.bloom_check(self.ctx, *self._to_arb(thing))

    def check160(self, thing):
        """Is 160-bit (20-Byte) bytes object in bloom filter -> boolean

        In theory, this is faster than `check`
        """
        if len(thing) != 20:
            raise ValueError("Check160 requires 20-Byte bytes/bytearray")
        return self.bloom.bloom_check160(self.ctx, thing)

    def add(self, thing):
        """Add object to bloom filter"""
        self.bloom.bloom_add(self.ctx, *self._to_arb(thing))

    def add160(self, thing):
        """Add 160bit (20-Byte) bytes object to bloom filter"""
        if len(thing) != 20:
            raise ValueError("Add160 requires 20-Byte bytes/bytearray")
        self.bloom.bloom_add160(self.ctx, thing)


if __name__ == '__main__':
    import os
    try:
        os.remove("test.blm")
    except FileNotFoundError:
        pass

    bloom = Bloom("test.blm", n=1000, p=1E-6)
    bloom.add("hello, world")
    bloom.add(b"works with bytes too")
    assert bloom.check("hello, world")
    assert bloom.check(b"hello, world")
    assert not bloom.check("please fail")

    h1 = bytes.fromhex("cf7c332804ab8ae1df7d7cbe7517b82edb83c680")
    h2 = bytes.fromhex("827b4aca20caf248d50233b90d711b12165ff2a6")
    assert not bloom.check160(h1)
    assert not bloom.check160(h2)
    bloom.add(h1)
    assert bloom.check160(h1)
    assert bloom.check(h1)
    assert not bloom.check160(h2)

    bloom2 = Bloom(None, size=1024, hashcount=10)
    assert not bloom2.check(h1)
    bloom2.add(h1)
    assert bloom2.check(h1)



