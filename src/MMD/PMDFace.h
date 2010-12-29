//***************
// PMD表情データ
//***************

#ifndef		_PMDFACE_H_
#define		_PMDFACE_H_

#include	"PMDTypes.h"


class cPMDFace
{
	private :
		char			m_szName[21];		// 表情名 (0x00 終端，余白は 0xFD)

		unsigned long	m_ulNumVertices;	// 表情頂点数

		PMD_FaceVtx		*m_pFaceVertices;	// 表情頂点データ

	public :
		cPMDFace( void );
		~cPMDFace( void );

		void initialize( const PMD_Face *pPMDFaceData, const cPMDFace *pPMDFaceBase );

		void setFace( Vector3 *pvec3Vertices );
		void blendFace( Vector3 *pvec3Vertices, float fRate );

		void release( void );

		inline const char *getName( void ){ return m_szName; }
};


#endif
