#include "voxel_generator_fastnoise_simd.h"

VoxelGeneratorFastNoiseSIMD::VoxelGeneratorFastNoiseSIMD() {
}

VoxelGeneratorFastNoiseSIMD::~VoxelGeneratorFastNoiseSIMD() {
	if (_noise_buffer != nullptr) {
		_noise->free_noise_set(_noise_buffer);
	}
}

void VoxelGeneratorFastNoiseSIMD::set_channel(VoxelBuffer::ChannelId channel) {
	ERR_FAIL_INDEX(channel, VoxelBuffer::MAX_CHANNELS);
	if (_channel != channel) {
		_channel = channel;
		emit_changed();
	}
}

VoxelBuffer::ChannelId VoxelGeneratorFastNoiseSIMD::get_channel() const {
	return _channel;
}

int VoxelGeneratorFastNoiseSIMD::get_used_channels_mask() const {
	return (1 << _channel);
}

void VoxelGeneratorFastNoiseSIMD::set_noise(Ref<FastNoiseSIMD> noise) {
	_noise = noise;
}

Ref<FastNoiseSIMD> VoxelGeneratorFastNoiseSIMD::get_noise() const {
	return _noise;
}

void VoxelGeneratorFastNoiseSIMD::set_inverse(bool inverse){
	_inverse = inverse;
}

bool VoxelGeneratorFastNoiseSIMD::get_inverse() const {
	return _inverse;
}

void VoxelGeneratorFastNoiseSIMD::set_adjustment(float adjustment){
	_adjustment = adjustment;
}

float VoxelGeneratorFastNoiseSIMD::get_adjustment() const {
	return _adjustment;
}

void VoxelGeneratorFastNoiseSIMD::set_height_start(int y) {
	_height_start = y;
}

int VoxelGeneratorFastNoiseSIMD::get_height_start() const {
	return _height_start;
}

void VoxelGeneratorFastNoiseSIMD::set_height_range(int hrange) {
	_height_range = hrange;
}

int VoxelGeneratorFastNoiseSIMD::get_height_range() const {
	return _height_range;
}

void VoxelGeneratorFastNoiseSIMD::generate_block(VoxelBlockRequest& input) {
	ERR_FAIL_COND(input.voxel_buffer.is_null());
	ERR_FAIL_COND(_noise.is_null());

	VoxelBuffer& buffer = **input.voxel_buffer;
	Vector3i origin_in_voxels = input.origin_in_voxels;
	FastNoiseSIMD& noise = **_noise;
	int lod = input.lod;
	Vector3i size = buffer.get_size();

	// Allocate noise buffer if not present. Deleted in destructor.
	if (_noise_buffer == nullptr) {
		_noise_buffer = _noise->get_empty_set(size.x * size.y * size.z);
	}

	const int air_type = 0;
	const int matter_type = 1;

	if (origin_in_voxels.y >= _height_start + _height_range) {

		// Fill with air
		if (_channel == VoxelBuffer::CHANNEL_SDF) {
			buffer.clear_channel_f(_channel, 100.0);
		}
		else if (_channel == VoxelBuffer::CHANNEL_TYPE) {
			buffer.clear_channel(_channel, air_type);
		}

	} else if (origin_in_voxels.y + (buffer.get_size().y << lod) < _height_start) {

		// Fill with matter
		if (_channel == VoxelBuffer::CHANNEL_SDF) {
			buffer.clear_channel_f(_channel, -100.0);
		}
		else if (_channel == VoxelBuffer::CHANNEL_TYPE) {
			buffer.clear_channel(_channel, matter_type);
		}

	} else {

		const float height_range_inv = 1.f / _height_range;
		//float iso_scale = 20.0 / (lod+1);	// Todo: This creates small slits between chunks
		float iso_scale = 20.0 / ((lod > 3) ? 2 : 1);
		float d = 0.0f;

		noise.fill_noise_set_3d(_noise_buffer, origin_in_voxels.x>>lod, origin_in_voxels.y>>lod, origin_in_voxels.z>>lod, size.x, size.y, size.z, 1<<lod);

		for (int x = 0, i=0; x < size.x; x++) {

			for (int y = 0;	y < size.y;	y++)   {
				int ly = origin_in_voxels.y + (y << lod);

				for (int z = 0; z < size.z; z++, i++) {

					//float n = noise_set[x * size.y * size.z + y * size.z + z];
					//float n = noise_set[i];
					float n = _noise_buffer[i];

					// Bias is what makes noise become "matter" the lower we go, and "air" the higher we go
					float t = (ly - _height_start) * height_range_inv;
					float bias = 2.0 * t - 1.0;

					if(_inverse)
						d = (_adjustment-n + bias) * iso_scale;
					else
						d = (n + _adjustment + bias) * iso_scale;

					// Set voxel
					if (_channel == VoxelBuffer::CHANNEL_SDF) {
						buffer.set_voxel_f(d, x, y, z, _channel);
					}
					else if (_channel == VoxelBuffer::CHANNEL_TYPE && d < 0) {
						buffer.set_voxel(matter_type, x, y, z, _channel);
					}
				}
			}
		}

	}
}

void VoxelGeneratorFastNoiseSIMD::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_channel", "channel"), &VoxelGeneratorFastNoiseSIMD::set_channel);
	ClassDB::bind_method(D_METHOD("get_channel"), &VoxelGeneratorFastNoiseSIMD::get_channel);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "channel", PROPERTY_HINT_ENUM, VoxelBuffer::CHANNEL_ID_HINT_STRING), "set_channel", "get_channel");

	ClassDB::bind_method(D_METHOD("set_inverse", "inverse"), &VoxelGeneratorFastNoiseSIMD::set_inverse);
	ClassDB::bind_method(D_METHOD("get_inverse"), &VoxelGeneratorFastNoiseSIMD::get_inverse);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "inverse"), "set_inverse", "get_inverse");

	ClassDB::bind_method(D_METHOD("set_adjustment", "adjustment"), &VoxelGeneratorFastNoiseSIMD::set_adjustment);
	ClassDB::bind_method(D_METHOD("get_adjustment"), &VoxelGeneratorFastNoiseSIMD::get_adjustment);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "adjustment", PROPERTY_HINT_RANGE, "-2,2,0.01"), "set_adjustment", "get_adjustment");

	ClassDB::bind_method(D_METHOD("set_noise", "noise"), &VoxelGeneratorFastNoiseSIMD::set_noise);
	ClassDB::bind_method(D_METHOD("get_noise"), &VoxelGeneratorFastNoiseSIMD::get_noise);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "noise", PROPERTY_HINT_RESOURCE_TYPE, "FastNoiseSIMD"), "set_noise", "get_noise");

	ClassDB::bind_method(D_METHOD("set_height_start", "hstart"), &VoxelGeneratorFastNoiseSIMD::set_height_start);
	ClassDB::bind_method(D_METHOD("get_height_start"), &VoxelGeneratorFastNoiseSIMD::get_height_start);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "height_start"), "set_height_start", "get_height_start");

	ClassDB::bind_method(D_METHOD("set_height_range", "hrange"), &VoxelGeneratorFastNoiseSIMD::set_height_range);
	ClassDB::bind_method(D_METHOD("get_height_range"), &VoxelGeneratorFastNoiseSIMD::get_height_range);
	// 512*64=32,768. Signed int max value 32,767. So range is 512*63. 
	ADD_PROPERTY(PropertyInfo(Variant::INT, "height_range", PROPERTY_HINT_RANGE, "512,32256,512"), "set_height_range", "get_height_range");

}
