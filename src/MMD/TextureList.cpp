//**********************
// テクスチャリスト管理
//**********************

#include	<stdio.h>
#include	<string.h>
#ifdef	_WIN32
#include	<gl/glut.h>
#include	<malloc.h>
#else
#ifdef __APPLE__
#include	<GLUT/glut.h>
#include	<stdlib.h>
#else
#include	<GL/glut.h>
#include	<malloc.h>
#endif
#endif
#include	"TextureList.h"

cTextureList	g_clsTextureList;

//================
// コンストラクタ
//================
cTextureList::cTextureList( void ) : m_pTextureList( NULL )
{
}

//==============
// デストラクタ
//==============
cTextureList::~cTextureList( void )
{
	TextureData	*pTemp = m_pTextureList,
				*pNextTemp;

	while( pTemp )
	{
		pNextTemp = pTemp->pNext;

		glDeleteTextures( 1, &pTemp->uiTexID );
		delete pTemp;

		pTemp = pNextTemp;
	}

	m_pTextureList = NULL;
}

//====================
// テクスチャIDを取得
//====================
unsigned int cTextureList::getTexture( const char *szFileName )
{
	unsigned int	uiTexID = 0xFFFFFFFF;

	// まずはすでに読み込まれているかどうか検索
	if( findTexture( szFileName, &uiTexID ) )
	{
		return uiTexID;
	}

	// なければファイルを読み込んでテクスチャ作成
	if( createTexture( szFileName, &uiTexID ) )
	{
		// 新しいリストノードを作成し先頭へ接続
		TextureData	*pNew = new TextureData;

		strcpy( pNew->szFileName, szFileName );
		pNew->uiTexID = uiTexID;
		pNew->uiRefCount = 1;

		pNew->pNext = m_pTextureList;
		m_pTextureList = pNew;

		return uiTexID;
	}

	return 0xFFFFFFFF;	// テクスチャ読み込みか作成失敗
}

//----------------------------------
// 読み込み済みのテクスチャから検索
//----------------------------------
bool cTextureList::findTexture( const char *szFileName, unsigned int *puiTexID )
{
	TextureData	*pTemp = m_pTextureList;

	while( pTemp )
	{
		if( strcmp( pTemp->szFileName, szFileName ) == 0 )
		{
			// 読み込み済みのテクスチャを発見
			*puiTexID = pTemp->uiTexID;
			pTemp->uiRefCount++;

			return true;
		}

		pTemp = pTemp->pNext;
	}

	return false;	// 発見できず
}

//--------------------------------------
// ファイルを読み込んでテクスチャを作成
//--------------------------------------
bool cTextureList::createTexture( const char *szFileName, unsigned int *puiTexID )
{
	FILE	*pFile;
	long	fFileSize;
	unsigned char
			*pData;

	// TODO: fetch sphere mapping
	char fileName[256];
	const char *tmp_p = strrchr(szFileName, '*');
	int fileNameLength;
	if(!tmp_p) fileNameLength = strlen( szFileName );
	else fileNameLength = tmp_p - szFileName;
	strncpy(fileName, szFileName, fileNameLength);
	fileName[fileNameLength] = 0x0;

	pFile = fopen( fileName, "rb" );
	//pFile = fopen( szFileName, "rb" );
	if( !pFile )	return false;	// ファイルが開けない

	// ファイルサイズ取得
	fseek( pFile, 0, SEEK_END );
	fFileSize = ftell(pFile);

	// メモリ確保
	pData = (unsigned char *)malloc( (size_t)fFileSize );

	// 読み込み
	fseek( pFile, 0, SEEK_SET );
	fread( pData, 1, (size_t)fFileSize, pFile );

	fclose( pFile );

	int		iLen = strlen( szFileName );
	bool	bRet = false;

	if( 	(szFileName[iLen - 3] == 'b' || szFileName[iLen - 3] == 'B') &&
			(szFileName[iLen - 2] == 'm' || szFileName[iLen - 2] == 'M') &&
			(szFileName[iLen - 1] == 'p' || szFileName[iLen - 1] == 'P')		)
	{
		bRet = createFromBMP( pData, puiTexID );
	}
	else if((szFileName[iLen - 3] == 's' || szFileName[iLen - 3] == 'S') &&
			(szFileName[iLen - 2] == 'p' || szFileName[iLen - 2] == 'P') &&
			(szFileName[iLen - 1] == 'h' || szFileName[iLen - 1] == 'H')		)
	{
		bRet = createFromBMP( pData, puiTexID );	// *.sph はBMP
	}
	else if((szFileName[iLen - 3] == 't' || szFileName[iLen - 3] == 'T') &&
			(szFileName[iLen - 2] == 'g' || szFileName[iLen - 2] == 'G') &&
			(szFileName[iLen - 1] == 'a' || szFileName[iLen - 1] == 'A')		)
	{
		bRet = createFromTGA( pData, puiTexID );
	}

	free( pData );

	return bRet;
}

