#include "bt.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void
bt_pretty_print_test(BT *tree)
{
        if (!tree || !tree->key) return;
        if (tree->left) {
                bt_pretty_print_test(tree->left);
        }

        printf("%p -> %d\n", (void *) tree, (int) (intptr_t) tree->value);

        if (tree->right) {
                bt_pretty_print_test(tree->right);
        }
}

static int
del_if_never(const char *key, void *value, void *ctx)
{
        (void) key;
        (void) value;
        (void) ctx;
        return 0;
}

static int
del_if_always(const char *key, void *value, void *ctx)
{
        (void) key;
        (void) value;
        (void) ctx;
        return 1;
}

static int
del_if_even_value(const char *key, void *value, void *ctx)
{
        (void) key;
        (void) ctx;
        return (((intptr_t) value) % 2) == 0;
}

static int
del_if_key_match_ctx(const char *key, void *value, void *ctx)
{
        (void) value;
        return strcmp(key, (const char *) ctx) == 0;
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

        /* Delete test */
        bt_del(&tree, "none");
        bt_del(&tree, "bb");
        assert(bt_get(&tree, "bb") == NULL);
        assert(bt_get(&tree, "") == (void *) 11L);
        assert(bt_get(&tree, "a") == (void *) 12L);
        assert(bt_get(&tree, "ccc") == (void *) 14L);
        assert(bt_get(&tree, "dddd") == (void *) 15L);
        bt_del(&tree, "");
        bt_del(&tree, "a");
        bt_del(&tree, "ccc");
        bt_del(&tree, "dddd");
        assert(bt_get(&tree, "") == NULL);
        assert(bt_get(&tree, "a") == NULL);
        assert(bt_get(&tree, "ccc") == NULL);
        assert(bt_get(&tree, "dddd") == NULL);
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

        {
                const char *expected[] = { "a", "b", "c", "d", "e", "f", "g" };
                int expected_index = 0;
                for_bt_each(iter, &tree) {
                        assert(expected_index < 7);
                        assert(strcmp(iter->key, expected[expected_index]) == 0);
                        expected_index++;
                }
                assert(expected_index == 7);
        }

        bt_destroy(&tree);
        assert(bt_iter(NULL) == NULL);

        /* bt_del_if tests */
        assert(bt_del_if(&tree, del_if_always, NULL) == 0);
        assert(bt_get(&tree, "none") == NULL);

        bt_add(&tree, "a", (void *) 1L);
        bt_add(&tree, "b", (void *) 2L);
        bt_add(&tree, "c", (void *) 3L);
        assert(bt_del_if(&tree, del_if_never, NULL) == 0);
        assert(bt_get(&tree, "a") == (void *) 1L);
        assert(bt_get(&tree, "b") == (void *) 2L);
        assert(bt_get(&tree, "c") == (void *) 3L);
        bt_destroy(&tree);

        bt_add(&tree, "a", (void *) 1L);
        bt_add(&tree, "b", (void *) 2L);
        bt_add(&tree, "c", (void *) 3L);
        assert(bt_del_if(&tree, del_if_always, NULL) == 3);
        assert(bt_get(&tree, "a") == NULL);
        assert(bt_get(&tree, "b") == NULL);
        assert(bt_get(&tree, "c") == NULL);
        bt_destroy(&tree);

        bt_add(&tree, "d", (void *) 1L);
        bt_add(&tree, "b", (void *) 2L);
        bt_add(&tree, "f", (void *) 3L);
        bt_add(&tree, "a", (void *) 4L);
        bt_add(&tree, "c", (void *) 5L);
        bt_add(&tree, "e", (void *) 6L);
        bt_add(&tree, "g", (void *) 7L);
        assert(bt_del_if(&tree, del_if_even_value, NULL) == 3);
        assert(bt_get(&tree, "a") == NULL);
        assert(bt_get(&tree, "d") == (void *) 1L);
        assert(bt_get(&tree, "b") == NULL);
        assert(bt_get(&tree, "f") == (void *) 3L);
        assert(bt_get(&tree, "c") == (void *) 5L);
        assert(bt_get(&tree, "e") == NULL);
        assert(bt_get(&tree, "g") == (void *) 7L);
        bt_destroy(&tree);

        bt_add(&tree, "d", (void *) 1L);
        bt_add(&tree, "b", (void *) 2L);
        bt_add(&tree, "f", (void *) 3L);
        assert(bt_del_if(&tree, del_if_key_match_ctx, (void *) "d") == 1);
        assert(bt_get(&tree, "d") == NULL);
        assert(bt_get(&tree, "b") == (void *) 2L);
        assert(bt_get(&tree, "f") == (void *) 3L);
        bt_destroy(&tree);

        bt_add(&tree, "d", (void *) 1L);
        bt_add(&tree, "b", (void *) 2L);
        bt_add(&tree, "f", (void *) 3L);
        assert(bt_del_if(&tree, del_if_key_match_ctx, (void *) "b") == 1);
        assert(bt_get(&tree, "b") == NULL);
        assert(bt_get(&tree, "d") == (void *) 1L);
        assert(bt_get(&tree, "f") == (void *) 3L);
        bt_destroy(&tree);

        return 0;
}


#define BT_IMPLEMENTATION
#include "bt.h"
