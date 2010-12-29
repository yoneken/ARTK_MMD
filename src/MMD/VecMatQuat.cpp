//************************************
// ベクトル、行列、クォータニオン演算
//************************************

#include	<math.h>
#include	<memory.h>
#include	"VecMatQuat.h"


void Vector3Add( Vector3 *pvec3Out, const Vector3 *pvec3Add1, const Vector3 *pvec3Add2 )
{
	pvec3Out->x = pvec3Add1->x + pvec3Add2->x;
	pvec3Out->y = pvec3Add1->y + pvec3Add2->y;
	pvec3Out->z = pvec3Add1->z + pvec3Add2->z;
}

void Vector3Sub( Vector3 *pvec3Out, const Vector3 *pvec3Sub1, const Vector3 *pvec3Sub2 )
{
	pvec3Out->x = pvec3Sub1->x - pvec3Sub2->x;
	pvec3Out->y = pvec3Sub1->y - pvec3Sub2->y;
	pvec3Out->z = pvec3Sub1->z - pvec3Sub2->z;
}

void Vector3MulAdd( Vector3 *pvec3Out, const Vector3 *pvec3Add1, const Vector3 *pvec3Add2, float fRate )
{
	pvec3Out->x = pvec3Add1->x + pvec3Add2->x * fRate;
	pvec3Out->y = pvec3Add1->y + pvec3Add2->y * fRate;
	pvec3Out->z = pvec3Add1->z + pvec3Add2->z * fRate;
}

void Vector3Normalize( Vector3 *pvec3Out, const Vector3 *pvec3Src )
{
	float fSqr = 1.0f / sqrtf( pvec3Src->x * pvec3Src->x + pvec3Src->y * pvec3Src->y + pvec3Src->z * pvec3Src->z );

	pvec3Out->x = pvec3Src->x * fSqr;
	pvec3Out->y = pvec3Src->y * fSqr;
	pvec3Out->z = pvec3Src->z * fSqr;
}

float Vector3DotProduct( const Vector3 *pvec3Src1, const Vector3 *pvec3Src2 )
{
	return (pvec3Src1->x * pvec3Src2->x + pvec3Src1->y * pvec3Src2->y + pvec3Src1->z * pvec3Src2->z);
}

void Vector3CrossProduct( Vector3 *pvec3Out, const Vector3 *pvec3Src1, const Vector3 *pvec3Src2 )
{
	pvec3Out->x = pvec3Src1->y * pvec3Src2->z - pvec3Src1->z * pvec3Src2->y;
	pvec3Out->y = pvec3Src1->z * pvec3Src2->x - pvec3Src1->x * pvec3Src2->z;
	pvec3Out->z = pvec3Src1->x * pvec3Src2->y - pvec3Src1->y * pvec3Src2->x;
}

void Vector3Lerp( Vector3 *pvec3Out, const Vector3 *pvec3Src1, const Vector3 *pvec3Src2, float fLerpValue )
{
	float	t0 = 1.0f - fLerpValue;

	pvec3Out->x = pvec3Src1->x * t0 + pvec3Src2->x * fLerpValue;
	pvec3Out->y = pvec3Src1->y * t0 + pvec3Src2->y * fLerpValue;
	pvec3Out->z = pvec3Src1->z * t0 + pvec3Src2->z * fLerpValue;
}

void Vector3Transform( Vector3 *pvec3Out, const Vector3 *pVec3In, const Matrix matTransform )
{
	Vector3	vec3Temp;

	vec3Temp.x = pVec3In->x * matTransform[0][0] + pVec3In->y * matTransform[1][0] + pVec3In->z * matTransform[2][0] + matTransform[3][0];
	vec3Temp.y = pVec3In->x * matTransform[0][1] + pVec3In->y * matTransform[1][1] + pVec3In->z * matTransform[2][1] + matTransform[3][1];
	vec3Temp.z = pVec3In->x * matTransform[0][2] + pVec3In->y * matTransform[1][2] + pVec3In->z * matTransform[2][2] + matTransform[3][2];

	*pvec3Out = vec3Temp;
}

