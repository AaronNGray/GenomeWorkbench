#ifndef CONNECT___NCBI_BUFFER__H
#define CONNECT___NCBI_BUFFER__H

/* $Id: ncbi_buffer.h 579276 2019-01-31 19:48:53Z lavr $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Author:  Denis Vakatov, Anton Lavrentiev
 *
 * File Description:
 *   Memory-resident storage area (to be used e.g. in I/O buffering)
 *
 * Handle:  BUF
 *
 * Functions:
 *   BUF_SetChunkSize
 *   BUF_Size
 *   BUF_Prepend[Ex]
 *   BUF_Append[Ex]
 *   BUF_Write
 *   BUF_Pushback
 *   BUF_Peek
 *   BUF_PeekAt
 *   BUF_PeekAtCB
 *   BUF_Read
 *   BUF_Erase
 *   BUF_Splice
 *   BUF_Destroy
 *
 */

#include <connect/connect_export.h>
#include <stddef.h>     /* ...to define "size_t"... */


#define BUF_DEF_CHUNK_SIZE  1024


/** @addtogroup BuffServices
 *
 * @{
 */


#ifdef __cplusplus
extern "C" {
#endif


struct SNcbiBuf;
typedef struct SNcbiBuf* BUF;  /**< handle of a buffer */


/*!
 * Set minimal size of a buffer memory chunk.
 * Return the actually set chunk size on success;  zero on error.
 * NOTE:  if "*pBuf" == NULL then create it;
 *        if "chunk_size" is passed 0 then set it to BUF_DEF_CHUNK_SIZE.
 */
extern NCBI_XCONNECT_EXPORT size_t BUF_SetChunkSize
(BUF*   pBuf,
 size_t chunk_size
 );


/*!
 * Return the number of bytes stored in "buf".
 * NOTE: return 0 if "buf" == NULL.
 */
extern NCBI_XCONNECT_EXPORT size_t BUF_Size(BUF buf);


/*!
 * Prepend a block of data (of the specified size) at the beginning of the
 * buffer (to be read first).  Note that unlike BUF_Pushback(), in this call
 * the data is not copied into the buffer but instead is just linked in from
 * the original location.  Return non-zero (true) if succeeded, zero (false)
 * if failed.
 */
extern NCBI_XCONNECT_EXPORT int/*bool*/ BUF_PrependEx
(BUF*   pBuf,
 void*  base,       /* base to be "free"d when the buffer chunk is unlinked  */
 size_t alloc_size, /* usable size of "data" (0 to make the use read-only)   */
 void*  data,       /* points to data to be prepended by linking in the list */
 size_t size        /* size of "data" occupied                               */
 );


/*!
 * Equivalent to BUF_PrependEx(pBuf, 0, 0, data, size)
 * NOTE: the prepended chunk is thus read-only and will not be auto-freed.
 */

extern NCBI_XCONNECT_EXPORT int/*bool*/ BUF_Prepend
(BUF*        pBuf,
 const void* data,
 size_t      size
);


/*!
 * Append a block of data (of the specified size) past the end of the buffer
 * (to be read last).  Note that unlike BUF_Write(), in this call the data is
 * not copied to the buffer but instead is just linked in from the original
 * location.  Return non-zero (true) if succeeded, zero (false) if failed.
 */
extern NCBI_XCONNECT_EXPORT int/*bool*/ BUF_AppendEx
(BUF*   pBuf,
 void*  base,       /* base to be "free"d when the chunk is unlinked        */
 size_t alloc_size, /* usable size of "data" (0 to make the use read-only)  */
 void*  data,       /* points to data to be appended by linking in the list */
 size_t size        /* size of "data" occupied                              */
 );


/*!
 * Equivalent to BUF_AppendEx(pBuf, 0, 0, data, size)
 * NOTE: the appended chunk is thus read-only and will not be auto-freed.
 */
extern NCBI_XCONNECT_EXPORT int/*bool*/ BUF_Append
(BUF*        pBuf,
 const void* data,
 size_t      size
 );


/*!
 * Add new data to the end of "*pBuf" (to be read last).
 * On error (failed memory allocation), return zero value;
 * otherwise return non-zero (i.e. including when "size" is passed as 0).
 * NOTE:  if "*pBuf" == NULL then create it if necessary (e.g. if size != 0).
 * NOTE:  BUF_Write() with "data" that reside immediately past the end of the
 * data (in the unoccupied space) of a chunk that was previously appended with
 * BUF_AppendEx() results in a zero-copy operation (just the pointers updated
 * internally as necessary).
 */
extern NCBI_XCONNECT_EXPORT /*bool*/int BUF_Write
(BUF*        pBuf,
 const void* data,
 size_t      size
 );


/*!
 * Write the data to the very beginning of "*pBuf" (to be read first).
 * Return non-zero if successful ("size"==0 is always so);  otherwise
 * return zero (failed memory allocation or NULL "src" of non-zero "size").
 * NOTE:  if "*pBuf" == NULL then create it.
 */
extern NCBI_XCONNECT_EXPORT /*bool*/int BUF_Pushback
(BUF*        pBuf,
 const void* data,
 size_t      size
 );


/*!
 * Equivalent to "BUF_PeekAt(buf, 0, data, size)", see description below.
 */
extern NCBI_XCONNECT_EXPORT size_t BUF_Peek
(BUF         buf,
 void*       data,
 size_t      size
 );


/*!
 * Copy up to "size" bytes stored in "buf" (starting at position "pos")
 * to a destination area pointed to by "data".
 * Return the # of copied bytes (can be less than "size").
 * Return zero and do nothing if "buf" is NULL or "pos" >= BUF_Size(buf).
 * If "data" is NULL, return the number of bytes what would have been copied;
 * in other words, the amount of data available in "buf" from position "pos",
 * not exceeding "size" bytes (0 results when "pos" is past the end of "buf").
 */
extern NCBI_XCONNECT_EXPORT size_t BUF_PeekAt
(BUF         buf,
 size_t      pos,
 void*       data,
 size_t      size
 );


/*!
 * Call "callback" repeatedly on up to "size" bytes stored in "buf" (starting
 * at position "pos"), in chunks.  Pass "cbdata" as an opaque parameter to the
 * "callback".  Processing stops when all buffer bytes (but no more than
 * "size" bytes) have been visited by the "callback", or when the "callback"
 * returns a value less than its passed "size" argument (the "callback" may
 * _not_ return a value greater than its "size" argument!).
 * Return the # of processed bytes (can be less than "size").
 * Return zero and do nothing if "buf" is NULL or "pos" >= BUF_Size(buf).
 * If "callback" is NULL, return the number of bytes that would have been
 * visited if a callback kept returning the size of data it was given to;
 * in other words, the amount of data available in "buf" from position "pos",
 * not exceeding "size" bytes (0 results when "pos" is past the end of "buf").
 */
extern NCBI_XCONNECT_EXPORT size_t BUF_PeekAtCB
(BUF         buf,
 size_t      pos,
 size_t    (*callback)(void* cbdata, const void* buf, size_t size),
 void*       cbdata,
 size_t      size
 );


/*!
 * Copy up to "size" bytes stored in "buf" to "data" and remove the copied
 * data from the "buf".
 * Return the # of copied-and/or-removed bytes (can be less than "size").
 * NOTE: if "buf"  == NULL then do nothing and return 0
 *       if "data" == NULL then do not copy data anywhere(still, remove it)
 */
extern NCBI_XCONNECT_EXPORT size_t BUF_Read
(BUF         buf,
 void*       data,
 size_t      size
 );


/*!
 * Make the buffer empty.
 * NOTE: do nothing if "buf" == NULL.
 */
extern NCBI_XCONNECT_EXPORT void BUF_Erase(BUF buf);


/*!
 * Append all contents of the source buffer "src" to the destination buffer
 * "*dst" (creating the buffer as necessary if "dst" is NULL), making the
 * source buffer "src" empty (as with BUF_Erase(src)).
 * Return non-zero if successful; 0 in case of an error.
 * NOTE: do nothing if "src" is either NULL or contains no data.
 * NOTE: the call re-links internal structures without copying any actual data.
 */
extern NCBI_XCONNECT_EXPORT int/*bool*/ BUF_Splice(BUF* dst, BUF src);


/*!
 * Destroy all buffer data.
 * NOTE: do nothing if "buf" == NULL.
 */
extern NCBI_XCONNECT_EXPORT void BUF_Destroy(BUF buf);


#ifdef __cplusplus
}
#endif


/* @} */

#endif /* CONNECT___NCBI_BUFFER__H */
