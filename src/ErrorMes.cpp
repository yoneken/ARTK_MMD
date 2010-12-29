//**********************
// エラーメッセージ表示
//**********************

#ifdef	_WIN32
#include	<windows.h>
#endif

#include	<stdio.h>
#include	"ErrorMes.h"

void ErrorMessage( const char *szErrMess )
{
#ifdef	_WIN32
	MessageBoxA( NULL, szErrMess, "Error", 0 );
#else
	fprintf( stderr, szErrMess );
#endif
}
