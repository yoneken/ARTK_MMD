//***********
// PMDモデル
//***********

#include	<stdio.h>
#include	<string.h>
#ifdef	_WIN32
#include	<gl/glut.h>
#include	<malloc.h>
#else
#ifdef __APPLE__
#include	<GLUT/glut.h>
#include	<stdlib.h>
#else
#include	<GL/glut.h>
#include	<malloc.h>
#endif
#endif
#include	"PMDModel.h"
#include	"TextureList.h"

#define	_DBG_BONE_DRAW		(0)
#define	_DBG_IK_DRAW		(0)
#define	_DBG_TEXTURE_DRAW	(0)

extern cTextureList		g_clsTextureList;


//--------------------------
// IKデータソート用比較関数
//--------------------------
static int compareFunc( const void *pA, const void *pB )
{
	return (int)(((cPMDIK *)pA)->getSortVal() - ((cPMDIK *)pB)->getSortVal());
}

//================
// コンストラクタ
//================
cPMDModel::cPMDModel( void ) : m_pvec3OrgPositionArray( NULL ), m_pvec3OrgNormalArray( NULL ), m_puvOrgTexureUVArray( NULL ),
									m_pOrgSkinInfoArray( NULL ), m_pvec3PositionArray( NULL ), m_pvec3NormalArray( NULL ), m_pIndices( NULL ),
										m_pMaterials( NULL ), m_pBoneArray( NULL ), m_pNeckBone( NULL), m_pIKArray( NULL ), m_pFaceArray( NULL ),
											m_pRigidBodyArray( NULL ), m_pConstraintArray( NULL )
{
}

//==============
// デストラクタ
//==============
cPMDModel::~cPMDModel( void )
{
	release();
}

//====================
// ファイルの読み込み
//====================
bool cPMDModel::load( const char *szFilePath )
{
	FILE	*pFile;
	long lFileSize;
	unsigned char *pData;

	pFile = fopen( szFilePath, "rb" );
	if( !pFile )	return false;	// ファイルが開けない

	// ファイルサイズ取得
	fseek( pFile, 0, SEEK_END );
	lFileSize = ftell(pFile);

	// メモリ確保
	pData = (unsigned char *)malloc( (size_t)lFileSize );

	// 読み込み
	fseek( pFile, 0, SEEK_SET );
	fread( pData, 1, (size_t)lFileSize, pFile );

	fclose( pFile );

	// モデルデータ初期化
	bool	bRet = initialize( szFilePath, pData, lFileSize );

	free( pData );

	return bRet;
}

