//*********************************
// PMDコンストレイント(ジョイント)
//*********************************

#ifndef		_PMDCONSTRAINT_H_
#define		_PMDCONSTRAINT_H_

#include	"PMDTypes.h"
#include	"PMDRigidBody.h"
#include	"../BulletPhysics/BulletPhysics.h"


class cPMDConstraint
{
	private :
		btGeneric6DofSpringConstraint	*m_pbtcConstraint;


	public :
		cPMDConstraint( void );
		~cPMDConstraint( void );

		bool initialize( const PMD_Constraint *pPMDConstraint, cPMDRigidBody *pRigidBodyA, cPMDRigidBody *pRigidBodyB );

		void release( void );
};

#endif	// _PMDCONSTRAINT_H_
