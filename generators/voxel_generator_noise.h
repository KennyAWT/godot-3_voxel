#ifndef VOXEL_GENERATOR_NOISE_H
#define VOXEL_GENERATOR_NOISE_H

#include "../util/float_buffer_3d.h"
#include "voxel_generator.h"
#include "modules/noise/noise.h"

class VoxelGeneratorNoise : public VoxelGenerator {
	GDCLASS(VoxelGeneratorNoise, VoxelGenerator)

public:
    enum BiasMode {
        Linear,
        Bounded,
        None
    };

	void set_channel(VoxelBuffer::ChannelId channel);
	VoxelBuffer::ChannelId get_channel() const;
	int get_used_channels_mask() const override;

    void set_bias_mode(BiasMode mode);
    BiasMode get_bias_mode() const;

	void set_height_start(real_t y);
	real_t get_height_start() const;

	void set_height_range(real_t hrange);
	real_t get_height_range() const;

    void set_iso_scale(float scale);
    float get_iso_scale() const;

    void set_invert(bool invert);
    bool get_invert() const;

    void set_noise_offset(float noise_offset);
    float get_noise_offset() const;

	void set_noise(Ref<Noise> noise);
	Ref<Noise> get_noise() const;

	void generate_block(VoxelBlockRequest &input) override;

protected:
	static void _bind_methods();

private:
	VoxelBuffer::ChannelId _channel = VoxelBuffer::CHANNEL_SDF;
	Ref<Noise> _noise;
//	FloatBuffer3D _noise_buffer;
	bool _invert = false;
    BiasMode _bias_mode = Linear;
    float _iso_scale = 20.f;
    float _noise_offset = 0.f;
    float _height_start = 0.f;
	float _height_range = 300.f;
};

VARIANT_ENUM_CAST(VoxelGeneratorNoise::BiasMode)

#endif // VOXEL_GENERATOR_NOISE_H

