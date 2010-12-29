//*********
// PMD剛体
//*********

#ifndef		_PMDRIGIDBODY_H_
#define		_PMDRIGIDBODY_H_

#include	"PMDTypes.h"
#include	"PMDBone.h"
#include	"../BulletPhysics/BulletPhysics.h"

class cPMDRigidBody
{
	private :
		btCollisionShape	*m_pbtColShape;
		btRigidBody			*m_pbtRigidBody;

		btTransform			m_bttrBoneOffset;
		btTransform			m_bttrInvBoneOffset;

		cPMDBone			*m_pBone;
		int					m_iType;
		bool				m_bNoCopyToBone;

		// Kinematic用モーションステート
		struct btKinematicMotionState : public btMotionState
		{
			btTransform m_graphicsWorldTrans;
			btTransform	m_BoneOffset;
			btTransform m_startWorldTrans;
			cPMDBone	*m_pBone;

			btKinematicMotionState( const btTransform& startTrans, const btTransform& boneOffset, cPMDBone *pBone )
											: m_BoneOffset( boneOffset ), m_startWorldTrans( startTrans ), m_pBone( pBone )
			{
			}

			///synchronizes world transform from user to physics
			virtual void getWorldTransform(btTransform& centerOfMassWorldTrans ) const
			{
				btTransform		bttrBoneTransform;

				bttrBoneTransform.setFromOpenGLMatrix( (float *)m_pBone->getLocalMatrix() );

				centerOfMassWorldTrans = bttrBoneTransform * m_BoneOffset;
			}

			///synchronizes world transform from physics to user
			///Bullet only calls the update of worldtransform for active objects
			virtual void setWorldTransform( const btTransform& centerOfMassWorldTrans )
			{
				m_graphicsWorldTrans = centerOfMassWorldTrans;
			}
		};

	public :
		cPMDRigidBody( void );
		~cPMDRigidBody( void );

		bool initialize( const PMD_RigidBody *pPMDRigidBody, cPMDBone *pBone );

		void fixPosition( float fElapsedFrame );
		void updateBoneTransform( void );
		void moveToBonePos( void );

		void release( void );

		btRigidBody *getRigidBody( void ){ return m_pbtRigidBody; }
};

#endif	// _PMDRIGIDBODY_H_
