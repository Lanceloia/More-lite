#include "WoodTexture.h"
#include "ResourceLayer/Factory.h"

WoodTexture::WoodTexture(const Json& json) : Texture <Spectrum >() {
	// ���� m x n �����(����Ϊ256��)��ע��˴�Ϊ���
	// ���˹��࣬����ľ�ƻ������ڸ����
	m = n = 6;		// m �� n ��
	
	// k��ʾľ�Ƶ��ܼ��̶�
	k = 5;
	
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			// ����һ��[0, 1)������������� 2Pi
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
	// ƽ���������Բ�ֵϵ������ƽ��
	// ����ʹ�����ʹľ�Ʊ�ɷ���
	auto fade = [](float x) -> float {
		return 3 * x * x - 2 * x * x * x;
	};

	// ��ȡ���������
	float u = texCoord.coord[0];
	float v = texCoord.coord[1];

	assert(u < 1.f);
	assert(v < 1.f);

	// ��������(px, py)
	float px = u * n;
	float py = v * m;


	// ����(px, py)���ĸ������
	// i�к�j�к�
	int i = floor(py);
	int j = floor(px);

	// �ڽ�������ţ����½�Ϊ0, ��ʱ����ת����Ϊ1, 2, 3 

	// ��ֵ����r
	float rx = px - j;
	float ry = py - i;

	// ����d
	Vector2f d0 = Vector2f(rx,  ry);
	Vector2f d1 = Vector2f(rx - 1, ry);
	Vector2f d2 = Vector2f(rx - 1, ry + 1);
	Vector2f d3 = Vector2f(rx,  ry + 1);

	// ���f
	// ͨ��ȡģʹ�����ƽ��
	float f0 = dot(d0, normal[i * n + j]);
	float f1 = dot(d1, normal[i * n + ((j + 1) % n)]);
	float f2 = dot(d2, normal[((i + m - 1) % m) * n + ((j + 1) % n)]);
	float f3 = dot(d3, normal[((i + m - 1) % m) * n + j]);

	// ƽ����ʹ���㴦�ĵ���Ϊ0
	rx = fade(rx);
	ry = fade(ry);

	float x_low = (1.f - rx) * f3 + rx * f2;
	float x_high = (1.f - rx) * f0 + rx * f1;
	float f = (1.f - ry) * x_low + ry * x_high;

	// ľ��Ч���ĺ���
	f = (f * k - floor(f * k));

	// ��ɫֵ 0xA47449
	float R = f * 0xA4 / 0xff;
	float G = f * 0x74 / 0xff;
	float B = f * 0x49 / 0xff;

	return Vector3f(R, G, B);
}

REGISTER_CLASS(WoodTexture, "woodTex")
