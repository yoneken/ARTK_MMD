//****************
// BulletÉâÉbÉpÅ[
//****************

#ifndef		_BULLETPHYSICS_H_
#define		_BULLETPHYSICS_H_

#ifdef	_WIN32
#pragma warning( disable : 4819 )
#endif

#include	<btBulletDynamicsCommon.h>


class cBulletPhysics
{
	private :
		btDefaultCollisionConfiguration			*m_pBtCollisionConfig;
		btCollisionDispatcher					*m_pBtCollisionDispatcher;
		btAxisSweep3							*m_pBtOverlappingPairCache;
		btSequentialImpulseConstraintSolver		*m_pBtSolver;
		btDiscreteDynamicsWorld					*m_pBtWorld;

		btCollisionShape						*m_pBtGroundShape;

	public :
		cBulletPhysics( void );
		~cBulletPhysics( void );

		bool initialize( void );

		void addToWorld( btRigidBody *pbtRB, unsigned short unGroupIdx, unsigned short unGroupMask );
		void addToWorld( btTypedConstraint *pctConstraint );
		void removeFromWorld( btRigidBody *pbtRB );
		void removeFromWorld( btTypedConstraint *pctConstraint );

		void update( float fElapsedFlame );

		void release( void );
};

#endif
