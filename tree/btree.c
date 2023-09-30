#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <strings.h>
#include "btree.h"

#define IS_UNDER_FLOW(node) (((node)->isLeaf) ? node->numKeys == 1 : node->numKeys == UNDER_FLOW)

/* find a key on array, return its index */
static int searchKey(int *a, int len, int key);

static bTree btInsertInternal(bTree b, int key, int *median);
static void btDeleteInternal(btNode *cur, int key, btNode *parent, int ppos);
static void reBalance(btNode *cur, btNode *parent, int ppos);
static void rmLeaf(btNode *cur, int pos, btNode *parent, int ppos);
static void rmInner(btNode *cur, int pos, btNode *parent, int ppos);

int main()
{
    bTree b;
    int i;
    b = btCreate();
    for (i = 1; i < 100; i++)
    {
        btInsert(&b, i);
    }
    for (i = 1; i < 88; i++)
    {
        btDelete(&b, i);
    }
    btDestroy(b);
    return 0;
}

bTree btCreate()
{
    bTree b = NULL;
    b = (bTree)malloc(sizeof(btNode));
    assert(b);
    b->isLeaf = true;
    b->numKeys = 0;
    return b;
}

void btDestroy(bTree t)
{

    if (t == NULL)
    {
        return;
    }
    int i;
    if (t->isLeaf)
    {
        free(t);
    }
    else
    {
        for (i = 0; i < t->numKeys + 1; i++)
        {
            if (t->kids[i] != NULL)
            {
                btDestroy(t->kids[i]);
#ifdef DEBUG
                t->kids[i] = NULL;
#endif
            }
        }
        free(t);
    }
}

static int searchKey(int *a, int len, int key)
{
    int lo = -1;
    int hi = len;
    int mid;
    // invariant: a[lo] < key <=a[hi]
    while (lo + 1 < hi)
    {
        mid = (lo + hi) / 2;
        if (a[mid] > key)
        {
            hi = mid;
        }
        else if (a[mid] < key)
        {
            lo = mid;
        }
        else
        {
            return mid;
        }
    }
    return hi;
}

bool btSearch(bTree t, int key)
{
    if (t->keys == 0)
        return false;
    int pos;
    pos = searchKey(t->keys, t->numKeys, key);
    if (pos < t->numKeys && t->keys[pos] == key)
    {
        return true;
    }
    else
    {
        return (!(t->isLeaf) && btSearch(t->kids[pos], key));
    }
}

static bTree btInsertInternal(bTree b, int key, int *median)
{
    int pos;
    pos = searchKey(b->keys, b->numKeys, key);
    if (pos < b->numKeys && b->keys[pos] == key)
    {
        /* nothing to do */
        return NULL;
    }
    // if this node is a leaf node, we can insert directly
    if (b->isLeaf)
    {
        if (pos + 1 <= b->numKeys)
        {
            memmove(&(b->keys[pos + 1]), &(b->keys[pos]), sizeof(*(b->keys)) * (b->numKeys - pos));
        }
        b->keys[pos] = key;
        b->numKeys++;
    }
    else
    {
        // get kid node to insert
        btNode *kid;
        int midKey;
        kid = btInsertInternal(b->kids[pos], key, &midKey);
        // if kid != NULL, it means kid node split into two node, and return new node and middle key
        if (kid)
        {
            memmove(&(b->keys[pos + 1]), &(b->keys[pos]), sizeof(*b->keys) * (b->numKeys - pos));
            memmove(&(b->kids[pos + 2]), &(b->kids[pos + 1]), sizeof(void *) * (b->numKeys - pos));
            b->keys[pos] = midKey;
            b->numKeys++;
            b->kids[pos + 1] = kid;
        }
    }
    if (b->numKeys >= MAX_KEYS)
    {
        int mid = b->numKeys / 2;
        btNode *newNode = (btNode *)malloc(sizeof(*b));
        // 0... mid ... numKeys-1 numKeys
        newNode->numKeys = b->numKeys - mid - 1;
        newNode->isLeaf = b->isLeaf;
        memmove(newNode->keys, &b->keys[mid + 1], sizeof(*(newNode->keys)) * newNode->numKeys);
        *median = b->keys[mid];
#ifdef DEBUG
        bzero(&b->keys[mid], sizeof(*b->keys) * (b->numKeys - mid));
#endif
        if (!b->isLeaf)
        {
            memmove(newNode->kids, &b->kids[mid + 1], sizeof(*(b->kids)) * (newNode->numKeys + 1));
#ifdef DEBUG
            bzero(&b->kids[mid + 1], sizeof(void *) * (b->numKeys - mid));
#endif
        }
        b->numKeys = mid;
        return newNode;
    }
    return NULL;
}

