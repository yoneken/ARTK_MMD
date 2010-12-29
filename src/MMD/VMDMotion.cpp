//***************
// VMDモーション
//***************

#include	<stdio.h>
#include	<stdlib.h>
#ifndef __APPLE__
#include <malloc.h>
#else
#include <stdlib.h>
#endif
#include	<string.h>
#include	"VMDMotion.h"


//------------------------------
// ボーンキーフレームソート用比較関数
//------------------------------
static int boneCompareFunc( const void *pA, const void *pB )
{
	return (int)(((BoneKeyFrame *)pA)->fFrameNo - ((BoneKeyFrame *)pB)->fFrameNo);
}

//------------------------------
// 表情キーフレームソート用比較関数
//------------------------------
static int faceCompareFunc( const void *pA, const void *pB )
{
	return (int)(((FaceKeyFrame *)pA)->fFrameNo - ((FaceKeyFrame *)pB)->fFrameNo);
}

//================
// コンストラクタ
//================
cVMDMotion::cVMDMotion( void ) : m_pMotionDataList( NULL ), m_pFaceDataList( NULL )
{
}

//==============
// デストラクタ
//==============
cVMDMotion::~cVMDMotion( void )
{
	release();
}

//====================
// ファイルの読み込み
//====================
bool cVMDMotion::load( const char *szFilePath )
{
	FILE	*pFile;
	long	lFileSize;
	unsigned char
			*pData;

	pFile = fopen( szFilePath, "rb" );
	if( !pFile )	return false;	// ファイルが開けない

	// ファイルサイズ取得
	fseek( pFile, 0, SEEK_END );
	lFileSize = ftell( pFile );

	// メモリ確保
	pData = (unsigned char *)malloc( lFileSize );

	// 読み込み
	fseek( pFile, 0, SEEK_SET );
	fread( pData, 1, lFileSize, pFile );

	fclose( pFile );

	// モーションデータ初期化
	bool	bRet = initialize( pData );

	free( pData );

	return bRet;
}

