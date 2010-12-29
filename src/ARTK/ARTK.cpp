//*******************
// ARToolKitラッパー
//*******************

#include	<math.h>
#include	"ARTK.h"
#include	<string.h>
#include	"../ErrorMes.h"


cARTK		g_clARTK;

//================
// コンストラクタ
//================
cARTK::cARTK( void ) : m_pARTImage( NULL ), m_pArglSettings( NULL ), m_bMarkerFound( false )
{
}

//==============
// デストラクタ
//==============
cARTK::~cARTK( void )
{
	cleanup();
}

//========
// 初期化
//========
bool cARTK::initialize( void )
{
	const char	*szCameraParamFName = "../Data/camera_para.dat";
	const char	*szPattFName  = "../Data/patt.00";
#ifdef _WIN32
	char		*szVConfFName = "../Data/WDM_camera_flipV.xml";
#else
	char		*szVConfFName = "";
#endif

	ARParam			sCamParamTemp;
	int				iCamImgSizeX,
					iCamImgSizeY;

	// カメラデバイスのオープン
	if( arVideoOpen( szVConfFName ) < 0 )
	{
		ErrorMessage( "Unable to open connection to camera.\n" );
		return false;
	}

	// カメラ画像のサイズを取得
	if( arVideoInqSize( &iCamImgSizeX, &iCamImgSizeY ) < 0 ) return false;

	// カメラパラメータファイルの読込み
	if( arParamLoad( szCameraParamFName, 1, &sCamParamTemp ) < 0 )
	{
		ErrorMessage( "Error loading parameter file for camera.\n" );
		return false;
	}

	// カメラパラメータのサイズ部分を変更
	arParamChangeSize( &sCamParamTemp, iCamImgSizeX, iCamImgSizeY, &m_sCameraParam );

	// ライブラリ内のカメラパラメータの初期化
	arInitCparam( &m_sCameraParam );

	// カメラ画像のキャプチャを開始
	if( arVideoCapStart() != 0 )
	{
		ErrorMessage( "Unable to begin camera data capture.\n" );
		return false;
	}

	// マーカーパターンファイルの読込み
    if( (m_iPattID = arLoadPatt( szPattFName )) < 0 )
    {
		ErrorMessage( "Pattern file load error !!\n" );
		return false;
    }

	m_uiARTImageSize = sizeof(ARUint8) * iCamImgSizeX * iCamImgSizeY * 3;
	m_pARTImage = (ARUint8 *)malloc( m_uiARTImageSize );
	memset( m_pARTImage, 0, m_uiARTImageSize );

	m_dViewScaleFactor = 0.16;
	m_dPattWidth = 40.0;
	m_bFirstTime = true;

	return true;
}

//==============
// arglの初期化
//==============
bool cARTK::initArgl( void )
{
	// arglをOpenGLの現在のコンテキスト用にセットアップ
	if( (m_pArglSettings = arglSetupForCurrentContext()) == NULL )
	{
		ErrorMessage( "arglSetupForCurrentContext() returned error.\n" );
		return false;
	}

	double	c = cos( 3.1515926 * 0.5 );
	double	s = sin( 3.1515926 * 0.5 );
	m_dmatRotX[ 0] = 1.0; m_dmatRotX[ 1] = 0.0; m_dmatRotX[ 2] = 0.0; m_dmatRotX[ 3] = 0.0;
	m_dmatRotX[ 4] = 0.0; m_dmatRotX[ 5] =   c; m_dmatRotX[ 6] =   s; m_dmatRotX[ 7] = 0.0;
	m_dmatRotX[ 8] = 0.0; m_dmatRotX[ 9] =  -s; m_dmatRotX[10] =   c; m_dmatRotX[11] = 0.0;
	m_dmatRotX[12] = 0.0; m_dmatRotX[13] = 0.0; m_dmatRotX[14] = 0.0; m_dmatRotX[15] = 1.0;

	MatrixRotationX( m_matRotX, -3.1415926f * 0.5f );

	return true;
}

