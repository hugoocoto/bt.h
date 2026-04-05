#include "bt.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

extern void bt_add(BT *, const char *key, void *value);  /* Add or replace a value with a given a key */
extern void *bt_get(BT *, const char *key);              /* Get a value with a given a key, or NULL */
extern void bt_destroy(BT *);                            /* Destroy the tree */
extern char *bt_get_key_addr(BT *tree, const char *key); /* Get the address of the key that matches key or NULL */

int
main(int argc, char *argv[])
{
        (void) argc, (void) argv;

        BT tree = { 0 };

        /* Add + Get test */
        bt_add(&tree, "", (void *) 1L);
        bt_add(&tree, "a", (void *) 2L);
        bt_add(&tree, "bb", (void *) 3L);
        bt_add(&tree, "ccc", (void *) 4L);
        bt_add(&tree, "dddd", (void *) 5L);
        assert(bt_get(&tree, "none") == NULL);
        assert(bt_get(&tree, "") == (void *) 1L);
        assert(bt_get(&tree, "a") == (void *) 2L);
        assert(bt_get(&tree, "bb") == (void *) 3L);
        assert(bt_get(&tree, "ccc") == (void *) 4L);
        assert(bt_get(&tree, "dddd") == (void *) 5L);
        assert(bt_get(&tree, "invalid") == NULL);
        assert(bt_get(&tree, "none") == NULL);
        assert(bt_get_key_addr(&tree, "") != NULL);
        assert(strcmp(bt_get_key_addr(&tree, "a"), "a") == 0);
        assert(strcmp(bt_get_key_addr(&tree, "bb"), "bb") == 0);
        assert(strcmp(bt_get_key_addr(&tree, "ccc"), "ccc") == 0);
        assert(strcmp(bt_get_key_addr(&tree, "dddd"), "dddd") == 0);
        assert(bt_get_key_addr(&tree, "invalid") == NULL);

        /* Update + Get test */
        bt_add(&tree, "", (void *) 11L);
        bt_add(&tree, "a", (void *) 12L);
        bt_add(&tree, "bb", (void *) 13L);
        bt_add(&tree, "ccc", (void *) 14L);
        bt_add(&tree, "dddd", (void *) 15L);
        assert(bt_get(&tree, "none") == NULL);
        assert(bt_get(&tree, "") == (void *) 11L);
        assert(bt_get(&tree, "a") == (void *) 12L);
        assert(bt_get(&tree, "bb") == (void *) 13L);
        assert(bt_get(&tree, "ccc") == (void *) 14L);
        assert(bt_get(&tree, "dddd") == (void *) 15L);
        assert(bt_get(&tree, "invalid") == NULL);
        assert(bt_get(&tree, "none") == NULL);
        bt_destroy(&tree);

        /* balance test; tester have to eval the result based on the
         * representation */
        bt_add(&tree, "a", (void *) 1L);
        bt_add(&tree, "b", (void *) 1L);
        bt_add(&tree, "c", (void *) 1L);
        bt_add(&tree, "d", (void *) 1L);
        bt_add(&tree, "e", (void *) 1L);
        bt_add(&tree, "f", (void *) 1L);
        bt_add(&tree, "g", (void *) 1L);
        bt_add(&tree, "h", (void *) 1L);
        bt_add(&tree, "i", (void *) 1L);
        bt_add(&tree, "j", (void *) 1L);
        bt_add(&tree, "k", (void *) 1L);
        bt_add(&tree, "l", (void *) 1L);
        bt_add(&tree, "m", (void *) 1L);
        bt_add(&tree, "n", (void *) 1L);
        bt_add(&tree, "o", (void *) 1L);
        assert(bt_get(&tree, "a") == (void *) 1L);
        assert(bt_get(&tree, "b") == (void *) 1L);
        assert(bt_get(&tree, "c") == (void *) 1L);
        assert(bt_get(&tree, "d") == (void *) 1L);
        assert(bt_get(&tree, "e") == (void *) 1L);
        assert(bt_get(&tree, "f") == (void *) 1L);
        assert(bt_get(&tree, "g") == (void *) 1L);
        assert(bt_get(&tree, "h") == (void *) 1L);
        assert(bt_get(&tree, "i") == (void *) 1L);
        assert(bt_get(&tree, "j") == (void *) 1L);
        assert(bt_get(&tree, "k") == (void *) 1L);
        assert(bt_get(&tree, "l") == (void *) 1L);
        assert(bt_get(&tree, "m") == (void *) 1L);
        assert(bt_get(&tree, "n") == (void *) 1L);
        assert(bt_get(&tree, "o") == (void *) 1L);
        assert(bt_get(&tree, "p") == NULL);
        bt_destroy(&tree);

        /* Reuse after destroy */
        bt_add(&tree, "z", (void *) 42L);
        assert(bt_get(&tree, "z") == (void *) 42L);
        assert(strcmp(bt_get_key_addr(&tree, "z"), "z") == 0);
        bt_destroy(&tree);

        return 0;
}


#define BT_IMPLEMENTATION
#include "bt.h"
