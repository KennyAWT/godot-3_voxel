#include <iostream>
#include "voxel_stream_fast_noise_simd.h"

void VoxelStreamFastNoiseSIMD::set_channel(VoxelBuffer::ChannelId channel) {
	ERR_FAIL_INDEX(channel, VoxelBuffer::MAX_CHANNELS);
	_channel = channel;
}

VoxelBuffer::ChannelId VoxelStreamFastNoiseSIMD::get_channel() const {
	return _channel;
}

void VoxelStreamFastNoiseSIMD::set_noise(Ref<FastNoiseSIMD> noise) {
	_noise = noise;
}

Ref<FastNoiseSIMD> VoxelStreamFastNoiseSIMD::get_noise() const {
	return _noise;
}

void VoxelStreamFastNoiseSIMD::set_inverse(bool inverse){
	_inverse = inverse;
}

bool VoxelStreamFastNoiseSIMD::get_inverse() const {
	return _inverse;
}

void VoxelStreamFastNoiseSIMD::set_adjustment(float adjustment){
	_adjustment = adjustment;
}

float VoxelStreamFastNoiseSIMD::get_adjustment() const {
	return _adjustment;
}

void VoxelStreamFastNoiseSIMD::set_height_start(real_t y) {
	_height_start = y;
}

real_t VoxelStreamFastNoiseSIMD::get_height_start() const {
	return _height_start;
}

void VoxelStreamFastNoiseSIMD::set_height_range(real_t hrange) {
	_height_range = hrange;
}

real_t VoxelStreamFastNoiseSIMD::get_height_range() const {
	return _height_range;
}

void VoxelStreamFastNoiseSIMD::emerge_block(Ref<VoxelBuffer> out_buffer, Vector3i origin_in_voxels, int lod) {

	ERR_FAIL_COND(out_buffer.is_null());
	//ERR_FAIL_COND(_noise.is_null());
	//ERR_FAIL_COND(_fnoise.is_null());

	//OpenSimplexNoise& noise = **_noise;
	FastNoiseSIMD& noise = **_noise;
	VoxelBuffer& buffer = **out_buffer;

/*	if (origin_in_voxels.y > _height_start + _height_range) {

		if (_channel == VoxelBuffer::CHANNEL_SDF) {
			buffer.clear_channel_f(_channel, 100.0);
		}
		else if (_channel == VoxelBuffer::CHANNEL_TYPE) {
			buffer.clear_channel(_channel, 0);
		}

	}
	else if (origin_in_voxels.y + (buffer.get_size().y << lod) < _height_start) {

		if (_channel == VoxelBuffer::CHANNEL_SDF) {
			buffer.clear_channel_f(_channel, -100.0);
		}
		else if (_channel == VoxelBuffer::CHANNEL_TYPE) {
			buffer.clear_channel(_channel, 1);
		}

	}
	else*/ {

		Vector3i size = buffer.get_size();
		float iso_scale = 1.0;// 6.4;
		float d = 0.0f;
		float* noise_set = noise.get_noise_set_3d(origin_in_voxels.x>>lod, origin_in_voxels.y>>lod, origin_in_voxels.z>>lod, size.x, size.y, size.z, 1<<lod);
		static long counter = 0;

		for (int x = 0, i=0; x < size.x; x++) {
			for (int y = 0;	y < size.y;	y++)   {
				for (int z = 0; z < size.z; z++, i++) {

					//float n = noise_set[x * size.y * size.z + y * size.z + z];
					float n = noise_set[i];

					// Apply height bias
					//float t = (ly - _height_start) / _height_range;
					float bias = 0;// 2.0 * t - 1.0;
					if(_inverse)
						d = (_adjustment-n + bias) * iso_scale;
					else
						d = (n + _adjustment + bias) * iso_scale;

					if(counter++ %100000 == 0)
						printf("Noise: %.3f\n", d);

					// Set voxel
					if (_channel == VoxelBuffer::CHANNEL_SDF) {
						buffer.set_voxel_f(d, x, y, z, _channel);
					}
					else if (_channel == VoxelBuffer::CHANNEL_TYPE && d < 0) {
						buffer.set_voxel(1, x, y, z, _channel);
					}
				}
			}
		}

		noise.free_noise_set(noise_set);

	}
}

void VoxelStreamFastNoiseSIMD::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_channel", "channel"), &VoxelStreamFastNoiseSIMD::set_channel);
	ClassDB::bind_method(D_METHOD("get_channel"), &VoxelStreamFastNoiseSIMD::get_channel);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "channel", PROPERTY_HINT_ENUM, VoxelBuffer::CHANNEL_ID_HINT_STRING), "set_channel", "get_channel");

	ClassDB::bind_method(D_METHOD("set_inverse", "inverse"), &VoxelStreamFastNoiseSIMD::set_inverse);
	ClassDB::bind_method(D_METHOD("get_inverse"), &VoxelStreamFastNoiseSIMD::get_inverse);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "inverse"), "set_inverse", "get_inverse");

	ClassDB::bind_method(D_METHOD("set_adjustment", "adjustment"), &VoxelStreamFastNoiseSIMD::set_adjustment);
	ClassDB::bind_method(D_METHOD("get_adjustment"), &VoxelStreamFastNoiseSIMD::get_adjustment);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "adjustment", PROPERTY_HINT_RANGE, "-2,2,0.01"), "set_adjustment", "get_adjustment");

	ClassDB::bind_method(D_METHOD("set_noise", "noise"), &VoxelStreamFastNoiseSIMD::set_noise);
	ClassDB::bind_method(D_METHOD("get_noise"), &VoxelStreamFastNoiseSIMD::get_noise);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "noise", PROPERTY_HINT_RESOURCE_TYPE, "FastNoiseSIMD"), "set_noise", "get_noise");

	ClassDB::bind_method(D_METHOD("set_height_start", "hstart"), &VoxelStreamFastNoiseSIMD::set_height_start);
	ClassDB::bind_method(D_METHOD("get_height_start"), &VoxelStreamFastNoiseSIMD::get_height_start);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "height_start"), "set_height_start", "get_height_start");

	ClassDB::bind_method(D_METHOD("set_height_range", "hrange"), &VoxelStreamFastNoiseSIMD::set_height_range);
	ClassDB::bind_method(D_METHOD("get_height_range"), &VoxelStreamFastNoiseSIMD::get_height_range);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "height_range"), "set_height_range", "get_height_range");

}
