//*******
// PMDIK
//*******

#ifdef	_WIN32
#include	<gl/glut.h>
#else
#ifdef __APPLE__
#include	<GLUT/glut.h>
#else
#include	<GL/glut.h>
#endif
#endif
#include	<stdio.h>
#include	<string.h>
#include	<math.h>
#include	"PMDIK.h"


//================
// コンストラクタ
//================
cPMDIK::cPMDIK( void ) : m_ppBoneList( NULL )
{
}

//==============
// デストラクタ
//==============
cPMDIK::~cPMDIK( void )
{
	release();
}

//========
// 初期化
//========
void cPMDIK::initialize( const PMD_IK *pPMDIKData, cPMDBone pBoneArray[] )
{
	release();


	// IKターゲットボーン
	m_pTargetBone = &(pBoneArray[pPMDIKData->nTargetNo]);

	// IK先端ボーン
	m_pEffBone = &(pBoneArray[pPMDIKData->nEffNo]);

	m_unCount = pPMDIKData->unCount;
	m_fFact = pPMDIKData->fFact * 3.1415926f;
	m_nSortVal = pPMDIKData->punLinkNo[0];

	// IKリンク配列の作成
	m_cbNumLink = pPMDIKData->cbNumLink;
	m_ppBoneList = new cPMDBone *[m_cbNumLink];
	for( unsigned char i = 0 ; i < m_cbNumLink ; i++ )
	{
		m_ppBoneList[i] = &(pBoneArray[ pPMDIKData->punLinkNo[i] ]);	// ボーン番号は降順で格納されている
		if( strncmp( m_ppBoneList[i]->getName(), "左ひざ", 20 ) == 0 ||  strncmp( m_ppBoneList[i]->getName(), "右ひざ", 20 ) == 0 )
		{
			m_ppBoneList[i]->m_bIKLimitAngle = true;
		}
	}
}

//======
// 更新
//======
void cPMDIK::update( void )
{
	Vector3	vec3OrgTargetPos;
	vec3OrgTargetPos.x = m_pTargetBone->m_matLocal[3][0];
	vec3OrgTargetPos.y = m_pTargetBone->m_matLocal[3][1];
	vec3OrgTargetPos.z = m_pTargetBone->m_matLocal[3][2];

	Vector3	vec3EffPos;
	Vector3	vec3TargetPos;

	for( short i = m_cbNumLink - 1 ; i >= 0 ; i-- ){ m_ppBoneList[i]->updateMatrix(); }
	m_pEffBone->updateMatrix();

	for( unsigned short it = 0 ; it < m_unCount ; it++ )
	{
		for( unsigned char cbLinkIdx = 0 ; cbLinkIdx < m_cbNumLink ; cbLinkIdx++ )
		{
			// エフェクタの位置の取得
			vec3EffPos.x = m_pEffBone->m_matLocal[3][0];
			vec3EffPos.y = m_pEffBone->m_matLocal[3][1];
			vec3EffPos.z = m_pEffBone->m_matLocal[3][2];

			// ワールド座標系から注目ノードの局所(ローカル)座標系への変換
			Matrix	matInvBone;
			MatrixInverse( matInvBone, m_ppBoneList[cbLinkIdx]->m_matLocal );

			// エフェクタ，到達目標のローカル位置
			Vector3Transform( &vec3EffPos, &vec3EffPos, matInvBone );
			Vector3Transform( &vec3TargetPos, &vec3OrgTargetPos, matInvBone );

			// 十分近ければ終了
			Vector3	vec3Diff;
			Vector3Sub( &vec3Diff, &vec3EffPos, &vec3TargetPos );
			if( Vector3DotProduct( &vec3Diff, &vec3Diff ) < 0.0000001f )	return;

			// (1) 基準関節→エフェクタ位置への方向ベクトル
			Vector3Normalize( &vec3EffPos, &vec3EffPos );

			// (2) 基準関節→目標位置への方向ベクトル
			Vector3Normalize( &vec3TargetPos, &vec3TargetPos );

			// ベクトル (1) を (2) に一致させるための最短回転量（Axis-Angle）
			//
			// 回転角
			float	fRotAngle = acosf( Vector3DotProduct( &vec3EffPos, &vec3TargetPos ) );

			if( 0.00000001f < fabsf( fRotAngle ) )
			{
				if( fRotAngle < -m_fFact )	fRotAngle = -m_fFact;
				else if( m_fFact < fRotAngle )	fRotAngle = m_fFact;

				// 回転軸
				Vector3 vec3RotAxis;

				Vector3CrossProduct( &vec3RotAxis, &vec3EffPos, &vec3TargetPos );
				if( Vector3DotProduct( &vec3RotAxis, &vec3RotAxis ) < 0.0000001f )	continue;

				Vector3Normalize( &vec3RotAxis, &vec3RotAxis );

				// 関節回転量の補正
				Vector4		vec4RotQuat;
				QuaternionCreateAxis( &vec4RotQuat, &vec3RotAxis, fRotAngle );

				if( m_ppBoneList[cbLinkIdx]->m_bIKLimitAngle )	limitAngle( &vec4RotQuat, &vec4RotQuat );

				QuaternionNormalize( &vec4RotQuat, &vec4RotQuat );

				QuaternionMultiply( &m_ppBoneList[cbLinkIdx]->m_vec4Rotation, &m_ppBoneList[cbLinkIdx]->m_vec4Rotation, &vec4RotQuat );
				QuaternionNormalize( &m_ppBoneList[cbLinkIdx]->m_vec4Rotation, &m_ppBoneList[cbLinkIdx]->m_vec4Rotation );

				for( short i = cbLinkIdx ; i >= 0 ; i-- ){ m_ppBoneList[i]->updateMatrix(); }
				m_pEffBone->updateMatrix();
			}
		}
	}
}

