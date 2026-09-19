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

/* Check that NODE's subtree is a red-black tree with keys between LO and HI
 * (NULL: no bound) and parent links to PARENT. Counts its entries in COUNT
 * and returns its black height. */
static int
check_node(BT *node, BT *parent, const char *lo, const char *hi, int *count)
{
        if (!node) return 1;
        assert(node->key);
        assert(node->parent == parent);
        assert(!lo || strcmp(lo, node->key) < 0);
        assert(!hi || strcmp(node->key, hi) < 0);
        assert(node->color == BT_C_RED || node->color == BT_C_BLACK);
        if (node->color == BT_C_RED) {
                assert(!node->left || node->left->color == BT_C_BLACK);
                assert(!node->right || node->right->color == BT_C_BLACK);
        }
        int left = check_node(node->left, node, lo, node->key, count);
        int right = check_node(node->right, node, node->key, hi, count);
        assert(left == right);
        (*count)++;
        return left + (node->color == BT_C_BLACK);
}

static int
height(BT *node)
{
        if (!node) return 0;
        int l = height(node->left), r = height(node->right);
        return 1 + (l > r ? l : r);
}

/* Check the whole tree, return how many entries it has */
static int
check_tree(BT *tree)
{
        int count = 0;
        if (!tree->key) {
                assert(!tree->left && !tree->right);
                return 0;
        }
        assert(!tree->parent);
        assert(tree->color == BT_C_BLACK);
        check_node(tree, NULL, NULL, NULL, &count);
        /* A red-black tree of n entries is at most 2 log2(n + 1) high */
        int limit = 0;
        for (int n = count + 1; n > 1; n /= 2) limit++;
        assert(height(tree) <= 2 * (limit + 1));
        return count;
}

static unsigned
rnd(void)
{
        static unsigned x = 2463534242u; /* xorshift: the same run every time */
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        return x;
}

static int
del_if_multiple_of_3(const char *key, void *value, void *ctx)
{
        (void) key;
        (void) ctx;
        return ((intptr_t) value) % 3 == 0;
}

/* Random adds and deletes, checked against a plain array after each one */
static void
test_random(void)
{
        enum { N = 2000, OPS = 40000 };
        static char present[N];
        BT tree = { 0 };
        int count = 0;
        char key[16];

        for (int op = 0; op < OPS; op++) {
                int k = (int) (rnd() % N);
                sprintf(key, "k%05d", k);
                /* Grow first, then shrink, then mixed */
                int add = op < OPS / 3 ? rnd() % 4 != 0 : op < 2 * OPS / 3 ? rnd() % 4 == 0 : rnd() % 2;
                if (add) {
                        bt_add(&tree, key, (void *) (intptr_t) (k + 1));
                        if (!present[k]) count++;
                        present[k] = 1;
                } else {
                        bt_del(&tree, key);
                        if (present[k]) count--;
                        present[k] = 0;
                }
                assert(check_tree(&tree) == count);
                assert((bt_get(&tree, key) != NULL) == present[k]);
        }
        for (int k = 0; k < N; k++) {
                sprintf(key, "k%05d", k);
                assert(bt_get(&tree, key) == (present[k] ? (void *) (intptr_t) (k + 1) : NULL));
        }

        /* The walk sees every entry, in order */
        int seen = 0;
        const char *last = NULL;
        BT *it;
        for_bt_each(it, &tree) {
                assert(!last || strcmp(last, it->key) < 0);
                last = it->key;
                seen++;
        }
        assert(seen == count);

        /* Deleting by predicate keeps it balanced */
        int threes = 0;
        for (int k = 0; k < N; k++) threes += present[k] && (k + 1) % 3 == 0;
        assert(bt_del_if(&tree, del_if_multiple_of_3, NULL) == (size_t) threes);
        assert(check_tree(&tree) == count - threes);

        /* Delete everything, in order */
        for (int k = 0; k < N; k++) {
                sprintf(key, "k%05d", k);
                bt_del(&tree, key);
                check_tree(&tree);
        }
        assert(check_tree(&tree) == 0 && bt_first(&tree) == NULL);
        bt_destroy(&tree);
}

/* Walks don't share state: they nest, on the same tree or on others */
static void
test_nested_walks(void)
{
        BT a = { 0 }, b = { 0 };
        const char *keys[] = { "m", "c", "x", "a", "e", "q", "z" };
        for (int i = 0; i < 7; i++) {
                bt_add(&a, keys[i], (void *) 1L);
                bt_add(&b, keys[i], (void *) 1L);
        }
        BT *i, *j;
        int pairs = 0, ordered = 0;
        for_bt_each(i, &a) {
                for_bt_each(j, &a) {
                        pairs++;
                        ordered += strcmp(i->key, j->key) < 0;
                }
                for_bt_each(j, &b) pairs++;
        }
        assert(pairs == 2 * 7 * 7);
        assert(ordered == 7 * 6 / 2);
        assert(bt_first(NULL) == NULL);
        bt_destroy(&a);
        assert(bt_first(&a) == NULL);
        bt_destroy(&b);
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
                const int expected_count = (int) (sizeof(expected) / sizeof(expected[0]));
                int expected_index = 0;
                for_bt_each(iter, &tree) {
                        assert(expected_index < expected_count);
                        assert(strcmp(iter->key, expected[expected_index]) == 0);
                        expected_index++;
                }
                assert(expected_index == expected_count);
        }

        bt_destroy(&tree);
        assert(bt_iter(NULL) == NULL);

        /* bt_del_if tests */
        assert(bt_del_if(&tree, del_if_always, NULL) == 0);
        assert(bt_get(&tree, "none") == NULL);
        assert(bt_get(&tree, "any_key") == NULL);

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
        {
                const char *expected[] = { "c", "d", "f", "g" };
                const int expected_count = (int) (sizeof(expected) / sizeof(expected[0]));
                int expected_index = 0;
                for_bt_each(iter, &tree) {
                        assert(expected_index < expected_count);
                        assert(strcmp(iter->key, expected[expected_index]) == 0);
                        expected_index++;
                }
                assert(expected_index == expected_count);
        }
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

        test_random();
        test_nested_walks();
        return 0;
}


#define BT_IMPLEMENTATION
#include "bt.h"
