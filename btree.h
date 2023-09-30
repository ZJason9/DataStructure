#define MAX_KEYS 5
//if keys of an internal node less than we should reblance the subtree
#define UNDER_FLOW MAX_KEYS / 2

typedef struct btNode
{
    bool isLeaf;
    int numKeys;
    int keys[MAX_KEYS];
    struct btNode *kids[MAX_KEYS + 1];
} btNode;

typedef btNode *bTree;

/*************************
*          API           *
**************************/

/** Create a B-Tree */
bTree btCreate();

/* insert a key */
void btInsert(bTree *bt, int key);
/* delete a key */
void btDelete(bTree *bt, int key);
/* search a key in a B-Tree */
bool btSearch(bTree t, int key);
/* Destroy a B-Tree */
void btDestroy(bTree t);