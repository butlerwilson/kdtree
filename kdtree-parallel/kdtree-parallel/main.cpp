#include "kdtree.h"
#include <iostream>
#include <fstream>
#include <string>

int init_meta_data(std::vector<std::vector<float> >  &meta, std::string filename,
	int &vcnt, int &vdim)
{
	int cnt = -1;
	std::string string;
	std::ifstream in(filename);

	if (!in) {
		std::cout << "open source file:" << filename << " failed" << std::endl;
		return 0;
	}
	in >> string >> vdim;

	while (!in.eof()) {
		++cnt;
		double v0, v1, v2;
		std::vector<float> tmpdb;
		std::getline(in, string);
		if (string.empty()) continue;
		sscanf_s(string.c_str(), "%lf %lf %lf", &v0, &v1, &v2);
		tmpdb.push_back(v0); tmpdb.push_back(v1); tmpdb.push_back(v2);
		meta.push_back(tmpdb);
	}
	in.close();
	vcnt = cnt - 1;

	return cnt;
}

int init_rand_meta_data(const char *filename, int &vcnt, int &vdim)
{
	int cnt = 0;

	std::ofstream out(filename);
	out << "dimensions: " << vdim << std::endl;
	for (; cnt < vcnt; ++cnt) {
		char buffer[1024];
		for (int dim = 0; dim < vdim; ++dim) {
			double v = ((rand() % 100) - 50) / 0.25;
			_snprintf_s(buffer, 20, 19, "%lf", v);
			out << buffer << " ";
		}
		out << "\n";
	}
	out.close();

	return 0;
}

int main(int argc, char **argv)
{
	int res = 0;
	int vertexc, vertexd;
	struct kd_tree *root;
	std::vector<float> target;
	std::string filename("meta.dat");
	std::vector<std::vector<float> > data;

	vertexc = 10000;
	vertexd = DIMENSIONS;
	std::cout << "vertex number:" << vertexc << std::endl;
	std::cout << "init the meta data..." << std::endl;
	init_rand_meta_data(filename.c_str(), vertexc, vertexd);
	res = init_meta_data(data, filename, vertexc, vertexd);
	if (res == 0) return 1;
	std::cout << "init the meta data complete." << std::endl;

	kdtree_parallel *kdtp = new kdtree_parallel(data, vertexc);

	std::cout << "create the KD-TREE..." << std::endl;
	kdtp->kdtp_create_tree(root);

	std::cout << "search taget:";
	float tmpdb = 0.0;
	for (int dim = 0; dim < vertexd; ++dim) {
		std::cin >> tmpdb;
		target.push_back(tmpdb);
	}
	bool bres = kdtp->kdtp_search_node_with_parallel(root, target);
	if (bres == true) {
		kdtp->kdtp_print_nearest();
	} else 
		std::cout << "Not fund the node." << std::endl;
	kdtp->kdtp_destroy(root);

	system("pause");
	return 0;
}