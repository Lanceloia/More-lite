#include "WoodTexture.h"
#include "ResourceLayer/Factory.h"

WoodTexture::WoodTexture(const Json& json) : Texture <Spectrum >() {
	// 生成 m x n 个格点(上限为256个)，注意此处为格点
	// 不宜过多，否则木纹会限制在格点内
	m = n = 6;		// m 行 n 列
	
	// k表示木纹的密集程度
	k = 5;
	
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			// 生成一个[0, 1)随机数，放缩到 2Pi
			float theta = float(rand()) / RAND_MAX * 2 * PI;
			
			float x = fm::cos(theta);
			float y = fm::sin(theta);
			normal[i * n + j] = Vector2f(x, y);
		}
	}
}

Spectrum WoodTexture::evaluate(const Intersection& intersection) const {
	TextureCoord texCoord = mapping->map(intersection);
	return evaluate(texCoord);
}

Spectrum WoodTexture::evaluate(const TextureCoord& texCoord) const {
	// 平滑函数，对插值系数进行平滑
	// 若不使用则会使木纹变成方块
	auto fade = [](float x) -> float {
		return 3 * x * x - 2 * x * x * x;
	};

	// 读取纹理采样点
	float u = texCoord.coord[0];
	float v = texCoord.coord[1];

	assert(u < 1.f);
	assert(v < 1.f);

	// 纹理坐标(px, py)
	float px = u * n;
	float py = v * m;


	// 计算(px, py)处的格子序号
	// i行号j列号
	int i = floor(py);
	int j = floor(px);

	// 邻近格点的序号，左下角为0, 逆时针旋转依次为1, 2, 3 

	// 插值比例r
	float rx = px - j;
	float ry = py - i;

	// 向量d
	Vector2f d0 = Vector2f(rx,  ry);
	Vector2f d1 = Vector2f(rx - 1, ry);
	Vector2f d2 = Vector2f(rx - 1, ry + 1);
	Vector2f d3 = Vector2f(rx,  ry + 1);

	// 点乘f
	// 通过取模使纹理可平铺
	float f0 = dot(d0, normal[i * n + j]);
	float f1 = dot(d1, normal[i * n + ((j + 1) % n)]);
	float f2 = dot(d2, normal[((i + m - 1) % m) * n + ((j + 1) % n)]);
	float f3 = dot(d3, normal[((i + m - 1) % m) * n + j]);

	// 平滑，使整点处的导数为0
	rx = fade(rx);
	ry = fade(ry);

	float x_low = (1.f - rx) * f3 + rx * f2;
	float x_high = (1.f - rx) * f0 + rx * f1;
	float f = (1.f - ry) * x_low + ry * x_high;

	// 木纹效果的后处理
	f = (f * k - floor(f * k));

	// 棕色值 0xA47449
	float R = f * 0xA4 / 0xff;
	float G = f * 0x74 / 0xff;
	float B = f * 0x49 / 0xff;

	return Vector3f(R, G, B);
}

REGISTER_CLASS(WoodTexture, "woodTex")