//-------------------------------
// BMPファイルからテクスチャ作成
//-------------------------------
bool cTextureList::createFromBMP( const unsigned char *pData, unsigned int *puiTexID )
{
#pragma pack( push, 1 )
	// BMPファイルヘッダ構造体
	struct BMPFileHeader
	{
		unsigned short	bfType;			// ファイルタイプ
		unsigned int	bfSize;			// ファイルサイズ
		unsigned short	bfReserved1;
		unsigned short	bfReserved2;
		unsigned int	bfOffBits;		// ファイル先頭から画像データまでのオフセット
	};

	// BMP情報ヘッダ構造体
	struct BMPInfoHeader
	{
		unsigned int	biSize;			// 情報ヘッダーのサイズ
		int				biWidth;		// 幅
		int				biHeight;		// 高さ(正ならば下から上、負ならば上から下)
		unsigned short	biPlanes;		// プレーン数(常に1)
		unsigned short	biBitCount;		// 1画素あたりのビット数
		unsigned int	biCompression;
		unsigned int	biSizeImage;
		int				biXPelsPerMeter;
		int				biYPelsPerMeter;
		unsigned int	biClrUsed;		// パレットの色数
		unsigned int	biClrImportant;
	};

	// パレットデータ
	struct RGBQuad
	{
		unsigned char	rgbBlue;
		unsigned char	rgbGreen;
		unsigned char	rgbRed;
		unsigned char	rgbReserved;
	};
#pragma pack( pop )

	// BMPファイルヘッダ
	BMPFileHeader	*pBMPFileHeader = (BMPFileHeader *)pData;

	if( pBMPFileHeader->bfType != ('B' | ('M' << 8)) )
	{
		return false;	// ファイルタイプが違う
	}

	// BMP情報ヘッダ
	BMPInfoHeader	*pBMPInfoHeader = (BMPInfoHeader *)(pData + sizeof(BMPFileHeader));

	if( pBMPInfoHeader->biBitCount == 1 || pBMPInfoHeader->biCompression != 0 )
	{
		return false;	// 1ビットカラーと圧縮形式には未対応
	}

	// カラーパレット
	RGBQuad			*pPalette = NULL;

	if( pBMPInfoHeader->biBitCount < 24 )
	{
		pPalette = (RGBQuad *)(pData + sizeof(BMPFileHeader) + sizeof(BMPInfoHeader));
	}

	// 画像データの先頭へ
	pData += pBMPFileHeader->bfOffBits;

	// 画像データの1ラインのバイト数
	unsigned int	uiLineByte = ((pBMPInfoHeader->biWidth * pBMPInfoHeader->biBitCount + 0x1F) & (~0x1F)) / 8;

	// テクスチャイメージの作成
	unsigned char	*pTexelData = (unsigned char *)malloc( pBMPInfoHeader->biWidth * pBMPInfoHeader->biHeight * 4 ),
					*pTexelDataTemp = pTexelData;

	if( pBMPInfoHeader->biBitCount == 4 )
	{
		// 4Bitカラー
		for( int h = pBMPInfoHeader->biHeight - 1 ; h >= 0 ; h-- )
		{
			const unsigned char *pLineTop = &pData[uiLineByte * h];

			for( int w = 0 ; w < (pBMPInfoHeader->biWidth >> 1) ; w++ )
			{
				*pTexelDataTemp = pPalette[(pLineTop[w] >> 4) & 0x0F].rgbRed;	pTexelDataTemp++;
				*pTexelDataTemp = pPalette[(pLineTop[w] >> 4) & 0x0F].rgbGreen;	pTexelDataTemp++;
				*pTexelDataTemp = pPalette[(pLineTop[w] >> 4) & 0x0F].rgbBlue;	pTexelDataTemp++;
				*pTexelDataTemp = 255;											pTexelDataTemp++;

				*pTexelDataTemp = pPalette[(pLineTop[w]     ) & 0x0F].rgbRed;	pTexelDataTemp++;
				*pTexelDataTemp = pPalette[(pLineTop[w]     ) & 0x0F].rgbGreen;	pTexelDataTemp++;
				*pTexelDataTemp = pPalette[(pLineTop[w]     ) & 0x0F].rgbBlue;	pTexelDataTemp++;
				*pTexelDataTemp = 255;											pTexelDataTemp++;
			}
		}
	}
	else if( pBMPInfoHeader->biBitCount == 8 )
	{
		// 8Bitカラー
		for( int h = pBMPInfoHeader->biHeight - 1 ; h >= 0 ; h-- )
		{
			const unsigned char *pLineTop = &pData[uiLineByte * h];

			for( int w = 0 ; w < pBMPInfoHeader->biWidth ; w++ )
			{
				*pTexelDataTemp = pPalette[pLineTop[w]].rgbRed;		pTexelDataTemp++;
				*pTexelDataTemp = pPalette[pLineTop[w]].rgbGreen;	pTexelDataTemp++;
				*pTexelDataTemp = pPalette[pLineTop[w]].rgbBlue;	pTexelDataTemp++;
				*pTexelDataTemp = 255;								pTexelDataTemp++;
			}
		}
	}
	else if( pBMPInfoHeader->biBitCount == 24 )
	{
		// 24Bitカラー
		for( int h = pBMPInfoHeader->biHeight - 1 ; h >= 0 ; h-- )
		{
			const unsigned char *pLineTop = &pData[uiLineByte * h];

			for( int w = 0 ; w < pBMPInfoHeader->biWidth ; w++ )
			{
				*pTexelDataTemp = pLineTop[w * 3 + 2];	pTexelDataTemp++;
				*pTexelDataTemp = pLineTop[w * 3 + 1];	pTexelDataTemp++;
				*pTexelDataTemp = pLineTop[w * 3    ];	pTexelDataTemp++;
				*pTexelDataTemp = 255;					pTexelDataTemp++;
			}
		}
	}
	else if( pBMPInfoHeader->biBitCount == 32 )
	{
		// 32Bitカラー
		for( int h = pBMPInfoHeader->biHeight - 1 ; h >= 0 ; h-- )
		{
			const unsigned char *pLineTop = &pData[uiLineByte * h];

			for( int w = 0 ; w < pBMPInfoHeader->biWidth ; w++ )
			{
				*pTexelDataTemp = pLineTop[w * 4 + 2];	pTexelDataTemp++;
				*pTexelDataTemp = pLineTop[w * 4 + 1];	pTexelDataTemp++;
				*pTexelDataTemp = pLineTop[w * 4    ];	pTexelDataTemp++;
				*pTexelDataTemp = 255;					pTexelDataTemp++;
			}
		}
	}

	// テクスチャの作成
	glGenTextures( 1, puiTexID );

	glBindTexture( GL_TEXTURE_2D, *puiTexID );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	glTexImage2D(	GL_TEXTURE_2D, 0, GL_RGBA, 
					pBMPInfoHeader->biWidth, pBMPInfoHeader->biHeight,
					0, GL_RGBA, GL_UNSIGNED_BYTE,
					pTexelData );

	float	fPrioritie = 1.0f;
	glPrioritizeTextures( 1, puiTexID, &fPrioritie );

	free( pTexelData );

	return true;
}

