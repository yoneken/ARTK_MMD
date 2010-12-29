//****************
// Bulletラッパー
//****************
/*
(参考) Monshoさんの「もんしょの巣穴」  http://monsho.hp.infoseek.co.jp/
*/

#include	"BulletPhysics.h"


cBulletPhysics			g_clBulletPhysics;


//================
// コンストラクタ
//================
cBulletPhysics::cBulletPhysics( void ) : m_pBtCollisionConfig( NULL ), m_pBtCollisionDispatcher( NULL ), m_pBtOverlappingPairCache( NULL ),
												m_pBtSolver( NULL ), m_pBtWorld( NULL ), m_pBtGroundShape( NULL )
{
}

//==============
// デストラクタ
//==============
cBulletPhysics::~cBulletPhysics( void )
{
	release();
}

//========
// 初期化
//========
bool cBulletPhysics::initialize( void )
{
	// コリジョンコンフィグを作成する
	m_pBtCollisionConfig = new btDefaultCollisionConfiguration();

	// コリジョンディスパッチャを作成する
	m_pBtCollisionDispatcher = new btCollisionDispatcher( m_pBtCollisionConfig );

	// コリジョンワールドの最大サイズを指定する
	btVector3	btv3WorldAabbMin( -3000.0f, -3000.0f, -3000.0f );
	btVector3	btv3WorldAabbMax(  3000.0f,  3000.0f,  3000.0f );
	int			iMaxProxies = 1024;
	m_pBtOverlappingPairCache = new btAxisSweep3( btv3WorldAabbMin, btv3WorldAabbMax, iMaxProxies );

	// の拘束計算ソルバを作成する
	m_pBtSolver = new btSequentialImpulseConstraintSolver();

	// ワールドの作成
	m_pBtWorld = new btDiscreteDynamicsWorld( m_pBtCollisionDispatcher, m_pBtOverlappingPairCache, m_pBtSolver, m_pBtCollisionConfig );

	// 重力設定
	m_pBtWorld->setGravity( btVector3( 0.0f, -9.8f * 2.0f, 0.0f ) );	// ネクタイなどがふわふわするのでなんとなく2倍しておく

	//-----------------------------------------------------
	// 床用として無限平面を作成
	m_pBtGroundShape = new btStaticPlaneShape( btVector3( 0.0f, 1.0f, 0.0f ), 0.0f );

	// 床のトランスフォームを設定
	btTransform		trGroundTransform;
	trGroundTransform.setIdentity();

	// MotionStateを作成する。剛体の姿勢制御をするもの
	btMotionState	*pMotionState = new btDefaultMotionState( trGroundTransform );

	// 剛体を作成する
	// 質量 0.0、慣性テンソル 0.0 ならこの剛体は動かない
	btRigidBody::btRigidBodyConstructionInfo	rbInfo( 0.0f, pMotionState, m_pBtGroundShape, btVector3( 0.0f, 0.0f, 0.0f ) );
	btRigidBody		*pRigidBody = new btRigidBody( rbInfo );

	// 物理ワールドに床を追加
	m_pBtWorld->addRigidBody( pRigidBody );

	return true;
}

//======================
// 剛体をワールドに追加
//======================
void cBulletPhysics::addToWorld( btRigidBody *pbtRB, unsigned short unGroupIdx, unsigned short unGroupMask )
{
	m_pBtWorld->addRigidBody( pbtRB, unGroupIdx, unGroupMask );
}

//==================================
// コンストレイントをワールドに追加
//==================================
void cBulletPhysics::addToWorld( btTypedConstraint *pctConstraint )
{
	m_pBtWorld->addConstraint( pctConstraint );
}

//========================
// 剛体をワールドから削除
//========================
void cBulletPhysics::removeFromWorld( btRigidBody *pbtRB )
{
	m_pBtWorld->removeCollisionObject( pbtRB );
}

//====================================
// コンストレイントをワールドから削除
//====================================
void cBulletPhysics::removeFromWorld( btTypedConstraint *pctConstraint )
{
	m_pBtWorld->removeConstraint( pctConstraint );
}

//======
// 更新
//======
void cBulletPhysics::update( float fElapsedFlame )
{
	float	fMilliSec = fElapsedFlame * (1000.0f / 30.0f);
//	float	fSubStep = fMilliSec * 0.06f;	// fMilliSec / (1000.0f / 60.0f)

//	m_pBtWorld->stepSimulation( fMilliSec, 1 * (int)fSubStep );
	m_pBtWorld->stepSimulation( fMilliSec, 1, fElapsedFlame / 30.0f );
}

//======
// 解放
//======
void cBulletPhysics::release( void )
{
	if( !m_pBtWorld )	return;

	for( int i = m_pBtWorld->getNumCollisionObjects() - 1 ; i >= 0 ; i-- )
	{
		btCollisionObject	*pObj = m_pBtWorld->getCollisionObjectArray()[i];
		btRigidBody			*pRigidBody = btRigidBody::upcast( pObj );

		if( pRigidBody && pRigidBody->getMotionState() )	delete pRigidBody->getMotionState();

		m_pBtWorld->removeCollisionObject( pObj );

		delete pObj;
	}

	if( m_pBtGroundShape )
	{
		delete m_pBtGroundShape;
		m_pBtGroundShape = NULL;
	}

	if( m_pBtWorld )
	{
		delete m_pBtWorld;
		m_pBtWorld = NULL;
	}

	if( m_pBtSolver )
	{
		delete m_pBtSolver;
		m_pBtSolver = NULL;
	}

	if( m_pBtOverlappingPairCache )
	{
		delete m_pBtOverlappingPairCache;
		m_pBtOverlappingPairCache = NULL;
	}

	if( m_pBtCollisionDispatcher )
	{
		delete m_pBtCollisionDispatcher;
		m_pBtCollisionDispatcher = NULL;
	}

	if( m_pBtCollisionConfig )
	{
		delete m_pBtCollisionConfig;
		m_pBtCollisionConfig = NULL;
	}
}
