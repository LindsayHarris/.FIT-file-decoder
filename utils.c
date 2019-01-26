/*  Assorted utility functions   */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "recordInfo.h"
#include "utils.h"

static TYPE_HANDLING typeDetails[] = 
{
  { .uInvalid.en=0xFF, 0x00, 1 },	    // enum
  { .uInvalid.sint8=0x7F, 0x01, 1 },	    // sint8
  { .uInvalid.uint8=0xFF, 0x02, 1 },	    // uint8
  { .uInvalid.sint16=0x7FFF, 0x83, 2 },   // sint16
  { .uInvalid.uint16=0xFFFF, 0x84, 2 },   // uint16
  { .uInvalid.sint32=0x7FFFFFFF, 0x85, 4 },	// sint32
  { .uInvalid.uint32=0xFFFFFFFF, 0x86, 4 },	// uint32
  { .uInvalid.string=NULL, 0x07, 0 },	    // null terminated string.
  { .uInvalid.float32=0xFFFFFFFF, 0x88, 4 },	// float32
  { .uInvalid.float64=0xFFFFFFFFFFFFFFFF, 0x89, 8 },	    // float64
  { .uInvalid.uint8z=0x00, 0x0A, 1 },		// uint8z
  { .uInvalid.uint16z=0x0000, 0x8B, 2 },		// uint16z
  { .uInvalid.uint32z=0x00000000, 0x8C, 4 },	// uint32z
  { .uInvalid.byte=NULL, 0x0D, 0 },		// array of bytes
  { .uInvalid.sint64=0x7FFFFFFFFFFFFFFF, 0x8E, 8 },	// sint64
  { .uInvalid.uint64=0xFFFFFFFFFFFFFFFF, 0x8F, 8 },	// uint64
  { .uInvalid.uint64z=0x0000000000000000, 0x90, 8 },	// uint64z

  { .uInvalid.en=0, 0, -1 },			// END MARKER
};


	// Next field is the number of seconds between the unix epoch and
	// the FIT epoch - very close to 20 years!
#define	FIT_EPOCH_OFFSET    631065600

//static u_long  G_fitEpochOffset = FIT_EPOCH_OFFSET;


// String/byte arrays are stored here and returned to the caller for
// that type of field.   Note there is only one buffer, so contents
// must be used before the next call to here!
static  unsigned char  aByteArray[260];	    // Small buffer at end.

DECODE_FIELD_VALUE
decodeFieldValue( FILE *pFD, FIELD_DATA *pFieldData, int bChangeEndian )
{
    int   ii;
    static DECODE_FIELD_VALUE dfvRet;			// Returned to caller.
    //  Step 1 - find the relevant data type.

    for( ii = 0; typeDetails[ii].fitLength >= 0; ++ii ) {
	if( typeDetails[ii].fitBaseType == pFieldData->fdType ) {
	    if( fread( &aByteArray, pFieldData->fdSize, 1, pFD ) != 1  ) {
		fprintf( stderr, "Data read in decodeFieldValue fails\n" );
		dfvRet.eRetCode = dfvERROR;
		dfvRet.u.uByte = "Data read in decodeFieldValue fails\n";

		return  dfvRet;
	    }
	    //  May need to swap the bytes to match our endianess.
	    //  The MSbit of the type field is set for types which are endian-sensitive.
	    if( bChangeEndian && (pFieldData->fdType & 0x80) ) {
		int  jj;
		u_char  auch[8];	// Temporary buffer to swap.

		for( jj = 0; jj < pFieldData->fdSize; ++jj )
		    auch[pFieldData->fdSize - jj - 1] = aByteArray[jj];
		for( jj = 0; jj < pFieldData->fdSize; ++jj )
		    aByteArray[jj] = auch[jj];
	    }

	    dfvRet.fdDefnNumber = pFieldData->fdDefnNumber;

	    //  Check for a valid entry.
	    if( memcmp( &typeDetails[ii].uInvalid.string, aByteArray,
						typeDetails[ii].fitLength ) == 0 ) {
		//   Matches the invalid pattern, so return as invalid.
		dfvRet.eRetCode = dfvINVALID;

		if( G.debug & DEBUG_DATA_INVALID ) {
		    fprintf( stderr, "Invalid data: global msg = %d, field type = %d/0x%x\n", 
					G.msgNumber, pFieldData->fdType, pFieldData->fdType );
		}
		return dfvRet;
	    }

	    switch( pFieldData->fdType ) {
	    case 0x00:		// enum type
	    case 0x02:		// 8 bits unsigned
	case 0x0A:		// !!!!! INVALID setting is DIFFERENT
		dfvRet.eRetCode = dfvINT;
		dfvRet.u.uLong = *((unsigned char *)aByteArray);
		break;

	    case 0x01:		// signed 8 bits
		dfvRet.eRetCode = dfvINT;
		dfvRet.u.uLong = *((char *)aByteArray);
		break;

	    case 0x07:		// NUL terminated string.
		dfvRet.eRetCode = dfvSTRING;
		dfvRet.u.uByte = &aByteArray[0];
		aByteArray[pFieldData->fdSize] = '\0';	    // JUST IN CASE (room is availabele).
		break;

	    case 0x0D:		// Byte array.
		dfvRet.eRetCode = dfvARRAY;
		dfvRet.u.uByte = &aByteArray[0];
		break;

	    case 0x83:		// signed 16 bits
		dfvRet.eRetCode = dfvINT;
		dfvRet.u.uLong = *((short *)aByteArray);
		break;

	    case 0x84:		// unsigned 16 bits
	case 0x8B:		// ZERO is undefined!!!!
		dfvRet.eRetCode = dfvINT;
		dfvRet.u.uLong = *((unsigned short *)aByteArray);
		break;

	    case 0x85:		// signed 32 bits
	case 0x8C:		// ZERO is undefined!!!!
		dfvRet.eRetCode = dfvINT;
		dfvRet.u.uLong = *((int *)aByteArray);
		break;

	    case 0x86:		// unsigned 32 bits
		dfvRet.eRetCode = dfvINT;
		dfvRet.u.uLong = *((unsigned int *)aByteArray);
		break;

	    case 0x88:		// 32 bit float
		dfvRet.eRetCode = dfvDOUBLE;
		dfvRet.u.uDouble = *((float *)aByteArray);
		break;

	    case 0x89:		// 32 bit float
		dfvRet.eRetCode = dfvDOUBLE;
		dfvRet.u.uDouble = *((double *)aByteArray);
		break;

	    case 0x8e:		// signed 64 bit
		dfvRet.eRetCode = dfvINT;
		dfvRet.u.uLong = *((long *)aByteArray);
		break;

	    case 0x8f:		// signed 64 bit
	case 0x90:		// ZERO is UNDEFINED!!!!!
		dfvRet.eRetCode = dfvINT;
		dfvRet.u.uLong = *((unsigned long *)aByteArray);
		break;
	    }
	    return dfvRet;
	}
    }
    if( G.debug & DEBUG_DATA_FIELD ) {
	fprintf( stderr, "Global message %d uses invalid base type 0x%x\n",
						G.msgNumber, pFieldData->fdType );
    }
    dfvRet.eRetCode = dfvERROR;
    dfvRet.u.uByte = "*** FAILED TO FIND entry\n";

    return dfvRet;
}


