/*   Decoding functions for .FIT files */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "fitStructs.h"		// Structures used in the file.
#include "recordInfo.h"		// Managing records

#include "decode.h"		// Generally accessable information about this file.
#include "utils.h"
#include "fitFieldID.h"
#include "fnProcDefn.h"


//  Record formats are defined in the file,  and there can be as many as
//  16 different record formats at any one time.  But any record fields 
//  can be changed when needed.

REC_INFO   recInfo[NUMBER_RI];

//  Keep the currently being processed definition header
dfnMsgHeader  dfnh;

static int checkCRC( const fitHeader *pFH );

//  Header is the first 14 bytes of the file, though older
//  versions use only 12 (no CRC).
//  Parameters:
//	pFD	   - the FILE structure to read from.
//	pFitHeader - where the data is placed.
//  Returns:
//	0 on error,  1 if pFitHeader is filled in and valid.

int
readHeader( FILE *pFD, fitHeader *pFitHeader )
{
    int  sz;

    //  Make sure to be at start of file.
    rewind( pFD );

    sz = fread( pFitHeader, sizeof( fitHeader ), 1, pFD );
    if( sz != 1 ) {
	fprintf( stderr, "Header read returns %d - should be 1\n", sz );
	if( feof( pFD ) ) {
	    fprintf( stderr, "Reached end of file!\n" );
	} else {
	    //  Some sort of error.
	    perror( "Error reading file" );
	}
	return 0;
    }


    if( strncmp( pFitHeader->achSign, ".FIT", 4 ) ) {
	fprintf( stderr, "This is not a .fit file - header signature does not match\n" );

	return  0;
    }

    if( pFitHeader->hdrSize > 12 ) {

	rewind( pFD );

	//  Earlier versions of the .fit files had no CRC on the header.
	if( !checkFileCRC( pFD, pFitHeader->hdrSize ) ) {
	    fprintf( stderr, "File header CRC check fails\nEither not a .fit file or has been changed.\n" );
	    return  0;
	}
    }

    //  All is good - move pointer to start of data.
    fseek( pFD, pFitHeader->hdrSize, SEEK_SET );

    return 1;
}



//  Read the next record in the file,  and return the data.
//  Parameters:
//	pFD - the file descriptor.
//  Returns:
//	Address of static buffer?

