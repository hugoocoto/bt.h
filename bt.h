/* * Copyright (c) 2026 Hugo Coto Florez
 *
 * This work is licensed under the Creative Commons Attribution 4.0
 * International License. To view a copy of this license, visit
 * http://creativecommons.org/licenses/by/4.0/
 *
 * SPDX-License-Identifier: CC-BY-4.0
 */

/* +---------------------------------------------------+
 * | Github: https://github.com/hugoocoto/bt.h         |
 * +---------------------------------------------------+
 * | Contributors:                                     |
 * | Hugo Coto Florez                                  |
 * +---------------------------------------------------+
 * */

/* red-Black Tree */

/* To initialize a BTree, you have to zero initialize a BT
 *
 * BT tree = {0};
 * (or BT* tree = calloc(1, sizeof(BT));  )
 *
 * Then you can do whatever you want
 *
 * bt_add(&tree, "key1", (void*) 1);
 * bt_add(&tree, "key2", (void*) 2);
 * bt_get(&tree, "key1") == 1;
 * bt_destroy(&tree);
 *
 * It's needed to include the implementation in one and only one source file, as
 * follows:
 *
 * #define BT_IMPLEMENTATION
 * #include "bt.h"
 *
 * You can define custom behaviour by defining one or more of this macros before
 * including the implementation:
 *
 * #define BT_COMPARE strcmp
 * #define BT_STRDUP strdup
 * #define BT_CALLOC calloc
 *
 * The defines above have the default values, the new functions have to had the
 * same signature.

 * To set a value delete callback, you have to define the macro BT_VALUE_DELETE
 * before including this file and set it to a function with the
 * Value_Delete_Callback signature
 */

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_XOPEN_SOURCE) || _XOPEN_SOURCE < 500
#undef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif

#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 200809L
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#ifndef BT_H_
#define BT_H_

typedef struct BT BT;
typedef void (*Value_Delete_Callback)(void *);

/* CRUnD Data structure (Create, Read, Update but not Delete) */
extern void bt_add(BT *, const char *key, void *value); /* Add or remplace a value with a given a key */
extern void *bt_get(BT *, const char *key);             /* Get a value with a given a key, or NULL */
extern void bt_destroy(BT *);                           /* Destroy the tree */
extern char *bt_get_key_addr(BT *, const char *key);    /* Get the address of the key that matches key or NULL */
extern BT *bt_iter(BT *);                               /* In-order iterator using local static state; tree starts/restarts iteration, NULL advances */

typedef enum BT_Color {
        BT_C_NONE,
        BT_C_RED,
        BT_C_BLACK,
} BT_Color;

struct BT {
        char *key;
        void *value;
        BT *right;
        BT *left;
        BT *parent;
        BT_Color color;
};

#endif // !BT_H_

#ifdef BT_IMPLEMENTATION

#ifndef BT_COMPARE
#define BT_COMPARE strcmp
#endif

#ifndef BT_STRDUP
#define BT_STRDUP strdup
#endif

#ifndef BT_CALLOC
#define BT_CALLOC calloc
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void
node_set(BT *node, const char *key, void *value, BT *parent)
{
        node->parent = parent;
        node->key = BT_STRDUP(key);
        node->value = value;
        node->color = BT_C_RED;
        node->left = node->right = 0;
}

static BT *
bt_rotate_left(BT *node)
{
        BT *new_root = node->right;
        assert(new_root);

        if (node->parent) {
                BT *parent = node->parent;
                BT *moved = new_root->left;

                new_root->parent = parent;
                if (parent->left == node)
                        parent->left = new_root;
                else
                        parent->right = new_root;

                new_root->left = node;
                node->parent = new_root;
                node->right = moved;
                if (moved) moved->parent = node;
                return new_root;
        }

        /* In-place rotation when node is the root object passed by the user. */
        BT *a = node->left;
        BT *b = new_root->left;
        BT *c = new_root->right;

        char *node_key = node->key;
        void *node_value = node->value;
        BT_Color node_color = node->color;

        node->key = new_root->key;
        node->value = new_root->value;
        node->color = new_root->color;

        new_root->key = node_key;
        new_root->value = node_value;
        new_root->color = node_color;

        new_root->left = a;
        if (a) a->parent = new_root;
        new_root->right = b;
        if (b) b->parent = new_root;
        new_root->parent = node;

        node->left = new_root;
        node->right = c;
        if (c) c->parent = node;
        return node;
}

static BT *
bt_rotate_right(BT *node)
{
        BT *new_root = node->left;
        assert(new_root);

        if (node->parent) {
                BT *parent = node->parent;
                BT *moved = new_root->right;

                new_root->parent = parent;
                if (parent->left == node)
                        parent->left = new_root;
                else
                        parent->right = new_root;

                new_root->right = node;
                node->parent = new_root;
                node->left = moved;
                if (moved) moved->parent = node;
                return new_root;
        }

        /* In-place rotation when node is the root object passed by the user. */
        BT *a = new_root->left;
        BT *b = new_root->right;
        BT *c = node->right;

        char *node_key = node->key;
        void *node_value = node->value;
        BT_Color node_color = node->color;

        node->key = new_root->key;
        node->value = new_root->value;
        node->color = new_root->color;

        new_root->key = node_key;
        new_root->value = node_value;
        new_root->color = node_color;

        new_root->left = b;
        if (b) b->parent = new_root;
        new_root->right = c;
        if (c) c->parent = new_root;
        new_root->parent = node;

        node->left = a;
        if (a) a->parent = node;
        node->right = new_root;
        return node;
}

