/*  Keeps record format data */

#ifndef _RECORD_INFO
#define _RECORD_INFO

#define	RI_MAX_FIELDS	256	    // Data records are limited to 255 bytes
				    // so one byte data records can go to 255.
#define NUMBER_RI	16	    // Can only be 16 record field layouts
#define MAX_FIELD_CONTENT   256	    // Max bytes per field content

#define	GLOBAL_REC_SZ	(RI_MAX_FIELDS * MAX_FIELD_CONTENT)

//   Bits in record header byte.
#define	RH_COMPTIMESTAMP    0x80	    // 0 for normal, 1 for compresed time stamp record
#define	RH_DEFN		    0x40	    // Set == definition, else data
#define	RH_SPECIFIC	    0x20	    // Data record 0 only, 1 = dev. fields in definition
#define	RH_reserved	    0x10	    // Reserved, always 0
#define	RH_MSG_TYPE	    0x0f	    // Local message ID (use only in this file)
#define	RH_MSG_TYPE_COMP    0x60	    // Compressed timestamp ID bits are here.

//  Field definitions structure.
typedef  struct _fieldData {
    u_char   fdDefnNumber;
    u_char   fdSize;		    //  Number of bytes for this element.
    u_char   fdType;		    //  Type of value (e.g signed int, unsigned char etc.)
} FIELD_DATA;
#define CB_FIELD_DATA  3	    // Bytes to read.

//  There will be a global array of these.   They are indexed by
//  the local message type of definition records.
typedef  struct _recInfo {
    u_int	riTimeStamp;	    // Last seen timestamp, used for compressed timestamp header.
    u_short	riNumber;	    // Number of fields for this record.
    u_short	riGlobalMessageNumber;	// Details in SDK
			//  The 2 below is to allow for developer fields.
    FIELD_DATA	riFd[2 * RI_MAX_FIELDS];   // The field data, only used up to riNumber.
    u_char	bChangeEndian;		// Change endian values for this record. 
} REC_INFO;

#endif
