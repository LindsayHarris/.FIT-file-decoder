/*  Record field values */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include    "utils.h"
#include    "recordInfo.h"
#include   "fnProcDefn.h"



//  Record data, which may include lat/lon data.
int
fnProcMsg20( u_short msgNumber, DECODE_FIELD_VALUE *pdfv )
{
    double  latitude,  longitude;
    u_long utc = 0;
    int    elevation;
    int	    iBitValue = 0;		// Monitor all data being set.

    for( ; pdfv->eRetCode != dfvEND; ++pdfv ) {
	//  Find the name!
	switch( pdfv->fdDefnNumber )
	{
	case 253:	    // UTC timestamp, FIT epoch.
	    utc = adjustTimeToUnixEpoch( pdfv->u.uLong );
	    if( utc > MIN_UTC_TIMESTAMP )
		iBitValue |= 0x01;	    // Legitimate.
	    break;

	case  0:	    // Latitude
	    latitude = semiCircleToDegrees( pdfv->u.uLong );
	    iBitValue |= 0x02;
	    break;

	case  1:	    // Longitude
	    longitude = semiCircleToDegrees( pdfv->u.uLong );
	    iBitValue |= 0x04;
	    break;

	case  2:	    // Longitude
	    elevation = pdfv->u.uLong / 5.0 - 500.0;
	    iBitValue |= 0x08;
	    break;

	default:
	    break;	    // Not interested.
	}
    }
    //  Output the time, latitude and longitude if all available, plus elevation if there.
    if( (iBitValue & 0x7) == 0x7 && utc != G.gpsLastTime ) {
	printf( "!locn,%d,%f,%f", utc, latitude, longitude );

	if( iBitValue & 0x08 )
	    printf( ",%d", elevation );
	printf( "\n" );

	G.gpsLastTime = utc;
    }


    return  0;	    // Is good.
}

// TIMESTAMP_CORRELATION record - type 162
// Connects UTC to internal timestamp, which starts from 0 when
// turned on.  THUS,  all events can be converted to a UTC timestamp
// without confusion.

int
fnProcMsg162( u_short msgNumber, DECODE_FIELD_VALUE *pdfv )
{
    u_long utc = 0;
    u_long sysTS = 0;

    for( ; pdfv->eRetCode != dfvEND; ++pdfv ) {
	//  Find the name!
	switch( pdfv->fdDefnNumber )
	{
	case 253:	    // UTC timestamp, FIT epoch.
	    utc = pdfv->u.uLong;
	    break;

	case  1:	    // System timestamp - time since power on
	    sysTS = pdfv->u.uLong;
	    break;

	default:
	    break;	    // Not interested.
	}
    }
    if( utc > 0 && sysTS > 0 ) {
	//  Have what we want.
	setEpochAdjustment( sysTS, utc );	// All set up.

	return  0;	    // Is good.
    }

    return -1;		    // No value obtained.
}



/* ******************************************************************
 *	GPS Metadata
 */

int
fnProcMsg160( u_short msgNumber, DECODE_FIELD_VALUE *pdfv )
{
    double  latitude,  longitude;
    u_long utc = 0;
    int    elevation;
    int	    iBitValue = 0;		// Monitor all data being set.

    for( ; pdfv->eRetCode != dfvEND; ++pdfv ) {
	//  Find the name!
	switch( pdfv->fdDefnNumber )
	{
	case 253:	    // UTC timestamp, FIT epoch.
	    utc = adjustTimeToUnixEpoch( pdfv->u.uLong );
	    iBitValue |= 0x01;
	    break;

	case  1:	    // Latitude
	    latitude = semiCircleToDegrees( pdfv->u.uLong );
	    iBitValue |= 0x02;
	    break;

	case  2:	    // Longitude
	    longitude = semiCircleToDegrees( pdfv->u.uLong );
	    iBitValue |= 0x04;
	    break;

	case  3:	    // Longitude
	    elevation = pdfv->u.uLong / 5.0 - 500.0;
	    iBitValue |= 0x08;
	    break;

	default:
	    break;	    // Not interested.
	}
    }
    //  Output the time, latitude and longitude if all available, plus elevation if there.
    if( (iBitValue & 0x7) == 0x7 && utc != G.gpsLastTime ) {
	printf( "!locn,%d,%f,%f", utc, latitude, longitude );

	if( iBitValue & 0x08 )
	    printf( ",%d", elevation );
	printf( "\n" );

	G.gpsLastTime = utc;
    }


    return  0;	    // Is good.
}

//  Message 23 is device info, specifically manufacturer's name - for heading line.
int
fnProcMsg23( u_short msgNumber, DECODE_FIELD_VALUE *pdfv )
{
#define	M23_CHAR_BUF_SZ 32

    int  ii;
    u_long manu_id;

    static  int bOnce = 0;


    char makerName[ M23_CHAR_BUF_SZ + 1 ];
    char prodName[ M23_CHAR_BUF_SZ + 1 ];

    extern  REC_MAP  manufacturer[];

    if( bOnce )
	return  0;	// Been here before.

    makerName[0] = prodName[0] = '\0';

    for( ; pdfv->eRetCode != dfvEND; ++pdfv ) {
	//  Find the name!
	switch( pdfv->fdDefnNumber )
	{
	case  2:	    // Manufacturer index into manufacturer array.
	    manu_id = pdfv->u.uLong;
	    for( ii = 0; manufacturer[ii].rm_Id >= 0; ++ii ) {
		if( manufacturer[ii].rm_Id == manu_id ) {
		    //  Bingo.
		    strncpy( makerName, manufacturer[ii].rm_name, M23_CHAR_BUF_SZ );
		    makerName[ M23_CHAR_BUF_SZ ] = '\0';

		    break;
		}
	    }
	    if( makerName[0] == '\0' ) {
		snprintf( makerName, M23_CHAR_BUF_SZ, "Unknown maker #%d", manu_id );
	    }
	    bOnce = 1;
	    break;

	case  27:	// Product name - string.
	    ii = strlen( pdfv->u.uByte );
	    if( ii > M23_CHAR_BUF_SZ )
		ii = M23_CHAR_BUF_SZ;

	    strncpy( prodName, pdfv->u.uByte, ii );
	    break;

	default:
	    break;	    // Not interested.
	}
    }

    if( prodName[0] == '\0' )
	strcpy( prodName, "-unknown product-" );

    printf( "@%s/%s\n", makerName, prodName );

    return  0;	    // Is good.

}


/* ******************************************************************
 *	Barometric data.
 */

int
fnProcMsg209( u_short msgNumber, DECODE_FIELD_VALUE *pdfv )
{
    u_long  utc;
    double  atmosPressure;
    int	    iBitValue = 0;		// Monitor all data being set.

    for( ; pdfv->eRetCode != dfvEND; ++pdfv ) {
	//  Find the name!
	switch( pdfv->fdDefnNumber )
	{
	case 253:	    // UTC timestamp, FIT epoch.
	    utc = adjustTimeToUnixEpoch( pdfv->u.uLong );
	    iBitValue |= 0x01;
	    break;

	case  2:	    // Barometric pressure, Pa
	    atmosPressure = pdfv->u.uLong / 100.0;	// hPa used by weather bureau
	    iBitValue |= 0x02;
	    break;

	default:
	    break;	    // Not interested.
	}
    }
    //  Output the time, latitude and longitude if all available, plus elevation if there.
    if( (iBitValue & 0x3) == 0x3 && utc != G.gpsLastTime ) {
	printf( "!baro,%d,%f\n", utc, atmosPressure );

    }


    return  0;	    // Is good.
}