void btInsert(bTree *bt, int key)
{
    btNode *newNode, *newRoot;
    bTree old = *bt;
    int mid;
    newNode = btInsertInternal(old, key, &mid);

    if (newNode)
    {
        newRoot = (btNode *)malloc(sizeof(*old));
        assert(newRoot);

        newRoot->numKeys = 1;
        newRoot->isLeaf = 0;
        newRoot->keys[0] = mid;
        newRoot->kids[0] = old;
        newRoot->kids[1] = newNode;
        *bt = newRoot;
    }
}

void btDelete(bTree *bt, int key)
{
    btNode *cur = *bt;
    btDeleteInternal(cur, key, NULL, 0);
    if (cur->numKeys == 0)
    {
        *bt = cur->kids[0];
        free(cur);
    }

    return;
}

static void btDeleteInternal(btNode *cur, int key, btNode *parent, int ppos)
{
    int pos;
    pos = searchKey(cur->keys, cur->numKeys, key);
    // key is not in the tree
    if (cur->isLeaf && pos < cur->numKeys && cur->keys[pos] != key)
    {
        return;
    }
    //key is found
    if (pos < cur->numKeys && cur->keys[pos] == key)
    {
        if (cur->isLeaf)
        {
            rmLeaf(cur, pos, parent, ppos);
        }
        else
        {
            rmInner(cur, pos, parent, ppos);
        }
        return;
    }
    else
    {
        // delete recursive
        btDeleteInternal(cur->kids[pos], key, cur, pos);
    }

    reBalance(cur, parent, ppos);
    return;
}

static void rmInner(btNode *cur, int pos, btNode *parent, int ppos)
{
    btNode *kid = cur->kids[pos];
    int nextPos;
    if (!IS_UNDER_FLOW(kid))
    {
        nextPos = kid->numKeys - 1;
        cur->keys[pos] = kid->keys[kid->numKeys - 1];
    }
    else
    {
        nextPos = 0;
        kid = cur->kids[pos + 1];
        cur->keys[pos] = kid->keys[0];
    }

    if (kid->isLeaf)
        rmLeaf(kid, nextPos, cur, pos);
    else
        rmInner(kid, nextPos, cur, pos);
    reBalance(cur, parent, ppos);
}

static void rmLeaf(btNode *cur, int pos, btNode *parent, int ppos)
{
    if (pos != cur->numKeys - 1)
    {
        memmove(&cur->keys[pos], &cur->keys[pos + 1], sizeof(*cur->keys) * (cur->numKeys - pos - 1));
    }
#ifdef DEBUG
    cur->keys[cur->numKeys - 1] = 0;
#endif
    cur->numKeys--;
    reBalance(cur, parent, ppos);
}

