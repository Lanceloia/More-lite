#include "Sphere.h"
#include <ResourceLayer/Factory.h>

Sphere::Sphere(const Json& json) : Shape(json) {
	center = fetchRequired<Point3f>(json, "center");
	radius = fetchRequired<float>(json, "radius");

	center = transform.toWorld(center);
	boundingBox = AABB(center - Vector3f(radius), center + Vector3f(radius));
	area = (4 * PI * radius * radius);
}

bool Sphere::rayIntersectShape(Ray& ray, int* primID, float* u,
	float* v) const {
	Point3f origin = ray.origin;
	Vector3f direction = ray.direction;
	Vector3f o2c = center - origin;
	float b = dot(o2c, direction);
	float c = o2c.length() * o2c.length() - radius * radius;
	float delta = b * b - c;
	if (delta <= 0)
		return false; // 不相交
	float sqrtDelta = fm::sqrt(delta);
	float t1 = b - sqrtDelta;
	float t2 = b + sqrtDelta;

	bool hit = false;
	if (ray.tNear <= t2 && t2 <= ray.tFar) {
		ray.tFar = t2;
		hit = true;
	}
	if (ray.tNear <= t1 && t1 <= ray.tFar) {
		ray.tFar = t1;
		hit = true;
	}
	if (hit) {
		*primID = 0;
		//* 计算u,v
		// TODO 需要考虑旋转
		Vector3f normal = normalize(ray.at(ray.tFar) - center);
		float cosTheta = normal[1];
		*v = fm::acos(cosTheta);
		if (std::abs(normal[2]) < 1e-4f) {
			*u = (normal[0] > .0f) ? (PI * .5f) : (PI * 1.5f);
		}
		else {
			float tanPhi = normal[0] / normal[2];
			*u = fm::atan(tanPhi); // u in [-.5f * PI, .5f * PI]
			if (normal[2] < .0f)
				*u += PI;
		}
	}
	return hit;
}

void Sphere::fillIntersection(float distance, int primID, float u, float v,
	Intersection* intersection) const {
	// u->phi, v->theta

	intersection->shape = this;
	intersection->distance = distance;
	Vector3f normal = Vector3f{ std::sin(v) * std::sin(u), std::cos(v),
							   std::sin(v) * std::cos(u) };

	intersection->normal = normal;

	//* 计算交点
	Point3f position = center + radius * normal;
	intersection->position = position;

	//* 计算纹理坐标
	intersection->texCoord = Vector2f{ u * INV_PI * .5f, v * INV_PI };

	// TODO 计算交点的切线和副切线
	Vector3f tangent{ 1.f, 0.f, .0f };
	Vector3f bitangent;
	if (std::abs(dot(tangent, intersection->normal)) > .9f) {
		tangent = Vector3f(.0f, 1.f, .0f);
	}
	bitangent = normalize(cross(tangent, intersection->normal));
	tangent = normalize(cross(intersection->normal, bitangent));
	intersection->tangent = tangent;
	intersection->bitangent = bitangent;
}

void Sphere::uniformSampleOnSurface(Vector2f sample, Intersection* intersection, float* pdf) const
{
	/*
		sample: 两个[0,1]之间的随机数
		intersection: 作为返回值使用，至少需要填写position和normal两个域
		pdf: 作为返回值使用，表示在这一物体表面上采样的概率密度函数，以面积计
	*/


	// https://www.pbr-book.org/3ed-2018/Monte_Carlo_Integration/2D_Sampling_with_Multidimensional_Transformations#ConcentricSampleDisk

	float u = sample.a[0];
	float v = sample.a[1];

	float x = fm::cos(2 * PI * v) * 2 * fm::sqrt(u * (1 - u));
	float y = fm::sin(2 * PI * v) * 2 * fm::sqrt(u * (1 - u));
	float z = 1 - 2 * u;

	intersection->position = this->center + this->radius * Vector3f(x, y, z);
	intersection->normal = Vector3f(x, y, z);

	*pdf = 1.0f / area;
}

REGISTER_CLASS(Sphere, "sphere")