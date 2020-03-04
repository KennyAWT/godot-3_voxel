#ifndef VOXEL_GENERATOR_FASTNOISE_SIMD_H
#define VOXEL_GENERATOR_FASTNOISE_SIMD_H

#include "../util/float_buffer_3d.h"
#include "voxel_generator.h"
#include "modules/fastnoise_simd/fastnoise_simd.h"

class VoxelGeneratorFastNoiseSIMD : public VoxelGenerator {
	GDCLASS(VoxelGeneratorFastNoiseSIMD, VoxelGenerator)

public:

	VoxelGeneratorFastNoiseSIMD();
	~VoxelGeneratorFastNoiseSIMD();

	void set_channel(VoxelBuffer::ChannelId channel);
	VoxelBuffer::ChannelId get_channel() const;
	int get_used_channels_mask() const override;

	void set_noise(Ref<FastNoiseSIMD> noise);
	Ref<FastNoiseSIMD> get_noise() const;

	void set_inverse(bool inverse);
	bool get_inverse() const;

	void set_adjustment(float adjustment);
	float get_adjustment() const;

	void set_height_start(int y);
	int get_height_start() const;

	void set_height_range(int hrange);
	int get_height_range() const;

	void generate_block(VoxelBlockRequest& input) override;

protected:
	static void _bind_methods();

private:
	VoxelBuffer::ChannelId _channel = VoxelBuffer::CHANNEL_SDF;
	Ref<FastNoiseSIMD> _noise;
	float* _noise_buffer = nullptr;
	bool _inverse = false;
	float _adjustment = 0;
	int _height_start = 0;
	int _height_range = 512;

};

#endif // VOXEL_GENERATOR_FASTNOISE_SIMD_H