//======================
// モデルデータの初期化
//======================
bool cPMDModel::initialize( const char *szFilePath, unsigned char *pData, unsigned long ulDataSize )
{
	release();
	m_clMotionPlayer.clear();

	m_bLookAt = false;

	unsigned char *pDataTop = pData;

	// ヘッダのチェック
	PMD_Header	*pPMDHeader = (PMD_Header *)pData;
	if( pPMDHeader->szMagic[0] != 'P' || pPMDHeader->szMagic[1] != 'm' || pPMDHeader->szMagic[2] != 'd' || pPMDHeader->fVersion != 1.0f )
		return false;	// ファイル形式が違う

	// get file path
	char filePath[256];
	int filePathLength = strrchr(szFilePath, '/')-szFilePath+1;
	strncpy(filePath, szFilePath, filePathLength);
	filePath[filePathLength] = 0x0;

	// モデル名のコピー
	strncpy( m_szModelName, pPMDHeader->szName, 20 );
	m_szModelName[20] = '\0';

	pData += sizeof( PMD_Header );

	unsigned long	ulSize;

	//-----------------------------------------------------
	// 頂点数取得
	m_ulNumVertices = *((unsigned int *)pData);
	pData += sizeof( unsigned int );

	// 頂点配列をコピー
	m_pvec3OrgPositionArray =  (Vector3 *)malloc( sizeof(Vector3)  * m_ulNumVertices );
	m_pvec3OrgNormalArray   =  (Vector3 *)malloc( sizeof(Vector3)  * m_ulNumVertices );
	m_puvOrgTexureUVArray   =    (TexUV *)malloc( sizeof(TexUV)    * m_ulNumVertices );
	m_pOrgSkinInfoArray     = (SkinInfo *)malloc( sizeof(SkinInfo) * m_ulNumVertices ); 

	PMD_Vertex	*pVertex = (PMD_Vertex *)pData;
	for( unsigned long i = 0 ; i < m_ulNumVertices ; i++, pVertex++ )
	{
		m_pvec3OrgPositionArray[i] =  pVertex->vec3Pos;
		m_pvec3OrgNormalArray[i]   =  pVertex->vec3Normal;
		m_puvOrgTexureUVArray[i]   =  pVertex->uvTex;

		m_pOrgSkinInfoArray[i].fWeight     = pVertex->cbWeight / 100.0f; 
		m_pOrgSkinInfoArray[i].unBoneNo[0] = pVertex->unBoneNo[0]; 
		m_pOrgSkinInfoArray[i].unBoneNo[1] = pVertex->unBoneNo[1]; 
	}

	ulSize = sizeof( PMD_Vertex ) * m_ulNumVertices;
	pData += ulSize;

	// スキニング用頂点配列作成
	m_pvec3PositionArray = (Vector3 *)malloc( sizeof(Vector3) * m_ulNumVertices );
	m_pvec3NormalArray   = (Vector3 *)malloc( sizeof(Vector3) * m_ulNumVertices );

	//-----------------------------------------------------
	// 頂点インデックス数取得
	m_ulNumIndices = *((unsigned int *)pData);
	pData += sizeof( unsigned int );

	// 頂点インデックス配列をコピー
	ulSize = sizeof( unsigned short ) * m_ulNumIndices;
	m_pIndices = (unsigned short *)malloc( ulSize );

	memcpy( m_pIndices, pData, ulSize );
	pData += ulSize;

	//-----------------------------------------------------
	// マテリアル数取得
	m_ulNumMaterials = *((unsigned int *)pData);
	pData += sizeof( unsigned int );

	// マテリアル配列をコピー
	m_pMaterials = (Material *)malloc( sizeof(Material) * m_ulNumMaterials );

	PMD_Material	*pPMDMaterial = (PMD_Material *)pData;
	for( unsigned long i = 0 ; i < m_ulNumMaterials ; i++ )
	{
		m_pMaterials[i].col4Diffuse = pPMDMaterial->col4Diffuse;

		m_pMaterials[i].col4Specular.r = pPMDMaterial->col3Specular.r;
		m_pMaterials[i].col4Specular.g = pPMDMaterial->col3Specular.g;
		m_pMaterials[i].col4Specular.b = pPMDMaterial->col3Specular.b;
		m_pMaterials[i].col4Specular.a = 1.0f;

		m_pMaterials[i].col4Ambient.r = pPMDMaterial->col3Ambient.r;
		m_pMaterials[i].col4Ambient.g = pPMDMaterial->col3Ambient.g;
		m_pMaterials[i].col4Ambient.b = pPMDMaterial->col3Ambient.b;
		m_pMaterials[i].col4Ambient.a = 1.0f;

		m_pMaterials[i].fShininess = pPMDMaterial->fShininess;
		m_pMaterials[i].ulNumIndices = pPMDMaterial->ulNumIndices;

		if( pPMDMaterial->szTextureFileName[0] )
		{
			filePath[filePathLength] = 0x0;
			strcat(&filePath[filePathLength], pPMDMaterial->szTextureFileName);
			unsigned int dbg_num = g_clsTextureList.getTexture( filePath );
			m_pMaterials[i].uiTexID = dbg_num;
			m_pMaterials[i].bSphereMap = isSphereMapTexName( pPMDMaterial->szTextureFileName );
		}
		else
		{
			m_pMaterials[i].uiTexID = 0xFFFFFFFF;
			m_pMaterials[i].bSphereMap = false;
		}

		pPMDMaterial++;
	}

	pData += sizeof(PMD_Material) * m_ulNumMaterials;

	//-----------------------------------------------------
	// ボーン数取得
	m_unNumBones = *((unsigned short *)pData);
	pData += sizeof( unsigned short );

	// ボーン配列を作成
	m_pBoneArray = new cPMDBone[m_unNumBones];

	for( unsigned short i = 0 ; i < m_unNumBones ; i++ )
	{
		m_pBoneArray[i].initialize( (const PMD_Bone *)pData, m_pBoneArray );

		// 首のボーンを保存
		if( strncmp( m_pBoneArray[i].getName(), "頭", 20 ) == 0 )	m_pNeckBone = &m_pBoneArray[i];

		pData += sizeof( PMD_Bone );	
	}
	for( unsigned short i = 0 ; i < m_unNumBones ; i++ )	m_pBoneArray[i].recalcOffset();

	//-----------------------------------------------------
	// IK数取得
	m_unNumIK = *((unsigned short *)pData);
	pData += sizeof( unsigned short );

	// IK配列を作成
	if( m_unNumIK )
	{
		m_pIKArray = new cPMDIK[m_unNumIK];

		for( unsigned short i = 0 ; i < m_unNumIK ; i++ )
		{
			m_pIKArray[i].initialize( (const PMD_IK *)pData, m_pBoneArray );
			pData += sizeof( PMD_IK ) + sizeof(unsigned short) * (((PMD_IK *)pData)->cbNumLink - 1);
		}
		qsort( m_pIKArray, m_unNumIK, sizeof(cPMDIK), compareFunc );
	}

	//-----------------------------------------------------
	// 表情数取得
	m_unNumFaces = *((unsigned short *)pData);
	pData += sizeof( unsigned short );

	// 表情配列を作成
	if( m_unNumFaces )
	{
		m_pFaceArray = new cPMDFace[m_unNumFaces];

		for( unsigned short i = 0 ; i < m_unNumFaces ; i++ )
		{
			m_pFaceArray[i].initialize( (const PMD_Face *)pData, &m_pFaceArray[0] );
			pData += sizeof( PMD_Face ) + sizeof(PMD_FaceVtx) * (((PMD_Face *)pData)->ulNumVertices - 1);
		}
	}

	//-----------------------------------------------------
	// ここから剛体情報まで読み飛ばし

	// 表情枠用表示リスト
	unsigned char	cbFaceDispNum = *((unsigned char *)pData);
	pData += sizeof( unsigned char );

	pData += sizeof( unsigned short ) * cbFaceDispNum;

	// ボーン枠用枠名リスト
	unsigned char	cbBoneDispNum = *((unsigned char *)pData);
	pData += sizeof( unsigned char );

	pData += sizeof( char ) * 50 * cbBoneDispNum;

	// ボーン枠用表示リスト
	unsigned int	ulBoneFrameDispNum = *((unsigned int *)pData);
	pData += sizeof( unsigned int );

	pData += 3 * ulBoneFrameDispNum;

	// 後続データの有無をチェック
	if( (unsigned long)pData - (unsigned long)pDataTop >= ulDataSize )	return true;

	// 英語名対応
	unsigned char	cbEnglishNameExist = *((unsigned char *)pData);
	pData += sizeof( unsigned char );

	if( cbEnglishNameExist )
	{
		// モデル名とコメント(英語)
		pData += sizeof( char ) * 276;

		// ボーン名(英語)
		pData += sizeof( char ) * 20 * m_unNumBones;

		// 表情名(英語)
		pData += sizeof( char ) * 20 * (m_unNumFaces - 1);	// "base"は英語名リストには含まれない

		// ボーン枠用枠名リスト(英語)
		pData += sizeof( char ) * 50 * (cbBoneDispNum);
	}

	// 後続データの有無をチェック(ここのチェックは不要かも)
	if( (unsigned long)pData - (unsigned long)pDataTop >= ulDataSize )	return true;

	// トゥーンテクスチャリスト
	pData += 100 * 10;		// ファイル名 100Byte * 10個

	// ここまで読み飛ばし
	//-----------------------------------------------------

	// 後続データの有無をチェック
	if( (unsigned long)pData - (unsigned long)pDataTop >= ulDataSize )	return true;

	//-----------------------------------------------------
	// 剛体数取得
	m_ulRigidBodyNum = *((unsigned int *)pData);
	pData += sizeof( unsigned int );

	// 剛体配列を作成
	if( m_ulRigidBodyNum )
	{
		m_pRigidBodyArray = new cPMDRigidBody[m_ulRigidBodyNum];

		for( unsigned long i = 0 ; i < m_ulRigidBodyNum ; i++ )
		{
			const PMD_RigidBody *pPMDRigidBody = (const PMD_RigidBody *)pData;

			cPMDBone	*pBone = NULL;
			if( pPMDRigidBody->unBoneIndex == 0xFFFF )	pBone = getBoneByName( "センター" );
			else										pBone = &m_pBoneArray[pPMDRigidBody->unBoneIndex];

			m_pRigidBodyArray[i].initialize( pPMDRigidBody, pBone );
			pData += sizeof( PMD_RigidBody );
		}
	}

	//-----------------------------------------------------
	// コンストレイント数取得
	m_ulConstraintNum = *((unsigned int *)pData);
	pData += sizeof( unsigned int );

	// コンストレイント配列を作成
	if( m_ulConstraintNum )
	{
		m_pConstraintArray = new cPMDConstraint[m_ulConstraintNum];

		for( unsigned long i = 0 ; i < m_ulConstraintNum ; i++ )
		{
			const PMD_Constraint *pPMDConstraint = (const PMD_Constraint *)pData;

			cPMDRigidBody	*pRigidBodyA = &m_pRigidBodyArray[pPMDConstraint->ulRigidA],
							*pRigidBodyB = &m_pRigidBodyArray[pPMDConstraint->ulRigidB];

			m_pConstraintArray[i].initialize( pPMDConstraint, pRigidBodyA, pRigidBodyB );
			pData += sizeof( PMD_Constraint );
		}
	}

	return true;
}