void Vector3Rotate( Vector3 *pvec3Out, const Vector3 *pVec3In, const Matrix matRotate )
{
	Vector3	vec3Temp;

	vec3Temp.x = pVec3In->x * matRotate[0][0] + pVec3In->y * matRotate[1][0] + pVec3In->z * matRotate[2][0];
	vec3Temp.y = pVec3In->x * matRotate[0][1] + pVec3In->y * matRotate[1][1] + pVec3In->z * matRotate[2][1];
	vec3Temp.z = pVec3In->x * matRotate[0][2] + pVec3In->y * matRotate[1][2] + pVec3In->z * matRotate[2][2];

	*pvec3Out = vec3Temp;
}

void MatrixIdentity( Matrix matOut )
{
	matOut[0][1] = matOut[0][2] = matOut[0][3] =
	matOut[1][0] = matOut[1][2] = matOut[1][3] =
	matOut[2][0] = matOut[2][1] = matOut[2][3] = 
	matOut[3][0] = matOut[3][1] = matOut[3][2] = 0.0f;
	matOut[0][0] = matOut[1][1] = matOut[2][2] = matOut[3][3] = 1.0f;
}

void MatrixRotationX( Matrix matOut, float fAngle )
{
	float	c = cosf( fAngle );
	float	s = sinf( fAngle );
	
	matOut[0][0] = 1.0f; matOut[0][1] = 0.0f; matOut[0][2] = 0.0f; matOut[0][3] = 0.0f;
	matOut[1][0] = 0.0f; matOut[1][1] =    c; matOut[1][2] =    s; matOut[1][3] = 0.0f;
	matOut[2][0] = 0.0f; matOut[2][1] =   -s; matOut[2][2] =    c; matOut[2][3] = 0.0f;
	matOut[3][0] = 0.0f; matOut[3][1] = 0.0f; matOut[3][2] = 0.0f; matOut[3][3] = 1.0f;
}

void MatrixMultiply( Matrix matOut, const Matrix matSrc1, const Matrix matSrc2 )
{
	Matrix	matTemp;
	int		i;
	
	for( i = 0 ; i < 4 ; i++ )
	{
		matTemp[i][0] =	matSrc1[i][0] * matSrc2[0][0] + matSrc1[i][1] * matSrc2[1][0] + matSrc1[i][2] * matSrc2[2][0] + matSrc1[i][3] * matSrc2[3][0];
		matTemp[i][1] =	matSrc1[i][0] * matSrc2[0][1] + matSrc1[i][1] * matSrc2[1][1] + matSrc1[i][2] * matSrc2[2][1] + matSrc1[i][3] * matSrc2[3][1];
		matTemp[i][2] =	matSrc1[i][0] * matSrc2[0][2] + matSrc1[i][1] * matSrc2[1][2] + matSrc1[i][2] * matSrc2[2][2] + matSrc1[i][3] * matSrc2[3][2];
		matTemp[i][3] =	matSrc1[i][0] * matSrc2[0][3] + matSrc1[i][1] * matSrc2[1][3] + matSrc1[i][2] * matSrc2[2][3] + matSrc1[i][3] * matSrc2[3][3];
	}

	for( i = 0 ; i < 4 ; i++ )
	{
		matOut[i][0] = matTemp[i][0];
		matOut[i][1] = matTemp[i][1];
		matOut[i][2] = matTemp[i][2];
		matOut[i][3] = matTemp[i][3];
	}
}

void MatrixInverse( Matrix matOut, const Matrix matSrc )
{
	Matrix	matTemp;
	memcpy( matTemp, matSrc, sizeof(Matrix) );

	MatrixIdentity( matOut );

	//掃き出し法
	for( int i = 0 ; i < 4 ; i++ )
	{
		float	buf = 1 / matTemp[i][i];
		for( int j = 0 ; j < 4 ; j++ )
		{
			matTemp[i][j] *= buf;
			matOut[i][j] *= buf;
		}
		for( int j = 0 ; j < 4 ; j++ )
		{
			if( i != j )
			{
				buf = matTemp[j][i];
				for( int k = 0 ; k < 4 ; k++ )
				{
					matTemp[j][k] -= matTemp[i][k] * buf;
					matOut[j][k] -= matOut[i][k] * buf;
				}
			}
		}
	}
}

