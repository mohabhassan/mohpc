#pragma once

#include "Global.h"
#include <stdint.h>
#include <limits.h>

#define M_PI_FLOAT 3.14159265358979323846f
#define NUMVERTEXNORMALS	162

#define DEG2RAD( a ) ( ( (a) * M_PI_FLOAT ) / 180.0f )
#define RAD2DEG( a ) ( ( (a) * 180.0f ) / M_PI_FLOAT )

namespace MOHPC
{
	typedef float vec_t;
	typedef float vec2_t[2];
	typedef float vec3_t[3];
	typedef float vec4_t[4];
	typedef vec4_t quat_t;
	typedef vec_t matrix_t[16];
	
	extern vec3_t vec3_origin;
	extern vec3_t bytedirs[NUMVERTEXNORMALS];

	class Vector;

	MOHPC_EXPORTS vec_t DotProduct(const vec3_t vec1, const vec3_t vec2);
	MOHPC_EXPORTS vec_t DotProduct2D(const vec2_t vec1, const vec2_t vec2);
	MOHPC_EXPORTS vec_t DotProduct4(const vec4_t vec1, const vec4_t vec2);
	MOHPC_EXPORTS double vrsqrt(double number);
	MOHPC_EXPORTS float vrsqrtf(float number);
	MOHPC_EXPORTS float AngleNormalize360(float angle);
	MOHPC_EXPORTS float AngleNormalize180(float angle);
	MOHPC_EXPORTS float AngleMod(float a);
	MOHPC_EXPORTS float AngleSubtract(float a1, float a2);
	MOHPC_EXPORTS void AnglesSubtract(vec3_t v1, vec3_t v2, vec3_t v3);
	MOHPC_EXPORTS float LerpAngle(float from, float to, float frac);
	MOHPC_EXPORTS void AngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
	MOHPC_EXPORTS void AngleVectorsLeft(const vec3_t angles, vec3_t forward, vec3_t left, vec3_t up);
	MOHPC_EXPORTS void AnglesToAxis(float angles[3], float axis[3][3]);
	MOHPC_EXPORTS void MatToQuat(float srcMatrix[3][3], quat_t destQuat);
	MOHPC_EXPORTS void EulerToQuat(float ang[3], float q[4]);
	MOHPC_EXPORTS void QuatToMat(const float q[4], float m[3][3]);
	MOHPC_EXPORTS void QuatSet(quat_t q, float x, float y, float z, float w);
	MOHPC_EXPORTS void QuatClear(quat_t q);
	MOHPC_EXPORTS void QuatInverse(quat_t q);
	MOHPC_EXPORTS void QuatToAngles(const quat_t q, vec3_t angles);
	MOHPC_EXPORTS void QuatNormalize(quat_t quat);
	MOHPC_EXPORTS void MatrixToEulerAngles(const float mat[3][3], vec3_t ang);
	MOHPC_EXPORTS void MatrixMultiply(const float in1[3][3], const float in2[3][3], float out[3][3]);
	MOHPC_EXPORTS void MatrixTransformVector(const vec3_t in, const float mat[3][3], vec3_t out);
	MOHPC_EXPORTS void TransposeMatrix(float in[3][3], float out[3][3]);
	MOHPC_EXPORTS void MatrixCopy(const matrix_t in, matrix_t out);
	MOHPC_EXPORTS void AddPointToBounds(const Vector& v, Vector& mins, Vector& maxs);
	MOHPC_EXPORTS void ClearBounds(Vector& mins, Vector& maxs);
	MOHPC_EXPORTS void AxisClear(vec3_t axis[3]);
	MOHPC_EXPORTS void AxisCopy(const vec3_t in[3], vec3_t out[3]);
	MOHPC_EXPORTS void SnapVector(vec3_t normal);
	MOHPC_EXPORTS void VecCopy(const vec3_t in, vec3_t out);
	MOHPC_EXPORTS void VecSet(vec3_t out, float x, float y, float z);
	MOHPC_EXPORTS void Vec4Copy(const vec4_t in, vec4_t out);
	MOHPC_EXPORTS void VecSubtract(const vec3_t veca, const vec3_t vecb, vec3_t out);
	MOHPC_EXPORTS void VecAdd(const vec3_t veca, const vec3_t vecb, vec3_t out);
	MOHPC_EXPORTS bool VecCompare(const vec3_t veca, const vec3_t vecb);
	MOHPC_EXPORTS bool Vec4Compare(const vec4_t veca, const vec4_t vecb);
	MOHPC_EXPORTS bool Vec4Compare(const vec4_t veca, const vec4_t vecb, float tolerance);
	MOHPC_EXPORTS void VecMatrixInverse(void* DstMatrix, const void* SrcMatrix);
	MOHPC_EXPORTS vec_t VectorLength(const vec3_t vec);
	MOHPC_EXPORTS vec_t VectorLengthSquared(const vec3_t vec);
	MOHPC_EXPORTS vec_t VectorNormalize(vec3_t vec);
	MOHPC_EXPORTS vec_t VectorNormalize2(const vec3_t v, vec3_t out);
	MOHPC_EXPORTS void VectorNormalizeFast(vec3_t vec);
	MOHPC_EXPORTS void VectorMA(const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc);
	MOHPC_EXPORTS void VectorInverse(vec3_t vec);
	MOHPC_EXPORTS void VecNegate(const vec3_t vec, vec3_t out);
	MOHPC_EXPORTS void VectorScale(const vec3_t in, vec_t scale, vec3_t out);
	MOHPC_EXPORTS void VectorClear(vec3_t vec);
	MOHPC_EXPORTS void CrossProduct(const vec3_t v1, const vec3_t v2, vec3_t cross);

	MOHPC_EXPORTS int DirToByte(const vec3_t dir);
	MOHPC_EXPORTS void ByteToDir(int b, vec3_t dir);
	MOHPC_EXPORTS Vector GetMovedir(float angle);
	MOHPC_EXPORTS float Random(float value);
	MOHPC_EXPORTS uint8_t AngleToByte(float v);
	MOHPC_EXPORTS float ByteToAngle(uint8_t v);
	MOHPC_EXPORTS uint16_t AngleToShort(float v);
	MOHPC_EXPORTS float ShortToAngle(uint16_t v);
	MOHPC_EXPORTS int BoundingBoxToInteger(vec3_t mins, vec3_t maxs);
	MOHPC_EXPORTS void IntegerToBoundingBox(int num, vec3_t mins, vec3_t maxs);

	template <typename INT>
	constexpr INT rotl(INT val, intptr_t len)
	{
		constexpr unsigned int mask = CHAR_BIT * sizeof(val) - 1;
		static_assert(std::is_unsigned<INT>::value, "Rotate Left only makes sense for unsigned types");
		return (val << len) | ((unsigned)val >> (-len & mask));
	}

	template <typename INT>
	constexpr INT rotr(INT val, intptr_t len)
	{
		constexpr unsigned int mask = CHAR_BIT * sizeof(val) - 1;
		static_assert(std::is_unsigned<INT>::value, "Rotate Right only makes sense for unsigned types");
		return (val >> len) | ((unsigned)val << (-len & mask));
	}
}
