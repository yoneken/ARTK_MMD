//*******************
// ARToolKitÉâÉbÉpÅ[
//*******************

#ifndef		_ARTK_H_
#define		_ARTK_H_

#include	<AR/config.h>
#include	<AR/video.h>
#include	<AR/param.h>
#include	<AR/ar.h>
#include	<AR/gsub_lite.h>

#include	"../MMD/VecMatQuat.h"


class cARTK
{
	private :
		ARParam			m_sCameraParam;
		int				m_iPattID;

		ARGL_CONTEXT_SETTINGS_REF
						m_pArglSettings;

		double			m_dmatRotX[16];
		Matrix			m_matRotX;

		unsigned int	m_uiARTImageSize;
		ARUint8			*m_pARTImage;
		bool			m_bMarkerFound;

		double			m_dPattWidth;
		double			m_dPattCenter[2];
		double			m_dPattTransMat[3][4];

		double			m_dViewScaleFactor;

		bool			m_bFirstTime;

	public :
		cARTK( void );
		~cARTK( void );

		bool initialize( void );
		bool initArgl( void );

		bool update( void );
		void display( void );

		void getCameraPos( Vector3 *pvec3CamPos );

		void getProjectionMat( double dMat[16] );
		void getModelViewMat( double dMat[16] );
		void addViewScaleFactor( double dAdd );

		void cleanup( void );


		inline bool isMarkerFound( void ){return m_bMarkerFound; }
};

extern cARTK	g_clARTK;

#endif	// _ARTK_H_
