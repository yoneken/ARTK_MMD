//****************
// モーション再生
//****************

#ifndef		_MOTIONPLAYER_H_
#define		_MOTIONPLAYER_H_

#include	"VMDMotion.h"

class cPMDModel;

class cMotionPlayer
{
	private :
		cPMDModel		*m_pPMDModel;
		cVMDMotion		*m_pVMDMotion;

		cPMDBone		**m_ppBoneList;
		cPMDFace		**m_ppFaceList;

		float			m_fOldFrame,
						m_fFrame;
		bool			m_bLoop;		// モーションをループするかどうか

		float			m_fInterpolateFrameMax;	// モーション補間用
		float			m_fInterpolateFrameNow;


		void getMotionPosRot( const MotionDataList *pMotionData, float fFrame, Vector3 *pvec3Pos, Vector4 *pvec4Rot );
		float getFaceRate( const FaceDataList *pFaceData, float fFrame );

	public :
		cMotionPlayer( void );
		~cMotionPlayer( void );

		void setup( cPMDModel *pPMDModel, cVMDMotion *pMotion, bool bLoop, float fInterpolateFrame );

		bool update( float fElapsedFrame );

		void clear( void );

};

#endif	// _MOTIONPLAYER_H_