//----------------------------------------------
// スフィアマップ用のテクスチャ名かどうかを返す
//----------------------------------------------
bool cPMDModel::isSphereMapTexName( const char *szTextureName )
{
	int		iLen = strlen( szTextureName );
	bool	bRet = false;

	if( 	(szTextureName[iLen - 3] == 's' || szTextureName[iLen - 3] == 'S') &&
			(szTextureName[iLen - 2] == 'p' || szTextureName[iLen - 2] == 'P') &&
			(szTextureName[iLen - 1] == 'h' || szTextureName[iLen - 1] == 'H')		)
	{
		bRet = true;
	}

	return bRet;
}

//======================
// 指定名のボーンを返す
//======================
cPMDBone *cPMDModel::getBoneByName( const char *szBoneName )
{
	cPMDBone *pBoneArray = m_pBoneArray;
	for( unsigned long i = 0 ; i < m_unNumBones ; i++, pBoneArray++ )
	{
		if( strncmp( pBoneArray->getName(), szBoneName, 20 ) == 0 )
			return pBoneArray;
	}

	return NULL;
}

//====================
// 指定名の表情を返す
//====================
cPMDFace *cPMDModel::getFaceByName( const char *szFaceName )
{
	cPMDFace *pFaceArray = m_pFaceArray;
	for( unsigned long i = 0 ; i < m_unNumFaces ; i++, pFaceArray++ )
	{
		if( strncmp( pFaceArray->getName(), szFaceName, 20 ) == 0 )
			return pFaceArray;
	}

	return NULL;
}

