#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX(a, b) (((a) >= (b)) ? (a) : (b))
typedef struct treeNode
{
	int data;
	struct treeNode *left;
	struct treeNode *right;
	int height;
} treeNode;

treeNode *insert(treeNode **root, int val);
void preOrder(treeNode *t);
void inOrder(treeNode *t);
treeNode *lRotate(treeNode *node);
treeNode *rRotate(treeNode *node);
int getHeight(treeNode *node);
int gethf(treeNode *node);

int main()
{
	int arr[] = {1, 2, 3, 4, 5};
	int i = 0;
	treeNode *root = NULL;
	for (; i < 5; i++)
	{
		insert(&root, arr[i]);
	}
	preOrder(root);
	printf("inOrder\n");
	inOrder(root);
	return 0;
}

treeNode *insert(treeNode **root, int val)
{
	treeNode *tmp;
	if (*root == NULL)
	{
		tmp = (treeNode *)malloc(sizeof(treeNode));
		if (tmp == NULL)
			return false;
		tmp->data = val;
		tmp->height = 1;
		*root = tmp;
		return tmp;
	}
	tmp = *root;

	tmp->height++;
	if (val < tmp->data) {
		tmp->left = insert(&(tmp->left), val);
		int hf = gethf(tmp);
		if(hf > 1){
			if(val > tmp->left->data)
				tmp = lRotate(tmp->left);
			tmp = rRotate(tmp);
		}
		
	} else {
		tmp->right = insert((&tmp->right), val);
		int hf = gethf(tmp);
		if(hf<-1){
			if(val < tmp->right->data)
				tmp = rRotate(tmp->right);
			tmp = lRotate(tmp);
		}
	}

	*root = tmp;
	return tmp;
}

void preOrder(treeNode *root)
{
	if (root == NULL)
		return;
	printf("%d\n", root->data);
	preOrder(root->left);
	preOrder(root->right);
}

void inOrder(treeNode *root)
{
	if (root == NULL)
		return;

	inOrder(root->left);
	printf("%d\n", root->data);
	inOrder(root->right);
}

/*left rotation makes
1. height of node and its left subtree minus 1
2. height of node->left and its right subtree add 1
3. height of node->left->left and its subtree stay stationary
*/
treeNode *lRotate(treeNode *node)
{
	treeNode *rn = node->right;
	treeNode *rln = rn->left;

	rn->left = node;
	node->right = rln;
	node->height = getHeight(node);
	rn->height = getHeight(rn);
	return rn;
}
treeNode *rRotate(treeNode *node)
{
	treeNode *ln = node->left;
	treeNode *lrn = ln->right;

	ln->right = node;
	node->left = lrn;
	node->height = getHeight(node);
	ln->height = getHeight(ln);
	return ln;
}

int getHeight(treeNode *node)
{
	if (node == NULL)
		return 0;
	return MAX(node->left,node->right)+1;
}

int gethf(treeNode *node){
	int lh = getHeight(node->left);
	int rh = getHeight(node->right);
	return lh - rh;
}