//  Check file checksum is correct.
//  Calculation starts at current file location, and the file pointer
//  is left at the last position of the calculation.   This can be
//  used to determine the length of the file, for instance.
//
//  Parameters:
//	pF    - the FILE * pointer to read the file.
//	cbCRC - number of bytes to read, entire file if == 0
//  Returns:
//	1 if correct, else 0

int
checkFileCRC( FILE *pF, u_long cbCRC )
{
    u_short  crc;
    int	     ch;
    u_long   cbRead;

    //   Header has a CRC on it - this table enables the checking.
    const u_short  crc_table[16] = {
	0x0000, 0xcc01, 0xd801, 0x1400, 0xf001, 0x3c00, 0x2800, 0xe401,
	0xa001, 0x6c00, 0x7800, 0xb401, 0x5000, 0x9c01, 0x8801, 0x4400
    };

    crc = 0;
    cbRead = 0;
    while( (ch = fgetc( pF )) != EOF && (cbCRC == 0 || cbRead < cbCRC) ) {
	u_short  tmp;

	++cbRead;

	//  Lower 4 bits
	tmp = crc_table[ crc & 0xf ];
	crc = (crc >> 4) & 0xfff;
	crc = crc ^ tmp ^ crc_table[ch & 0xf];

	//  Upper 4 bits
	tmp = crc_table[ crc & 0xf ];
	crc = (crc >> 4) & 0xfff;
	crc = crc ^ tmp ^ crc_table[ (ch >> 4) & 0xf ];
    }


    return  (crc == 0);
}

//  Convert "semicircles" to fractional degrees.

double
semiCircleToDegrees( int  semiC )
{
    //  Semicircles are scaled in some strange way.

    return  semiC / 2147483648.0 * 180.0;
}


/* *****************************************************************
 *	adjustTimeToUnixEpoch
 *  The timestamp values in FIT files are based on a different epoch
 *  to C library functions.  But some FIT timestamps are not UTC
 *  values but just the system's internal clock.   If the latter is
 *  the case,  the value is returned adjusted to the start of this data.
 *  Parameter:
 *	time - the time value in a FIT record timestamp.
 *  Returns:
 *	The time adjusted to unix epoch if appropriate, otherwise
 *	returns parameter, as it is likely UTC in unix epoch.
 */

u_long
adjustTimeToUnixEpoch( u_long  time )
{
    if( time < MIN_UTC_TIMESTAMP )
	time += G.fitEpochOffset;

    return  time;
}


/* *********************************************************
 *	setEpochAdjustment
 *  Function to adjust the FIT epoch (which is about 20 years
 *  after the UNIX epoch) to the UNIX time, and also adjust
 *  for the delay between starting the local clock and finding
 *  the GPS time for records.
 *
 *  Parameters:
 *	sysTimeStamp - usually just the elapsed time from
 *			powering on the device.
 *	utcTimeStamp - the time derived from the GPS system,
 *			based the FIT time epoch.
 *  Returns:
 *	<nothing>
 */

void
setEpochAdjustment( u_long  sysTimeStamp, u_long  utcTimeStamp )
{
    if( utcTimeStamp >= MIN_UTC_TIMESTAMP ) {
	//  Have a UTC timevalue, and the system timestamp offset.
	//  GPS records use the system timestamp (i.e. seconds since turned on)
	//  so adjust the newly received UTC timestamp back to turn on time.

	G.fitEpochOffset = utcTimeStamp + FIT_EPOCH_OFFSET - sysTimeStamp;
    }

    return;
}