//====================
// モーションのセット
//====================
void cPMDModel::setMotion( cVMDMotion *pVMDMotion, bool bLoop, float fInterpolateFrame )
{
	m_clMotionPlayer.setup( this, pVMDMotion, bLoop, fInterpolateFrame );
}

//==========================================
// 剛体の位置を現在のボーンの位置にあわせる
//==========================================
void cPMDModel::resetRigidBodyPos( void )
{
	for( unsigned long i = 0 ; i < m_ulRigidBodyNum ; i++ )
	{
		m_pRigidBodyArray[i].moveToBonePos();
	}
}

//==============================
// モーションによるボーンの更新
//==============================
bool cPMDModel::updateMotion( float fElapsedFrame )
{
	bool	bMotionEnd = false;

	// モーション更新前に表情をリセット
	if( m_pFaceArray )	m_pFaceArray[0].setFace( m_pvec3OrgPositionArray );

	// モーション更新
	bMotionEnd = m_clMotionPlayer.update( fElapsedFrame );

	// ボーン行列の更新
	for( unsigned short i = 0 ; i < m_unNumBones ; i++ )
	{
		m_pBoneArray[i].updateMatrix();
	}

	// ボーン位置あわせ
	for( unsigned long i = 0 ; i < m_ulRigidBodyNum ; i++ )
	{
		m_pRigidBodyArray[i].fixPosition( fElapsedFrame );
	}

	// IKの更新
	for( unsigned short i = 0 ; i < m_unNumIK ; i++ )
	{
		m_pIKArray[i].update();
	}

	return bMotionEnd;
}

