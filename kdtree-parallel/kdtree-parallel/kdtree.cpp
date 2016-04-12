#include "kdtree.h"

kdtree_parallel::kdtree_parallel(std::vector<std::vector<float> > data, int vcnt)
{
	int index = 0;
	vertexc = vcnt;

	for (it_t it = data.begin(); it != data.end(); ++it) {
		meta.push_back(*it);
//		std::cout << (*it)[0] << " " << (*it)[1] << " " << (*it)[2] << " " << std::endl;
	}
	work_size = 1024;
	nearest_distance = INFINITY;
	vertexd = meta[0].size();
	nearest_node = new kd_tree;
	parallel = new clParallel;
	parallel->kdtp_clInitOpenCLenvs();
//	parallel->kdtp_clShowOpenCLInfo();
	parallel->kdtp_clConstructProgram("kdtree.cl");
}

kdtree_parallel::~kdtree_parallel()
{
}

int kdtree_parallel::kdtp_create_tree(struct kd_tree * &root)
{
	std::cout << "build k-dimensions tree..." << std::endl;
	kdtp_create_tree(meta.begin(), meta.end(), root);

	return 0;
}

int kdtree_parallel::kdtp_create_tree(it_t startindex, it_t endindex, struct kd_tree * &root)
{
	if (startindex >= meta.end() || endindex < meta.begin()) {
		root = NULL;
		return 0;
	}
	if (endindex - startindex == 1) {
		root = new kd_tree;
		root->kdt_dimensions = -1;
		root->kdt_vec = *startindex;
		root->kdt_left = root->kdt_right = NULL;

	}
	else if (endindex - startindex > 1) {

		int split = 0;
		it_t midindex;
		std::vector<float> median;

		root = new kd_tree;
		split = kdtp_ensure_split_with_parallel(startindex, endindex);			//it can use parallel	
		kdtp_sort_by_split(startindex, endindex, split);
		median = kdtp_find_median(startindex, endindex, midindex);

		root->kdt_dimensions = split;
		root->kdt_splitline = median[split];
		root->kdt_vec = median;

		//the second argument is behind the last element
		kdtp_create_tree(startindex, midindex, root->kdt_left);
		kdtp_create_tree(midindex + 1, endindex, root->kdt_right);
	}
	else
		root = NULL;

	return 0;
}

int kdtree_parallel::kdtp_sort_by_split(it_t startindex, it_t endindex, int split)
{
	//sort required the second argument point to the next position
	if (endindex != meta.end())	++endindex;

	sort(startindex, endindex, compare(split));

	return 0;
}

/* caculate the variance:
* variance = 1/n((ave - x1)^2 + (ave - x2)^2 + .... + (ave - xn)^2)
*/
int kdtree_parallel::kdtp_ensure_split_with_parallel(it_t startindex, it_t endindex)
{
	float *tmp;
	int ind = 0;
	cl_mem mem, vmem;
	size_t *gwsize = new size_t[vertexd];
	size_t *lwsize = new size_t[vertexd];
	float *variance = new float[vertexd];

	cl_mem_flags flags = CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR;

	size_t sindex = std::distance(meta.begin(), startindex);
	size_t eindex = std::distance(meta.begin(), endindex);
	//keep the eindex as the next last one index
	if (endindex != meta.end())	++eindex;
	size_t size = (eindex - sindex) * vertexd * sizeof(float);
	tmp = new float[size];
	for (size_t ii = sindex; ii < eindex; ++ii) {
		for (size_t k = 0; k < DIMENSIONS; ++k)
		tmp[ind++] = meta[ii][k];
	}

	mem = parallel->kdtp_clCreateBuffer(flags, size, tmp);
	vmem = parallel->kdtp_clCreateBuffer(CL_MEM_WRITE_ONLY, vertexd * sizeof(float), NULL);
	parallel->kdtp_clConstructKernel("opencl_vecadd", 0, sizeof(cl_mem), &mem);
	parallel->kdtp_clConstructKernel("opencl_vecadd", 1, sizeof(size_t), &vertexc);
	parallel->kdtp_clConstructKernel("opencl_vecadd", 2, sizeof(size_t), &vertexd);
	parallel->kdtp_clConstructKernel("opencl_vecadd", 3, sizeof(float *), &vmem);
	gwsize[0] = vertexd; lwsize[0] = vertexc;
	parallel->kdtp_clEnqueueRunKernel(1, gwsize, NULL, 0, NULL, NULL);

	parallel->kdtp_clReadBufferFromKernel(vmem, vertexd * sizeof(float), variance);
//	parallel->kdtp_clFinishKernel();

	int index = 0;
	for (size_t ii = 0; ii < vertexd; ++ii) {
		if (variance[index] < variance[ii])
			index = ii;
	}

	delete []tmp;
	delete []gwsize;
	delete []lwsize;
	delete []variance;

	return index;
}

std::vector<float> kdtree_parallel::kdtp_find_median(it_t startindex, it_t endindex, it_t &midindex)
{
	size_t si = std::distance(meta.begin(), startindex);
	size_t ei = std::distance(meta.begin(), endindex);
	if (endindex == meta.end()) ei -= 1;

	size_t mid = (ei - si) / 2;
	midindex = startindex + mid;

	return *midindex;
}

float kdtree_parallel::kdtp_distance(std::vector<float> one, std::vector<float> two)
{
	//x->y = sqrt((x1-x2)^2 + (y1-y2)^2 + ...)
	float sum = 0.0;

	for (size_t ii = 0; ii < vertexd; ++ii) {
		sum += pow(one[ii] - two[ii], 2);
	}

	return sqrt(sum);
}

