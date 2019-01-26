/*  Utility functions header for converting file data */


#ifndef _UTILS_H
#define	_UTILS_H
#include    "recordInfo.h"

//  Info about data types in FIT files, and how to handle them.

//   This union makes it easier to have an array of how to handle
//   various values/sizes in the FIT data fields.
//   NOTE:  the fields with a 'z' on the end have the invalid value as 0
//   whereas 'z-less' ending is all FF..  or 7FFF... (signed).

typedef  union {
	unsigned char	en;
	unsigned char	uint8;
	signed char 	sint8;
	unsigned short	uint16;
	signed short	sint16;
	signed int	sint32;
	unsigned int	uint32;
	unsigned char *	string;	    // UTF-8 character string.
	float		float32;
	double		float64;
	unsigned char	uint8z;	    // Default is 0x00, as opposed to 0xFF
	unsigned short	uint16z;
	unsigned int	uint32z;
	unsigned char * byte;	    // Byte array
	signed long	sint64;
	unsigned long	uint64;
	unsigned long	uint64z;
} U_TYPES;


typedef struct _typeHandling {
	U_TYPES	  uInvalid;		    // The invalid pattern
	int	  fitBaseType;	    // Value from field definition
	int	  fitLength;	    // Number of bytes in field; 0 = array/string.
} TYPE_HANDLING;

//  Value returned from reading a field value.
typedef union {
    long    uLong;		// Any integer value
    double  uDouble;		// Any floating point value
    unsigned char  *uByte;	// Generic memory pointer
} UNION_RETVAL;

typedef enum { dfvNULL, dfvINT, dfvDOUBLE, dfvSTRING, dfvARRAY,
					    dfvINVALID, dfvERROR, dfvEND } DFV_ENUM;

typedef struct _decodeRet {
    UNION_RETVAL  u;		    // Value changed to standard types (see above).
    DFV_ENUM	  eRetCode;	    // What TYPE it is.
    u_char	  fdDefnNumber;	    // What it is.
} DECODE_FIELD_VALUE;

//  The Global structure, a neat way to keep commonly used global variables
//  under some civilised control.

typedef  struct _global {
    u_long  debug;		// Enables debugging output.
    int	    outputSelect;	// Control output information.
    int	    bIsBigEndian;	// 0 == little endian
    u_long  gpsLastTime;	// Reduce GPS settings to one per second.
    int	    msgNumber;		// Global message number (as opposed to local/field)
    u_long  fitEpochOffset;	// # seconds in Unix epoch at start of this file creation
} GLOBAL;

extern  GLOBAL  G;
//  G.debug is a bit array enabling output messages.
#define	DEBUG_DEFN	    0x0001
#define DEBUG_DEFN_FIELD    0x0002
#define	DEBUG_DEV_DEFN	    0x0004	// Debug developer fields.
#define	DEBUG_DEV_DEFN_FIELD	0x0008
#define	DEBUG_REC_HEADER    0x0010	// Print record header, and file position
#define	DEBUG_DATA	    0x0020	// Header information for data records.
#define	DEBUG_DATA_FIELD    0x0040	// Show field data, including values.
#define DEBUG_DATA_INVALID  0x0080	// Report invalid values!
#define	DEBUG_GLOBL_MSG	    0x0100	// Issues with global message numbers + decoders

//   G.output selects what, if any, 'normal' output is enabled.
#define	OUTPUT_GPS	    0x0001	// GPS track and time reported.

//  Function prototypes for this section of decoding.
DECODE_FIELD_VALUE decodeFieldValue( FILE *pFD, FIELD_DATA *pFieldData, int bigEndian );

//  CRC calculation on the .fit file
int checkFileCRC( FILE *pF, u_long cbCRC );

//  Convert semicircle integer values to decimal fractional degrees.
double semiCircleToDegrees( int  semiC );


//  Minimum value for a wall clock value, otherwise internal clock value.
#define MIN_UTC_TIMESTAMP   0x10000000


// Time adjusting functions.
void  setEpochAdjustment( u_long  sysTimeStamp, u_long  utcTimeStamp );

u_long  adjustTimeToUnixEpoch( u_long  time );

#endif