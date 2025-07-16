//
// Created by pawel on 30.06.25.
//

#pragma once

#ifndef VARIABLE_TREE_H
#define VARIABLE_TREE_H

#endif //VARIABLE_TREE_H

#include <string.h>

struct variable_tree_node {
	struct variable_tree_node* next;
	struct variable_tree_node* child;

	char letter;
	int value;
};

/* Recursively search for node indexed by string */
struct variable_tree_node* variable_tree_search_internal(
	struct variable_tree_node* node, char* string, int index) {

	if(!node)
		return NULL;

	/* If found destination node return it */
	if(index + 1 == strlen(string) && node->letter == string[index]) {
		return node;
	}

	/* Look in child node if letter at index position is correct */
	if(node->letter == string[index]) {
		return variable_tree_search_internal(node->child, string, index + 1);
	}
	else {
		return variable_tree_search_internal(node->next, string, index);
	}
}

/* Searches for node indexed by string.
 Returns either pointer to found node or NULL if couldn't find one.

 Arguments:
 * root - pointer to tree root node
 * string - index where value is to be stored
 */
struct variable_tree_node* variable_tree_search(
	struct variable_tree_node* root, char* string) {

	return variable_tree_search_internal(root->child, string, 0);
}

/* Recursively creates nodes and inserts value to the tree */
int variable_tree_add_value_internal(
	struct variable_tree_node* node,
	char* string,
	int value,
	int index) {

	/* If found destination node check if it already has value and return.
	 * If it hasn't then write new value and return.
	 * If it has then return error value (node already exists).
	 */
	if(index + 1 == strlen(string) && node->letter == string[index]) {
		if(node->value == -1) {
			node->value = value;
			return 0;
		}
		else {
			return 1;
		}
	}

	/* Check if letter is found */
	if(node->letter == string[index]) {
		/* Create new child if absent */
		if(!node->child) {
			node->child = malloc(sizeof(struct variable_tree_node));
			memset(node->child, 0, sizeof(struct variable_tree_node));
			node->child->letter = string[index + 1];
			node->child->value = -1;
		}
		/* Look in child node */
		return variable_tree_add_value_internal(node->child, string, value,
			index + 1);
	}
	else {
		/* If node->next is NULL then haven't found node with desired letter;
		 * have to create new one */
		if(!node->next) {
			node->next = malloc(sizeof(struct variable_tree_node));
			memset(node->next, 0, sizeof(struct variable_tree_node));
			node->next->letter = string[index];
			node->next->value = -1;
		}
		/* Look on another node */
		return variable_tree_add_value_internal(node->next, string, value, index);
	}
}

/* Adds value to variable tree
 Returns 0 if executed without errors; returns 1 if node already exists

 Arguments:
 * root - pointer to tree root node
 * string - index where value is to be stored
 * value - value to be stored
 */
int variable_tree_add_value(
	struct variable_tree_node* root, char* string, int value) {

	if(!root->child) {
		root->child = malloc(sizeof(struct variable_tree_node));
		memset(root->child, 0, sizeof(struct variable_tree_node));
		root->child->letter = string[0];
		root->child->value = -1;
	}
	return variable_tree_add_value_internal(root->child, string, value, 0);
}

/* Free memory allocated for tree.
 This function does not return anything.

 Arguments:
 * root - pointer to tree root node
 */
void variable_tree_dispose(struct variable_tree_node* root) {

	if(root->next) {
		variable_tree_dispose(root->next);
		free(root->next);
	}
	if(root->child) {
		variable_tree_dispose(root->child);
		free(root->child);
	}
}

