/* Experiment with decoding ANT/.fit files */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "fitStructs.h"
#include "decode.h"
#include "recordInfo.h"
#include "utils.h"

GLOBAL  G = {
    0,			// debug:  debug output control
    0,			// bIsBigEndian:  0 == little endian
    0,			// Last GPS time record - stop high frequency settings
    0,			// Current global message number being processed.
};					// Global variables.

u_char  achFieldBuffer[ GLOBAL_REC_SZ ];	// Record fields go in here from file.

void Usage( int, const char *, const char * );

int
main( int argc, char **argv )
{
    int	    iRes;	    // Return codes
    FILE   *fFit;
    fitHeader  fh;	    // Header record in file.
    int	    cbFileExCRC;    // Length of file excluding 2 CRC bytes at end.
    union {
	unsigned short es;
	unsigned char  eb[2];
    } endian;

    char  achMessage[256];	    // For whatever
    int  opt;

G.debug = 0xffffff20;
G.debug = 0x0020;
G.debug = 0;

    while( (opt = getopt( argc, argv, "ad:" ) ) != -1 ) {
	switch( opt ) {
	case 'd':
	    G.debug = atoi( optarg );
	    sscanf( optarg, "%li", &G.debug );
	    break;

	case 'a':	    // Do not filter output data.
	    G.outputSelect = OUTPUT_GPS;	// May be refined later.
	    break;

	default:    // '?'
	    snprintf( achMessage, sizeof( achMessage ),
			"Unrecognised option character '%c'", opt );
	    Usage( 1, argv[0], achMessage );	    // Does not return.
	}
    }
    //  Only one parameter required - but must be just one.
    if( (optind + 1) != argc ) {
	Usage( 1,  argv[0],
		optind == argc ? "Missing file name" : "Only one file name" );
    }

    // Determine endianness of this CPU.
    endian.es = 1;
    G.bIsBigEndian = (endian.eb[0] == 0);		//   Global result, use in decoding.


    fFit = fopen( argv[optind], "rb" );
    if( fFit == NULL ) {
	//  Not too happy.
	int  xErrno = errno;		// For safety
	char achMsg[256];

	snprintf( achMsg, sizeof( achMsg ), "%s: %s", argv[optind], strerror( xErrno ) );

	Usage( 2, argv[0], achMsg );
    }
    //  Check the file's CRC.
    if( checkFileCRC( fFit, 0 ) == 0 ) {
	Usage( 3, argv[optind], "File checksum comparison fails\n" );
    }

    //  Reduce the file size by the 2 bytes of CRC at the end of the file.
    //  Otherwise it thinks it's the start of a new record, making us unhappy.
    cbFileExCRC = ftell( fFit ) - 2;
    

    iRes = readHeader( fFit, &fh );

    while( ftell( fFit ) < cbFileExCRC && readRecord( fFit ) )
	    ;

    exit( 0 );
}

char *achArgs[] = {"-d values are thus:=- ",
	 "DEBUG_DEFN          0x0001" ,
	 "DEBUG_DEFN_FIELD    0x0002" ,
	 "DEBUG_DEV_DEFN      0x0004      // Debug developer fields." ,
	 "DEBUG_DEV_DEFN_FIELD    0x0008" ,
	 "DEBUG_REC_HEADER    0x0010      // Print record header, and file position" ,
	 "DEBUG_DATA          0x0020      // Header information for data records." ,
	 "DEBUG_DATA_FIELD    0x0040      // Show field data, including values." ,
	 "DEBUG_DATA_INVALID  0x0080      // Report invalid values!" ,
	 "DEBUG_GLOBL_MSG     0x0100" ,
};
#define NUM_PARAMS  (sizeof( achArgs ) / sizeof( achArgs[0] ))

//  Common error then exit reporting function.
//  Parameters:
//	exitCode - value passed to exit()
//	progName - our name
//	msg      - additional information for exit.
//  Returns:
//	DOES NOT RETURN.

void
Usage( int exitCode, const char *progName, const char *msg )
{
    int   ii;

    fprintf( stderr, "Usage: %s -d XX -n <.fit file>\n", progName );
    if( strlen( msg ) > 0 )
	fprintf( stderr, "%s\n", msg );

    for( ii = 0; ii < NUM_PARAMS; ++ii )
	fprintf( stderr, "%s\n", achArgs[ii] );

    exit( exitCode );
}