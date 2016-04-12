#include <iostream>
#include <fstream>
#include <string>
#include "kd_tree.h"

int init_rand_meta_data(const char *filename, int &vcnt, int &vdim);
int init_meta_data(std::vector<std::vector<double> >  &meta, std::string filename,
	int &vcnt, int &vdim);

int main(int argc, char **argv)
{
	int vcnt = 0;
	int vdim = 0;
	struct kd_tree *root = NULL;	//kd-tree root
	std::vector<std::vector<double> >  meta;
	
	vcnt = 10000;
	vdim = DIMENSIONS;

	std::cout << "vertex number:" << vcnt << std::endl;
	std::cout << "init the meta data..." << std::endl;
	init_rand_meta_data("meta.dat", vcnt, vdim);
	vcnt = init_meta_data(meta, "meta.dat", vcnt, vdim);
	std::cout << "init the meta data complete." << std::endl;
	if (vcnt == 0) return 1;
	kd_tree_c *kd_tree = new kd_tree_c(meta, vcnt);

	std::cout << "create the KD-TREE..." << std::endl;
	kd_tree->kdt_create_tree(root);

	std::cout << "search taget:";
	double tmpdb = 0.0;
	std::vector<double> tmp;
	for (size_t dim = 0; dim < meta[0].size(); ++dim) {
		std::cin >> tmpdb;
		tmp.push_back(tmpdb);  
	}
	struct kd_tree *node = NULL;
	bool res = kd_tree->kdt_search_node(root, tmp, 1);
	if (res == true) {
		std::cout << "found the most nearest node:";
		kd_tree->kdt_print_nearest();
	}
	else
		std::cout << "Not fund the node." << std::endl;

	kd_tree->kdt_destroy(root);

	system("pause");
	return 0;
}

int init_meta_data(std::vector<std::vector<double> >  &meta, std::string filename,
	int &vcnt, int &vdim)
{
	int cnt = 0;
	std::string string;
	std::ifstream in(filename);

	if (!in) {
		std::cout << "open source file:" << filename << " failed" << std::endl;
		return 0;
	}
	in >> string >> vdim;

	while (!in.eof()) {
		++cnt;
		double v;
		size_t wn = 0, wcnt = 0;
		std::vector<double> tmpdb;
		std::getline(in, string);
		if (string.empty()) continue;
		for (int ii = 0; ii < vdim; ++ii) {
			wn = sscanf_s(string.c_str() + wcnt, "%lf", &v);
			wcnt = wcnt + wn + 1;
			tmpdb.push_back(v);
		}
		meta.push_back(tmpdb);
	}
	in.close();
	vcnt = cnt;
	
	return cnt;
}

int init_rand_meta_data(const char *filename, int &vcnt, int &vdim)
{
	double cnt = 0.0;

	std::ofstream out(filename);
	out << "dimensions: " << vdim << std::endl;
	for (; cnt < vcnt; ++cnt) {
		char buffer[1024];
		for (int dim = 0; dim < vdim; ++dim) {
			double v = ((rand() % 100) - 50) / 3.14;
			_snprintf_s(buffer, 20, 19, "%lf", v);
			out << buffer << " ";
		}
		out << "\n";
	}
	out.close();

	return 0;
}