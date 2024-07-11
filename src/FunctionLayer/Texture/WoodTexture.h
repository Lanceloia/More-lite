// (newly created) WoodTexture.h

#include "Mipmap.h"
#include "Texture.h"
#include <CoreLayer/ColorSpace/Spectrum.h>
class WoodTexture : public Texture <Spectrum > {
public:
	WoodTexture() = delete;
	WoodTexture(const Json& json);

	virtual Spectrum evaluate(const Intersection& intersection) const override;

	virtual Spectrum evaluate(const TextureCoord& texCoord) const override;
private:
	// necessary data
    // your code here ...

#define MAXNUM 256

	int m, n, k;
	Vector2f normal[MAXNUM];
};
