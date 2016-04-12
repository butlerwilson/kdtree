#include <algorithm>

#include "kd_tree.h"


// init the vec data set
kd_tree_c::kd_tree_c(std::vector<std::vector<double>> org_data, int vcnt)
{
	for (it_t it = org_data.begin(); it != org_data.end(); ++it) {
		meta.push_back(*it);
	}
	vertexc = vcnt;
	vertexd = meta[0].size();
}

int kd_tree_c::kdt_create_tree(struct kd_tree * &root)
{
	kdt_create_tree(meta.begin(), meta.end(), root);

	return 0;
}

int kd_tree_c::kdt_create_tree(it_t startindex, it_t endindex, struct kd_tree * &root)
{
	if (startindex >= meta.end() || endindex < meta.begin()) {
		root = NULL;
		return 0;
	}

	if ((startindex == endindex || (endindex - startindex == 1))) {
		root = new kd_tree;
		root->kdt_dimensions = -1;
		root->vec = *startindex;
		root->kdt_left = root->kdt_right = NULL;

	} else if (startindex < endindex) {

		int split = 0;
		it_t midindex;
		std::vector<double> median;

		root = new kd_tree;
		split = kdt_ensure_split(startindex, endindex);
		kdt_sort_by_split(startindex, endindex, split);
		median = kdt_find_median(startindex, endindex, midindex);

		root->kdt_dimensions = split;
		root->kdt_splitline = median[split];
		root->vec = median;

		kdt_create_tree(startindex, midindex - 1, root->kdt_left);
		kdt_create_tree(midindex + 1, endindex, root->kdt_right);
	} 
	else
		root = NULL;

	return 0;
}

std::vector<double> kd_tree_c::kdt_find_median(it_t startindex, it_t endindex, it_t &midindex)
{
	size_t si = std::distance(meta.begin(), startindex);
	size_t ei = std::distance(meta.begin(), endindex);

	size_t mid = (si + ei) / 2;
	midindex = meta.begin() + mid;

	return *midindex;
}

int kd_tree_c::kdt_sort_by_split(it_t startindex, it_t endindex, int split)
{
	//sort required the second argument point to the next position
	if (endindex != meta.end())	++endindex;

	sort(startindex, endindex, compare(split));

	return 0;
}

/* caculate the variance:
* variance = 1/n((ave - x1)^2 + (ave - x2)^2 + .... + (ave - xn)^2)
*/
int kd_tree_c::kdt_ensure_split(it_t startindex, it_t endindex)
{
	double *sum = new double[vertexd];
	double *average = new double[vertexd];
	double *variance = new double[vertexd];

	size_t sindex = std::distance(meta.begin(), startindex);
	size_t eindex = std::distance(meta.begin(), endindex);
	//keep the eindex as the next last one index
	if (endindex != meta.end())	++eindex;

	for (size_t ii = 0; ii < vertexd; ++ii)
		sum[ii] = 0.0;

	for (size_t ii = sindex; ii < eindex; ++ii) {
		for (size_t jj = 0; jj < vertexd; ++jj) {
			sum[jj] += meta[ii][jj];
		}
	}

	for (size_t ii = 0; ii < vertexd; ++ii) {
		average[ii] = sum[ii] / (eindex - sindex);
	}

	for (size_t ii = sindex; ii < eindex; ++ii) {
		for (size_t jj = 0; jj < vertexd; ++jj) {
			sum[jj] += pow(average[jj] - meta[ii][jj], 2);
		}
	}

	for (size_t ii = 0; ii < vertexd; ++ii) {
		variance[ii] = 1.0 / (eindex -sindex) * sum[ii];
	}

	int index = 0;
	for (size_t ii = 0; ii < vertexd; ++ii) {
		if (variance[index] < variance[ii])
			index = ii;
	}

	delete sum;
	delete average;
	delete variance;

	return index;
}

double kd_tree_c::kdt_distance(std::vector<double> one, std::vector<double> two)
{
	//x->y = sqrt((x1-x2)^2 + (y1-y2)^2 + ...)
	double sum = 0.0;

	for (size_t ii = 0; ii < vertexd; ++ii) {
		sum += pow(one[ii] - two[ii], 2);
	}

	return sqrt(sum);
}

//behind order recurse to delete all kd-tree node
void kd_tree_c::kdt_destroy(struct kd_tree *root)
{
	if (root != NULL) {
		kdt_destroy(root->kdt_left);
		kdt_destroy(root->kdt_right);
		if (root->kdt_left == NULL && root->kdt_right == NULL) {
			delete root;
			root = NULL;
		}
	}
}

kd_tree_c::~kd_tree_c()
{
}

bool kd_tree_c::kdt_search_node(struct kd_tree *root, std::vector<double> target, size_t k)
{
	double res = 0.0;
	double count = 0.0;
	double distance = INFINITY;
	struct kd_tree *tmp = root;
	struct kd_tree kdt_node;
	struct kd_tree *last_visit = NULL;
	std::stack<struct kd_tree *> tmp_path;

	if (root == NULL) {
		nearest_node = NULL;
		return false;
	}

	kdt_node.vec = target;
	kdt_node.kdt_left = kdt_node.kdt_right = NULL;

	while (tmp != NULL) {
		tmp_path.push(tmp);

		res = kdt_node_compare(kdt_node, *tmp, tmp->kdt_dimensions);
		if (res > 0) {
			tmp = tmp->kdt_right;
		}
		else if (res < 0) {
			tmp = tmp->kdt_left;
		}
		else
			tmp = NULL;
	}

	while (!tmp_path.empty()) {
		double tmp_dis = 0.0;
		struct kd_tree *tmp_node = tmp_path.top();
		tmp_path.pop();
		tmp_dis = kdt_distance(kdt_node.vec, tmp_node->vec);
/*		std::cout << "distance:" << tmp_dis << "v ";
		for (int ii = 0; ii < vertexd; ++ii)
			std::cout << tmp_node->vec[ii] << " ";
		std::cout << std::endl;
*/
		if (tmp_dis < distance) {
			//the new nearest node
			distance = tmp_dis;
			nearest_node = tmp_node;
			if (tmp_node->kdt_dimensions != -1) {
				if (last_visit == tmp_node->kdt_left) {
					if (tmp_node->kdt_right != NULL)
						tmp_path.push(tmp_node->kdt_right);
				}
				else if (last_visit == tmp_node->kdt_right)
					if (tmp_node->kdt_left != NULL)
						tmp_path.push(tmp_node->kdt_left);
			}
			last_visit = tmp_node;
		}
	}
	return true;
}

int kd_tree_c::kdt_visite_tree(kd_tree * root)
{
	if (root) {
		std::cout << "split tag:" << root->kdt_dimensions << std::endl;
		std::cout << "visit point:";
		for (size_t ii = 0; ii < vertexd; ++ii)
			std::cout << root->vec[ii] << " ";
		std::cout << std::endl;
		if (root->kdt_left)		kdt_visite_tree(root->kdt_left);
		if (root->kdt_right)	kdt_visite_tree(root->kdt_right);
	}

	return 0;
}
