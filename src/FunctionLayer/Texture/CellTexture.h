// (newly created) WoodTexture.h

#include "Mipmap.h"
#include "Texture.h"
#include <CoreLayer/ColorSpace/Spectrum.h>
class CellTexture : public Texture <Spectrum > {
public:
	CellTexture() = delete;
	CellTexture(const Json& json);

	virtual Spectrum evaluate(const Intersection& intersection) const override;

	virtual Spectrum evaluate(const TextureCoord& texCoord) const override;
private:
	// necessary data
    // your code here ...
#define MAXNUM 4096

	int m, n;
	Vector2f p[MAXNUM];
	Vector2f b[MAXNUM];
};
