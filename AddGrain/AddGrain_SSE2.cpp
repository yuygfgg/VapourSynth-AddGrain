#if ADDGRAIN_X86|| defined(__arm__) || defined(__aarch64__)
#include "AddGrain.h"

template<typename pixel_t, typename noise_t>
void updateFrame_sse2(const void* _srcp, void* _dstp, const int width, const int height, const ptrdiff_t stride, const int noisePlane, const int noiseOffs,
                      const AddGrainData* const VS_RESTRICT d) noexcept {
    auto srcp{ reinterpret_cast<const pixel_t*>(_srcp) };
    auto dstp{ reinterpret_cast<pixel_t*>(_dstp) };
    auto pNW{ reinterpret_cast<noise_t*>(d->pN[noisePlane]) + noiseOffs };

    for (auto y{ 0 }; y < height; y++) {
        for (auto x{ 0 }; x < width; x += d->step) {
            if constexpr (std::is_same_v<pixel_t, uint8_t>) {
                Vec16c sign{ -0x80 };
                auto val{ Vec16c().load_a(srcp + x) };
                auto nz{ Vec16c().load(pNW + x) };
                val ^= sign;
                val = add_saturated(val, nz);
                val ^= sign;
                val.store_nt(dstp + x);
            } else if constexpr (std::is_same_v<pixel_t, uint16_t>) {
                Vec8s sign{ -0x8000 };
                auto val{ Vec8s().load_a(srcp + x) };
                auto nz{ Vec8s().load(pNW + x) };
                val ^= sign;
                val = add_saturated(val, nz);
                val ^= sign;
                min(Vec8us(val), d->peak).store_nt(dstp + x);
            } else {
                auto val{ Vec4f().load_a(srcp + x) };
                auto nz{ Vec4f().load(pNW + x) };
                (val + nz).store_nt(dstp + x);
            }
        }

        srcp += stride;
        dstp += stride;
        pNW += d->nStride[noisePlane];
    }
}

template void updateFrame_sse2<uint8_t, int8_t>(const void* _srcp, void* _dstp, const int width, const int height, const ptrdiff_t stride, const int noisePlane, const int noiseOffs, const AddGrainData* const VS_RESTRICT d) noexcept;
template void updateFrame_sse2<uint16_t, int16_t>(const void* _srcp, void* _dstp, const int width, const int height, const ptrdiff_t stride, const int noisePlane, const int noiseOffs, const AddGrainData* const VS_RESTRICT d) noexcept;
template void updateFrame_sse2<float, float>(const void* _srcp, void* _dstp, const int width, const int height, const ptrdiff_t stride, const int noisePlane, const int noiseOffs, const AddGrainData* const VS_RESTRICT d) noexcept;
#endif
