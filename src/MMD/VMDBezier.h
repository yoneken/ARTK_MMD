//*****************
// VMD用ベジェ曲線
//*****************

#ifndef		_VMDBEZIER_H_
#define		_VMDBEZIER_H_


class cVMDBezier
{
	private :
		#define	YVAL_DIV_NUM	16

		float	m_fYValue[YVAL_DIV_NUM + 1];

		bool	m_bLinear;


		float getYValue( float fX, float fX1, float fY1, float fX2, float fY2 );

	public :
		void initialize( float fPointX1, float fPointY1, float fPointX2, float fPointY2 );

		float getInterValue( float fT );
};

#endif	// _VMDBEZIER_H_
