/*
	ARTK_MMD
	Copyright (C) 2009  PY

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifdef	_WIN32
#include	<windows.h>
#include	<locale.h>

#include	<gl/glut.h>
#else
#ifdef __APPLE__
#include	<GLUT/glut.h>
#else
#include	<GL/glut.h>
#endif
#endif
#include	<stdio.h>

#include	"ARTK/ARTK.h"
#include	"BulletPhysics/BulletPhysics.h"

#include	"MMD/PMDModel.h"
#include	"MMD/VMDMotion.h"

extern cBulletPhysics	g_clBulletPhysics;

static cPMDModel	g_clPMDModel;
static cVMDMotion	g_clVMDMotion;
static bool			g_bDispFPS;
static Matrix		g_matPlanarProjection;

static void init( void );
static void cleanup( void );
static void display( void );
static void resize( int w, int h );
static void visibility( int visible );
static void idle( void );
static float getElapsedFrame( void );
static void keyboard( unsigned char key, int x, int y );
static void menu( int value );
static void openNewModel( void );
static void openNewMotion( void );
static void calcFps( void );
static void drawString( const char *szStr, int iPosX, int iPosY );


//==========
// main関数
//==========
int main( int argc, char *argv[] )
{
	// GLUTの初期化
	glutInit( &argc, argv );

	// ARToolKitの初期化
	if( !g_clARTK.initialize() )
	{
		exit( -1 );
	}

	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL );
	glutInitWindowSize( 640, 480 );
	glutCreateWindow( "ARTK MMD" );

	// arglの初期化
	if( !g_clARTK.initArgl() )
	{
		exit( -1 );
	}

	glutDisplayFunc( display );
	glutReshapeFunc( resize );
	glutVisibilityFunc( visibility );
	glutKeyboardFunc( keyboard );
	atexit( cleanup );

    glutCreateMenu( menu );
		glutAddMenuEntry( "Open Model(PMD)", 1 );
		glutAddMenuEntry( "Open Motion(VMD)", 2 );
		glutAddMenuEntry( "Look at me! (On/Off)", 3 );
	glutAttachMenu( GLUT_RIGHT_BUTTON );

	init();

	glutMainLoop();

	return 0;
}

//--------
// 初期化
//--------
void init( void )
{
	glClearColor( 0.0f, 0.0f, 1.0f, 1.0f );
	glClearStencil( 0 );
	glEnable( GL_DEPTH_TEST );

	glEnable( GL_NORMALIZE );

	glEnable( GL_TEXTURE_2D );
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	glEnable( GL_CULL_FACE );
	glCullFace( GL_FRONT );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glEnable( GL_ALPHA_TEST );
	glAlphaFunc( GL_GEQUAL, 0.05f );

	const float	fLightPos[] = { 0.45f, 0.55f , 1.0f, 0.0f };
	const float	fLightDif[] = { 0.9f, 0.9f, 0.9f, 1.0f },
				fLightAmb[] = { 0.9f, 0.9f, 0.9f, 1.0f };

	glLightfv( GL_LIGHT0, GL_POSITION, fLightPos );
	glLightfv( GL_LIGHT0, GL_DIFFUSE, fLightDif );
	glLightfv( GL_LIGHT0, GL_AMBIENT, fLightAmb );
	glLightfv( GL_LIGHT0, GL_SPECULAR, fLightAmb );
	glEnable( GL_LIGHT0 );

	glEnable( GL_LIGHTING );

	// 平面投影行列の作成
	Vector4		vec4Plane = { 0.0f, 1.0f, 0.0f, 0.0f };		// { a, b, c, d } → ax + by + cz + d = 0 (投影したい平面の方程式)
	Vector3		vec4LightPos = { 10.0f, 70.0f, -20.0f };	// ライトの位置

	MatrixPlanarProjection( g_matPlanarProjection, &vec4Plane, &vec4LightPos );

	g_clBulletPhysics.initialize();

	g_bDispFPS = false;
}

//------------------
// プログラム終了時
//------------------
void cleanup( void )
{
	g_clPMDModel.release();
	g_clBulletPhysics.release();
}

//----------
// 表示関数
//----------
void display( void )
{
	glClear( GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	glDisable( GL_CULL_FACE );
	glDisable( GL_ALPHA_TEST );
	glDisable( GL_BLEND );

	g_clARTK.display();

	glEnable( GL_CULL_FACE );
	glEnable( GL_ALPHA_TEST );
	glEnable( GL_BLEND );

	if( g_clARTK.isMarkerFound() )
	{
		double	dMat[16];

		glMatrixMode( GL_PROJECTION );
		g_clARTK.getProjectionMat( dMat );
		glLoadMatrixd( dMat );

		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();

		glPushMatrix();

			g_clARTK.getModelViewMat( dMat );
			glLoadMatrixd( dMat );

			glScalef( 1.0f, 1.0f, -1.0f );	// 左手系 → 右手系
			g_clPMDModel.render();

			// ステンシルバッファに影の形を描画
			glDisable( GL_CULL_FACE );
			glDisable( GL_TEXTURE_2D );
			glDisable( GL_LIGHTING );

			glEnable( GL_STENCIL_TEST );
			glStencilFunc( GL_ALWAYS, 1, ~0 );
			glStencilOp( GL_REPLACE, GL_KEEP, GL_REPLACE );

			glColorMask( 0, 0, 0, 0 );
			glDepthMask( 0 );

				glMultMatrixf( (const float *)g_matPlanarProjection );
				g_clPMDModel.renderForShadow();		// 影用の描画

			glColorMask( 1, 1, 1, 1 );

			// ステンシルバッファの影の形を塗りつぶす
			float	fWndW = glutGet( GLUT_WINDOW_WIDTH ),
					fWndH = glutGet( GLUT_WINDOW_HEIGHT );

			glStencilFunc( GL_EQUAL, 1, ~0);
			glStencilOp( GL_KEEP, GL_KEEP ,GL_KEEP );

			glDisable( GL_DEPTH_TEST );

			glMatrixMode( GL_PROJECTION );
			glPushMatrix();
			glLoadIdentity();
			gluOrtho2D( 0.0f, fWndW, 0.0f, fWndH );

			glMatrixMode( GL_MODELVIEW );
			glPushMatrix();
			glLoadIdentity();

				glColor4f( 0.2f, 0.2f, 0.2f, 0.5f );

				glBegin( GL_TRIANGLE_FAN );
					glVertex2f(  0.0f, fWndH );
					glVertex2f( fWndW, fWndH );
					glVertex2f( fWndW,  0.0f );
					glVertex2f(  0.0f,  0.0f );
				glEnd();

			glMatrixMode( GL_PROJECTION );
			glPopMatrix();
			glMatrixMode( GL_MODELVIEW );
			glPopMatrix();

			glDepthMask( 1 );
			glEnable( GL_LIGHTING );
			glEnable( GL_DEPTH_TEST );
			glDisable( GL_STENCIL_TEST );

		glPopMatrix();
	}

	calcFps();

	glutSwapBuffers();
}

//--------------------------------
// ウインドウがリサイズされたとき
//--------------------------------
void resize( int w, int h )
{
	int		iResizeW = (float)h * 4.0f / 3.0f;
	int		iResizeH = (float)w * 3.0f / 4.0f;

	if( iResizeW < w )
		iResizeH = h;
	else
		iResizeW = w;

	glutReshapeWindow( iResizeW, iResizeH );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	glViewport( 0, 0, iResizeW, iResizeH );
	
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
}

//----------------------------------------
// ウインドウのビジビリティが変化したとき
//----------------------------------------
void visibility( int visible )
{
	if( visible == GLUT_VISIBLE )	glutIdleFunc( idle );
	else							glutIdleFunc( NULL );
}

//------------
// アイドル時
//------------
void idle( void )
{
	g_clARTK.update();

	Vector3	vecCamPos;
	g_clARTK.getCameraPos( &vecCamPos );

	float		fElapsedFrame = getElapsedFrame();

	if( fElapsedFrame > 10.0f )	fElapsedFrame = 10.0f;

	g_clPMDModel.updateMotion( fElapsedFrame );
	g_clPMDModel.updateNeckBone( &vecCamPos );
	g_clBulletPhysics.update( fElapsedFrame );
	g_clPMDModel.updateSkinning();

	glutPostRedisplay();
}

//-----------------------------------------------
// 経過フレーム数を返す(1.0 = 1/30s = 33.333ms)
//-----------------------------------------------
float getElapsedFrame( void )
{
	static int	s_iPrevTime = 0;
	int			iTime = glutGet( GLUT_ELAPSED_TIME );
	float		fDiffTime;

	if( s_iPrevTime == 0 )	s_iPrevTime = iTime;
	fDiffTime = (float)(iTime - s_iPrevTime) * (30.0f / 1000.0f);

	s_iPrevTime = iTime;

	return fDiffTime;
}

//----------------------------
// キーボード入力があったとき
//----------------------------
void keyboard( unsigned char key, int x, int y )
{
	switch( key )
	{
		case 'o':	// モデル読込み
		case 'O':
			openNewModel();
			break;

		case 'm':	// モーション読込み
		case 'M':
			openNewMotion();
			break;

		case 'l':	// カメラ目線モードOn/Off
		case 'L':
			g_clPMDModel.toggleLookAtFlag();
			break;

		case 'f':	// フレームレート表示On/Off
		case 'F':
			g_bDispFPS = !g_bDispFPS;
			break;

		case '+':	// モデル拡大
			g_clARTK.addViewScaleFactor( -0.01 );
			break;

		case '-':	// モデル縮小
			g_clARTK.addViewScaleFactor(  0.01 );
			break;
	}
}

//--------------------------------
// メニューの項目が選択されたとき
//--------------------------------
void menu( int value )
{
	switch( value )
	{
		case 1 :	// モデル読込み
			openNewModel();
			break;

		case 2 :	// モーション読込み
			openNewMotion();
			break;

		case 3 :	// カメラ目線モードOn/Off
			g_clPMDModel.toggleLookAtFlag();
			break;
	}
}

//----------------------
// モデルファイルを開く
//----------------------
void openNewModel( void )
{
#ifdef	_WIN32
	OPENFILENAME	ofn;
	wchar_t			wszFileName[256],
					wszFile[64];
	char			szFileName[256];

	ZeroMemory( &wszFileName, 256 );
	ZeroMemory( &wszFile,      64 );
	ZeroMemory( &ofn, sizeof(OPENFILENAME) );
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = L"PMD File(*.pmd)\0*.pmd\0\0";
	ofn.lpstrFile = wszFileName;
	ofn.lpstrFileTitle = wszFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = sizeof(wszFile);
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = L"pmd";
	ofn.lpstrTitle = L"Open";

	if( GetOpenFileName( &ofn ) )
	{
		SetCurrentDirectory( wszFileName );

		setlocale( LC_ALL, "Japanese_Japan.932" );
	    wcstombs( szFileName, wszFileName, 255 );

		g_clPMDModel.load( szFileName );
	}
#else
	char	*szFileName = "../tmp/miku.pmd";
	g_clPMDModel.load( szFileName );
#endif
}

//--------------------------
// モーションファイルを開く
//--------------------------
void openNewMotion( void )
{
#ifdef	_WIN32
	OPENFILENAME	ofn;
	wchar_t			wszFileName[256],
					wszFile[64];
	char			szFileName[256];

	ZeroMemory( &wszFileName, 256 );
	ZeroMemory( &wszFile,      64 );
	ZeroMemory( &ofn, sizeof(OPENFILENAME) );
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = L"VMD File(*.vmd)\0*.vmd\0\0";
	ofn.lpstrFile = wszFileName;
	ofn.lpstrFileTitle = wszFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = sizeof(wszFile);
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = L"vmd";
	ofn.lpstrTitle = L"Open";

	if( GetOpenFileName( &ofn ) )
	{
		SetCurrentDirectory( wszFileName );

		setlocale( LC_ALL, "Japanese_Japan.932" );
	    wcstombs( szFileName, wszFileName, 255 );

		g_clVMDMotion.load( szFileName );

		g_clPMDModel.setMotion( &g_clVMDMotion, false );
		g_clPMDModel.updateMotion( 0.0f );
		g_clPMDModel.resetRigidBodyPos();
	}
#else
	char	*szFileName = "../tmp/dance.vmd";
	g_clVMDMotion.load( szFileName );

	g_clPMDModel.setMotion( &g_clVMDMotion, false );
	g_clPMDModel.updateMotion( 0.0f );
	g_clPMDModel.resetRigidBodyPos();

#endif
}

//-------------------
// FPSの計測(精度低)
//-------------------
void calcFps( void )
{
	static int		iT1 = 0,
					iT2 = 0,
					iCnt = 0;
	static float	fFps = 0.0f;

	iT1 = glutGet( GLUT_ELAPSED_TIME );
	if( iT1 - iT2 >= 1000 )
	{
		fFps = (1000.0f * iCnt) / (float)(iT1 - iT2);
		iT2 = iT1;
		iCnt = 0;
	}
	iCnt++; 

	if( g_bDispFPS )
	{
		char	szStr[16];
		sprintf( szStr, "%6.2f", fFps );
		drawString( szStr, 8, 8 );
	}
}

//------------
// 文字列描画
//------------
void drawString( const char *szStr, int iPosX, int iPosY )
{
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_LIGHTING );
	glDisable( GL_TEXTURE_2D );

	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D( 0.0f, (float)glutGet( GLUT_WINDOW_WIDTH ), 0.0f, (float)glutGet( GLUT_WINDOW_HEIGHT ) );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

	glRasterPos2i( iPosX, glutGet( GLUT_WINDOW_HEIGHT ) - (iPosY + 15) );

	while( *szStr )
	{
		glutBitmapCharacter( GLUT_BITMAP_9_BY_15, *szStr );
		szStr++;
 	}

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_LIGHTING );
}
