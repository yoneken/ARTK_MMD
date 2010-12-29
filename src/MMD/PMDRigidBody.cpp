//*********
// PMD剛体
//*********

#include	<memory.h>
#include	"PMDRigidBody.h"

extern cBulletPhysics		g_clBulletPhysics;


//================
// コンストラクタ
//================
cPMDRigidBody::cPMDRigidBody( void ) : m_pBone( NULL ), m_pbtColShape( NULL ), m_pbtRigidBody( NULL )
{
}

//==============
// デストラクタ
//==============
cPMDRigidBody::~cPMDRigidBody( void )
{
	release();
}

//========
// 初期化
//========
bool cPMDRigidBody::initialize( const PMD_RigidBody *pPMDRigidBody, cPMDBone *pBone )
{
	// シェイプの作成
	switch( pPMDRigidBody->cbShapeType )
	{
		case 0 :	// 球
			m_pbtColShape = new btSphereShape( pPMDRigidBody->fWidth );
			break;

		case 1 :	// 箱
			m_pbtColShape = new btBoxShape( btVector3( pPMDRigidBody->fWidth, pPMDRigidBody->fHeight, pPMDRigidBody->fDepth ) );
			break;

		case 2 :	// カプセル
			m_pbtColShape = new btCapsuleShape( pPMDRigidBody->fWidth, pPMDRigidBody->fHeight );
			break;
	}

	// 質量と慣性テンソルの設定
	btScalar	btsMass( 0.0f );
	btVector3	btv3LocalInertia( 0.0f, 0.0f ,0.0f );

	// ボーン追従でない場合は質量を設定
	if( pPMDRigidBody->cbRigidBodyType != 0 )	btsMass = pPMDRigidBody->fMass;

	// 慣性テンソルの計算
	if( btsMass != 0.0f )	m_pbtColShape->calculateLocalInertia( btsMass, btv3LocalInertia );

	// ボーンの位置取得
	Vector3		vec3BonePos;
	pBone->getPos( &vec3BonePos );

	// ボーンオフセット用トランスフォーム作成
	btMatrix3x3	btmRotationMat;
	btmRotationMat.setEulerZYX( pPMDRigidBody->vec3Rotation.x, pPMDRigidBody->vec3Rotation.y, pPMDRigidBody->vec3Rotation.z );

	btTransform		bttrBoneOffset;

	bttrBoneOffset.setIdentity();
	bttrBoneOffset.setOrigin( btVector3( pPMDRigidBody->vec3Position.x, pPMDRigidBody->vec3Position.y, pPMDRigidBody->vec3Position.z ) );
	bttrBoneOffset.setBasis( btmRotationMat );

	// 剛体の初期トランスフォーム作成
	btTransform		bttrTransform;

	bttrTransform.setIdentity();
	bttrTransform.setOrigin( btVector3( vec3BonePos.x, vec3BonePos.y, vec3BonePos.z ) );
	bttrTransform = bttrTransform * bttrBoneOffset;

	// MotionStateの作成
	btMotionState	*pbtMotionState;

	switch( pPMDRigidBody->cbRigidBodyType )
	{
		case 0 : pbtMotionState = new btKinematicMotionState( bttrTransform, bttrBoneOffset, pBone );	break;
		case 1 : pbtMotionState = new btDefaultMotionState( bttrTransform );							break;
		case 2 : pbtMotionState = new btDefaultMotionState( bttrTransform );							break;
		//case 2 : pbtMotionState = new btKinematicMotionState( bttrTransform, bttrBoneOffset, pBone );	break;
	}

	// 剛体のパラメータの設定
	btRigidBody::btRigidBodyConstructionInfo	btRbInfo( btsMass, pbtMotionState, m_pbtColShape, btv3LocalInertia );
	btRbInfo.m_linearDamping  = pPMDRigidBody->fLinearDamping;	// 移動減
	btRbInfo.m_angularDamping = pPMDRigidBody->fAngularDamping;	// 回転減
	btRbInfo.m_restitution    = pPMDRigidBody->fRestitution;	// 反発力
	btRbInfo.m_friction       = pPMDRigidBody->fFriction;		// 摩擦力
	btRbInfo.m_additionalDamping = true;

	// 剛体の作成
	m_pbtRigidBody = new btRigidBody( btRbInfo );

	// Kinematic設定
	if( pPMDRigidBody->cbRigidBodyType == 0 )
	{
		m_pbtRigidBody->setCollisionFlags( m_pbtRigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT );
		m_pbtRigidBody->setActivationState( DISABLE_DEACTIVATION );
	}
	m_pbtRigidBody->setSleepingThresholds( 0.0f, 0.0f );

	// 剛体をシミュレーションワールドに追加
	g_clBulletPhysics.addToWorld( m_pbtRigidBody, 0x0001 << pPMDRigidBody->cbColGroupIndex, pPMDRigidBody->unColGroupMask );

	m_bttrBoneOffset = bttrBoneOffset;
	m_bttrInvBoneOffset = bttrBoneOffset.inverse();
	m_pBone = pBone;
	m_iType = pPMDRigidBody->cbRigidBodyType;

	if( strncmp( pBone->getName(), "センター", 20 ) == 0 )	m_bNoCopyToBone = true;
	else													m_bNoCopyToBone = false;

	return true;
}