//behind order recurse to delete all kd-tree node
void kdtree_parallel::kdtp_destroy(struct kd_tree *root)
{
	if (root != NULL) {
		kdtp_destroy(root->kdt_left);
		kdtp_destroy(root->kdt_right);
		if (root->kdt_left == NULL && root->kdt_right == NULL) {
			delete root;
			root = NULL;
		}
	}
}

bool kdtree_parallel::list_tree_covert_to_liner_struct(struct kd_tree *root, cl_float4 *node_set)
{
	size_t index = 0;
	std::queue<struct kd_tree *> queue;

	if (root == NULL)	return false;
	queue.push(root);
	while (!queue.empty()) {
		struct kd_tree *tmp = queue.front();
		queue.pop();
		node_set[index].x = tmp->kdt_vec[0];
		node_set[index].y = tmp->kdt_vec[1];
		node_set[index].z = tmp->kdt_vec[2];
		node_set[index].w = 0.0;

//		std::cout << node_set[index].x << " " << node_set[index].y << " " << node_set[index].z << std::endl;

		if (tmp->kdt_left)	queue.push(tmp->kdt_left);
		if (tmp->kdt_right)	queue.push(tmp->kdt_right);
		++index;
	}

	return true;
}

bool kdtree_parallel::kdtp_search_node_with_parallel(struct kd_tree *root, std::vector<float> target)
{
	cl_float3 targ;
	float distance = INFINITY;
	cl_event event, *event_list = NULL;
	cl_float4 *liner_store_kdtree_node = new cl_float4[vertexc];
	size_t loopcount = vertexc % work_size == 0 ? vertexc / work_size : vertexc / work_size + 1;

	event_list = new cl_event[1];
	targ.x = target[0]; targ.y = target[1]; targ.z = target[2];
	list_tree_covert_to_liner_struct(root, liner_store_kdtree_node);
	for (size_t ii = 0; ii < loopcount; ++ii)
	{
		size_t offset = ii * work_size;
		size_t count = vertexc - offset > work_size ? work_size : vertexc - offset;
		size_t size = count * sizeof(cl_float4);
		cl_mem_flags flags = CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR;
		cl_mem mem = parallel->kdtp_clCreateBuffer(flags, size , liner_store_kdtree_node + offset);
		parallel->kdtp_clConstructKernel("opencl_cal_distance", 0, sizeof(cl_mem), &mem);
		parallel->kdtp_clConstructKernel("opencl_cal_distance", 1, sizeof(size_t), &count);
		parallel->kdtp_clConstructKernel("opencl_cal_distance", 2, sizeof(cl_float4), &targ);
		parallel->kdtp_clEnqueueRunKernel(1, &work_size, NULL, ii ? 1 : 0, ii ? event_list : NULL, &event);
		parallel->kdtp_clReadBufferFromKernel(mem, size, liner_store_kdtree_node + offset);
		parallel->kdtp_clFinishKernel();
		event_list[0] = event;
	}

	kdtp_ensure_nearest_distance(liner_store_kdtree_node);

	return true;
}

void kdtree_parallel::kdtp_ensure_nearest_distance(cl_float4 *liner)
{
	cl_float3 destnode;

	for (size_t ii = 0; ii < vertexc; ++ii) {
/*		std::cout << "index:[" << ii << "]<" << liner[ii].x << ",\t";
		std::cout << liner[ii].y << ",\t";
		std::cout << liner[ii].z << ">";
		std::cout << "\t\tdistance:" << liner[ii].w << std::endl;
*/	
		if (liner[ii].z < nearest_distance) {
			nearest_distance = liner[ii].w;
			destnode.x = liner[ii].x;
			destnode.y = liner[ii].y;
			destnode.z = liner[ii].z;
		}
	}
	nearest_node->kdt_vec.push_back(destnode.x);
	nearest_node->kdt_vec.push_back(destnode.y);
	nearest_node->kdt_vec.push_back(destnode.z);
}

bool kdtree_parallel::kdtp_search_node(struct kd_tree *root, std::vector<float> target, size_t k)
{
	float res = 0.0;
	float count = 0.0;
	float distance = INFINITY;
	struct kd_tree *tmp = root;
	struct kd_tree kdt_node;
	struct kd_tree *last_visit = NULL;
	std::stack<struct kd_tree *> tmp_path;

	if (root == NULL) {
		nearest_node = NULL;
		return false;
	}

	kdt_node.kdt_vec = target;
	kdt_node.kdt_left = kdt_node.kdt_right = NULL;

	while (tmp != NULL) {
		tmp_path.push(tmp);

		res = kdtp_node_compare(kdt_node, *tmp, tmp->kdt_dimensions);
		std::cout << "visit:" << ++count << std::endl;
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
		float tmp_dis = 0.0;
		struct kd_tree *tmp_node = tmp_path.top();
		tmp_path.pop();
		tmp_dis = kdtp_distance(kdt_node.kdt_vec, tmp_node->kdt_vec);
		std::cout << "distance:" << tmp_dis << "v ";
		for (size_t ii = 0; ii < vertexd; ++ii)
			std::cout << tmp_node->kdt_vec[ii] << " ";
		std::cout << std::endl;

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

int kdtree_parallel::kdtp_visite_tree(kd_tree * root)
{
	if (root) {
		std::cout << "split tag:" << root->kdt_dimensions << std::endl;
		std::cout << "visit point:";
		for (size_t ii = 0; ii < vertexd; ++ii)
			std::cout << root->kdt_vec[ii] << " ";
		std::cout << std::endl;
		if (root->kdt_left)		kdtp_visite_tree(root->kdt_left);
		if (root->kdt_right)	kdtp_visite_tree(root->kdt_right);
	}

	return 0;
}