//-------------------------------
// TGAファイルからテクスチャ作成
//-------------------------------
bool cTextureList::createFromTGA( const unsigned char *pData, unsigned int *puiTexID )
{
#pragma pack( push, 1 )
	struct TGAFileHeader
	{
		unsigned char	tfIdFieldLength;
		unsigned char	tfColorMapType;
		unsigned char	tfImageType;
		unsigned short	tfColorMapIndex;
		unsigned short	tfColorMapLength;
		unsigned char	tfColorMapSize;
		unsigned short	tfImageOriginX;
		unsigned short	tfImageOriginY;
		unsigned short	tfImageWidth;
		unsigned short	tfImageHeight;
        unsigned char	tfBitPerPixel;
        unsigned char	tfDiscripter;
	};
#pragma pack( pop )

	TGAFileHeader	*pTgaFileHeader = (TGAFileHeader *)pData;

/*
0	イメージなし
1	インデックスカラー（256色）
2	フルカラー
3	白黒
9	インデックスカラー。RLE圧縮
A	フルカラー。RLE圧縮
B	白黒。RLE圧縮
*/
	if( pTgaFileHeader->tfImageType != 0x02 && pTgaFileHeader->tfImageType != 0x0A )
	{
		// 非対応フォーマット
		return false;
	}

	pData += sizeof( TGAFileHeader );

	unsigned char	*pTexelData = (unsigned char *)malloc( pTgaFileHeader->tfImageWidth * pTgaFileHeader->tfImageHeight * 4 ),
					*pTexelDataTemp = pTexelData;

	if( pTgaFileHeader->tfImageType == 0x02 && pTgaFileHeader->tfBitPerPixel == 24 )
	{
		// 非圧縮24Bitカラー
		if( pTgaFileHeader->tfDiscripter & 0x20 )
		{
			// 上から下へ
			for( int h = 0 ; h < pTgaFileHeader->tfImageHeight ; h++ )
			{
				const unsigned char *pLineTop = &pData[(pTgaFileHeader->tfImageWidth * 3) * h];

				for( int w = 0 ; w < pTgaFileHeader->tfImageWidth ; w++ )
				{
					*pTexelDataTemp = pLineTop[w * 3 + 2];	pTexelDataTemp++;
					*pTexelDataTemp = pLineTop[w * 3 + 1];	pTexelDataTemp++;
					*pTexelDataTemp = pLineTop[w * 3    ];	pTexelDataTemp++;
					*pTexelDataTemp = 255;					pTexelDataTemp++;
				}
			}
		}
		else
		{
			// 下から上へ
			for( int h = pTgaFileHeader->tfImageHeight - 1 ; h >= 0 ; h-- )
			{
				const unsigned char *pLineTop = &pData[(pTgaFileHeader->tfImageWidth * 3) * h];

				for( int w = 0 ; w < pTgaFileHeader->tfImageWidth ; w++ )
				{
					*pTexelDataTemp = pLineTop[w * 3 + 2];	pTexelDataTemp++;
					*pTexelDataTemp = pLineTop[w * 3 + 1];	pTexelDataTemp++;
					*pTexelDataTemp = pLineTop[w * 3    ];	pTexelDataTemp++;
					*pTexelDataTemp = 255;					pTexelDataTemp++;
				}
			}
		}
	}
	else if( pTgaFileHeader->tfImageType == 0x02 && pTgaFileHeader->tfBitPerPixel == 32 )
	{
		// 非圧縮32Bitカラー
		if( pTgaFileHeader->tfDiscripter & 0x20 )
		{
			// 上から下へ
			for( int h = 0 ; h < pTgaFileHeader->tfImageHeight ; h++ )
			{
				const unsigned char *pLineTop = &pData[(pTgaFileHeader->tfImageWidth * 4) * h];

				for( int w = 0 ; w < pTgaFileHeader->tfImageWidth ; w++ )
				{
					*pTexelDataTemp = pLineTop[w * 4 + 2];	pTexelDataTemp++;
					*pTexelDataTemp = pLineTop[w * 4 + 1];	pTexelDataTemp++;
					*pTexelDataTemp = pLineTop[w * 4    ];	pTexelDataTemp++;
					*pTexelDataTemp = pLineTop[w * 4 + 3];	pTexelDataTemp++;
				}
			}
		}
		else
		{
			// 下から上へ
			for( int h = pTgaFileHeader->tfImageHeight - 1 ; h >= 0 ; h-- )
			{
				const unsigned char *pLineTop = &pData[(pTgaFileHeader->tfImageWidth * 4) * h];

				for( int w = 0 ; w < pTgaFileHeader->tfImageWidth ; w++ )
				{
					*pTexelDataTemp = pLineTop[w * 4 + 2];	pTexelDataTemp++;
					*pTexelDataTemp = pLineTop[w * 4 + 1];	pTexelDataTemp++;
					*pTexelDataTemp = pLineTop[w * 4    ];	pTexelDataTemp++;
					*pTexelDataTemp = pLineTop[w * 4 + 3];	pTexelDataTemp++;
				}
			}
		}
	}
	else if( pTgaFileHeader->tfImageType == 0x0A )
	{
		// 圧縮24/32Bitカラー
		if( pTgaFileHeader->tfDiscripter & 0x20 )
		{
			// 上から下へ
			short	nPosX = 0,
					nPosY = 0;

            while( nPosY < pTgaFileHeader->tfImageHeight )
            {
                bool	bCompress =	((*pData) & 0x80) == 0x80;
                short	nLength = ((*pData) & 0x7F) + 1;

				pData++;

                if( bCompress )
                {
                    for( short i = 0 ; i < nLength ; i++ )
                    {
						*pTexelDataTemp = pData[2];	pTexelDataTemp++;
						*pTexelDataTemp = pData[1];	pTexelDataTemp++;
						*pTexelDataTemp = pData[0];	pTexelDataTemp++;

						if( pTgaFileHeader->tfBitPerPixel == 32 )	*pTexelDataTemp = pData[3];
						else										*pTexelDataTemp = 255;
						pTexelDataTemp++;

						nPosX++;
						if( pTgaFileHeader->tfImageWidth <= nPosX )
						{
							nPosX = 0;
							nPosY++;
						}
                    }

					if( pTgaFileHeader->tfBitPerPixel == 32 )	pData += 4;
					else										pData += 3;
                }
                else
                {
                    for( short i = 0 ; i < nLength ; i++ )
                    {
						*pTexelDataTemp = pData[2];	pTexelDataTemp++;
						*pTexelDataTemp = pData[1];	pTexelDataTemp++;
						*pTexelDataTemp = pData[0];	pTexelDataTemp++;

						if( pTgaFileHeader->tfBitPerPixel == 32 )	*pTexelDataTemp = pData[3];
						else										*pTexelDataTemp = 255;
						pTexelDataTemp++;

						if( pTgaFileHeader->tfBitPerPixel == 32 )	pData += 4;
						else										pData += 3;

                        nPosX++;
                        if( pTgaFileHeader->tfImageWidth <= nPosX )
                        {
                            nPosX = 0;
                            nPosY++;
                        }
                    }
                }
            }
		}
		else
		{
			// 下から上へ
			short	nPosX = 0,
					nPosY = pTgaFileHeader->tfImageHeight - 1;

            while( 0 <= nPosY )
            {
                bool	bCompress =	((*pData) & 0x80) == 0x80;
                short	nLength = ((*pData) & 0x7F) + 1;

				pData++;

                if( bCompress )
                {
                    for( short i = 0 ; i < nLength ; i++ )
                    {
						pTexelDataTemp = &pTexelData[(nPosX + nPosY * pTgaFileHeader->tfImageWidth) * 4];

						*pTexelDataTemp = pData[2];	pTexelDataTemp++;
						*pTexelDataTemp = pData[1];	pTexelDataTemp++;
						*pTexelDataTemp = pData[0];	pTexelDataTemp++;

						if( pTgaFileHeader->tfBitPerPixel == 32 )	*pTexelDataTemp = pData[3];
						else										*pTexelDataTemp = 255;

						nPosX++;
						if( pTgaFileHeader->tfImageWidth <= nPosX )
						{
							nPosX = 0;
							nPosY--;
						}
                    }

					if( pTgaFileHeader->tfBitPerPixel == 32 )	pData += 4;
					else										pData += 3;
                }
                else
                {
                    for( short i = 0 ; i < nLength ; i++ )
                    {
						pTexelDataTemp = &pTexelData[(nPosX + nPosY * pTgaFileHeader->tfImageWidth) * 4];

						*pTexelDataTemp = pData[2];	pTexelDataTemp++;
						*pTexelDataTemp = pData[1];	pTexelDataTemp++;
						*pTexelDataTemp = pData[0];	pTexelDataTemp++;

						if( pTgaFileHeader->tfBitPerPixel == 32 )	*pTexelDataTemp = pData[3];
						else										*pTexelDataTemp = 255;

						if( pTgaFileHeader->tfBitPerPixel == 32 )	pData += 4;
						else										pData += 3;

                        nPosX++;
                        if( pTgaFileHeader->tfImageWidth <= nPosX )
                        {
                            nPosX = 0;
                            nPosY--;
                        }
                    }
                }
            }
		}
	}

	// テクスチャの作成
	glGenTextures( 1, puiTexID );

	glBindTexture( GL_TEXTURE_2D, *puiTexID );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	glTexImage2D(	GL_TEXTURE_2D, 0, GL_RGBA, 
					pTgaFileHeader->tfImageWidth, pTgaFileHeader->tfImageHeight,
					0, GL_RGBA, GL_UNSIGNED_BYTE,
					pTexelData );

	float	fPrioritie = 1.0f;
	glPrioritizeTextures( 1, puiTexID, &fPrioritie );

	free( pTexelData );

	return true;
}

