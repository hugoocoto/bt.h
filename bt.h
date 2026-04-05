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

/* unBalanced Tree -- uncolored red-Black Tree */

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
 * #define BT_FPRINTF fprintf
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

#include <stdio.h>

typedef struct BT BT;
typedef void (*Value_Delete_Callback)(void *);

/* CRUnD Data structure (Create, Read, Update but not Delete) */
extern void bt_add(BT *, const char *key, void *value); /* Add or remplace a value with a given a key */
extern void *bt_get(BT *, const char *key);            /* Get a value with a given a key, or NULL */
extern void bt_destroy(BT *);                          /* Destroy the tree */
extern void bt_write(BT *, FILE *);                    /* Print the tree to FILE* f */
extern void bt_write_pretty(BT *, FILE *);             /* Print the tree but prettier */
extern char *bt_get_key_addr(BT *, const char *key);   /* Get the address of the key that matches key or NULL */

typedef enum BT_Dir {
        BT_LEFT,
        BT_RIGHT,
} BT_Dir;

typedef enum BT_Color {
        BT_C_NONE,
        BT_C_RED,
        BT_C_BLACK,
} BT_Color;

struct BT {
        char *key;
        void *value;

        struct BT *parent;
        union {
                struct {
                        BT *right;
                        BT *left;
                };
                struct BT *child[2];
        };

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

#ifndef BT_FPRINTF
#define BT_FPRINTF fprintf
#endif

#ifndef BT_MAX_PRETTY_DEPTH
#define BT_MAX_PRETTY_DEPTH 1024
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

// static BT_Dir
// direction(BT *node)
// {
//         assert(node && node->parent);
//         return node == node->parent->right ? BT_RIGHT : BT_LEFT;
// }
//
// static BT *
// rotate_subtree(BT **tree, BT *sub, BT_Dir dir)
// {
//         BT *new_root = sub->child[1 - dir];
//         BT *new_child = new_root->child[dir];
//
//         sub->child[1 - dir] = new_child;
//
//         if (new_child) new_child->parent = sub;
//         new_child->child[dir] = sub;
//
//         new_root->parent = sub->parent;
//         sub->parent = new_root;
//         if (sub->parent) {
//                 sub->parent->child[sub == sub->parent->right] = new_root;
//         } else {
//                 *tree = new_root;
//         }
//
//         return new_root;
// }

static void
node_set(BT *node, const char *key, void *value, BT *parent)
{
        node->parent = parent;
        node->key = BT_STRDUP(key);
        node->value = value;
        node->color = BT_C_BLACK;
        node->left = node->right = 0;
}

void
bt_add(BT *tree, const char *key, void *value)
{
        if (!tree->key) {
                node_set(tree, key, value, NULL);
                return;
        }

        BT *node = tree;
        for (;;) {
                if (node->key == 0) {
                        node_set(node, key, value, node->parent);
                        return;
                }

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
                        return;
                }
        }
}

static int
bt_node_in_path(BT *node, BT **path, size_t depth)
{
        for (size_t i = 0; i < depth; i++)
                if (path[i] == node) return 1;
        return 0;
}

static void
bt_write_pretty_r(BT *tree, FILE *f, int indent, char orientation, BT **path, size_t depth, size_t max_depth)
{
        const int indent_inc = 8;
        if (!tree || !tree->key) return;
        if (bt_node_in_path(tree, path, depth)) {
                BT_FPRINTF(f, "%*.*s[cycle]\n", indent + indent_inc, indent + indent_inc, "");
                return;
        }
        if (depth >= max_depth) {
                BT_FPRINTF(f, "%*.*s[depth-limit]\n", indent + indent_inc, indent + indent_inc, "");
                return;
        }
        path[depth] = tree;
        if (tree->left) {
                bt_write_pretty_r(tree->left, f, indent + indent_inc, '/', path, depth + 1, max_depth);
        }
        if (indent)
                BT_FPRINTF(f, "%*.*s%c%*.*s", indent - 1, indent - 1, "", orientation, 1, 1, "");
        BT_FPRINTF(f, "\033[%d;%dm%*s\033[0m\n",
                   (tree->color == BT_C_BLACK) ? 47 : 41,
                   (tree->color == BT_C_BLACK) ? 30 : 37,
                   indent_inc, tree->key);
        if (tree->right) {
                bt_write_pretty_r(tree->right, f, indent + indent_inc, '\\', path, depth + 1, max_depth);
        }
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

void
bt_write_pretty(BT *tree, FILE *f)
{
        BT *path[BT_MAX_PRETTY_DEPTH];
        bt_write_pretty_r(tree, f, 0, '|', path, 0, BT_MAX_PRETTY_DEPTH);
}

void
bt_write(BT *tree, FILE *f)
{
        if (!tree) return;
        if (tree->left) {
                bt_write(tree->left, f);
        }
        BT_FPRINTF(f, "%s\n", tree->key);
        if (tree->right) {
                bt_write(tree->right, f);
        }
}

#endif // !BT_IMPLEMENTATION

#ifdef __cplusplus
}
#endif