//================================
// 首のボーンをターゲットに向ける
//================================
void cPMDModel::updateNeckBone( const Vector3 *pvec3LookTarget, float fLimitXD, float fLimitXU, float fLimitY )
{
	if( m_pNeckBone && m_bLookAt )
	{
		m_pNeckBone->lookAt( pvec3LookTarget, fLimitXD, fLimitXU, fLimitY );

		unsigned short i;
		for( i = 0 ; i < m_unNumBones ; i++ )
		{
			if( m_pNeckBone == &m_pBoneArray[i] )	break;
		}

		for( ; i < m_unNumBones ; i++ )
		{
			m_pBoneArray[i].updateMatrix();
		}
	}
}

//====================
// 頂点スキニング処理
//====================
void cPMDModel::updateSkinning( void )
{
	// 物理演算反映
	for( unsigned long i = 0 ; i < m_ulRigidBodyNum ; i++ )
	{
		m_pRigidBodyArray[i].updateBoneTransform();
	}

	// スキニング用行列の更新
	for( unsigned short i = 0 ; i < m_unNumBones ; i++ )
	{
		m_pBoneArray[i].updateSkinningMat();
	}

	// 頂点スキニング
	Matrix	matTemp;
	for( unsigned long i = 0 ; i < m_ulNumVertices ; i++ )
	{
		if( m_pOrgSkinInfoArray[i].fWeight == 0.0f )
		{
			Vector3Transform( &m_pvec3PositionArray[i], &m_pvec3OrgPositionArray[i], m_pBoneArray[m_pOrgSkinInfoArray[i].unBoneNo[1]].m_matSkinning );
			Vector3Rotate( &m_pvec3NormalArray[i], &m_pvec3OrgNormalArray[i], m_pBoneArray[m_pOrgSkinInfoArray[i].unBoneNo[1]].m_matSkinning );
		}
		else if( m_pOrgSkinInfoArray[i].fWeight >= 0.9999f )
		{
			Vector3Transform( &m_pvec3PositionArray[i], &m_pvec3OrgPositionArray[i], m_pBoneArray[m_pOrgSkinInfoArray[i].unBoneNo[0]].m_matSkinning );
			Vector3Rotate( &m_pvec3NormalArray[i], &m_pvec3OrgNormalArray[i], m_pBoneArray[m_pOrgSkinInfoArray[i].unBoneNo[0]].m_matSkinning );
		}
		else
		{
			MatrixLerp( matTemp,	m_pBoneArray[m_pOrgSkinInfoArray[i].unBoneNo[0]].m_matSkinning,
									m_pBoneArray[m_pOrgSkinInfoArray[i].unBoneNo[1]].m_matSkinning,		m_pOrgSkinInfoArray[i].fWeight );

			Vector3Transform( &m_pvec3PositionArray[i], &m_pvec3OrgPositionArray[i], matTemp );
			Vector3Rotate( &m_pvec3NormalArray[i], &m_pvec3OrgNormalArray[i], matTemp );
		}
	}
}

//====================
// モデルデータの描画
//====================
void cPMDModel::render( void )
{
	if( !m_pvec3OrgPositionArray )	return;

	unsigned short	*pIndices = m_pIndices;

	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	// 頂点座標、法線、テクスチャ座標の各配列をセット
	glVertexPointer( 3, GL_FLOAT, 0, m_pvec3PositionArray );
	glNormalPointer( GL_FLOAT, 0, m_pvec3NormalArray );
	glTexCoordPointer( 2, GL_FLOAT, 0, m_puvOrgTexureUVArray );

	glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
	glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
	glDisable( GL_CULL_FACE );

	for( unsigned long i = 0 ; i < m_ulNumMaterials ; i++ )
	{
		// マテリアル設定
		glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE,  (float *)&(m_pMaterials[i].col4Diffuse)  );
		glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT,  (float *)&(m_pMaterials[i].col4Ambient)  );
		glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, (float *)&(m_pMaterials[i].col4Specular) );
		glMaterialf(  GL_FRONT_AND_BACK, GL_SHININESS, m_pMaterials[i].fShininess );

		//if( m_pMaterials[i].col4Diffuse.a < 1.0f )	glDisable( GL_CULL_FACE );
		//else											glEnable( GL_CULL_FACE );

		if( m_pMaterials[i].uiTexID != 0xFFFFFFFF )
		{
			// テクスチャありならBindする
			glEnable( GL_TEXTURE_2D );
			unsigned int dbg_num = m_pMaterials[i].uiTexID;
			glBindTexture( GL_TEXTURE_2D, dbg_num );

			if( m_pMaterials[i].bSphereMap )
			{
				glEnable( GL_TEXTURE_GEN_S );
				glEnable( GL_TEXTURE_GEN_T );
			}
			else
			{
				glDisable( GL_TEXTURE_GEN_S );
				glDisable( GL_TEXTURE_GEN_T );
			}
		}
		else
		{
			// テクスチャなし
			glDisable( GL_TEXTURE_2D );
		}

		unsigned short *dbg_p = (unsigned short *)malloc( m_pMaterials[i].ulNumIndices );
		memcpy( dbg_p, pIndices, m_pMaterials[i].ulNumIndices );
		free( dbg_p );

		// 頂点インデックスを指定してポリゴン描画
		glDrawElements( GL_TRIANGLES, m_pMaterials[i].ulNumIndices, GL_UNSIGNED_SHORT, pIndices );

		pIndices += m_pMaterials[i].ulNumIndices;
	}

	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_NORMAL_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );

	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );

