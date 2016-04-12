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

#include "clParallel.h"
#include <vector>
#include <stack>
#include <queue>

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

#define DIMENSIONS	3

struct kd_tree {
	int kdt_dimensions;
	float kdt_splitline;
	struct kd_tree *kdt_left;
	struct kd_tree *kdt_right;
	std::vector<float> kdt_vec;
};

typedef std::vector<std::vector<float> > vvd;
typedef std::vector< std::vector<float> >::iterator it_t;

class kdtree_parallel {
public:
	kdtree_parallel(std::vector<std::vector<float>> data, int vcnt);
	~kdtree_parallel();

	/* Create the k-dimension tree*/
	int kdtp_create_tree(struct kd_tree * &root);
	int kdtp_create_tree(it_t startindex, it_t endindex, struct kd_tree * &root);

	/* Ensure which dimension will split, return the split value.
	* 1: x dimension  2: y dimension  3: z dimension 4: x dimension 5: y dimension...
	*/
	int kdtp_ensure_split_with_parallel(it_t startindex, it_t endindex);

	/*sort the metal in asc order from index start to end*/
	int kdtp_sort_by_split(it_t startindex, it_t endindex, int split);

	/* Based on the split, find the median int the cector*/
	std::vector<float> kdtp_find_median(it_t startindex, it_t endindex, it_t &midindex);

	/* Search the node in the KD-tree, if found return true and node point to this node, or
	* return false and set node point null.
	*/
	bool kdtp_search_node(struct kd_tree *root, std::vector<float> target, size_t k);
	bool kdtp_search_node_with_parallel(struct kd_tree *root, std::vector<float> target);
	
	void kdtp_ensure_nearest_distance(cl_float4 *liner);

	/*visit the k-dimensions tree as previous order*/
	int kdtp_visite_tree(struct kd_tree *root);

	/* Get the distance between two point*/
	float kdtp_distance(std::vector<float> one, std::vector<float> two);

	void kdtp_print_nearest()
	{
		std::cout << "found nearest distance:" << std::endl;;
		std::cout << "<" << nearest_node->kdt_vec[0] << ",\t";
		std::cout << nearest_node->kdt_vec[1] << ",\t";
		std::cout << nearest_node->kdt_vec[2] << ">";
		std::cout << "\t\tdistance:" << nearest_distance << std::endl;
	}

	float kdtp_node_compare(struct kd_tree kdt_n1, struct kd_tree kdt_n2, int split)
	{
		//the leaf node
		if (split == -1)
			return 0.0;

		float tmp = kdt_n1.kdt_vec[split] - kdt_n2.kdt_vec[split];
		return tmp;
	}

	void kdtp_destroy(struct kd_tree *root);
private:
	size_t vertexc;
	size_t vertexd;
	size_t work_size;
	clParallel *parallel;
	float nearest_distance;

	struct kd_tree *nearest_node;
	std::stack<struct kd_tree *> path;		//search path
	std::vector<std::vector<float> > meta;

	bool list_tree_covert_to_liner_struct(struct kd_tree *root, cl_float3 *node_set);
};

//define for the sort function
class compare {
private:
	int dim;
public:
	compare(int d) :dim(d) {}

	bool operator()	(std::vector<float> vec1, std::vector<float> vec2)
	{
		return vec1[dim] < vec2[dim];
	}

};
