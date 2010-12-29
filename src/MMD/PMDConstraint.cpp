//*********************************
// PMDコンストレイント(ジョイント)
//*********************************

#include	"PMDConstraint.h"

extern cBulletPhysics		g_clBulletPhysics;


//================
// コンストラクタ
//================
cPMDConstraint::cPMDConstraint( void ) : m_pbtcConstraint( NULL )
{
}

//==============
// デストラクタ
//==============
cPMDConstraint::~cPMDConstraint( void )
{
	release();
}

//========
// 初期化
//========
bool cPMDConstraint::initialize( const PMD_Constraint *pPMDConstraint, cPMDRigidBody *pRigidBodyA, cPMDRigidBody *pRigidBodyB )
{
	// 回転行列作成
	btMatrix3x3	btmRotationMat;

	btmRotationMat.setEulerZYX( pPMDConstraint->vec3Rotation.x, pPMDConstraint->vec3Rotation.y, pPMDConstraint->vec3Rotation.z );

	// コンストレイントのトランスフォームを作成
	btTransform		bttrTransform;

	bttrTransform.setIdentity();
	bttrTransform.setOrigin( btVector3( pPMDConstraint->vec3Position.x, pPMDConstraint->vec3Position.y, pPMDConstraint->vec3Position.z ) );
	bttrTransform.setBasis( btmRotationMat );

	// 剛体A,Bから見たコンストレイントのトランスフォームを作成 
	btTransform		bttrRigidAInvTransform = pRigidBodyA->getRigidBody()->getWorldTransform().inverse(),
					bttrRigidBInvTransform = pRigidBodyB->getRigidBody()->getWorldTransform().inverse();

	bttrRigidAInvTransform = bttrRigidAInvTransform * bttrTransform;
	bttrRigidBInvTransform = bttrRigidBInvTransform * bttrTransform;

	m_pbtcConstraint = new btGeneric6DofSpringConstraint( *pRigidBodyA->getRigidBody(), *pRigidBodyB->getRigidBody(),
																	bttrRigidAInvTransform, bttrRigidBInvTransform, true );

	// 各種制限パラメータのセット
	m_pbtcConstraint->setLinearLowerLimit( btVector3( pPMDConstraint->vec3PosLimitL.x, pPMDConstraint->vec3PosLimitL.y, pPMDConstraint->vec3PosLimitL.z ) );
	m_pbtcConstraint->setLinearUpperLimit( btVector3( pPMDConstraint->vec3PosLimitU.x, pPMDConstraint->vec3PosLimitU.y, pPMDConstraint->vec3PosLimitU.z ) );

	m_pbtcConstraint->setAngularLowerLimit( btVector3( pPMDConstraint->vec3RotLimitL.x, pPMDConstraint->vec3RotLimitL.y, pPMDConstraint->vec3RotLimitL.z ) );
	m_pbtcConstraint->setAngularUpperLimit( btVector3( pPMDConstraint->vec3RotLimitU.x, pPMDConstraint->vec3RotLimitU.y, pPMDConstraint->vec3RotLimitU.z ) );

		// 0 : translation X
	if( pPMDConstraint->vec3SpringPos.x != 0.0f )
	{
		m_pbtcConstraint->enableSpring( 0, true );
		m_pbtcConstraint->setStiffness( 0, pPMDConstraint->vec3SpringPos.x );
	}

		// 1 : translation Y
	if( pPMDConstraint->vec3SpringPos.y != 0.0f )
	{
		m_pbtcConstraint->enableSpring( 1, true );
		m_pbtcConstraint->setStiffness( 1, pPMDConstraint->vec3SpringPos.y );
	}

		// 2 : translation Z
	if( pPMDConstraint->vec3SpringPos.z != 0.0f )
	{
		m_pbtcConstraint->enableSpring( 2, true );
		m_pbtcConstraint->setStiffness( 2, pPMDConstraint->vec3SpringPos.z );
	}

		// 3 : rotation X (3rd Euler rotational around new position of X axis, range [-PI+epsilon, PI-epsilon] )
		// 4 : rotation Y (2nd Euler rotational around new position of Y axis, range [-PI/2+epsilon, PI/2-epsilon] )
		// 5 : rotation Z (1st Euler rotational around Z axis, range [-PI+epsilon, PI-epsilon] )
	m_pbtcConstraint->enableSpring( 3, true );	m_pbtcConstraint->setStiffness( 3, pPMDConstraint->vec3SpringRot.x );
	m_pbtcConstraint->enableSpring( 4, true );	m_pbtcConstraint->setStiffness( 4, pPMDConstraint->vec3SpringRot.y );
	m_pbtcConstraint->enableSpring( 5, true );	m_pbtcConstraint->setStiffness( 5, pPMDConstraint->vec3SpringRot.z );

	// シミュレーションワールドに追加
	g_clBulletPhysics.addToWorld( m_pbtcConstraint );

	return true;
}

//======
// 解放
//======
void cPMDConstraint::release( void )
{
	if( m_pbtcConstraint )
	{
		g_clBulletPhysics.removeFromWorld( m_pbtcConstraint );

		delete m_pbtcConstraint;
		m_pbtcConstraint = NULL;
	}
}
