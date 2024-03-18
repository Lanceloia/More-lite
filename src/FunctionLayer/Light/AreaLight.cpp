#include "AreaLight.h"
#include <ResourceLayer/Factory.h>

AreaLight::AreaLight(const Json& json) : Light(json) {
	type = LightType::AreaLight;
	shape = Factory::construct_class<Shape>(json["shape"]);

	// 保证energy与power不同时存在，且必定存在一个
	energy = fetchOptional<Spectrum>(json, "energy", 0.0f);
	Spectrum power = fetchOptional<Spectrum>(json, "power", 0.0f);

	if (!energy.isZero())
	{
		// do nothing
	}
	else // if (!power.isZero())
	{
		// 将power转化为energy
		energy = power / (PI * shape->getArea());
	}

	rank = fetchOptional<float>(json, "rank", 0.0f);
	if (rank != 0.0f)
		useRank = true;
}

Spectrum AreaLight::evaluateEmission(const Intersection& intersection,
	const Vector3f& wo) const {
	Spectrum retRadiance;
	if (!useRank)
		retRadiance = energy;
	else {
		// normal的模长为1, 法线方向可能是反的，加绝对值
		float cos_theta = abs(dot(wo, intersection.normal) / wo.length());
		float alpha = (rank + 1.0f) * std::pow(cos_theta, rank);
		retRadiance = alpha * energy;

	}
	return retRadiance;
}

LightSampleResult AreaLight::sample(const Intersection& shadingPoint,
	const Vector2f& sample) const {
	Intersection sampleResult;
	float pdf;
	shape->uniformSampleOnSurface(sample, &sampleResult, &pdf);
	Vector3f shadingPoint2sample = sampleResult.position - shadingPoint.position;

	Spectrum retRadiance;
	if (!useRank)
		retRadiance = energy;
	else {
		// normal的模长为1, 法线方向可能是反的，加绝对值
		float cos_theta = abs(dot(shadingPoint2sample, sampleResult.normal)) / shadingPoint2sample.length();
		float alpha = (rank + 1.0f) * std::pow(cos_theta, rank);
		retRadiance = alpha * energy;
	}

	return { retRadiance,
			normalize(shadingPoint2sample),
			shadingPoint2sample.length() - EPSILON,
			sampleResult.normal,
			pdf,
			false,
			type };
}

REGISTER_CLASS(AreaLight, "areaLight")