void MatrixLerp( Matrix matOut, Matrix matSrc1, Matrix matSrc2, float fLerpValue )
{
	float	*fOut  = (float *)matOut,
			*fSrc1 = (float *)matSrc1,
			*fSrc2 = (float *)matSrc2,
			fT = 1.0f - fLerpValue;

	*fOut++ = *fSrc1++ * fLerpValue + *fSrc2++ * fT;
	*fOut++ = *fSrc1++ * fLerpValue + *fSrc2++ * fT;
	*fOut++ = *fSrc1++ * fLerpValue + *fSrc2++ * fT;
	*fOut++ = *fSrc1++ * fLerpValue + *fSrc2++ * fT;

	*fOut++ = *fSrc1++ * fLerpValue + *fSrc2++ * fT;
	*fOut++ = *fSrc1++ * fLerpValue + *fSrc2++ * fT;
	*fOut++ = *fSrc1++ * fLerpValue + *fSrc2++ * fT;
	*fOut++ = *fSrc1++ * fLerpValue + *fSrc2++ * fT;

	*fOut++ = *fSrc1++ * fLerpValue + *fSrc2++ * fT;
	*fOut++ = *fSrc1++ * fLerpValue + *fSrc2++ * fT;
	*fOut++ = *fSrc1++ * fLerpValue + *fSrc2++ * fT;
	*fOut++ = *fSrc1++ * fLerpValue + *fSrc2++ * fT;

	*fOut++ = *fSrc1++ * fLerpValue + *fSrc2++ * fT;
	*fOut++ = *fSrc1++ * fLerpValue + *fSrc2++ * fT;
	*fOut++ = *fSrc1++ * fLerpValue + *fSrc2++ * fT;
	*fOut++ = *fSrc1++ * fLerpValue + *fSrc2++ * fT;
}

void MatrixPlanarProjection( Matrix matOut, const Vector4 *pvec4Plane, const Vector3 *pvec3LightPos )
{
	float	fDot =	pvec4Plane->x * pvec3LightPos->x +
						pvec4Plane->y * pvec3LightPos->y +
							pvec4Plane->z * pvec3LightPos->z + 
								pvec4Plane->w;

	matOut[0][0] = fDot - pvec3LightPos->x * pvec4Plane->x;
	matOut[1][0] =      - pvec3LightPos->x * pvec4Plane->y;
	matOut[2][0] =      - pvec3LightPos->x * pvec4Plane->z;
	matOut[3][0] =      - pvec3LightPos->x * pvec4Plane->w;

	matOut[0][1] =      - pvec3LightPos->y * pvec4Plane->x;
	matOut[1][1] = fDot - pvec3LightPos->y * pvec4Plane->y;
	matOut[2][1] =      - pvec3LightPos->y * pvec4Plane->z;
	matOut[3][1] =      - pvec3LightPos->y * pvec4Plane->w;

	matOut[0][2] =      - pvec3LightPos->z * pvec4Plane->x;
	matOut[1][2] =      - pvec3LightPos->z * pvec4Plane->y;
	matOut[2][2] = fDot - pvec3LightPos->z * pvec4Plane->z;
	matOut[3][2] =      - pvec3LightPos->z * pvec4Plane->w;

	matOut[0][3] =      - pvec4Plane->x;
	matOut[1][3] =      - pvec4Plane->y;
	matOut[2][3] =      - pvec4Plane->z;
	matOut[3][3] = fDot - pvec4Plane->w;
}

void QuaternionCreateAxis( Vector4 *pvec4Out, const Vector3 *pvec3Axis, float fRotAngle )
{
	if( fabsf( fRotAngle ) < 0.0001f )
	{
		pvec4Out->x = pvec4Out->y = pvec4Out->z = 0.0f;
		pvec4Out->w = 1.0f;
	}
	else
	{
		fRotAngle *= 0.5f;
		float	fTemp = sinf( fRotAngle );

		pvec4Out->x = pvec3Axis->x * fTemp;
		pvec4Out->y = pvec3Axis->y * fTemp;
		pvec4Out->z = pvec3Axis->z * fTemp;
		pvec4Out->w = cosf( fRotAngle );
	}
}

