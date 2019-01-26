/*  Include file for fnProc* files, to handle message processing */

#ifndef _FN_PROC_DEFN
#define _FN_PROC_DEFN


typedef int (* FN_PROC)( u_short, DECODE_FIELD_VALUE * );

typedef struct _fnp {
    u_short	fnpMsgId;
    FN_PROC     msgProcFunc;
    u_short	msgNumber;
    u_short	msgMaxNumber;
    char       *pchName;
} FN_PROC_MSG;

int  processRecord( u_short msgNumber, DECODE_FIELD_VALUE *pdfv );

//  Generic table for mapping number to text - widely used.

typedef  struct  _RecMap {
    int     rm_Id;
    char   *rm_name;
} REC_MAP;

#endif