static void reBalance(btNode *cur, btNode *parent, int ppos)
{
    btNode *sibling;
    if ((parent == NULL) ||
        (cur->isLeaf && cur->numKeys >= 1) ||
        (!cur->isLeaf && cur->numKeys >= UNDER_FLOW))
        return;

    int threshold = cur->isLeaf ? 1 : UNDER_FLOW;
    // try to get a key from left sibling
    if (ppos > 0 && (sibling = parent->kids[ppos - 1])->numKeys > threshold)
    {
        // borrow from left sibling
        int tmp = parent->keys[ppos - 1];
        // move parent key to cur node
        parent->keys[ppos - 1] = sibling->keys[sibling->numKeys - 1];
#ifdef DEBUG
        sibling->keys[sibling->numKeys - 1] = 0;
#endif
        // move array one unit to the right
        memmove(&cur->keys[1], &cur->keys[0], sizeof(*cur->keys) * (cur->numKeys));
        cur->keys[0] = tmp;

        // TODO remove sibling kid node
        if (!cur->isLeaf)
        {
            memmove(&cur->kids[1], &cur->kids[0], sizeof(void *) * (cur->numKeys + 1));
            cur->kids[0] = sibling->kids[sibling->numKeys + 1];
#ifdef DEBUG
            sibling->kids[sibling->numKeys + 1] = NULL;
#endif
        }
        cur->numKeys++;
        sibling->numKeys--;
    }
    // try to get a key from right sibling
    else if (ppos < parent->numKeys && (sibling = parent->kids[ppos + 1])->numKeys > threshold)
    {
        // borrow from right sibling
        int tmp = parent->keys[ppos];
        parent->keys[ppos] = sibling->keys[0];
        memmove(&(sibling->keys[0]), &(sibling->keys[1]), sizeof(*sibling->keys) * (sibling->numKeys - 1));
#ifdef DEBUG
        sibling->keys[sibling->numKeys - 1] = 0;
        sibling->keys[sibling->numKeys] = 0;
#endif
        cur->keys[cur->numKeys] = tmp;
        if (!cur->isLeaf)
        {
            cur->kids[cur->numKeys + 1] = sibling->kids[0];
            memmove(&sibling->kids[0], &sibling->kids[1], sizeof(void *) * sibling->numKeys);
#ifdef DEBUG
            sibling->kids[sibling->numKeys] = NULL;
#endif
        }
        cur->numKeys++;
        sibling->numKeys--;
    }
    else
    {
        // there is no spare key in siblings
        // we should merge keys in cur, a sibling and a separator into one node.
        // then remove cur or sibling  
        if (ppos > 0)
        {
            // merge into left sibling

            sibling = parent->kids[ppos - 1];
            sibling->keys[sibling->numKeys] = parent->keys[ppos - 1];
            memmove(&sibling->keys[sibling->numKeys + 1], &cur->keys, (sizeof(*cur->keys)) * cur->numKeys);
            memmove(&parent->keys[ppos - 1], &parent->keys[ppos], sizeof(*parent->keys) * (parent->numKeys));
#ifdef DEBUG
            parent->keys[parent->numKeys - 1] = 0;
#endif
            memmove(&parent->kids[ppos], &parent->kids[ppos + 1], sizeof(void *) * (parent->numKeys - ppos - 1));
#ifdef DEBUG
            parent->kids[parent->numKeys + 1] = NULL;
#endif
            sibling->numKeys += 1 + cur->numKeys;
            parent->numKeys--;
            free(cur);
        }
        else
        {
            // ppos == 0
            sibling = parent->kids[ppos + 1];
            // copy parent key to cur
            cur->keys[cur->numKeys] = parent->keys[ppos];
            cur->numKeys++;
            // copy keys of sibling to cur
            memmove(&cur->keys[cur->numKeys], &sibling->keys, sizeof(*cur->keys) * sibling->numKeys);
#ifdef DEBUG
            bzero(sibling->keys, sizeof(*sibling->keys) * sibling->numKeys);
#endif
            memmove(&cur->kids[cur->numKeys], &sibling->kids[0], sizeof(void *) * (sibling->numKeys + 1));
            // move all keys in parent one unit towards the left
            memmove(&parent->keys, &parent->keys[1], sizeof(*parent->keys) * (parent->numKeys - 1));
#ifdef DEBUG
            parent->keys[parent->numKeys - 1] = 0;
#endif
            // move all kids pointer towards the left
            memmove(&parent->kids[1], &parent->kids[2], sizeof(void *) * (parent->numKeys));

            cur->numKeys += sibling->numKeys;
            parent->numKeys--;
            free(sibling);
        }
    }
}