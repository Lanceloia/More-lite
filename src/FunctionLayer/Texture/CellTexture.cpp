#include "CellTexture.h"
#include "ResourceLayer/Factory.h"

CellTexture::CellTexture(const Json& json) : Texture <Spectrum >() {
	// ���� m x n ������(����Ϊ1024��)�������������һȦ���ܷ���
	// ע��˴�Ϊ����
	m = n = 16;		// m �� n ��
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			// �������ĵ�����
			p[i * n + j] = Vector2f(-0.5f + i, -0.5f + j);
			
			// ����һ��[0, 1)������������� 2Pi
			float theta = float(rand()) / RAND_MAX * 2 * PI;
			float rho = 0.1f;
			b[i * n + j] = Vector2f(rho * fm::cos(theta), rho * fm::sin(theta));
		}
	}
}

Spectrum CellTexture::evaluate(const Intersection& intersection) const {
	// your code here ...
	TextureCoord texCoord = mapping->map(intersection);
	return evaluate(texCoord);
}

Spectrum CellTexture::evaluate(const TextureCoord& texCoord) const {
	// ��ȡ���������
	float u = texCoord.coord[0];
	float v = texCoord.coord[1];

	assert(u < 1.f);
	assert(v < 1.f);

	// ����u,v�����ĸ������
	// i�к�j�к�
	int i = floor(v * (m - 2)) + 1;
	int j = floor(u * (n - 2)) + 1;

	// ������ӵ����
	// ���Ͻ�Ϊ0, ˳ʱ����ת����Ϊ1, 2, 3, ..., 7
	int ln = i * n + j;
	int l[8] = { 
		(i - 1) * n + (j - 1),
		(i - 1) * n + (j    ),
		(i - 1) * n + (j + 1),
		
		(i    ) * n + (j + 1),

		(i + 1) * n + (j + 1),
		(i + 1) * n + (j    ),
		(i + 1) * n + (j - 1),

		(i    ) * n + (j - 1)
	};

	// �����꣬������
	float px = v * (m - 2);
	float py = u * (n - 2);

	float bn_x = p[ln][0] + b[ln][0];
	float bn_y = p[ln][1] + b[ln][1];
	
	// ��С�����d_point
	float d_point = Vector2f(px - bn_x, py - bn_y).len();
	// ��С�߾���d_edge
	float d_edge = INFINITY;

	for (int k = 0; k < 8; k++) {
		float bk_x = p[l[k]][0] + b[l[k]][0];
		float bk_y = p[l[k]][1] + b[l[k]][1];

		float bk1_x = p[l[(k + 1) % 8]][0] + b[l[(k + 1) % 8]][0];
		float bk1_y = p[l[(k + 1) % 8]][1] + b[l[(k + 1) % 8]][1];

		// d_edge_k = d_point_k * sin<V_lk_p, V_lk_lk1>
		Vector2f v1 = Vector2f(px - bk_x, py - bk_y);
		Vector2f v2 = Vector2f(bk1_x - bk_x, bk1_y - bk_y);

		float cos_theta = dot(v1, v2) / (v1.len(), v2.len());
		float sin_theta = sqrt(1 - cos_theta * cos_theta);
		
		float d_point_k = v1.len();
		float d_edge_k = sin_theta * d_point_k;

		if (d_point_k < d_point)
			d_point = d_point_k;
		if (d_edge_k < d_edge)
			d_edge = d_edge_k;
	}

	// printf("%f, %f\n", d_point, d_edge);
	float f = clamp(d_edge, 0.f, 1.f);
	return Vector3f(f, f, f);
}

REGISTER_CLASS(CellTexture, "cellTex")
