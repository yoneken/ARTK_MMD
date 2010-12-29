//***********
// PMDボーン
//***********

#ifndef	_PMDBONE_H_
#define	_PMDBONE_H_

#include	"PMDTypes.h"


class cPMDBone
{
	private :
		char			m_szName[21];

		bool			m_bIKLimitAngle;	// IK時に角度制限をするかどうか

		Vector3			m_vec3OrgPosition;
		Vector3			m_vec3Offset;

		Matrix			m_matInvTransform;	// 初期値のボーンを原点に移動させるような行列

		const cPMDBone	*m_pParentBone;
		cPMDBone		*m_pChildBone;

		// 以下は現在の値
		Vector3			m_vec3Position;
		Vector4			m_vec4Rotation;

		Matrix			m_matLocal;

		Vector4			m_vec4LookRotation;

	public :
		Matrix			m_matSkinning;		// 頂点デフォーム用行列

		cPMDBone( void );
		~cPMDBone( void );

		void initialize( const PMD_Bone *pPMDBoneData, const cPMDBone pBoneArray[] );
		void recalcOffset( void );

		void reset( void );

		void updateMatrix( void );
		void lookAt( const Vector3 *pvecTargetPos, float fLimitXD, float fLimitXU, float fLimitY  );
		void updateSkinningMat( void );

		void debugDraw( void );

		inline const char *getName( void ){ return m_szName; }
		inline void getOrgPos( Vector3 *pvec3Out ){ *pvec3Out = m_vec3OrgPosition; }
		inline void getPos( Vector3 *pvec3Out ){ pvec3Out->x = m_matLocal[3][0]; pvec3Out->y = m_matLocal[3][1]; pvec3Out->z = m_matLocal[3][2]; }
		inline Matrix &getLocalMatrix( void ){ return m_matLocal; };

	friend class cMotionPlayer;
	friend class cPMDIK;
	friend class cPMDRigidBody;
};

#endif	// _PMDMODEL_H_
