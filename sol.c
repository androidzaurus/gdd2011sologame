#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define LEFT_NODE 0
#define RIGHT_NODE 1

struct _global {
	int num;
} globals;

struct bintree {
	struct bintree *parent;
	struct bintree *left;
	struct bintree *right;
	int *array;
	int num;
	int removed;
	int point;
	int depth;
};

struct game {
	struct bintree *root;
	int *array;
	int num;
	int mindepth;
	int ruleddepth;
};

void freebintree(struct bintree *node)
{
	if (node->left)
		freebintree(node->left);
	if (node->right)
		freebintree(node->right);
	free(node->array);
	free(node);
}

void cleanup(struct game *game)
{
	freebintree(game->root);
	if (game->array)
		free(game->array);
	free(game);
}

FILE *readglobal(char *name)
{
	FILE *f;
	char buf[1024];
	f = fopen(name, "r");
	fgets(buf, sizeof(buf), f);
	sscanf(buf, "%d", &globals.num);
	return f;
}

int readnums(struct game *game, char *str)
{
	char *p;
	int i, j;
	int *array = game->array;

	for (i=j=0, p=str ; *p != '\0' ; i++, p++) {
		if (*p == ' ' || *p == '\n') {
			sscanf(str, "%d", array++);
			str = p + 1;
			if (*p == '\n')
				break;
		}
	}

#ifdef DEBUG
	for (i=0, array=game->array ; i<game->num ; i++)
		printf("%d ", *array++);
	printf("\n");
#endif

	return game->num;
}

struct game *readgame(FILE *f)
{
	char *buf = (char *)calloc(1, 1024);
	struct game *game = (struct game *)calloc(1, sizeof(struct game));

	fgets(buf, 1024, f);
	sscanf(buf, "%d", &game->num);

	game->array = (int *)calloc(game->num, sizeof(int));

	fgets(buf, 1024, f);
	readnums(game, buf);

	free(buf);
	return game;
}

struct bintree *newnode(struct game *game, struct bintree *parent)
{
	struct bintree *n;
	
	n = (struct bintree *)calloc(1, sizeof(struct bintree));

	n->parent = parent;
	n->array  = (int *)calloc(game->num, sizeof(int));
	if (parent) {
		n->num = parent->num;
		n->removed = parent->removed;
		n->depth = parent->depth + 1;
		memcpy(n->array, parent->array, sizeof(int) * n->num);
	}
	else {
		n->num = game->num;
		memcpy(n->array, game->array, sizeof(int) * n->num);
	}

	return n;
};

int evalremove5(int *array, int num, int removed)
{
	int i, point, count;
	
	for (i=count=point=0 ; i<num ; i++, array++) {
		if (*array < 0)
			continue;
		if (*array % 5 == 0) {
			count++;
			if (*array % 2 == 1)
				point++;
		}
	}
	if (count == num - removed)
		return count;
	return point;
}

struct bintree *half(struct game *game, struct bintree *parent)
{
	int i, num;
	int *p;

	struct bintree *node = newnode(game, parent);
	parent->left = node;

	for (i=0, p=node->array, num=node->num ; i<num ; i++, p++) {
		if (*p < 0)
			continue;
		*p /= 2;
	}

	return node;
}

struct bintree *remove5(struct game *game, struct bintree *parent)
{
	int i, num;
	int *p;

	struct bintree *node = newnode(game, parent);
	parent->right = node;

	for (i=0, p=node->array, num=node->num ; i<num ; i++, p++) {
		if (*p < 0)
			continue;
		if (*p % 5 == 0) {
			*p = -1;
			node->removed++;
		}
	}

	return node;
}

int ruledengine(struct game *game, struct bintree *node)
{
	struct bintree *next;

	if (node->num == node->removed) {
		game->mindepth = game->ruleddepth = node->depth;
		return node->depth;
	}

	next = evalremove5(node->array, node->num, node->removed) ?
		remove5(game, node) : half(game, node);

	return ruledengine(game, next);
}

int walkaroundengine(struct game *game, struct bintree *node)
{
#ifdef DEBUG
	printf("%d\r", node->depth);
#endif

	if (node->num == node->removed) {
		if (node->depth < game->mindepth)
			game->mindepth = node->depth;
		return game->mindepth;
	}

	if (node->depth == game->mindepth)
		return node->depth;

	walkaroundengine(game, remove5(game, node));
	walkaroundengine(game, half(game, node));

	freebintree(node->right);
	freebintree(node->left);
	node->right = node->left = NULL;

	return game->mindepth;
}

int engine(struct game *game)
{
	game->root = newnode(game, NULL);
	ruledengine(game, game->root);
	freebintree(game->root);
	game->root = newnode(game, NULL);
	walkaroundengine(game, game->root);
	return game->mindepth;
}

int main(int argc, char *argv[])
{
	FILE *fp;
	int i;
	
	fp = readglobal("soldata.txt");
	for (i=0 ; i<globals.num ; i++) {
		struct game *game = readgame(fp);
		engine(game);
		printf("%d\n", game->mindepth);
		cleanup(game);
	}
	
	fclose(fp);
	return 0;
}
