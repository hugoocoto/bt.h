# Height-Balanced Binary Search Tree (insert balancing)

Insertion now performs simple functional balancing.

## Public API

Read `./bt.h`
 
## Usage

- Copy `./bt.h` to your project and 
- `#define BT_IMPLEMENTATION` before including it only once. 
- Then you can `#include "bt.h"` and use it where you want.

See `./test.c` for an example

## Contributing

Take a look to [./CONTRIBUTING.md](./CONTRIBUTING.md).

## Common issues
- implicit declaration of function `strdup`: include 'bt.h' before string.h or
  define the `_XOPEN_SOURCE >= 500` or `_POSIX_C_SOURCE >= 200809L` macros.
