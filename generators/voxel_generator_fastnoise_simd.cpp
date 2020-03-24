#include <core/os/os.h>
#include "voxel_generator_fastnoise_simd.h"

#ifdef FASTNOISESIMD_ENABLED

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

void VoxelGeneratorFastNoiseSIMD::set_invert(bool invert){
	_invert = invert;
}

bool VoxelGeneratorFastNoiseSIMD::get_invert() const {
	return _invert;
}

void VoxelGeneratorFastNoiseSIMD::set_bias_mode(VoxelGeneratorFastNoiseSIMD::BiasMode mode){
	_bias_mode = mode;
}

VoxelGeneratorFastNoiseSIMD::BiasMode VoxelGeneratorFastNoiseSIMD::get_bias_mode() const {
	return _bias_mode;
}

void VoxelGeneratorFastNoiseSIMD::set_iso_scale(float scale){
	_iso_scale = scale;
}

float VoxelGeneratorFastNoiseSIMD::get_iso_scale() const {
	return _iso_scale;
}

void VoxelGeneratorFastNoiseSIMD::set_value_adjustment(float value_adjustment){
	_value_adjustment = value_adjustment;
}

float VoxelGeneratorFastNoiseSIMD::get_value_adjustment() const {
	return _value_adjustment;
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
	//uint64_t time = OS::get_singleton()->get_ticks_usec();
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

	if (_bias_mode!=None &&  origin_in_voxels.y >= _height_start + _height_range) {
		// Fill with air
		if (_channel == VoxelBuffer::CHANNEL_SDF) {
			buffer.clear_channel_f(_channel, 100.0);
		}
		else if (_channel == VoxelBuffer::CHANNEL_TYPE) {
			buffer.clear_channel(_channel, air_type);
		}
	} else if (_bias_mode!=None && origin_in_voxels.y + (size.y << lod) < _height_start ) {
		// Fill with matter
		if (_channel == VoxelBuffer::CHANNEL_SDF) {
			buffer.clear_channel_f(_channel, -100.0);
		}
		else if (_channel == VoxelBuffer::CHANNEL_TYPE) {
			buffer.clear_channel(_channel, matter_type);
		}
	} else {

		noise.fill_noise_set_3d(_noise_buffer, origin_in_voxels.x>>lod, origin_in_voxels.y>>lod, origin_in_voxels.z>>lod, size.x, size.y, size.z, 1<<lod);
		/*
		ArraySlice<uint8_t> data;
		bool uniform = buffer.is_uniform(_channel);
		bool raw = buffer.get_channel_raw(_channel, data);
		printf("Buffer (%d,%d,%d): uniform: %d, raw: %d, depth: %d, chsize: %zd\n",
			origin_in_voxels.x, origin_in_voxels.y, origin_in_voxels.z,
			uniform, raw,
			buffer.get_channel_depth(_channel), data.size()
		);

		if (uniform || !raw) {
			return;
		}
		*/

		// lod 0-3 100
		// lod 4 40
		// lod 5 3040
		// lod 6 20
		// lod 7 10
		// lod 8 5

		// 6+ 1.0f
		// 5
		// 4
		// 3
		// 2
		// 1
		// 0 100.f

		//float iso_scale = 40.0 / (lod+1);	// This tremendously smooths out the "popping" effect when switching LODs, but creates small slits between some chunks
		//float iso_scale = ((lod > 3) ? 10 : 40);

		//float iso_scale = clamp(50.f - (lod-3)*10.f, 10.f, 50.f);
		//float iso_scale = clamp(40.f - (lod-4)*10.f, 5.f, 40.f);
		//float iso_scale = ((lod < 3) ? 100.f : 100.f/(lod-1));

		// float iso_scale;
		// switch(lod) {
		// 	case 0:		
		// 	case 1:		
		// 	case 2:
		// 	case 3:		iso_scale=24.f; break;
		// 	case 4:		iso_scale=16.f; break;
		// 	case 5: 	iso_scale=10.f; break;
		// 	default:	iso_scale=4.0f; break;
		// 	// case 0:		iso_scale=24.f; break;
		// 	// case 1:		iso_scale=16.f; break;
		// 	// case 2: 	iso_scale=10.f; break;
		// 	// case 3: 	iso_scale=8.f; break;
		// 	// case 0:		iso_scale=8.f; break;
		// 	// case 1: 	iso_scale=7.f; break;
		// 	// case 2: 	iso_scale=6.f; break;
		// 	// case 3: 	iso_scale=5.f; break;
		// 	default:	iso_scale=4.0f; break;
		// }
		// float iso_scale = ((lod < 4) ? 100.f : 100/lod); 
		const float height_range_inv = 1.f / _height_range;
		float d = 0.0f;
		float bias=0.f;

		for (int x = 0, i=0; x < size.x; x++) {

			for (int y = 0;	y < size.y;	y++)   {
				int ly = origin_in_voxels.y + (y << lod);

				for (int z = 0; z < size.z; z++, i++) {

					//float n = noise_set[x * size.y * size.z + y * size.z + z];
					//float n = noise_set[i];
					float n = _noise_buffer[i];

					// Bias is what makes noise become "matter" the lower we go, and "air" the higher we go
					float t = (ly - _height_start) * height_range_inv;
					if(_bias_mode==Linear) {
						bias = 2.f * t - 1.f;
					} else if (_bias_mode==Bounded) {
						//bias = step(.1f,t) + step(.9f,t) - 1.f;
						bias = Math::smoothstep(0.f, .1f, t) + Math::smoothstep(.9f, 1.f, t)-1.f;
					} else { 
						bias = 0.f;
					}

					if(_invert)
						d = (_value_adjustment-n + bias) * _iso_scale;
					else
						d = (n + _value_adjustment + bias) * _iso_scale;

					// Set voxel
					if (_channel == VoxelBuffer::CHANNEL_SDF) {
						buffer.set_voxel_f(d, x, y, z, _channel);

						/*
						//buffer.set_voxel(real_to_raw_voxel(value, _channels[channel_index].depth), x, y, z, channel_index);
						int val = clamp(static_cast<int>(128.f * d + 128.f), 0, 0xff);
						data[i] = val;
						//return y + _size.y * (x + _size.x * z);
						*/
					}
					else if (_channel == VoxelBuffer::CHANNEL_TYPE && d < 0) {
						buffer.set_voxel(matter_type, x, y, z, _channel);
					}
				}
			}
		}
	}
	//printf("VGFNS: Noise generation time: %ld us\n", OS::get_singleton()->get_ticks_usec()-time);
}

