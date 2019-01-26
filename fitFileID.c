//  Basically just defines the assorted values in FIT files

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "fitFieldID.h"
//#include "mesgNumVals.h"

fileIDcodes fileCodes[] = {
#include "fileDetails.h"
};

fileIDcodes unknownFileCode = { 0xfff, "-unknown-", "is not known" };

#define  NUM_FILE_CODES (sizeof( fileCodes ) / sizeof( fileIDcodes ))

//  Message numbers - the global values used to map field values to global.
msgNumber  msgNums[] = {
#include  "mesgNumVals.h"
};

#define	NUM_MSG_VALS    (sizeof( msgNums ) / sizeof( msgNumber ))
msgNumber  unknownMsgNum = { 0, 0, "-unknown-" };

fileIDcodes *
findFileID( int id )
{
    int  ii;

    for( ii = 0; ii < NUM_FILE_CODES; ++ii ) {
	if( fileCodes[ii].fileID == id )
	    return  &fileCodes[ii];
    }
    
    return  &unknownFileCode;
    
}

msgNumber  *
findMsgNumber( u_short id )
{
    u_short   ii;

    for( ii = 0; ii < NUM_MSG_VALS; ++ii ) {
	if( msgNums[ii].msgNum == id ||
	    (id > msgNums[ii].msgNum && id <= msgNums[ii].msgMaxValue) ) {

	    return  &msgNums[ii];
	}
    }

    return  &unknownMsgNum;
}


#if 0
main( int argc, char **argv )
{
    int  ii;
    int  count = sizeof( fileCodes ) / sizeof( fileIDcodes );
    printf( "Found %d file IDs\n", count );

    for( ii = 0; ii < count; ++ii ) {
	printf( "%2d: id = %d, name = %s\n", ii, fileCodes[ii].fileID, fileCodes[ii].pchShortName );
    }

    return 0;
}
#endif