//----------------------------
// ボーンの回転角度を制限する
//----------------------------
void cPMDIK::limitAngle( Vector4 *pvec4Out, const Vector4 *pvec4Src )
{
	Vector3	vec3Angle;

	// XYZ軸回転の取得
	QuaternionToEuler( &vec3Angle, pvec4Src );

	// 角度制限
	if( vec3Angle.x < -3.14159f )	vec3Angle.x = -3.14159f;
	if( -0.002f < vec3Angle.x )		vec3Angle.x = -0.002f;
	vec3Angle.y = 0.0f;
	vec3Angle.z = 0.0f;

	// XYZ軸回転からクォータニオンへ
	QuaternionCreateEuler( pvec4Out, &vec3Angle );
}

//==============
// デバッグ描画
//==============
void cPMDIK::debugDraw( void )
{
	for( unsigned char i = 0 ; i < m_cbNumLink ; i++ )
	{
		glPushMatrix();

			glMultMatrixf( (const float *)m_ppBoneList[i]->m_matLocal );

			glColor4f( 0.0f, 1.0f, 1.0f, 1.0f );

			glDisable( GL_TEXTURE_2D );
			glutSolidCube( 0.3f );

		glPopMatrix();

		glPushMatrix();

			glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

			if( m_ppBoneList[i]->m_pParentBone )
			{
				glBegin( GL_LINES );
					glVertex3f( m_ppBoneList[i]->m_pParentBone->m_matLocal[3][0], m_ppBoneList[i]->m_pParentBone->m_matLocal[3][1], m_ppBoneList[i]->m_pParentBone->m_matLocal[3][2] );
					glVertex3f(                m_ppBoneList[i]->m_matLocal[3][0],                m_ppBoneList[i]->m_matLocal[3][1],                m_ppBoneList[i]->m_matLocal[3][2] );
				glEnd();
			}

		glPopMatrix();
	}

	glPushMatrix();

		glMultMatrixf( (const float *)m_pEffBone->m_matLocal );

		glColor4f( 1.0f, 1.0f, 0.0f, 1.0f );

		glDisable( GL_TEXTURE_2D );
		glutSolidCube( 0.3f );

	glPopMatrix();

	glPushMatrix();

		glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

		if( m_pEffBone->m_pParentBone )
		{
			glBegin( GL_LINES );
				glVertex3f( m_pEffBone->m_pParentBone->m_matLocal[3][0], m_pEffBone->m_pParentBone->m_matLocal[3][1], m_pEffBone->m_pParentBone->m_matLocal[3][2] );
				glVertex3f(                m_pEffBone->m_matLocal[3][0],                m_pEffBone->m_matLocal[3][1],                m_pEffBone->m_matLocal[3][2] );
			glEnd();
		}

	glPopMatrix();

	glPushMatrix();

		glMultMatrixf( (const float *)m_pTargetBone->m_matLocal );

		glColor4f( 1.0f, 0.0f, 0.0f, 1.0f );

		glDisable( GL_TEXTURE_2D );
		glutSolidCube( 0.3f );

	glPopMatrix();
}

//======
// 解放
//======
void cPMDIK::release( void )
{
	if( m_ppBoneList )
	{
		delete [] m_ppBoneList;
		m_ppBoneList = NULL;
	}
}
