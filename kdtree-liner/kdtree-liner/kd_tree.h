#pragma once
/*
*									KD-tree
*
* In computer science, a k-d tree (short for k-dimensional tree) is a
* space-partitioning data structure for organizing points in a k-dimensional space.
* k-d trees are a useful data structure for several applications, such as searches
* involving a multidimensional search key (e.g. range searches and nearest neighbor
* searches). k-d trees are a special case of binary space partitioning trees.
*/

/*
* KD-tree is a binary tree, every node has two children node, one store left subtree
* and the another sotre the right subtree. Also we define dim variable as dimensions.
* We need a median to split the space.
* struct kd_tree {
*	int kdt_vector[N];
*	int kdt_median;
*	int kdt_dimensions;
*	struct kd_tree *kdt_left;
*	struct kd_tree *kdt_right;
*	struct kd_tree *kdt_parent;
* };
*/

#ifndef _KD_TREE_H_
#define _KD_TREE_H_

#include <vector>
#include <stack>
#include <iostream>

#define DIMENSIONS	3

/* show the divided coordinate int this graph.
*
* 3 coordinates like this:
* <1, 2> <2, 1> <3, 2>
*		 ^											 kd-tree:
*		6|													      <1, 2>
*		5|															||
*		4|	   |												   /  \
*		3|	   |											  	  /	   \
*		2|--*--|--*----										   <2, 1> <3, 2>
*		1|	   *
*       |	   |
*	    0--------------------------->
*		    1  2  3  4  5  6  7  8
*/

struct kd_tree {
	std::vector<double> vec;		//space node N dimension coordinate
	int kdt_dimensions;				//space dimensions
	double kdt_splitline;			//median for split the space
	struct kd_tree *kdt_left;		//left sub-KD-tree
	struct kd_tree *kdt_right;		//right sub-KD-tree
	struct kd_tree *kdt_parent;		//the parent node
};

typedef std::vector< std::vector<double> >::iterator it_t;

class kd_tree_c {
public:

	kd_tree_c(std::vector<std::vector<double>> org_data, int vcnt);

	/* Create the k-dimension tree*/
	int kdt_create_tree(struct kd_tree * &root);
	int kdt_create_tree(it_t startindex, it_t endindex, struct kd_tree * &root);

	/* Ensure which dimension will split, return the split value.
	* 1: x dimension  2: y dimension  3: z dimension 4: x dimension 5: y dimension...
	*/
	int kdt_ensure_split(it_t startindex, it_t endindex);

	/*sort the metal in asc order from index start to end*/
	int kdt_sort_by_split(it_t startindex, it_t endindex, int split);

	/* Based on the split, find the median int the cector*/
	std::vector<double> kdt_find_median(it_t startindex, it_t endindex, it_t &midindex);

	/* Search the node in the KD-tree, if found return true and node point to this node, or
	* return false and set node point null.
	*/
	bool kdt_search_node(struct kd_tree *root, std::vector<double> target, size_t k);

	/*visit the k-dimensions tree as previous order*/
	int kdt_visite_tree(struct kd_tree *root);

	/* Get the distance between two point*/
	double kdt_distance(std::vector<double> one, std::vector<double> two);
	
	void kdt_print_nearest()
	{
		if (nearest_node) {
			for (size_t ii = 0; ii < vertexd; ++ii)
				std::cout << nearest_node->vec[ii] << " ";
		}
		std::cout << std::endl;
	}

	double kdt_node_compare(struct kd_tree kdt_n1, struct kd_tree kdt_n2, int split)
	{
		//the leaf node
		if (split == -1)
			return 0.0;
		
		double tmp = kdt_n1.vec[split] - kdt_n2.vec[split];
		return tmp;
	}

	void kdt_destroy(struct kd_tree *root);

	~kd_tree_c();

private:
	size_t vertexc;
	size_t vertexd;
	struct kd_tree *nearest_node;
	std::vector<std::vector<double>> meta;	//the metal data set
	std::stack<struct kd_tree *> path;		//search path

};

class compare {
private:
	int dim;
public:
	compare(int d) :dim(d) {}

	bool operator()	(std::vector<double> vec1, std::vector<double> vec2)
	{
		return vec1[dim] < vec2[dim];
	}

};

#endif