static int
bt_is_red(BT *node)
{
        return node && node->key && node->color == BT_C_RED;
}

static BT *
bt_grandparent(BT *node)
{
        return (node && node->parent) ? node->parent->parent : NULL;
}

static void
bt_insert_fixup(BT *tree, BT *node)
{
        while (node && node->parent && node->parent->color == BT_C_RED) {
                BT *parent = node->parent;
                BT *grandparent = bt_grandparent(node);
                if (!grandparent) break;

                if (parent == grandparent->left) {
                        BT *uncle = grandparent->right;
                        if (bt_is_red(uncle)) {
                                parent->color = BT_C_BLACK;
                                uncle->color = BT_C_BLACK;
                                grandparent->color = BT_C_RED;
                                node = grandparent;
                                continue;
                        }
                        if (node == parent->right) {
                                node = parent;
                                bt_rotate_left(node);
                                parent = node->parent;
                                grandparent = bt_grandparent(node);
                                if (!parent || !grandparent) break;
                        }
                        parent->color = BT_C_BLACK;
                        grandparent->color = BT_C_RED;
                        bt_rotate_right(grandparent);
                        continue;
                } else {
                        BT *uncle = grandparent->left;
                        if (bt_is_red(uncle)) {
                                parent->color = BT_C_BLACK;
                                uncle->color = BT_C_BLACK;
                                grandparent->color = BT_C_RED;
                                node = grandparent;
                                continue;
                        }
                        if (node == parent->left) {
                                node = parent;
                                bt_rotate_right(node);
                                parent = node->parent;
                                grandparent = bt_grandparent(node);
                                if (!parent || !grandparent) break;
                        }
                        parent->color = BT_C_BLACK;
                        grandparent->color = BT_C_RED;
                        bt_rotate_left(grandparent);
                        continue;
                }
        }

        tree->color = BT_C_BLACK;
}

static BT *
bt_add_new_node(BT *tree, const char *key, void *value)
{
        BT *node = tree;
        for (;;) {
                int cmp = BT_COMPARE(node->key, key);

                if (cmp > 0) {
                        if (!node->left) {
                                node->left = (BT *) BT_CALLOC(1, sizeof(BT));
                                node->left->parent = node;
                        }
                        node = node->left;
                }

                if (cmp < 0) {
                        if (!node->right) {
                                node->right = (BT *) BT_CALLOC(1, sizeof(BT));
                                node->right->parent = node;
                        }
                        node = node->right;
                }

                if (cmp == 0) {
/*                   */ #ifdef BT_VALUE_DELETE
                        BT_VALUE_DELETE(node->value);
/*                   */ #endif
                        node->value = value;
                        return NULL;
                }

                if (node->key == 0) {
                        node_set(node, key, value, node->parent);
                        return node;
                }
        }
}

void
bt_add(BT *tree, const char *key, void *value)
{
        if (!tree->key) {
                node_set(tree, key, value, NULL);
                tree->color = BT_C_BLACK;
                return;
        }

        BT *inserted = bt_add_new_node(tree, key, value);
        if (inserted) bt_insert_fixup(tree, inserted);
}

static BT *
bt_node_get(BT *tree, const char *key)
{
        BT *node = tree;
        int cmp;
        for (;;) {
                if (!node || !node->key) return NULL;
                cmp = BT_COMPARE(node->key, key);
                if (cmp > 0) node = node->left;
                if (cmp < 0) node = node->right;
                if (cmp == 0) return node;
        }
}

char *
bt_get_key_addr(BT *tree, const char *key)
{
        BT *node = bt_node_get(tree, key);
        return node ? node->key : NULL;
}


void *
bt_get(BT *tree, const char *key)
{
        BT *node = bt_node_get(tree, key);
        return node == NULL ? NULL : node->value;
}

static BT *
bt_iter_rec(BT *root, BT *current, int next_mode)
{
        if (!next_mode) {
                if (!root || !root->key) return NULL;
                if (!root->left) return root;
                return bt_iter_rec(root->left, NULL, 0);
        }

        if (!root || !root->key || !current || !current->key) return NULL;
        int cmp = BT_COMPARE(root->key, current->key);
        if (cmp > 0) {
                BT *left_candidate = bt_iter_rec(root->left, current, 1);
                return left_candidate ? left_candidate : root;
        }
        return bt_iter_rec(root->right, current, 1);
}

BT *
bt_iter(BT *tree)
{
        static BT *iter_root = NULL;
        static BT *iter_current = NULL;

        if (tree) {
                iter_root = tree;
                iter_current = bt_iter_rec(tree, NULL, 0);
                return iter_current;
        }
        if (!iter_root || !iter_current) return NULL;
        iter_current = bt_iter_rec(iter_root, iter_current, 1);
        return iter_current;
}

void
bt_destroy(BT *node)
{
        if (!node) return;
        if (node->left) {
                bt_destroy(node->left);
                free(node->left);
        }
        if (node->right) {
                bt_destroy(node->right);
                free(node->right);
        }
/*   */ #ifdef BT_VALUE_DELETE
        BT_VALUE_DELETE(node->value);
/*   */ #endif
        if (node->key) free(node->key);
        node->key = NULL;
        node->value = NULL;
        node->parent = NULL;
        node->left = NULL;
        node->right = NULL;
        node->color = BT_C_NONE;
}

#endif // !BT_IMPLEMENTATION

#ifdef __cplusplus
}
#endif