void QuaternionCreateEuler( Vector4 *pvec4Out, const Vector3 *pvec3EulerAngle )
{
	float	xRadian = pvec3EulerAngle->x * 0.5f;
	float	yRadian = pvec3EulerAngle->y * 0.5f;
	float	zRadian = pvec3EulerAngle->z * 0.5f;
	float	sinX = sinf( xRadian );
	float	cosX = cosf( xRadian );
	float	sinY = sinf( yRadian );
	float	cosY = cosf( yRadian );
	float	sinZ = sinf( zRadian );
	float	cosZ = cosf( zRadian );

	// XYZ
	pvec4Out->x = sinX * cosY * cosZ - cosX * sinY * sinZ;
	pvec4Out->y = cosX * sinY * cosZ + sinX * cosY * sinZ;
	pvec4Out->z = cosX * cosY * sinZ - sinX * sinY * cosZ;
	pvec4Out->w = cosX * cosY * cosZ + sinX * sinY * sinZ;
}

void QuaternionNormalize( Vector4 *pvec4Out, const Vector4 *pvec4Src )
{
	float fSqr = 1.0f / sqrtf( pvec4Src->x * pvec4Src->x + pvec4Src->y * pvec4Src->y + pvec4Src->z * pvec4Src->z + pvec4Src->w * pvec4Src->w );

	pvec4Out->x = pvec4Src->x * fSqr;
	pvec4Out->y = pvec4Src->y * fSqr;
	pvec4Out->z = pvec4Src->z * fSqr;
	pvec4Out->w = pvec4Src->w * fSqr;
}

void QuaternionMultiply( Vector4 *pvec4Out, const Vector4 *pvec4Src1, const Vector4 *pvec4Src2 )
{
	float	px, py, pz, pw;
	float	qx, qy, qz, qw;

	px = pvec4Src1->x; py = pvec4Src1->y; pz = pvec4Src1->z; pw = pvec4Src1->w;
	qx = pvec4Src2->x; qy = pvec4Src2->y; qz = pvec4Src2->z; qw = pvec4Src2->w;

	pvec4Out->x = pw * qx + px * qw + py * qz - pz * qy;
	pvec4Out->y = pw * qy - px * qz + py * qw + pz * qx;
	pvec4Out->z = pw * qz + px * qy - py * qx + pz * qw;
	pvec4Out->w = pw * qw - px * qx - py * qy - pz * qz;
}

void QuaternionSlerp( Vector4 *pvec4Out, const Vector4 *pvec4Src1, const Vector4 *pvec4Src2, float fLerpValue )
{
#if	0
	// Slerp
	float	dot = pvec4Src1->x * pvec4Src2->x + pvec4Src1->y * pvec4Src2->y + pvec4Src1->z * pvec4Src2->z + pvec4Src1->w * pvec4Src2->w;

	// 反転処理
	Vector4	vec4CorrectTarget;

	if(dot < 0.f)
	{
		vec4CorrectTarget.x = -pvec4Src2->x;
		vec4CorrectTarget.y = -pvec4Src2->y;
		vec4CorrectTarget.z = -pvec4Src2->z;
		vec4CorrectTarget.w = -pvec4Src2->w;
		dot = -dot;
	}
	else
	{
		vec4CorrectTarget = *pvec4Src2;
	}

	// 誤差対策
	if(dot >= 1.f){ dot = 1.f; }
	float radian = acosf( dot );

	if( fabsf( radian ) < 0.0000000001f ){ *pvec4Out = vec4CorrectTarget; return; }

	float inverseSin = 1.f / sinf( radian );
	float t0 = sinf( (1.f - fLerpValue) * radian ) * inverseSin;
	float t1 = sinf( fLerpValue * radian ) * inverseSin;

	pvec4Out->x = pvec4Src1->x * t0 + vec4CorrectTarget.x * t1;
	pvec4Out->y = pvec4Src1->y * t0 + vec4CorrectTarget.y * t1;
	pvec4Out->z = pvec4Src1->z * t0 + vec4CorrectTarget.z * t1;
	pvec4Out->w = pvec4Src1->w * t0 + vec4CorrectTarget.w * t1;
#else
	// Qlerp
	float	qr = pvec4Src1->x * pvec4Src2->x + pvec4Src1->y * pvec4Src2->y + pvec4Src1->z * pvec4Src2->z + pvec4Src1->w * pvec4Src2->w;
	float	t0 = 1.0f - fLerpValue;

	if( qr < 0 )
	{
		pvec4Out->x = pvec4Src1->x * t0 - pvec4Src2->x * fLerpValue;
		pvec4Out->y = pvec4Src1->y * t0 - pvec4Src2->y * fLerpValue;
		pvec4Out->z = pvec4Src1->z * t0 - pvec4Src2->z * fLerpValue;
		pvec4Out->w = pvec4Src1->w * t0 - pvec4Src2->w * fLerpValue;
	}
	else
	{
		pvec4Out->x = pvec4Src1->x * t0 + pvec4Src2->x * fLerpValue;
		pvec4Out->y = pvec4Src1->y * t0 + pvec4Src2->y * fLerpValue;
		pvec4Out->z = pvec4Src1->z * t0 + pvec4Src2->z * fLerpValue;
		pvec4Out->w = pvec4Src1->w * t0 + pvec4Src2->w * fLerpValue;
	}
	QuaternionNormalize( pvec4Out, pvec4Out );
#endif
}

