#include "NormalIntegtator.h"
#include <FunctionLayer/Texture/ImageTexture.h>

ImageTexture tex = ImageTexture(Json());

Spectrum NormalIntegrator::li(const Ray &ray, const Scene &scene,
                              std::shared_ptr<Sampler> sampler) const {
  auto intersectionOpt = scene.rayIntersect(ray);
  if (!intersectionOpt.has_value())
    return Spectrum(.0f);
  return Spectrum((intersectionOpt.value().normal + Vector3f(1.f)) * .5f);
}
REGISTER_CLASS(NormalIntegrator, "normal")