//==========================
// モーションデータの初期化
//==========================
bool cVMDMotion::initialize( unsigned char *pData )
{
	release();

	// ヘッダのチェック
	VMD_Header	*pVMDHeader = (VMD_Header *)pData;
	if( strncmp( pVMDHeader->szHeader, "Vocaloid Motion Data 0002", 30 ) != 0 )
		return false;	// ファイル形式が違う

	pData += sizeof( VMD_Header );

	//-----------------------------------------------------
	// ボーンのキーフレーム数を取得
	unsigned int	ulNumBoneKeyFrames = *((unsigned int *)pData);
	pData += sizeof( unsigned int );

	// まずはモーションデータ中のボーンごとのキーフレーム数をカウント
	VMD_Motion		*pVMDMotion = (VMD_Motion *)pData;
	MotionDataList	*pMotTemp;

	m_fMaxFrame = 0.0f;
	for( unsigned long i = 0 ; i < ulNumBoneKeyFrames ; i++, pVMDMotion++ )
	{
		if( m_fMaxFrame < (float)pVMDMotion->ulFrameNo )	m_fMaxFrame = (float)pVMDMotion->ulFrameNo;	// 最大フレーム更新

		pMotTemp = m_pMotionDataList;

		while( pMotTemp )
		{
			if( strncmp( pMotTemp->szBoneName, pVMDMotion->szBoneName, 15 ) == 0 )
			{
				// リストに追加済みのボーン
				pMotTemp->ulNumKeyFrames++;
				break;
			}
			pMotTemp = pMotTemp->pNext;
		}

		if( !pMotTemp )
		{
			// リストにない場合は新規ノードを追加
			MotionDataList	*pNew = new MotionDataList;

			strncpy( pNew->szBoneName, pVMDMotion->szBoneName, 15 );	pNew->szBoneName[15] = '\0';
			pNew->ulNumKeyFrames = 1;

			pNew->pNext = m_pMotionDataList;
			m_pMotionDataList = pNew;
		}
	}

	// キーフレーム配列を確保
	pMotTemp = m_pMotionDataList;
	m_ulNumMotionNodes = 0;
	while( pMotTemp )
	{
		pMotTemp->pKeyFrames = new BoneKeyFrame[pMotTemp->ulNumKeyFrames];
		pMotTemp->ulNumKeyFrames = 0;		// 配列インデックス用にいったん0にする
		pMotTemp = pMotTemp->pNext;

		m_ulNumMotionNodes++;
	}
	
	// ボーンごとにキーフレームを格納
	pVMDMotion = (VMD_Motion *)pData;

	for( unsigned long i = 0 ; i < ulNumBoneKeyFrames ; i++, pVMDMotion++ )
	{
		pMotTemp = m_pMotionDataList;

		while( pMotTemp )
		{
			if( strncmp( pMotTemp->szBoneName, pVMDMotion->szBoneName, 15 ) == 0 )
			{
				BoneKeyFrame	*pKeyFrame = &(pMotTemp->pKeyFrames[pMotTemp->ulNumKeyFrames]);

				pKeyFrame->fFrameNo     = (float)pVMDMotion->ulFrameNo;
				pKeyFrame->vec3Position = pVMDMotion->vec3Position;
				QuaternionNormalize( &pKeyFrame->vec4Rotation, &pVMDMotion->vec4Rotation );

				pKeyFrame->clPosXInterBezier.initialize( pVMDMotion->cInterpolationX[0], pVMDMotion->cInterpolationX[4], pVMDMotion->cInterpolationX[8], pVMDMotion->cInterpolationX[12] );
				pKeyFrame->clPosYInterBezier.initialize( pVMDMotion->cInterpolationY[0], pVMDMotion->cInterpolationY[4], pVMDMotion->cInterpolationY[8], pVMDMotion->cInterpolationY[12] );
				pKeyFrame->clPosZInterBezier.initialize( pVMDMotion->cInterpolationZ[0], pVMDMotion->cInterpolationZ[4], pVMDMotion->cInterpolationZ[8], pVMDMotion->cInterpolationZ[12] );
				pKeyFrame->clRotInterBezier.initialize( pVMDMotion->cInterpolationRot[0], pVMDMotion->cInterpolationRot[4], pVMDMotion->cInterpolationRot[8], pVMDMotion->cInterpolationRot[12] );

				pMotTemp->ulNumKeyFrames++;

				break;
			}
			pMotTemp = pMotTemp->pNext;
		}
	}

	// キーフレーム配列を昇順にソート
	pMotTemp = m_pMotionDataList;

	while( pMotTemp )
	{
		qsort( pMotTemp->pKeyFrames, pMotTemp->ulNumKeyFrames, sizeof( BoneKeyFrame ), boneCompareFunc );
		pMotTemp = pMotTemp->pNext;
	}

	pData += sizeof( VMD_Motion ) * ulNumBoneKeyFrames;

	//-----------------------------------------------------
	// 表情のキーフレーム数を取得
	unsigned int	ulNumFaceKeyFrames = *((unsigned int *)pData);
	pData += sizeof( unsigned int );

	// モーションデータ中の表情ごとのキーフレーム数をカウント
	VMD_Face		*pVMDFace = (VMD_Face *)pData;
	FaceDataList	*pFaceTemp;

	for( unsigned long i = 0 ; i < ulNumFaceKeyFrames ; i++, pVMDFace++ )
	{
		if( m_fMaxFrame < (float)pVMDFace->ulFrameNo )	m_fMaxFrame = (float)pVMDFace->ulFrameNo;	// 最大フレーム更新

		pFaceTemp = m_pFaceDataList;

		while( pFaceTemp )
		{
			if( strncmp( pFaceTemp->szFaceName, pVMDFace->szFaceName, 15 ) == 0 )
			{
				// リストに追加済み
				pFaceTemp->ulNumKeyFrames++;
				break;
			}
			pFaceTemp = pFaceTemp->pNext;
		}

		if( !pFaceTemp )
		{
			// リストにない場合は新規ノードを追加
			FaceDataList	*pNew = new FaceDataList;

			strncpy( pNew->szFaceName, pVMDFace->szFaceName, 15 );	pNew->szFaceName[15] = '\0';
			pNew->ulNumKeyFrames = 1;

			pNew->pNext = m_pFaceDataList;
			m_pFaceDataList = pNew;
		}
	}

	// キーフレーム配列を確保
	pFaceTemp = m_pFaceDataList;
	m_ulNumFaceNodes = 0;
	while( pFaceTemp )
	{
		pFaceTemp->pKeyFrames = new FaceKeyFrame[pFaceTemp->ulNumKeyFrames];
		pFaceTemp->ulNumKeyFrames = 0;		// 配列インデックス用にいったん0にする
		pFaceTemp = pFaceTemp->pNext;

		m_ulNumFaceNodes++;
	}
	
	// 表情ごとにキーフレームを格納
	pVMDFace = (VMD_Face *)pData;

	for( unsigned long i = 0 ; i < ulNumFaceKeyFrames ; i++, pVMDFace++ )
	{
		pFaceTemp = m_pFaceDataList;

		while( pFaceTemp )
		{
			if( strncmp( pFaceTemp->szFaceName, pVMDFace->szFaceName, 15 ) == 0 )
			{
				FaceKeyFrame	*pKeyFrame = &(pFaceTemp->pKeyFrames[pFaceTemp->ulNumKeyFrames]);

				pKeyFrame->fFrameNo = (float)pVMDFace->ulFrameNo;
				pKeyFrame->fRate    =        pVMDFace->fFactor;

				pFaceTemp->ulNumKeyFrames++;

				break;
			}
			pFaceTemp = pFaceTemp->pNext;
		}
	}

	// キーフレーム配列を昇順にソート
	pFaceTemp = m_pFaceDataList;

	while( pFaceTemp )
	{
		qsort( pFaceTemp->pKeyFrames, pFaceTemp->ulNumKeyFrames, sizeof( FaceKeyFrame ), faceCompareFunc );
		pFaceTemp = pFaceTemp->pNext;
	}

	return true;
}

//======
// 解放
//======
void cVMDMotion::release( void )
{
	// モーションデータの解放
	MotionDataList	*pMotTemp = m_pMotionDataList,
					*pNextMotTemp;

	while( pMotTemp )
	{
		pNextMotTemp = pMotTemp->pNext;

		delete [] pMotTemp->pKeyFrames;

		delete pMotTemp;

		pMotTemp = pNextMotTemp;
	}

	m_pMotionDataList = NULL;

	// 表情データの解放
	FaceDataList	*pFaceTemp = m_pFaceDataList,
					*pNextFaceTemp;

	while( pFaceTemp )
	{
		pNextFaceTemp = pFaceTemp->pNext;

		delete [] pFaceTemp->pKeyFrames;

		delete pFaceTemp;

		pFaceTemp = pNextFaceTemp;
	}

	m_pFaceDataList = NULL;
}