//==================
// マーカー検出処理
//==================
bool cARTK::update( void )
{
	ARUint8			*pImage;
	ARMarkerInfo	*pMarkerInfo;
    int             iNumDetectedMarker;

	// カメラ画像の取得
	if( (pImage = (ARUint8 *)arVideoGetImage()) == NULL ) {
        arUtilSleep(2);
        return false;
    }
	memcpy( m_pARTImage, pImage, m_uiARTImageSize );

	m_bMarkerFound = false;

	// カメラ画像からマーカーを検出
	if( arDetectMarker( m_pARTImage, 130, &pMarkerInfo, &iNumDetectedMarker ) < 0 )
	{
		exit( -1 );
	}

	// 検出されたマーカー情報の中から一番信頼性の高いものを探す
	int		k = -1;
	for( int j = 0 ; j < iNumDetectedMarker ; j++ )
	{
		if( pMarkerInfo[j].id == m_iPattID )
		{
			if( k == -1 || pMarkerInfo[j].cf > pMarkerInfo[k].cf )	k = j;
		}
	}

	if( k != -1 )
	{
		// カメラのトランスフォーム行列を取得
		if( m_bFirstTime )
			arGetTransMat( &(pMarkerInfo[k]), m_dPattCenter, m_dPattWidth, m_dPattTransMat );
		else
			arGetTransMatCont( &(pMarkerInfo[k]), m_dPattTransMat, m_dPattCenter, m_dPattWidth, m_dPattTransMat );

		m_bFirstTime = false;

		m_bMarkerFound = true;
	}

	// 次のカメラ画像のキャプチャを開始
	arVideoCapNext();

	return true;
}

//==============
// 描画時の処理
//==============
void cARTK::display( void )
{
	// カメラ画像を描画
	arglDispImage( m_pARTImage, &m_sCameraParam, 1.0, m_pArglSettings );	// zoom = 1.0.
}

//====================
// カメラの位置の取得
//====================
void cARTK::getCameraPos( Vector3 *pvec3CamPos )
{
	double		dInvPattTransMat[3][4];

	arUtilMatInv( m_dPattTransMat, dInvPattTransMat );

	pvec3CamPos->x = (float)dInvPattTransMat[0][3];
	pvec3CamPos->y = (float)dInvPattTransMat[1][3];
	pvec3CamPos->z = (float)dInvPattTransMat[2][3];
	Vector3Transform( pvec3CamPos, pvec3CamPos, m_matRotX );

	pvec3CamPos->x *= (float)m_dViewScaleFactor;
	pvec3CamPos->y *= (float)m_dViewScaleFactor;
	pvec3CamPos->z *= (float)m_dViewScaleFactor;
}

//============================
// プロジェクション行列の取得
//============================
void cARTK::getProjectionMat( double dMat[16] )
{
								//        near  far
	arglCameraFrustumRH( &m_sCameraParam, 0.1, 400.0, dMat );
}

//========================
// モデルビュー行列の取得
//========================
void cARTK::getModelViewMat( double dMat[16] )
{
	double	dmatTemp[16];

	arglCameraViewRH( m_dPattTransMat, dmatTemp, m_dViewScaleFactor );

	for( int i = 0 ; i < 4 ; i++ )
	{
		int		idx = i << 2;
		dMat[idx + 0] =	m_dmatRotX[idx] * dmatTemp[0] + m_dmatRotX[idx + 1] * dmatTemp[4] + m_dmatRotX[idx + 2] * dmatTemp[ 8] + m_dmatRotX[idx + 3] * dmatTemp[12];
		dMat[idx + 1] =	m_dmatRotX[idx] * dmatTemp[1] + m_dmatRotX[idx + 1] * dmatTemp[5] + m_dmatRotX[idx + 2] * dmatTemp[ 9] + m_dmatRotX[idx + 3] * dmatTemp[13];
		dMat[idx + 2] =	m_dmatRotX[idx] * dmatTemp[2] + m_dmatRotX[idx + 1] * dmatTemp[6] + m_dmatRotX[idx + 2] * dmatTemp[10] + m_dmatRotX[idx + 3] * dmatTemp[14];
		dMat[idx + 3] =	m_dmatRotX[idx] * dmatTemp[3] + m_dmatRotX[idx + 1] * dmatTemp[7] + m_dmatRotX[idx + 2] * dmatTemp[11] + m_dmatRotX[idx + 3] * dmatTemp[15];
	}
}

//==============
// 拡大率の増減
//==============
void cARTK::addViewScaleFactor( double dAdd )
{
	m_dViewScaleFactor += dAdd;

	// 数字が小さいほどモデルは大きく表示される
	if( m_dViewScaleFactor < 0.01 )	m_dViewScaleFactor = 0.01;
	if( 0.80 < m_dViewScaleFactor )	m_dViewScaleFactor = 0.80;
}

//======================
// ライブラリの終了処理
//======================
void cARTK::cleanup( void )
{
	if( m_pARTImage )
	{
		free( m_pARTImage );
		m_pARTImage = NULL;
	}

	if( m_pArglSettings )
	{
		arglCleanup( m_pArglSettings );
		m_pArglSettings = NULL;
	}
	arVideoCapStop();
	arVideoClose();
}
