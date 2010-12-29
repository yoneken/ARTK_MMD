//*******
// PMDIK
//*******

#ifndef		_PMDIK_H_
#define		_PMDIK_H_

#include	"PMDTypes.h"
#include	"PMDBone.h"

class cPMDIK
{
	private :
		cPMDBone		*m_pTargetBone;	// IKターゲットボーン
		cPMDBone		*m_pEffBone;	// IK先端ボーン

		unsigned short	m_unCount;
		float			m_fFact;
		short			m_nSortVal;

		unsigned char	m_cbNumLink;	// IKを構成するボーンの数
		cPMDBone		**m_ppBoneList;	// IKを構成するボーンの配列

		void limitAngle( Vector4 *pvecOut, const Vector4 *pvecSrc );

	public :
		cPMDIK( void );
		~cPMDIK( void );

		void initialize( const PMD_IK *pPMDIKData, cPMDBone pBoneArray[] );

		void update( void );
		void debugDraw( void );

		void release();

		inline short getSortVal( void ){ return m_nSortVal; }
};

#endif	// _PMDIK_H_
