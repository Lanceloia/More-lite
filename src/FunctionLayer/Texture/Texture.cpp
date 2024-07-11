#include "Texture.h"

// #define TEST_TEXTURE

TextureCoord UVMapping::map(const Intersection& intersection) const {

#ifdef TEST_TEXTURE
	Vector3f p = intersection.position - Point3f(0.f, 0.f, 0.f);
	Vector3f np = p / p.length();
	float phi = acosf(np[2]);
	float theta = abs(np[0]) < EPSILON ? 0.f : atanf(np[1] / np[0]);

	float u, v;
	// Sphere
	u = theta / (2 * PI);
	v = phi / PI;
	if (u < 0.f)
		u = 1.f + u;
	if (v < 0.f)
		v = -v;

	// Cylinder
	u = theta / (2 * PI);
	v = np[2];
	if (u < 0.f)
		u = 1.f + u;
	if (v < 0.f)
		v = 1.f + v;

	// Plane
	Vector3f va = Vector3f(1.f, 0.f, 0.f);
	Vector3f vb = Vector3f(0.f, 1.f, 0.f);
	u = dot(va, p);
	v = dot(vb, p);

	u = u - floor(u);
	v = v - floor(v);

	return TextureCoord{ Vector2f(u ,v),
					Vector2f{intersection.dudx, intersection.dvdx},
					Vector2f{intersection.dudy, intersection.dvdy} };
#else
	return TextureCoord{ intersection.texCoord,
						Vector2f{intersection.dudx, intersection.dvdx},
						Vector2f{intersection.dudy, intersection.dvdy} };
	
#endif
}