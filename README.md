# Red-Black Tree

A red-black tree with string keys: adding, finding and deleting are
O(log n), and deleting keeps it balanced too.

Walks in key order (`bt_first`/`bt_next`, or `for_bt_each`) keep no state of
their own, so they can nest. The older `bt_iter` still works, but has one state
for the whole program.

## Public API

Read `./bt.h`
 
## Usage

- Copy `./bt.h` to your project and 
- `#define BT_IMPLEMENTATION` before including it only once. 
- Then you can `#include "bt.h"` and use it where you want.

See `./test.c` for an example

## Common issues
- implicit declaration of function `strdup`: include 'bt.h' before string.h or
  define the `_XOPEN_SOURCE >= 500` or `_POSIX_C_SOURCE >= 200809L` macros.