int
readRecord( FILE *pFD )
{
    //  First character of message is a header byte, defining the record.
    int  headerByte;		// See what sort of message it is.  Just a byte!
    int  localMsgNdx;		// Data message layout id
    int  ii;			// Classic loop variable.

    extern REC_INFO recInfo[NUMBER_RI];		// Layout data all in here.

    headerByte = fgetc( pFD );
    if( headerByte == EOF )
	return 0;			// !!!!! WILL PROBABLY CHANGE.

    //  Most header records encode format index in lower 4 bits.
    if( headerByte & RH_COMPTIMESTAMP ) {
	//  Only 2 bits for format data, and shifted left!
printf( "\n\n\n***********************\n C O M P R E S S E D  T I M E   S T A M P   HDR\n*******\n");
	localMsgNdx = (headerByte & RH_MSG_TYPE_COMP) >> 5;	// Only 4 bits in header.
    } else {
	localMsgNdx = headerByte & RH_MSG_TYPE;	// Only 4 bits in header.
    }

    if( G.debug & DEBUG_REC_HEADER ) {
	fprintf( stderr, "Header: 0x%x, local message: %d, file position: 0x%x/0%o/%d\n",
			headerByte, localMsgNdx, ftell( pFD ) - 1, ftell(pFD) - 1, ftell(pFD) - 1 );
    }
    //  Good to go - what do we have?
    if( headerByte & RH_DEFN ) {
	//   A header record.
	extern  dfnMsgHeader   dfnh;		// File structure for header.
	long lOffset;
	msgNumber  *pBadFileID;

	lOffset = ftell( pFD );		// Ensure file pointer is at correct place after read.

	if( fread( &dfnh, CB_DFN_MSG_HEADER, 1, pFD ) != 1 ) {
	    if( !feof( pFD ) ) {
		//  Not so good.
		fprintf( stderr, "No header data read, but not EOF\n" );
	    }
	    return  0;		// All done for now.
	}
	lOffset += CB_DFN_MSG_HEADER;

	//  Global message number is endian dependent!
	recInfo[localMsgNdx].bChangeEndian = (dfnh.architecture ^ G.bIsBigEndian) & 0x01;
	if( recInfo[localMsgNdx].bChangeEndian ) {
	    //  Swap bytes
	    union {
		unsigned char ach[2];
		unsigned short ush;
	    } _swap;
	    unsigned char achTemp;

	    _swap.ush = dfnh.globalMsgNum;
	    achTemp = _swap.ach[0];
	    _swap.ach[0] = _swap.ach[1];
	    _swap.ach[1] = achTemp;

	    dfnh.globalMsgNum = _swap.ush;
	}
	if( G.debug & DEBUG_DEFN ) {
	    pBadFileID = findMsgNumber( dfnh.globalMsgNum );
	    fprintf( stderr,
			"Defn header: global msg = \"%s\"/%d/0x%x, local msg num = %d, num fields = %d, file location = 0x%x/0%o\n",
			pBadFileID->pchName, dfnh.globalMsgNum, dfnh.globalMsgNum, localMsgNdx,
			dfnh.numFields, ftell( pFD ), ftell( pFD ) );
	}

	recInfo[localMsgNdx].riNumber = dfnh.numFields;
	G.msgNumber = recInfo[localMsgNdx].riGlobalMessageNumber = dfnh.globalMsgNum;

	//  Grab the field data.
	for( ii = 0; ii < dfnh.numFields; ++ii ) {
	    if( fread( &recInfo[localMsgNdx].riFd[ii], CB_FIELD_DATA, 1, pFD ) != 1 ) {
		fprintf( stderr, "fread() of field data %d fails\n", ii );
		return  0;
	    }
	    if( G.debug & DEBUG_DEFN_FIELD ) {
		fprintf( stderr, "field[%d] => (defnNum = %d, cb = %d, type = 0x%02x)\n", ii,
			    recInfo[localMsgNdx].riFd[ii].fdDefnNumber,
			    recInfo[localMsgNdx].riFd[ii].fdSize,
			    recInfo[localMsgNdx].riFd[ii].fdType );
	    }
	}
	//  Is there developer data?  Need to add it.
	if( headerByte & RH_SPECIFIC ) {
	    //  Yep - developer data - next byte is the number of fields.
	    int  numDevFields;
	    int  jj;

	    numDevFields = fgetc( pFD );
	    if( numDevFields == EOF ) {
		fprintf( stderr, "Unexpected EOF/error for developer field count at file location %l\n", ftell( pFD ) );
		return 0;
	    }
	    for( jj = ii; jj < (numDevFields + ii); ++jj ) {
		if( fread( &recInfo[localMsgNdx].riFd[jj], CB_FIELD_DATA, 1, pFD ) != 1 ) {
		    fprintf( stderr, "fread() of field data %d fails\n", jj );
		    return  0;
		}
		if( G.debug & DEBUG_DEV_DEFN_FIELD ) {
		    fprintf( stderr, "DEV field[%d] => (defnNum = %d, cb = %d, type = 0x%02x)\n",
			    jj, recInfo[localMsgNdx].riFd[jj].fdDefnNumber,
			    recInfo[localMsgNdx].riFd[jj].fdSize,
			    recInfo[localMsgNdx].riFd[jj].fdType );
		}
	    }
	}

	return  1;	    // All done?
    } else {
	//  Data record.
	u_char  dataBuffer[ 260 ];	// Maximum record size is 255 bytes.
	int	iDecodeStatus;		// How the specific purpose decoder likes data.
	FIELD_DATA *pfData;
	REC_INFO   *pRI;
	extern  u_char  achFieldBuffer[];
	DECODE_FIELD_VALUE   dfv[ RI_MAX_FIELDS ];	    // Hold data for record processing
	DECODE_FIELD_VALUE   *pdfv;			// Walk through above, ignore INVALID data
	msgNumber   *pMsg;
	
	pRI = &recInfo[localMsgNdx];
	pfData = &pRI->riFd[0];

	if( G.debug & DEBUG_DATA ) {
	    pMsg = findMsgNumber( recInfo[localMsgNdx].riGlobalMessageNumber );
	    fprintf( stderr, "localMsgNdx = %d, # fields = %d, type = %s\n",
			localMsgNdx, recInfo[localMsgNdx].riNumber, pMsg->pchName );
	}
	//  Read fields in separately, for easier processing.
	pdfv = dfv;
	for( ii = 0; ii < pRI->riNumber; ++ii, ++pfData ) {

	    //  Some fields have no data.
	    *pdfv = decodeFieldValue( pFD, pfData, recInfo[localMsgNdx].bChangeEndian );

	    if( G.debug & DEBUG_DATA_FIELD ) {
		fprintf( stderr, "defNum = %3d, type # = 0x%02x, value = ",
				pfData->fdDefnNumber, pfData->fdType );
		//  Output depends upon the type of value returned.
		switch( pdfv->eRetCode ) {
		case dfvINT:
		    fprintf( stderr, "%d\n", pdfv->u.uLong );
		    break;

		case  dfvDOUBLE:
		    fprintf( stderr, "%f\n", pdfv->u.uDouble );
		    break;

		case  dfvSTRING:
		    fprintf( stderr, "'%s'\n", pdfv->u.uByte );
		    break;

		case  dfvARRAY:
		    fprintf( stderr, " -- byte array --\n" );
		    break;

		case  dfvINVALID:
		    fprintf( stderr, " -- invalid --\n" );
		    break;
		}
	    }

	    if( pdfv->eRetCode != dfvINVALID )
		++pdfv;				// Keep it!
	}

	pdfv->eRetCode = dfvEND;		// All done
	iDecodeStatus = processRecord( recInfo[localMsgNdx].riGlobalMessageNumber, dfv );

	return 1;	// Ready for next one.
    }

}
