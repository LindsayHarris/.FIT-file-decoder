/*   The values used in FIT field definition values  */

//#define FIELD_TIMESTAMP	    253		// Timestamp - usually UTC

//  File code information.

#include    "fileCodes.h"

typedef struct _fileID {
    int	    fileID;
    const char *pchShortName;
    const char *pchDescription;
} fileIDcodes;

fileIDcodes *findFileID( int );

// Message number/type names + values and range if defined.
typedef struct _msgNumVal {
    const u_short   msgNum;
    const u_short   msgMaxValue;	    // If > msgNum
    const char     *pchName;
} msgNumber;

msgNumber  *findMsgNumber( u_short );


