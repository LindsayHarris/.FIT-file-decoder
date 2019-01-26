/*   Initial part of message processing functions */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include    "utils.h"
#include    "recordInfo.h"
#include   "fnProcDefn.h"

int fnProcMsg0( u_short, DECODE_FIELD_VALUE * );
int fnProcMsg1( u_short, DECODE_FIELD_VALUE * );
int fnProcMsg20( u_short, DECODE_FIELD_VALUE * );	// record
int fnProcMsg23( u_short, DECODE_FIELD_VALUE * );	// device info
int fnProcMsg160( u_short, DECODE_FIELD_VALUE * );	// GPS metadata
int fnProcMsg162( u_short, DECODE_FIELD_VALUE * );	// timestamp correlation
int fnProcMsg209( u_short, DECODE_FIELD_VALUE * );	// barometric data
int fnProcMsg0xFF00( u_short, DECODE_FIELD_VALUE * );	// manufacturer's usage
// int fnProcMsg2( u_short, DECODE_FIELD_VALUE * );
// int fnProcMsg3( u_short, DECODE_FIELD_VALUE * );
// int fnProcMsg4( u_short, DECODE_FIELD_VALUE * );
// int fnProcMsg5( u_short, DECODE_FIELD_VALUE * );
// int fnProcMsg6( u_short, DECODE_FIELD_VALUE * );
// int fnProcMsg7( u_short, DECODE_FIELD_VALUE * );
// int fnProcMsg8( u_short, DECODE_FIELD_VALUE * );
// int fnProcMsg9( u_short, DECODE_FIELD_VALUE * );

//  Debug print of record.
void printProcMsgX( u_short msgNumber, DECODE_FIELD_VALUE *pdfv );


FN_PROC_MSG  fnProcCallTable[] = {
#include   "fnProcMsg.h"
};

#define	NUMBER_PROCESS	(sizeof( fnProcCallTable ) / sizeof( fnProcCallTable[0] ))


//   Process message, if code to do so.
//   Returns: 0 is good, 1 is error, 2 is not handled.

int
processRecord( u_short msgNumber, DECODE_FIELD_VALUE *pdfv )
{
    int   ii;

    for( ii = 0; ii < NUMBER_PROCESS; ++ii ) {
	//  Is it in the table?   [Not all functions are handled]
	if( fnProcCallTable[ii].fnpMsgId == msgNumber ||
		(fnProcCallTable[ii].msgMaxNumber >= fnProcCallTable[ii].msgNumber &&
		 fnProcCallTable[ii].msgNumber >= msgNumber &&
		 fnProcCallTable[ii].msgMaxNumber <= msgNumber) ) {
	    //   Got this one.
	    int  iRet;

	    iRet = fnProcCallTable[ii].msgProcFunc( msgNumber, pdfv );

	    return iRet;
	}
    }
    //  Terra Incognita!

    printProcMsgX( msgNumber, pdfv );

    return 2;
}

//  Workers.

//  DUMMY for debug print.
void
printProcMsgX( u_short msgNumber, DECODE_FIELD_VALUE *pdfv )
{
    if( G.debug & DEBUG_GLOBL_MSG ) {
	fprintf( stderr, "NO FUNCTION TO HANDLE global message number = %d\n", msgNumber );

	for( ; pdfv->eRetCode != dfvEND; ++pdfv ) {
	    printf( "%d => ", pdfv->fdDefnNumber );
	    switch( pdfv->eRetCode ) {
	    case  dfvINT:
		printf( "%d/0x%x, ", pdfv->u.uLong, pdfv->u.uLong );
		break;

	    case  dfvDOUBLE:
		printf( "%f,", pdfv->u.uDouble );
		break;

	    case  dfvARRAY:
		printf( "%s, ", pdfv->u.uByte );
		break;

	    default:
		printf( "%d - type not handled, ", pdfv->eRetCode );
		break;
	    }
	    printf( "\n" );
	}
	printf( "\n" );
    }

    return;
}

int
fnProcMsg0( u_short msgNumber, DECODE_FIELD_VALUE *pdfv )
{
    printProcMsgX( msgNumber, pdfv );
    
    return 0;
}

int
fnProcMsg1( u_short msgNumber, DECODE_FIELD_VALUE *pdfv )
{
    printProcMsgX( msgNumber, pdfv );
    
    return 0;
}

int
fnProcMsg0xFF00( u_short msgNumber, DECODE_FIELD_VALUE *pdfv )
{
    if( G.debug & DEBUG_GLOBL_MSG )
	fprintf( stderr, "fnProcMsgoxFF00 - manufaturer restricted: " );

    printProcMsgX( msgNumber, pdfv );
    
    return 0;
}