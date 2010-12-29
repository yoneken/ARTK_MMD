//****************
// モーション再生
//****************

#include	<stdio.h>
#ifndef __APPLE__
#include <malloc.h>
#else
#include <stdlib.h>
#endif
#include	<string.h>
#include	"PMDModel.h"
#include	"MotionPlayer.h"


//================
// コンストラクタ
//================
cMotionPlayer::cMotionPlayer( void ) : m_pVMDMotion( NULL ), m_ppBoneList( NULL ), m_ppFaceList( NULL )
{
}

//==============
// デストラクタ
//==============
cMotionPlayer::~cMotionPlayer( void )
{
	clear();
}

//==========================
// 再生するモーションの設定
//==========================
void cMotionPlayer::setup( cPMDModel *pPMDModel, cVMDMotion *pMotion, bool bLoop, float fInterpolateFrame )
{
	clear();


	m_pPMDModel = pPMDModel;
	m_pVMDMotion = pMotion;

	//---------------------------------------------------------
	// 操作対象ボーンのポインタを設定する
	MotionDataList	*pMotionDataList = m_pVMDMotion->getMotionDataList();
	m_ppBoneList = new cPMDBone *[m_pVMDMotion->getNumMotionNodes()];
	cPMDBone		**ppBone = m_ppBoneList;
	while( pMotionDataList )
	{
		(*ppBone) = pPMDModel->getBoneByName( pMotionDataList->szBoneName );

		pMotionDataList = pMotionDataList->pNext;
		ppBone++;
	}

	//---------------------------------------------------------
	// 操作対象表情のポインタを設定する
	FaceDataList	*pFaceDataList = m_pVMDMotion->getFaceDataList();
	m_ppFaceList = new cPMDFace *[m_pVMDMotion->getNumFaceNodes()];
	cPMDFace		**ppFace = m_ppFaceList;
	while( pFaceDataList )
	{
		(*ppFace) = pPMDModel->getFaceByName( pFaceDataList->szFaceName );

		pFaceDataList = pFaceDataList->pNext;
		ppFace++;
	}

	// 変数初期値設定
	m_fOldFrame = m_fFrame = 0.0f;
	m_bLoop = bLoop;

	if( fInterpolateFrame != 0.0f )
	{
		m_fInterpolateFrameMax = 1.0f / fInterpolateFrame;
		m_fInterpolateFrameNow = 0.0f;
	}
	else
	{
		m_fInterpolateFrameMax = 0.0f;
		m_fInterpolateFrameNow = 0.0f;
	}
}

//================
// モーション更新
//================
bool cMotionPlayer::update( float fElapsedFrame )
{
	if( !m_pVMDMotion )		return true;

	float	fInterpolateRate = 1.0f;

	if( m_fInterpolateFrameMax != 0.0f )
	{
		m_fInterpolateFrameNow += fElapsedFrame;
		fInterpolateRate = m_fInterpolateFrameNow * m_fInterpolateFrameMax;

		if( fInterpolateRate > 1.0f )	fInterpolateRate = 1.0f;
	}

	//---------------------------------------------------------
	// 指定フレームのデータでボーンを動かす
	MotionDataList	*pMotionDataList = m_pVMDMotion->getMotionDataList();
	cPMDBone		**ppBone = m_ppBoneList;
	Vector3			vec3Position;
	Vector4			vec4Rotation;

	if( fInterpolateRate == 1.0f )
	{
		// 補間なし
		while( pMotionDataList )
		{
			if( *ppBone )
			{
				getMotionPosRot( pMotionDataList, m_fFrame, &vec3Position, &vec4Rotation );

				(*ppBone)->m_vec3Position = vec3Position;
				(*ppBone)->m_vec4Rotation = vec4Rotation;
			}

			pMotionDataList = pMotionDataList->pNext;
			ppBone++;
		}
	}
	else
	{
		// 補間あり
		while( pMotionDataList )
		{
			if( *ppBone )
			{
				getMotionPosRot( pMotionDataList, m_fFrame, &vec3Position, &vec4Rotation );

				Vector3Lerp( &((*ppBone)->m_vec3Position), &((*ppBone)->m_vec3Position), &vec3Position, fInterpolateRate );
				QuaternionSlerp( &((*ppBone)->m_vec4Rotation), &((*ppBone)->m_vec4Rotation), &vec4Rotation, fInterpolateRate );
			}

			pMotionDataList = pMotionDataList->pNext;
			ppBone++;
		}
	}

	//---------------------------------------------------------
	// 指定フレームのデータで表情を変形する
	FaceDataList	*pFaceDataList = m_pVMDMotion->getFaceDataList();
	cPMDFace		**ppFace = m_ppFaceList;
	float			fFaceRate;

	while( pFaceDataList )
	{
		if( *ppFace )
		{
			fFaceRate = getFaceRate( pFaceDataList, m_fFrame );

			if( fFaceRate == 1.0f )			(*ppFace)->setFace(   m_pPMDModel->m_pvec3OrgPositionArray );
			else if( 0.001f < fFaceRate )	(*ppFace)->blendFace( m_pPMDModel->m_pvec3OrgPositionArray, fFaceRate );
		}

		pFaceDataList = pFaceDataList->pNext;
		ppFace++;
	}

	//---------------------------------------------------------
	// フレームを進める
	bool	bMotionFinshed = false;

	m_fOldFrame = m_fFrame;
	m_fFrame += fElapsedFrame;

	if( m_bLoop )
	{
		if( m_fOldFrame >= m_pVMDMotion->getMaxFrame() )
		{
			m_fOldFrame = 0.0f;
			m_fFrame = m_fFrame - m_pVMDMotion->getMaxFrame();
		}
	}

	if( m_fFrame >= m_pVMDMotion->getMaxFrame() )
	{
		m_fFrame = m_pVMDMotion->getMaxFrame();
		bMotionFinshed = true;
	}

	return bMotionFinshed;
}

