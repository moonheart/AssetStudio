#include "unitycrunch.h"
#include <stdint.h>
#include <algorithm>
#include "unitycrunch/crn_decomp.h"

bool unity_crunch_unpack_level(const uint8_t* data, uint32_t data_size, uint32_t level_index, void** ret, uint32_t* ret_size) {
	unitycrnd::crn_texture_info tex_info;
	if (!unitycrnd::crnd_get_texture_info(data, data_size, &tex_info))
	{
		return false;
	}

	unitycrnd::crnd_unpack_context pContext = unitycrnd::crnd_unpack_begin(data, data_size);
	if (!pContext)
	{
		return false;
	}

	const crn_uint32 width = std::max(1U, tex_info.m_width >> level_index);
	const crn_uint32 height = std::max(1U, tex_info.m_height >> level_index);
	const crn_uint32 blocks_x = std::max(1U, (width + 3) >> 2);
	const crn_uint32 blocks_y = std::max(1U, (height + 3) >> 2);
	const crn_uint32 row_pitch = blocks_x * unitycrnd::crnd_get_bytes_per_dxt_block(tex_info.m_format);
	const crn_uint32 face_count = tex_info.m_faces;
	const crn_uint32 face_size = row_pitch * blocks_y;
	const crn_uint32 total_face_size = face_size * face_count;

	void* out_ptrs[cCRNMaxFaces]{};
	if (face_count == 1) 
	{
		out_ptrs[0] = new uint8_t[face_size]{};
		*ret_size = face_size;
	}
	else if (1 < face_count < 7) 
	{
		for (uint8_t i = 0; i < face_count; i++) 
		{
			out_ptrs[i] = new uint8_t[face_size]{};
		}
		*ret_size = total_face_size;
	}
	else 
	{
		return false;
	}

	if (!unitycrnd::crnd_unpack_level(pContext, out_ptrs, face_size, row_pitch, level_index))
	{
		unitycrnd::crnd_unpack_end(pContext);
		return false;
	}

	uint8_t* buff = new uint8_t[total_face_size]{};
	for (uint8_t i = 0; i < face_count; i++) 
	{
		memcpy(buff + (face_size * i), out_ptrs[i], face_size);
		free(out_ptrs[i]);
	}
	*ret = buff;

	unitycrnd::crnd_unpack_end(pContext);
	return true;
}