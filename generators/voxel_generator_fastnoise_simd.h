#ifndef VOXEL_GENERATOR_FASTNOISE_SIMD_H
#define VOXEL_GENERATOR_FASTNOISE_SIMD_H

#ifdef FASTNOISESIMD_ENABLED

#include <modules/fastnoise_simd/fastnoise_simd.h>

#include "../util/float_buffer_3d.h"
#include "modules/voxel/generators/voxel_generator.h"

class VoxelGeneratorFastNoiseSIMD : public VoxelGenerator {
	GDCLASS(VoxelGeneratorFastNoiseSIMD, VoxelGenerator)

public:

	enum BiasMode {
		Linear,
		Bounded,
		None
	};

	VoxelGeneratorFastNoiseSIMD();
	~VoxelGeneratorFastNoiseSIMD();

	void set_channel(VoxelBuffer::ChannelId channel);
	VoxelBuffer::ChannelId get_channel() const;
	int get_used_channels_mask() const override;

	void set_bias_mode(BiasMode mode);
	BiasMode get_bias_mode() const;

	void set_height_start(int y);
	int get_height_start() const;

	void set_height_range(int hrange);
	int get_height_range() const;

	void set_iso_scale(float scale);
	float get_iso_scale() const;

	void set_invert(bool invert);
	bool get_invert() const;

	void set_noise_offset(float noise_offset);
	float get_noise_offset() const;

	void set_noise(Ref<FastNoiseSIMD> noise);
	Ref<FastNoiseSIMD> get_noise() const;

	void generate_block(VoxelBlockRequest& input) override;

protected:
	static void _bind_methods();

private:
	VoxelBuffer::ChannelId _channel = VoxelBuffer::CHANNEL_SDF;
	Ref<FastNoiseSIMD> _noise;
	float* _noise_buffer = nullptr;
	bool _invert = false;
	BiasMode _bias_mode = Linear;
	float _iso_scale = 20.f;
	float _noise_offset = 0.f;
	int _height_start = 0;
	int _height_range = 512;

};

VARIANT_ENUM_CAST(VoxelGeneratorFastNoiseSIMD::BiasMode)

#endif // FASTNOISESIMD_ENABLED

#endif // VOXEL_GENERATOR_FASTNOISE_SIMD_H