//==================
// ボーン位置あわせ
//==================
void cPMDRigidBody::fixPosition( float fElapsedFrame )
{
	if( m_iType == 2 )
	{
		Vector3		vec3BonePos;
		m_pBone->getPos( &vec3BonePos );

		btTransform		bttrRbTransform = m_pbtRigidBody->getCenterOfMassTransform();

		bttrRbTransform.setOrigin( btVector3( 0.0f, 0.0f, 0.0f ) );
		bttrRbTransform = m_bttrBoneOffset * bttrRbTransform;

		bttrRbTransform.setOrigin( bttrRbTransform.getOrigin() + btVector3( vec3BonePos.x, vec3BonePos.y, vec3BonePos.z ) );
//		bttrRbTransform.setBasis( m_pbtRigidBody->getWorldTransform().getBasis() );

//		m_pbtRigidBody->setCenterOfMassTransform( bttrRbTransform );

		float	fRate = 0.2f * fElapsedFrame;
		if( fRate > 1.0f )	fRate = 1.0f;
		m_pbtRigidBody->translate( (bttrRbTransform.getOrigin() - m_pbtRigidBody->getCenterOfMassTransform().getOrigin()) * fRate );
/*
		m_pbtRigidBody->setLinearVelocity( btVector3( 0.0f, 0.0f, 0.0f ) );
		m_pbtRigidBody->setAngularVelocity( btVector3( 0.0f, 0.0f, 0.0f ) );
		m_pbtRigidBody->setInterpolationLinearVelocity( btVector3( 0.0f, 0.0f, 0.0f ) );
		m_pbtRigidBody->setInterpolationAngularVelocity( btVector3( 0.0f, 0.0f, 0.0f ) );
		m_pbtRigidBody->setInterpolationWorldTransform( m_pbtRigidBody->getCenterOfMassTransform() );
		m_pbtRigidBody->clearForces();
*/
	}
}

//==================================================================================
// ボーンの姿勢を剛体の姿勢と一致させる(そのフレームのシミュレーション終了後に呼ぶ)
//==================================================================================
void cPMDRigidBody::updateBoneTransform( void )
{
	if( m_iType != 0 && !m_bNoCopyToBone )
	{
		btTransform		bttrBoneTransform = m_pbtRigidBody->getCenterOfMassTransform() * m_bttrInvBoneOffset;

		bttrBoneTransform.getOpenGLMatrix( (float *)m_pBone->m_matLocal );
	}
}

//========================================
// 剛体をボーンの位置へ強制的に移動させる
//========================================
void cPMDRigidBody::moveToBonePos( void )
{
	if( m_iType != 0 )
	{
		Vector3		vec3BonePos;
		m_pBone->getPos( &vec3BonePos );

		btTransform		bttrRbTransform = m_pbtRigidBody->getCenterOfMassTransform();

		bttrRbTransform.setOrigin( btVector3( 0.0f, 0.0f, 0.0f ) );
		bttrRbTransform = m_bttrBoneOffset * bttrRbTransform;

		bttrRbTransform.setOrigin( bttrRbTransform.getOrigin() + btVector3( vec3BonePos.x, vec3BonePos.y, vec3BonePos.z ) );
		bttrRbTransform.setBasis( m_pbtRigidBody->getWorldTransform().getBasis() );

		m_pbtRigidBody->setCenterOfMassTransform( bttrRbTransform );

		m_pbtRigidBody->setLinearVelocity( btVector3( 0.0f, 0.0f, 0.0f ) );
		m_pbtRigidBody->setAngularVelocity( btVector3( 0.0f, 0.0f, 0.0f ) );
		m_pbtRigidBody->setInterpolationLinearVelocity( btVector3( 0.0f, 0.0f, 0.0f ) );
		m_pbtRigidBody->setInterpolationAngularVelocity( btVector3( 0.0f, 0.0f, 0.0f ) );
		m_pbtRigidBody->setInterpolationWorldTransform( m_pbtRigidBody->getCenterOfMassTransform() );
		m_pbtRigidBody->clearForces();
	}
}

//======
// 解放
//======
void cPMDRigidBody::release( void )
{
	if( m_pbtRigidBody )
	{
		if( m_pbtRigidBody->getMotionState() )	delete m_pbtRigidBody->getMotionState();
		g_clBulletPhysics.removeFromWorld( m_pbtRigidBody );

		delete m_pbtRigidBody;
		m_pbtRigidBody = NULL;
	}

	if( m_pbtColShape )
	{
		delete m_pbtColShape;
		m_pbtColShape = NULL;
	}
}
