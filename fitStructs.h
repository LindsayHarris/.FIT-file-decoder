/*  Structure data for .FIT files */

#include <sys/types.h>

#ifndef _FIT_STRUCTS_H
#define _FIT_STRUCTS_H

//  Structure defining the first few bytes of a .FIT file.

typedef struct _FitHeader {
    u_char	hdrSize;    // Size of header structure.
    u_char	protoVer;   // Protocol version.
    u_short	profVer;    // Profile version.
    u_int	lenData;    // Length of data section of file
    u_char	achSign[4]; // Signature of header: must be ".FIT"
    u_short	crc;	    // CRC of first 12 bytes here.
} fitHeader;
#define CB_FIT_HEADER	14	// Number of bytes in the file.

//  Structure to handle the bytes 1 - 4 of definition records.

typedef struct _DfnMsgHeader {
    u_char	reserved;	// ?
    u_char	architecture;	// 0 = little endian, 1 = big endian
    u_short	globalMsgNum;	// index to which layout applies to data records
    u_char	numFields;	// Number of fields
} dfnMsgHeader;
#define	CB_DFN_MSG_HEADER	    5	    // Number of bytes to read.

#endif