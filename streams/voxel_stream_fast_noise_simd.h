#ifndef VOXEL_STREAM_FAST_NOISE_SIMD_H
#define VOXEL_STREAM_FAST_NOISE_SIMD_H

#include "../util/float_buffer_3d.h"
#include "voxel_stream.h"
#include "modules/noise/fast_noise_simd.h"

class VoxelStreamFastNoiseSIMD : public VoxelStream {
	GDCLASS(VoxelStreamFastNoiseSIMD, VoxelStream)

public:

	void set_channel(VoxelBuffer::ChannelId channel);
	VoxelBuffer::ChannelId get_channel() const;

	void set_noise(Ref<FastNoiseSIMD> noise);
	Ref<FastNoiseSIMD> get_noise() const;

	void set_height_start(real_t y);
	real_t get_height_start() const;

	void set_height_range(real_t hrange);
	real_t get_height_range() const;

	void emerge_block(Ref<VoxelBuffer> out_buffer, Vector3i origin_in_voxels, int lod);

protected:
	static void _bind_methods();

private:
	VoxelBuffer::ChannelId _channel = VoxelBuffer::CHANNEL_TYPE;
	Ref<FastNoiseSIMD> _noise;
	//FloatBuffer3D _noise_buffer;
	float _height_start = 0;
	float _height_range = 300;

};

#endif // VOXEL_STREAM_NOISE_PLUS_H