void VoxelGeneratorFastNoiseSIMD::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_channel", "channel"), &VoxelGeneratorFastNoiseSIMD::set_channel);
	ClassDB::bind_method(D_METHOD("get_channel"), &VoxelGeneratorFastNoiseSIMD::get_channel);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "channel", PROPERTY_HINT_ENUM, VoxelBuffer::CHANNEL_ID_HINT_STRING), "set_channel", "get_channel");


	ClassDB::bind_method(D_METHOD("set_invert", "invert"), &VoxelGeneratorFastNoiseSIMD::set_invert);
	ClassDB::bind_method(D_METHOD("get_invert"), &VoxelGeneratorFastNoiseSIMD::get_invert);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "invert"), "set_invert", "get_invert");

	ClassDB::bind_method(D_METHOD("set_bias_mode", "mode"), &VoxelGeneratorFastNoiseSIMD::set_bias_mode);
	ClassDB::bind_method(D_METHOD("get_bias_mode"), &VoxelGeneratorFastNoiseSIMD::get_bias_mode);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "bias_mode", PROPERTY_HINT_ENUM, "Linear,Bounded,None"), "set_bias_mode", "get_bias_mode");

	ClassDB::bind_method(D_METHOD("set_iso_scale", "scale"), &VoxelGeneratorFastNoiseSIMD::set_iso_scale);
	ClassDB::bind_method(D_METHOD("get_iso_scale"), &VoxelGeneratorFastNoiseSIMD::get_iso_scale);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "iso_scale", PROPERTY_HINT_RANGE, ".1,100,0.1"), "set_iso_scale", "get_iso_scale");

	ClassDB::bind_method(D_METHOD("set_value_adjustment", "value_adjustment"), &VoxelGeneratorFastNoiseSIMD::set_value_adjustment);
	ClassDB::bind_method(D_METHOD("get_value_adjustment"), &VoxelGeneratorFastNoiseSIMD::get_value_adjustment);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "value_adjustment", PROPERTY_HINT_RANGE, "-2,2,0.01"), "set_value_adjustment", "get_value_adjustment");

	ClassDB::bind_method(D_METHOD("set_noise", "noise"), &VoxelGeneratorFastNoiseSIMD::set_noise);
	ClassDB::bind_method(D_METHOD("get_noise"), &VoxelGeneratorFastNoiseSIMD::get_noise);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "noise", PROPERTY_HINT_RESOURCE_TYPE, "FastNoiseSIMD"), "set_noise", "get_noise");

	ClassDB::bind_method(D_METHOD("set_height_start", "hstart"), &VoxelGeneratorFastNoiseSIMD::set_height_start);
	ClassDB::bind_method(D_METHOD("get_height_start"), &VoxelGeneratorFastNoiseSIMD::get_height_start);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "height_start"), "set_height_start", "get_height_start");

	ClassDB::bind_method(D_METHOD("set_height_range", "hrange"), &VoxelGeneratorFastNoiseSIMD::set_height_range);
	ClassDB::bind_method(D_METHOD("get_height_range"), &VoxelGeneratorFastNoiseSIMD::get_height_range);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "height_range"), "set_height_range", "get_height_range");

}

#endif // FASTNOISESIMD_ENABLED
