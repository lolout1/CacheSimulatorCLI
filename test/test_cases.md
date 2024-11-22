# Test Cases

## 1. Direct Mapped Cache
```bash
# Test file: direct_mapped.txt
x0000
x1000
x0000
x1000
x2000
```
Expected output shows conflict misses between addresses.

## 2. Set Associative with LRU
```bash
# Test file: set_associative_lru.txt
x0000
x1000
x2000
x0000
x1000
```
Expected output shows LRU replacement behavior.

## 3. Set Associative with MRU
```bash
# Test file: set_associative_mru.txt
x0000
x1000
x2000
x0000
x1000
```
Expected output shows MRU replacement behavior.

## 4. Power of 2 Rounding
```bash
./cache_simulator -N 16 -B 1000 -I 3 -w 2 -f test.txt
```
Should round B to 1024 (2^10) and I to 4 (2^2).

## 5. Address Space Bounds
```bash
# Test file: bounds.txt
x0000
xFFFF
```
Tests full 16-bit address space range.
