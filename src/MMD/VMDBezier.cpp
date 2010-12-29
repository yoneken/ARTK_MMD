//*****************
// VMD用ベジェ曲線
//*****************

#include	<math.h>
#include	"VMDBezier.h"


//========
// 初期化
//========
void cVMDBezier::initialize( float fPointX1, float fPointY1, float fPointX2, float fPointY2 )
{
	if( fPointX1 == fPointY1 && fPointX2 == fPointY2 )
	{
		m_bLinear = true;
	}
	else
	{
		fPointX1 = (fPointX1 / 127.0f) * 3.0f;
		fPointY1 = (fPointY1 / 127.0f) * 3.0f;

		fPointX2 = (fPointX2 / 127.0f) * 3.0f;
		fPointY2 = (fPointY2 / 127.0f) * 3.0f;

		// 高速化のためテーブル化しておく
		m_fYValue[0] = 0.0f;
		m_fYValue[YVAL_DIV_NUM] = 1.0f;

		float	fAddX = 1.0f / (float)YVAL_DIV_NUM;

		for( int i = 1 ; i < YVAL_DIV_NUM ; i++ )
		{
			m_fYValue[i] = getYValue( fAddX * i, fPointX1, fPointY1, fPointX2, fPointY2 );
		}

		m_bLinear = false;
	}
}

//------------------------
// X値に対応するY値を返す
//------------------------
float cVMDBezier::getYValue( float fX, float fX1, float fY1, float fX2, float fY2 )
{
	float	fT = fX;
	float	fInvT = 1.0f - fT;

	for( int i = 0 ; i < 32 ; i++ )
	{
		float	fTempX = fInvT*fInvT*fT*fX1 + fInvT*fT*fT*fX2 + fT*fT*fT;

		fTempX -= fX;
		if( fabsf( fTempX ) < 0.0001f )
		{
			break;
		}
		else
		{
			fT -= fTempX * 0.5f;
			fInvT = 1.0f - fT;
		}
	}

	return fInvT*fInvT*fT*fY1 + fInvT*fT*fT*fY2 + fT*fT*fT;
}

//==================
// 曲線補間値を返す
//==================
float cVMDBezier::getInterValue( float fX )
{
	if( m_bLinear )	return fX;	// 線形補間

	// テーブルを線形補間する
	fX *= (float)YVAL_DIV_NUM;

	int		iIdx = (int)fX;

	fX -= iIdx;

	return m_fYValue[iIdx] * (1.0f - fX) + m_fYValue[iIdx + 1] * fX;
}