void QuaternionToMatrix( Matrix matOut, const Vector4 *pvec4Quat )
{
	float	x2 = pvec4Quat->x * pvec4Quat->x * 2.0f;
	float	y2 = pvec4Quat->y * pvec4Quat->y * 2.0f;
	float	z2 = pvec4Quat->z * pvec4Quat->z * 2.0f;
	float	xy = pvec4Quat->x * pvec4Quat->y * 2.0f;
	float	yz = pvec4Quat->y * pvec4Quat->z * 2.0f;
	float	zx = pvec4Quat->z * pvec4Quat->x * 2.0f;
	float	xw = pvec4Quat->x * pvec4Quat->w * 2.0f;
	float	yw = pvec4Quat->y * pvec4Quat->w * 2.0f;
	float	zw = pvec4Quat->z * pvec4Quat->w * 2.0f;

	matOut[0][0] = 1.0f - y2 - z2;
	matOut[0][1] = xy + zw;
	matOut[0][2] = zx - yw;
	matOut[1][0] = xy - zw;
	matOut[1][1] = 1.0f - z2 - x2;
	matOut[1][2] = yz + xw;
	matOut[2][0] = zx + yw;
	matOut[2][1] = yz - xw;
	matOut[2][2] = 1.0f - x2 - y2;

	matOut[0][3] = matOut[1][3] = matOut[2][3] = matOut[3][0] = matOut[3][1] = matOut[3][2] = 0.0f;
	matOut[3][3] = 1.0f;
}


void QuaternionToEuler( Vector3 *pvecAngle, const Vector4 *pvec4Quat )
{
	// XYZ軸回転の取得
	// Y回転を求める
	float	x2 = pvec4Quat->x + pvec4Quat->x;
	float	y2 = pvec4Quat->y + pvec4Quat->y;
	float	z2 = pvec4Quat->z + pvec4Quat->z;
	float	xz2 = pvec4Quat->x * z2;
	float	wy2 = pvec4Quat->w * y2;
	float	temp = -(xz2 - wy2);

	// 誤差対策
	if( temp >= 1.f ){ temp = 1.f; }
	else if( temp <= -1.f ){ temp = -1.f; }

	float	yRadian = asinf(temp);

	// 他の回転を求める
	float	xx2 = pvec4Quat->x * x2;
	float	xy2 = pvec4Quat->x * y2;
	float	zz2 = pvec4Quat->z * z2;
	float	wz2 = pvec4Quat->w * z2;

	if( yRadian < 3.1415926f * 0.5f )
	{
		if( yRadian > -3.1415926f * 0.5f )
		{
			float	yz2 = pvec4Quat->y * z2;
			float	wx2 = pvec4Quat->w * x2;
			float	yy2 = pvec4Quat->y * y2;
			pvecAngle->x = atan2f( (yz2 + wx2), (1.f - (xx2 + yy2)) );
			pvecAngle->y = yRadian;
			pvecAngle->z = atan2f( (xy2 + wz2), (1.f - (yy2 + zz2)) );
		}
		else
		{
			pvecAngle->x = -atan2f( (xy2 - wz2), (1.f - (xx2 + zz2)) );
			pvecAngle->y = yRadian;
			pvecAngle->z = 0.f;
		}
	}
	else
	{
		pvecAngle->x = atan2f( (xy2 - wz2), (1.f - (xx2 + zz2)) );
		pvecAngle->y = yRadian;
		pvecAngle->z = 0.f;
	}
}