//==============
// デバッグ表示
//==============
void cTextureList::debugDraw( void )
{
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_LIGHTING );

	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D( 0.0f, (float)glutGet( GLUT_WINDOW_WIDTH ), 0.0f, (float)glutGet( GLUT_WINDOW_HEIGHT ) );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

	glEnable( GL_TEXTURE_2D );

	TextureData	*pTemp = m_pTextureList;
	float		fPosX = 0.0f,
				fPosY = glutGet( GLUT_WINDOW_HEIGHT );

	#define		DISP_SIZE	64.0f

	while( pTemp )
	{
		glBindTexture( GL_TEXTURE_2D, pTemp->uiTexID );

		glBegin( GL_TRIANGLE_FAN );
			glTexCoord2f( 1.0f, 1.0f );	glVertex2f( fPosX,             fPosY - DISP_SIZE );
			glTexCoord2f( 1.0f, 0.0f );	glVertex2f( fPosX + DISP_SIZE, fPosY - DISP_SIZE );
			glTexCoord2f( 0.0f, 0.0f );	glVertex2f( fPosX + DISP_SIZE, fPosY             );
			glTexCoord2f( 0.0f, 1.0f );	glVertex2f( fPosX,             fPosY             );
		glEnd();

		fPosX += DISP_SIZE + 2.0f;
		if( fPosX >= glutGet( GLUT_WINDOW_WIDTH ) - DISP_SIZE )
		{
			fPosX  =             0.0f;
			fPosY -= DISP_SIZE + 2.0f;
		}

		pTemp = pTemp->pNext;
	}

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_LIGHTING );
}

//================
// テクスチャ解放
//================
void cTextureList::releaseTexture( unsigned int uiTexID )
{
	TextureData	*pTemp = m_pTextureList,
				*pPrevTemp = NULL;

	while( pTemp )
	{
		if( pTemp->uiTexID == uiTexID )
		{
			pTemp->uiRefCount--;
			if( pTemp->uiRefCount <= 0 )
			{
				glDeleteTextures( 1, &pTemp->uiTexID );

				if( pPrevTemp )	pPrevTemp->pNext = pTemp->pNext;
				else			m_pTextureList = pTemp->pNext;

				delete pTemp;
			}

			return;
		}

		pPrevTemp = pTemp;
		pTemp = pTemp->pNext;
	}
}