//----------------------------------------
// キーフレームを補間して位置と回転を返す
//----------------------------------------
void cMotionPlayer::getMotionPosRot( const MotionDataList *pMotionData, float fFrame, Vector3 *pvec3Pos, Vector4 *pvec4Rot )
{
	unsigned long	i;
	unsigned long	ulNumKeyFrame = pMotionData->ulNumKeyFrames;

	// 最終フレームを過ぎていた場合
	if( fFrame > pMotionData->pKeyFrames[ulNumKeyFrame - 1].fFrameNo )
	{
		fFrame = pMotionData->pKeyFrames[ulNumKeyFrame - 1].fFrameNo;
	}

	// 現在の時間がどのキー近辺にあるか
	for( i = 0 ; i < ulNumKeyFrame ; i++ )
	{
		if( fFrame <= pMotionData->pKeyFrames[i].fFrameNo )
		{
			break;
		}
	}

	// 前後のキーを設定
	long	lKey0,
			lKey1;

	lKey0 = i - 1;
	lKey1 = i;

	if( lKey0 <= 0 )			lKey0 = 0;
	if( i == ulNumKeyFrame )	lKey1 = ulNumKeyFrame - 1;

	// 前後のキーの時間
	float	fTime0 = pMotionData->pKeyFrames[lKey0].fFrameNo;
	float	fTime1 = pMotionData->pKeyFrames[lKey1].fFrameNo;

	// 前後のキーの間でどの位置にいるか
	if( lKey0 != lKey1 )
	{
		float	fLerpValue = fLerpValue = (fFrame - fTime0) / (fTime1 - fTime0);
		float	fPosLerpValue;

		fPosLerpValue = pMotionData->pKeyFrames[lKey1].clPosXInterBezier.getInterValue( fLerpValue );
		pvec3Pos->x = pMotionData->pKeyFrames[lKey0].vec3Position.x * (1.0f - fPosLerpValue) + pMotionData->pKeyFrames[lKey1].vec3Position.x * fPosLerpValue;

		fPosLerpValue = pMotionData->pKeyFrames[lKey1].clPosYInterBezier.getInterValue( fLerpValue );
		pvec3Pos->y = pMotionData->pKeyFrames[lKey0].vec3Position.y * (1.0f - fPosLerpValue) + pMotionData->pKeyFrames[lKey1].vec3Position.y * fPosLerpValue;

		fPosLerpValue = pMotionData->pKeyFrames[lKey1].clPosZInterBezier.getInterValue( fLerpValue );
		pvec3Pos->z = pMotionData->pKeyFrames[lKey0].vec3Position.z * (1.0f - fPosLerpValue) + pMotionData->pKeyFrames[lKey1].vec3Position.z * fPosLerpValue;


		float	fRotLerpValue = pMotionData->pKeyFrames[lKey1].clRotInterBezier.getInterValue( fLerpValue );
		QuaternionSlerp( pvec4Rot, &(pMotionData->pKeyFrames[lKey0].vec4Rotation), &(pMotionData->pKeyFrames[lKey1].vec4Rotation), fRotLerpValue );
	}
	else
	{
		*pvec3Pos = pMotionData->pKeyFrames[lKey0].vec3Position;
		*pvec4Rot = pMotionData->pKeyFrames[lKey0].vec4Rotation;
	}
}

//--------------------------------------------
// キーフレームを補間して表情ブレンド率を返す
//--------------------------------------------
float cMotionPlayer::getFaceRate( const FaceDataList *pFaceData, float fFrame )
{
	unsigned long	i;
	unsigned long	ulNumKeyFrame = pFaceData->ulNumKeyFrames;

	// 最終フレームを過ぎていた場合
	if( fFrame > pFaceData->pKeyFrames[ulNumKeyFrame - 1].fFrameNo )
	{
		fFrame = pFaceData->pKeyFrames[ulNumKeyFrame - 1].fFrameNo;
	}

	// 現在の時間がどのキー近辺にあるか
	for( i = 0 ; i < ulNumKeyFrame ; i++ )
	{
		if( fFrame <= pFaceData->pKeyFrames[i].fFrameNo )
		{
			break;
		}
	}

	// 前後のキーを設定
	long	lKey0,
			lKey1;

	lKey0 = i - 1;
	lKey1 = i;

	if( lKey0 <= 0 )			lKey0 = 0;
	if( i == ulNumKeyFrame )	lKey1 = ulNumKeyFrame - 1;

	// 前後のキーの時間
	float	fTime0 = pFaceData->pKeyFrames[lKey0].fFrameNo;
	float	fTime1 = pFaceData->pKeyFrames[lKey1].fFrameNo;

	// 前後のキーの間でどの位置にいるか
	float	fLerpValue;
	if( lKey0 != lKey1 )
	{
		fLerpValue = (fFrame - fTime0) / (fTime1 - fTime0);
		return (pFaceData->pKeyFrames[lKey0].fRate * (1.0f - fLerpValue)) + (pFaceData->pKeyFrames[lKey1].fRate * fLerpValue);
	}
	else
	{
		return pFaceData->pKeyFrames[lKey0].fRate;
	}
}

//==================================
// 設定されたモーションをクリアする
//==================================
void cMotionPlayer::clear( void )
{
	m_pVMDMotion = NULL;

	if( m_ppBoneList )
	{
		delete [] m_ppBoneList;
		m_ppBoneList = NULL;
	}

	if( m_ppFaceList )
	{
		delete [] m_ppFaceList;
		m_ppFaceList = NULL;
	}
}
