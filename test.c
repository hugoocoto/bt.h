#include "bt.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void bt_add(BT *, const char *key, void *value);  /* Add or replace a value with a given a key */
extern void *bt_get(BT *, const char *key);              /* Get a value with a given a key, or NULL */
extern void bt_destroy(BT *);                            /* Destroy the tree */
extern char *bt_get_key_addr(BT *tree, const char *key); /* Get the address of the key that matches key or NULL */
extern BT *bt_iter(BT *);                                /* Iterate in-order; pass tree to start, NULL for next */

enum { BT_TEST_MAX_PRETTY_DEPTH = 1024 };

static int
node_in_path(BT *node, BT **path, size_t depth)
{
        for (size_t i = 0; i < depth; i++)
                if (path[i] == node) return 1;
        return 0;
}

static void
bt_pretty_print_test_r(BT *tree, int indent, char orientation, BT **path, size_t depth, size_t max_depth)
{
        const int indent_inc = 8;

        if (!tree || !tree->key) return;
        if (node_in_path(tree, path, depth)) {
                printf("%*.*s[cycle]\n", indent + indent_inc, indent + indent_inc, "");
                return;
        }
        if (depth >= max_depth) {
                printf("%*.*s[depth-limit]\n", indent + indent_inc, indent + indent_inc, "");
                return;
        }

        path[depth] = tree;
        if (tree->left)
                bt_pretty_print_test_r(tree->left, indent + indent_inc, '/', path, depth + 1, max_depth);

        if (indent)
                printf("%*.*s%c%*.*s", indent - 1, indent - 1, "", orientation, 1, 1, "");
        /* In these tests, value is always an integer encoded as (void *). */
        printf("%*s%p -> %d\n", indent_inc, "", (void *) tree, (int) (intptr_t) tree->value);

        if (tree->right)
                bt_pretty_print_test_r(tree->right, indent + indent_inc, '\\', path, depth + 1, max_depth);
}

static void
bt_pretty_print_test(BT *tree)
{
        BT *path[BT_TEST_MAX_PRETTY_DEPTH] = { 0 };
        bt_pretty_print_test_r(tree, 0, ' ', path, 0, BT_TEST_MAX_PRETTY_DEPTH);
}

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
        bt_pretty_print_test(&tree);
        bt_destroy(&tree);

        /* Reuse after destroy */
        bt_add(&tree, "z", (void *) 42L);
        assert(bt_get(&tree, "z") == (void *) 42L);
        assert(strcmp(bt_get_key_addr(&tree, "z"), "z") == 0);
        bt_destroy(&tree);

        /* Iterator test (in-order: smallest to largest) */
        bt_add(&tree, "d", (void *) 1L);
        bt_add(&tree, "b", (void *) 1L);
        bt_add(&tree, "f", (void *) 1L);
        bt_add(&tree, "a", (void *) 1L);
        bt_add(&tree, "c", (void *) 1L);
        bt_add(&tree, "e", (void *) 1L);
        bt_add(&tree, "g", (void *) 1L);

        BT *iter = bt_iter(&tree);
        assert(iter != NULL);
        assert(strcmp(iter->key, "a") == 0);
        iter = bt_iter(NULL);
        assert(iter != NULL);
        assert(strcmp(iter->key, "b") == 0);
        iter = bt_iter(NULL);
        assert(iter != NULL);
        assert(strcmp(iter->key, "c") == 0);
        iter = bt_iter(&tree);
        assert(iter != NULL);
        assert(strcmp(iter->key, "a") == 0);
        iter = bt_iter(NULL);
        assert(iter != NULL);
        assert(strcmp(iter->key, "b") == 0);
        iter = bt_iter(NULL);
        assert(iter != NULL);
        assert(strcmp(iter->key, "c") == 0);
        iter = bt_iter(NULL);
        assert(iter != NULL);
        assert(strcmp(iter->key, "d") == 0);
        iter = bt_iter(NULL);
        assert(iter != NULL);
        assert(strcmp(iter->key, "e") == 0);
        iter = bt_iter(NULL);
        assert(iter != NULL);
        assert(strcmp(iter->key, "f") == 0);
        iter = bt_iter(NULL);
        assert(iter != NULL);
        assert(strcmp(iter->key, "g") == 0);
        assert(bt_iter(NULL) == NULL);
        assert(bt_iter(NULL) == NULL);
        bt_destroy(&tree);
        assert(bt_iter(NULL) == NULL);

        return 0;
}


#define BT_IMPLEMENTATION
#include "bt.h"