#if	_DBG_BONE_DRAW
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_LIGHTING );

	for( unsigned short i = 0 ; i < m_unNumBones ; i++ )
	{
		m_pBoneArray[i].debugDraw();
	}

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_LIGHTING );
#endif

#if	_DBG_IK_DRAW
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_LIGHTING );

	for( unsigned short i = 0 ; i < m_unNumIK ; i++ )
	{
		m_pIKArray[i].debugDraw();
	}

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_LIGHTING );
#endif

#if	_DBG_TEXTURE_DRAW
	g_clsTextureList.debugDraw();
#endif
}

//========================
// モデルデータの影用描画
//========================
void cPMDModel::renderForShadow( void )
{
	if( !m_pvec3OrgPositionArray )	return;

	glEnableClientState( GL_VERTEX_ARRAY );

	glVertexPointer( 3, GL_FLOAT, 0, m_pvec3PositionArray );

	glDrawElements( GL_TRIANGLES, m_ulNumIndices, GL_UNSIGNED_SHORT, m_pIndices );

	glDisableClientState( GL_VERTEX_ARRAY );
}

//====================
// モデルデータの解放
//====================
void cPMDModel::release( void )
{
	if( m_pvec3OrgPositionArray )
	{
		free( m_pvec3OrgPositionArray );
		m_pvec3OrgPositionArray = NULL;
	}

	if( m_pvec3OrgNormalArray )
	{
		free( m_pvec3OrgNormalArray );
		m_pvec3OrgNormalArray = NULL;
	}

	if( m_puvOrgTexureUVArray )
	{
		free( m_puvOrgTexureUVArray );
		m_puvOrgTexureUVArray = NULL;
	}

	if( m_pOrgSkinInfoArray )
	{
		free( m_pOrgSkinInfoArray );
		m_pOrgSkinInfoArray = NULL;
	}

	if( m_pvec3PositionArray )
	{
		free( m_pvec3PositionArray );
		m_pvec3PositionArray = NULL;
	}

	if( m_pvec3NormalArray )
	{
		free( m_pvec3NormalArray );
		m_pvec3NormalArray = NULL;
	}

	if( m_pIndices )
	{
		free( m_pIndices );
		m_pIndices = NULL;
	}

	if( m_pMaterials )
	{
		for( unsigned long i = 0 ; i < m_ulNumMaterials ; i++ )
		{
			if( m_pMaterials[i].uiTexID != 0xFFFFFFFF )
				g_clsTextureList.releaseTexture( m_pMaterials[i].uiTexID );
		}

		free( m_pMaterials );
		m_pMaterials = NULL;
	}

	if( m_pBoneArray )
	{
		delete [] m_pBoneArray;
		m_pBoneArray = NULL;
		m_pNeckBone = NULL;
	}

	if( m_pIKArray )
	{
		delete [] m_pIKArray;
		m_pIKArray = NULL;
	}

	if( m_pFaceArray )
	{
		delete [] m_pFaceArray;
		m_pFaceArray = NULL;
	}

	if( m_pConstraintArray )
	{
		delete [] m_pConstraintArray;
		m_pConstraintArray = NULL;
		m_ulConstraintNum = 0;
	}

	if( m_pRigidBodyArray )
	{
		delete [] m_pRigidBodyArray;
		m_pRigidBodyArray = NULL;
		m_ulRigidBodyNum = 0;
	}
}
