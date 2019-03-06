#include <iostream>
#include <vector>
#include <skepu2.hpp>
#include "tests_common.hpp"


int sum(int overlap, size_t stride, const int* v) {
	int res = 0;
	for(int i = -overlap; i <= overlap; ++i) {
		res += v[i*stride];
	}
	return res;
}

auto skeleton = skepu2::MapOverlap(sum);

double runTest(const size_t size, const std::string& backend) {
	skepu2::Timer timer;
	
	skeleton.setOverlap(5);
	
	for(size_t iteration = 0; iteration < NUM_REPEATS; ++iteration) {
		skepu2::Vector<int> in1(size);
		skepu2::Vector<int> out(size);
		in1.randomize();
		timer.start();
		skeleton(out, in1);
		timer.stop();
	}
	std::cout << "Backend: " << backend << " Load: " << size << " Time: " << timer.getMedianTime() << std::endl;
	
	return timer.getMedianTime();
}

int main(int argc, char* argv[]) {
	std::vector<size_t> problemSizes;
	std::vector<double> cpuTimes;
	std::vector<double> gpuTimes;
	std::vector<double> hybridTimes;
	
	for(size_t i = 100000; i <= 4000000; i += 100000)
		problemSizes.push_back(i);
	
	printInfo("Running OpenMP CPU backend");
	skepu2::BackendSpec specCPU(skepu2::Backend::Type::OpenMP);
	specCPU.setCPUThreads(16);
	skeleton.setBackend(specCPU);
	for(size_t size : problemSizes) {
		double time = runTest(size, "OpenMP");
		cpuTimes.push_back(time);
	}
	
	printInfo("Running CUDA GPU backend");
	skepu2::BackendSpec specGPU(skepu2::Backend::Type::CUDA);
	specGPU.setDevices(1);
	skeleton.setBackend(specGPU);
	for(size_t size : problemSizes) {
		double time = runTest(size, "CUDA");
		gpuTimes.push_back(time);
	}
	
	printInfo("Running Hybrid backend");
// 	skepu2::BackendSpec specHybrid(skepu2::Backend::Type::Hybrid);
// 	specHybrid.setCPUThreads(16);
// 	specHybrid.setDevices(1);
// 	skeleton.setBackend(specHybrid);

	skepu2::backend::tuner::hybridTune(skeleton, 16, 1, 50000, 4000000);
	skeleton.resetBackend();
	for(size_t size : problemSizes) {
 		float percentage = 0.46;
// 		specHybrid.setCPUPartitionRatio(percentage);
		hybridTimes.push_back(runTest(size, "Hybrid"));
	}
	
	savePerformanceTest("times_mapoverlap.csv", problemSizes, cpuTimes, gpuTimes, hybridTimes);
		
	printOk("Test ended successfully");
	return 0;
}
