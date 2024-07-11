#include <CoreLayer/Math/Math.h>
#include <FunctionLayer/Camera/Pinhole.h>
#include <FunctionLayer/Integrator/Integrator.h>
#include <FunctionLayer/Sampler/Sampler.h>
#include <FunctionLayer/Scene/Scene.h>
#include <FunctionLayer/Texture/Mipmap.h>
#include <ResourceLayer/Factory.h>
#include <ResourceLayer/FileUtil.h>
#include <ResourceLayer/Image.h>
#include <ResourceLayer/JsonUtil.h>
#include <chrono>
#include <fstream>
#include <regex>
#include <stdio.h>
#include <windows.h>
#include "threadpool.cpp"

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

#define DATA_PARALLEL
#define USE_CPU 6

inline void printProgress(float percentage) {
	int val = (int)(percentage * 100);
	int lpad = (int)(percentage * PBWIDTH);
	int rpad = PBWIDTH - lpad;
	printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
	fflush(stdout);
}

void data_parallel(int x, int y, int width, int height, int spp,
	std::shared_ptr<Camera> camera,
	std::shared_ptr<Scene> scene,
	std::shared_ptr<Integrator> integrator,
	std::shared_ptr<Sampler> sampler)
{
	Vector2f NDC{ (float)x / width, (float)y / height };
	Spectrum li(.0f);
	for (int i = 0; i < spp; ++i) {
		Ray ray = camera->sampleRayDifferentials(
			CameraSample{ sampler->next2D() }, NDC);
		li += integrator->li(ray, *scene, sampler);
	}
	camera->film->deposit({ x, y }, li / spp);
}

int main(int argc, char** argv) {
	const std::string sceneDir = std::string(argv[1]);
	FileUtil::setWorkingDirectory(sceneDir);
	std::string sceneJsonPath = FileUtil::getFullPath("scene.json");
	std::ifstream fstm(sceneJsonPath);
	Json json = Json::parse(fstm);
	auto camera = Factory::construct_class<Camera>(json["camera"]);
	auto scene = std::make_shared<Scene>(json["scene"]);
	auto integrator = Factory::construct_class<Integrator>(json["integrator"]);
	auto sampler = Factory::construct_class<Sampler>(json["sampler"]);
	int spp = sampler->xSamples * sampler->ySamples;
	int width = camera->film->size[0], height = camera->film->size[1];

	auto start = std::chrono::system_clock::now();

#ifdef DATA_PARALLEL
	ThreadPool pool(USE_CPU);
	pool.init();
	printf("Submitting...\n");
#endif

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
#ifdef DATA_PARALLEL
			pool.submit(data_parallel,
				x, y, width, height, spp,
				camera, scene, integrator, sampler);
#else
			data_parallel(x, y, width, height, spp,
				camera, scene, integrator, sampler);
#endif
			int finished = x + y * width;
			if (finished % 5 == 0) {
				printProgress((float)finished / (height * width));
			}
		}
	}

#ifdef DATA_PARALLEL
	printProgress(1.f);
	printf("\n");
	printf("Rendering...\n");
	int remain;
	// 等待全部线程执行完毕
	while (remain = pool.queue_remain()) {
		int finished = height * width - remain;
		printProgress((float)finished / (height * width));
		Sleep(500);
	}
	Sleep(1000);
	printProgress(1.f);	// must before shutdown
	printf("\n");
	pool.shutdown();

#endif

	auto end = std::chrono::system_clock::now();

	printf("\nRendering costs %.2fs\n",
		(std::chrono::duration_cast<std::chrono::milliseconds>(end - start))
		.count() /
		1000.f);

	//* 目前支持输出为png/hdr两种格式
	std::string outputName =
		fetchRequired<std::string>(json["output"], "filename");
	if (std::regex_match(outputName, std::regex("(.*)(\\.png)"))) {
		camera->film->savePNG(outputName.c_str());
	}
	else if (std::regex_match(outputName, std::regex("(.*)(\\.hdr)"))) {
		camera->film->saveHDR(outputName.c_str());
	}
	else {
		std::cout << "Only support output as PNG/HDR\n";
	}
}
