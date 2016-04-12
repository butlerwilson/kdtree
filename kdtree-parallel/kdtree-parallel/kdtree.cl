#define EXCHANGE_NUMBER(point, offset0, offset1)	{\
	float tmp[3];\
	tmp[0] = point[offset0];\
	tmp[1] = point[offset0 + 1];\
	tmp[2] = point[offset0 + 2];\
	point[offset0] = point[offset1];\
	point[offset0 + 1] = point[offset1 + 1];\
	point[offset0 + 2] = point[offset1 + 2];\
	point[offset1] = tmp[0];\
	point[offset1 + 1] = tmp[1];\
	point[offset1 + 2] = tmp[2];\
}

__kernel void opencl_sort(__global float *data, const uint size, const uint vertexd, const uint dim)
{
	int pos = 0;
	uint allsize = size * vertexd;
	uint offset = 1;

	if (dim > vertexd) return;
	for (int ii = dim; ii < allsize; ii = offset * vertexd + dim) {
		uint offset1 = 1;
		pos = ii;
		for (int k = ii; k < allsize; k = offset1 * vertexd + dim) {
			if (data[k] > data[pos])
				pos = k;
		}
		EXCHANGE_NUMBER(data, pos - dim, ii);
	}
}

__kernel void opencl_vecadd(__global float *data, const uint vertexc, const uint vertexd, __global float *result)
{
	float sum = 0.0;
	float average = 0.0;
	size_t id = get_global_id(0);

	if (id >= vertexd) return;	//beyond the range

	for (size_t ii = 0; ii < vertexc * vertexd; ii += vertexd) {
		sum += data[ii + id];
	}

	average = sum / vertexc;
	sum = 0.0;
	for (size_t ii = 0; ii < vertexc * vertexd; ii += vertexd)
		sum = sum + pow(average - data[ii + id], 2);

	result[id] = sum / vertexc;
}

__kernel void opencl_cal_distance(__global float4 *node_set, uint count, float3 target)
{
	size_t id = get_global_id(0);
	if (id >= count) return;
	float4 node = node_set[id];

	node_set[id].w = sqrt(pow(node.x - target.x, 2) + pow(node.y - target.y, 2) + pow(node.z - target.z, 2));
//	printf("kernel<%d-%d>:%f %f %f %f\n", id, count, node.x, node.y, node.z, node_set[id].w);
}