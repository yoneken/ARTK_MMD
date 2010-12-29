//************************************
// ベクトル、行列、クォータニオン演算
//************************************

#ifndef		_VECMATQUAT_H_
#define		_VECMATQUAT_H_

#pragma pack( push, 1 )

// 3要素ベクトル
struct Vector3
{
	float	x, y, z;
};

// 4要素ベクトル(クォータニオン兼用)
struct Vector4
{
	float	x, y, z, w;
};

// テクスチャUV
struct TexUV
{
	float	u, v;
};

// アルファ無しカラー
struct Color3
{
	float	r, g, b;
};

// アルファありカラー
struct Color4
{
	float	r, g, b, a;
};

// 行列
typedef float	Matrix[4][4];

#pragma pack( pop )


extern void Vector3Add( Vector3 *pvec3Out, const Vector3 *pvec3Add1, const Vector3 *pvec3Add2 );
extern void Vector3Sub( Vector3 *pvec3Dst, const Vector3 *pvec3Sub1, const Vector3 *pvec3Sub2 );
extern void Vector3MulAdd( Vector3 *pvec3Out, const Vector3 *pvec3Add1, const Vector3 *pvec3Add2, float fRate );
extern void Vector3Normalize( Vector3 *pvec3Out, const Vector3 *pvec3Src );
extern float Vector3DotProduct( const Vector3 *pvec3Src1, const Vector3 *pvec3Src2 );
extern void Vector3CrossProduct( Vector3 *pvec3Out, const Vector3 *pvec3Src1, const Vector3 *pvec3Src2 );
extern void Vector3Lerp( Vector3 *pvec3Out, const Vector3 *pvec3Src1, const Vector3 *pvec3Src2, float fLerpValue );
extern void Vector3Transform( Vector3 *pvec3Out, const Vector3 *pVec3In, const Matrix matTransform );
extern void Vector3Rotate( Vector3 *pvec3Out, const Vector3 *pVec3In, const Matrix matRotate );

extern void MatrixIdentity( Matrix matOut );
extern void MatrixRotationX( Matrix matOut, float fAngle );
extern void MatrixMultiply( Matrix matOut, const Matrix matSrc1, const Matrix matSrc2 );
extern void MatrixInverse( Matrix matOut, const Matrix matSrc );
extern void MatrixLerp( Matrix matOut, Matrix matSrc1, Matrix matSrc2, float fLerpValue );
extern void MatrixPlanarProjection( Matrix matOut, const Vector4 *pvec4Plane, const Vector3 *pvec3LightPos );

extern void QuaternionCreateAxis( Vector4 *pvec4Out, const Vector3 *pvec3Axis, float fRotAngle );
extern void QuaternionCreateEuler( Vector4 *pvec4Out, const Vector3 *pvec3EulerAngle );
extern void QuaternionNormalize( Vector4 *pvec4Out, const Vector4 *pvec4Src );
extern void QuaternionMultiply( Vector4 *pvec4Out, const Vector4 *pvec4Src1, const Vector4 *pvec4Src2 );
extern void QuaternionSlerp( Vector4 *pvec4Out, const Vector4 *pvec4Src1, const Vector4 *pvec4Src2, float fLerpValue );
extern void QuaternionToMatrix( Matrix matOut, const Vector4 *pvec4Quat );
extern void QuaternionToEuler( Vector3 *pvecAngle, const Vector4 *pvec4Quat );

#endif	// _VECMATQUAT_H_
