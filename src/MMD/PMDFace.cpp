//***************
// PMD表情データ
//***************

#ifdef	_WIN32
#include	<gl/glut.h>
#include	<stdlib.h>
#else
#ifdef __APPLE__
#include	<GLUT/glut.h>
#include	<stdlib.h>
#else
#include	<GL/glut.h>
#include	<stdlib.h>
#endif
#endif
#include	<stdio.h>
#include	<string.h>
#include	"PMDFace.h"

//================
// コンストラクタ
//================
cPMDFace::cPMDFace( void ) : m_pFaceVertices( NULL )
{
}

//==============
// デストラクタ
//==============
cPMDFace::~cPMDFace( void )
{
	release();
}

//========
// 初期化
//========
void cPMDFace::initialize( const PMD_Face *pPMDFaceData, const cPMDFace *pPMDFaceBase )
{
	release();


	// 表情名のコピー
	strncpy( m_szName, pPMDFaceData->szName, 20 );
	m_szName[20] = '\0';

	// 表情頂点データのコピー
	m_ulNumVertices = pPMDFaceData->ulNumVertices;

	m_pFaceVertices = (PMD_FaceVtx *)malloc( sizeof(PMD_FaceVtx) * m_ulNumVertices );
	memcpy( m_pFaceVertices, pPMDFaceData->pVertices, sizeof(PMD_FaceVtx) * m_ulNumVertices );

	// baseとの相対インデックスを絶対インデックスに変換
	if( pPMDFaceBase && pPMDFaceBase != this )
	{
		PMD_FaceVtx	*pVertex = m_pFaceVertices;
		for( unsigned long i = 0 ; i < m_ulNumVertices ; i++, pVertex++ )
		{
			Vector3Add( &pVertex->vec3Pos, &pPMDFaceBase->m_pFaceVertices[pVertex->ulIndex].vec3Pos, &pVertex->vec3Pos );
			pVertex->ulIndex = pPMDFaceBase->m_pFaceVertices[pVertex->ulIndex].ulIndex;
		}
	}
}

//============
// 表情を適用
//============
void cPMDFace::setFace( Vector3 *pvec3Vertices )
{
	if( !m_pFaceVertices )	return;

	PMD_FaceVtx		*pVertex = m_pFaceVertices;
	for( unsigned long i = 0 ; i < m_ulNumVertices ; i++, pVertex++ )
	{
		pvec3Vertices[pVertex->ulIndex] = pVertex->vec3Pos;
	}
}

//======================
// 表情をブレンドで適用
//======================
void cPMDFace::blendFace( Vector3 *pvec3Vertices, float fRate )
{
	if( !m_pFaceVertices )	return;

	PMD_FaceVtx		*pVertex = m_pFaceVertices;
	unsigned long	ulIndex;
	for( unsigned long i = 0 ; i < m_ulNumVertices ; i++, pVertex++ )
	{
		ulIndex = pVertex->ulIndex;
		Vector3Lerp( &pvec3Vertices[ulIndex], &pvec3Vertices[ulIndex], &pVertex->vec3Pos, fRate );
	}
}

//======
// 解放
//======
void cPMDFace::release( void )
{
	if( m_pFaceVertices )
	{
		free( m_pFaceVertices );
		m_pFaceVertices = NULL;
	}
}
