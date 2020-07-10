/*  $Id: zlib.cpp 551452 2017-11-17 18:37:27Z ivanov $
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
 * Authors:  Vladimir Ivanov
 *           Jean-loup Gailly, Mark Adler
 *           (used a part of zlib library code from: gzio.c, uncompr.c)
 *
 * File Description:  ZLib Compression API
 *
 * NOTE: The zlib documentation can be found here: 
 *           http://zlib.org,  
 *           http://www.gzip.org/zlib/manual.html
 */


#include <ncbi_pch.hpp>
#include <corelib/ncbi_limits.h>
#include <corelib/ncbifile.hpp>
#include <util/compress/zlib.hpp>
#include <util/error_codes.hpp>
#include <zlib.h>

#include <stdio.h>


#define NCBI_USE_ERRCODE_X   Util_Compress

BEGIN_NCBI_SCOPE


// Define some macro if not defined
#ifndef DEF_MEM_LEVEL
#  if MAX_MEM_LEVEL >= 8
#    define DEF_MEM_LEVEL 8
#  else
#    define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#  endif
#endif

// Identify as Unix by default.
#ifndef OS_CODE
#  define OS_CODE 0x03
#endif

// Macro to check flags
#define F_ISSET(mask) ((GetFlags() & (mask)) == (mask))

// Get compression stream pointer
#define STREAM ((z_stream*)m_Stream)

// Limit 'size_t' values to max values of other used types to avoid overflow
#define LIMIT_SIZE_PARAM_LONG(value)  if (value > (size_t)kMax_Long) value = kMax_Long
#define LIMIT_SIZE_PARAM_UINT(value)  if (value > kMax_UInt) value = kMax_UInt
#define LIMIT_SIZE_PARAM_STREAMSIZE(value) \
    if (value > (size_t)numeric_limits<std::streamsize>::max()) \
        value = (size_t)numeric_limits<std::streamsize>::max()

// Maximum size of gzip file header
const size_t kMaxHeaderSize = 1024*4;

// Maximum size of the chuck compressed by zlib,
// but there is no limit to the number of chunks in the buffer.
const unsigned int kMaxChunkSize = kMax_UInt;



//////////////////////////////////////////////////////////////////////////////
//
// CZipCompression
//

CZipCompression::CZipCompression(ELevel level,  int window_bits,
                                 int mem_level, int strategy)
    : CCompression(level)
{
    m_WindowBits = ( window_bits == kZlibDefaultWbits )  ? MAX_WBITS : window_bits;
    m_MemLevel   = ( mem_level == kZlibDefaultMemLevel ) ? DEF_MEM_LEVEL : mem_level;
    m_Strategy   = ( strategy == kZlibDefaultStrategy )  ? Z_DEFAULT_STRATEGY : strategy;
    // Initialize the compressor stream structure
    m_Stream = new z_stream;
    if ( m_Stream ) {
        memset(m_Stream, 0, sizeof(z_stream));
    }
    return;
}


CZipCompression::~CZipCompression()
{
    delete STREAM;
    return;
}


// gzip magic header
const unsigned char gz_magic[2] = {0x1f, 0x8b};

// gzip flag byte
#define ASCII_FLAG   0x01 // bit 0 set: file probably ascii text
#define HEAD_CRC     0x02 // bit 1 set: header CRC present
#define EXTRA_FIELD  0x04 // bit 2 set: extra field present
#define ORIG_NAME    0x08 // bit 3 set: original file name present
#define COMMENT      0x10 // bit 4 set: file comment present
#define RESERVED     0xE0 // bits 5..7: reserved


// Returns length of the .gz header if it exists or 0 otherwise.
// If 'info' not NULL, fill it with information from header.
static
size_t s_CheckGZipHeader(const void* src_buf, size_t src_len,
                         CZipCompression::SFileInfo* info = 0)
{
    unsigned char* buf = (unsigned char*)src_buf;
    // .gz header cannot be less than 10 bytes
    if (src_len < 10) {
        return 0;
    }
    // Check the gzip magic header
    if (buf[0] != gz_magic[0]  ||
        buf[1] != gz_magic[1]) {
        return 0;
    }
    int method = buf[2];
    int flags  = buf[3];
    if (method != Z_DEFLATED  ||  (flags & RESERVED) != 0) {
        return 0;
    }
    // Header length: 
    // gz_magic (2) + method (1) + flags (1) + time, xflags and OS code (6)
    size_t header_len = 10; 

    if ( info ) {
        info->mtime = CCompressionUtil::GetUI4(buf+4);
    }

    // Skip the extra fields
    if ((flags & EXTRA_FIELD) != 0) {
        if (header_len + 2 > src_len) {
            return 0;
        }
        size_t len = buf[header_len++];
        len += ((size_t)buf[header_len++])<<8;
        header_len += len;
    }
    // Skip the original file name
    if ((flags & ORIG_NAME) != 0) {
        size_t pos = header_len;
        while (header_len < src_len  &&  buf[header_len++] != 0);
        if ( info ) {
            info->name.assign((char*)buf+pos, header_len-pos);
        }
    }
    // Skip the file comment
    if ((flags & COMMENT) != 0) {
        size_t pos = header_len;
        while (header_len < src_len  &&  buf[header_len++] != 0);
        if ( info ) {
            info->comment.assign((char*)buf+pos, header_len-pos);
        }
    }
    // Skip the header CRC
    if ((flags & HEAD_CRC) != 0) {
        header_len += 2;
    }
    if (header_len > src_len) {
        return 0;
    }
    return header_len;
}


static 
size_t s_WriteGZipHeader(void* dst_buf, size_t dst_size,
                         const CZipCompression::SFileInfo* info = 0)
{
    char* buf = (char*)dst_buf;

    // .gz header cannot be less than 10 bytes
    if (dst_size < 10) {
        // error
        return 0;
    }
    unsigned char flags = 0;
    size_t header_len = 10;  // set first 10 bytes of header later

    // Store the original file name.
    // Store it only if buffer have enough size.
    if ( info  &&  !info->name.empty()  &&  dst_size > (info->name.length() + header_len) ) {
        flags |= ORIG_NAME;
        strncpy((char*)buf+header_len, info->name.data(),info->name.length());
        header_len += info->name.length();
        buf[header_len++] = '\0';
    }
    // Store file comment.
    // Store it only if buffer have enough size.
    if ( info  &&  !info->comment.empty()  &&
         dst_size > (info->comment.length() + header_len) ) {
        flags |= COMMENT;
        strncpy((char*)buf+header_len, info->comment.data(),
                info->comment.length());
        header_len += info->comment.length();
        buf[header_len++] = '\0';
    }

    // Set beginning of header
    memset(buf, 0, 10);
    buf[0] = gz_magic[0];
    buf[1] = gz_magic[1];
    buf[2] = Z_DEFLATED;
    buf[3] = flags;
    /* 4-7 mtime */
    if ( info  &&  info->mtime ) {
        CCompressionUtil::StoreUI4(buf+4, (unsigned long)info->mtime);
    }
    /* 8 - xflags == 0*/
    buf[9] = OS_CODE;

    return header_len;
}


static 
size_t s_WriteGZipFooter(void* buf, size_t buf_size, size_t total, unsigned long crc)
{
    // .gz footer have 8 bytes
    if (buf_size < 8) {
        // error
        return 0;
    }
    // 4 bytes for CRC value of the uncompressed data
    CCompressionUtil::StoreUI4(buf, crc);
    // 4 bytes for the size of the original (uncompressed) data modulo 2^32
    // For files > 4GB, gzip stores just the last 32 bits of the file size.
    CCompressionUtil::StoreUI4((unsigned char*)buf + 4, total & 0xFFFFFFFFL);
    return 8;
}


static
void s_CollectFileInfo(const string& filename, CZipCompression::SFileInfo& info)
{
    CFile file(filename);
    info.name = file.GetName();
    time_t mtime;
    file.GetTimeT(&mtime);
    info.mtime = mtime;
}


static 
unsigned long s_UpdateCRC32(unsigned long crc, const void* buf, size_t len)
{
#if !defined(NCBI_OS_DARWIN)  &&  (ZLIB_VERNUM >= 0x1290)
    return crc32_z(0L, (unsigned char*)buf, len);

#else
    unsigned char* ptr = (unsigned char*)buf;
    while (len) {
        unsigned int n = len > kMaxChunkSize ? kMaxChunkSize : (unsigned int)len;
        crc = crc32(crc, ptr, n);
        ptr += n;
        len -= n;
    }
    return crc;
#endif
}


inline
unsigned long s_GetCRC32(const void* buf, size_t len)
{
    return s_UpdateCRC32(0L, buf, len);
}


CVersionInfo CZipCompression::GetVersion(void) const
{
    return CVersionInfo(ZLIB_VERSION, "zlib");
}


bool CZipCompression::CompressBuffer(
                      const void* src_buf, size_t  src_len,
                      void*       dst_buf, size_t  dst_size,
                      /* out */   size_t* dst_len)
{
    *dst_len = 0;

    // Check parameters
    if (!src_len  &&  !F_ISSET(fAllowEmptyData)) {
        src_buf = NULL;
    }
    if (!src_buf || !dst_buf || !dst_len) {
        SetError(Z_STREAM_ERROR, "bad argument");
        ERR_COMPRESS(48, FormatErrorMessage("CZipCompression::CompressBuffer"));
        return false;
    }

    size_t header_len = 0;
    int errcode = Z_OK;
    // Pointer to the current positions in destination buffer
    unsigned char* dst = (unsigned char*)dst_buf;
    
    // Write gzip file header
    if ( F_ISSET(fWriteGZipFormat) ) {
        header_len = s_WriteGZipHeader(dst_buf, dst_size);
        if (!header_len) {
            SetError(Z_STREAM_ERROR, "Cannot write gzip header");
            ERR_COMPRESS(50, FormatErrorMessage("CZipCompression::CompressBuffer"));
            return false;
        }
        dst += header_len;
        dst_size -= header_len;
    }

    STREAM->zalloc = NULL;
    STREAM->zfree  = NULL;
    STREAM->opaque = NULL;

    errcode = deflateInit2_(STREAM, GetLevel(), Z_DEFLATED,
                            header_len ? -m_WindowBits : m_WindowBits,
                            m_MemLevel, m_Strategy,
                            ZLIB_VERSION, (int)sizeof(z_stream));
    if (errcode == Z_OK) {
    
        STREAM->next_in   = (unsigned char*)src_buf;
        STREAM->avail_in  = 0;
        STREAM->next_out  = dst;
        STREAM->avail_out = 0;

        size_t left = src_len;
        do {
            if (STREAM->avail_in == 0) {
                STREAM->avail_in = left > kMaxChunkSize ? kMaxChunkSize : (unsigned int)left;
                left -= STREAM->avail_in;
            }
            if (STREAM->avail_out == 0) {
                STREAM->avail_out = dst_size > kMaxChunkSize ? kMaxChunkSize : (unsigned int)dst_size;
                dst_size -= STREAM->avail_out;
            }
            errcode = deflate(STREAM, left ? Z_NO_FLUSH : Z_FINISH);
        } 
        while (errcode == Z_OK);

        // Calculate length of output data.
        // Don't use STREAM->total_out here, for source buffers greater
        // than 4GB it works if (SIZEOF_LONG > 4) only.
        *dst_len = STREAM->next_out - (unsigned char*)dst_buf;
        deflateEnd(STREAM);
    }

    SetError(errcode, zError(errcode));
    if ( errcode != Z_STREAM_END) {
        ERR_COMPRESS(53, FormatErrorMessage("CZipCompression::CompressBuffer"));
        return false;
    }

    // Write gzip file footer
    if ( F_ISSET(fWriteGZipFormat) ) {
        unsigned long crc = s_GetCRC32(src_buf, src_len);
        size_t footer_len = s_WriteGZipFooter(STREAM->next_out, dst_size + STREAM->avail_out, src_len, crc);
        if ( !footer_len ) {
            SetError(-1, "Cannot write gzip footer");
            ERR_COMPRESS(54, FormatErrorMessage("CZipCompressor::CompressBuffer"));
            return false;
        }
        *dst_len += footer_len;
    }

    return true;
}


bool CZipCompression::DecompressBuffer(
                      const void* src_buf, size_t  src_len,
                      void*       dst_buf, size_t  dst_size,
                      /* out */            size_t* dst_len)
{
    *dst_len = 0;

    // Check parameters
    if ( !src_len ) {
        if ( F_ISSET(fAllowEmptyData) ) {
            SetError(Z_OK);
            return true;
        }
        src_buf = NULL;
    }
    if (!src_buf || !dst_buf || !dst_len) {
        SetError(Z_STREAM_ERROR, "bad argument");
        ERR_COMPRESS(55, FormatErrorMessage("CZipCompression::DecompressBuffer"));
        return false;
    }

    unsigned int gzip_chunk_count = 0;
    int errcode = Z_OK;
    // Pointers to the current positions in buffers
    unsigned char* src = (unsigned char*)src_buf;
    unsigned char* dst = (unsigned char*)dst_buf;
    size_t dst_size_arg = dst_size;

    do {
        STREAM->next_in   = src;
        STREAM->avail_in  = 0;
        STREAM->zalloc    = NULL;
        STREAM->zfree     = NULL;
        STREAM->opaque    = NULL;

        // Check file header
        size_t header_len = 0;
        if (F_ISSET(fCheckFileHeader)) {
            // Check gzip header in the buffer
            header_len = s_CheckGZipHeader(src_buf, src_len);
            if (gzip_chunk_count  &&  !header_len) {
                // No more gzip chunks in the concatenated archive, stop processing
                break;
            }
            src += header_len;
            src_len -= header_len;
            gzip_chunk_count++;
        }

        // "window bits" is passed < 0 to tell that there is no zlib header.
        // Note that in this case inflate *requires* an extra "dummy" byte
        // after the compressed stream in order to complete decompression and
        // return Z_STREAM_END. Here the gzip's CRC32 ensures that 4 bytes are
        // present after the compressed stream.

        errcode = inflateInit2_(STREAM, header_len ? -m_WindowBits :m_WindowBits,
                                ZLIB_VERSION, (int)sizeof(z_stream));
        if (errcode != Z_OK) {
            break;
        }

        STREAM->next_in   = src;
        STREAM->avail_in  = 0;
        STREAM->next_out  = dst;
        STREAM->avail_out = 0;

        size_t left = src_len;
        do {
            if (STREAM->avail_in == 0) {
                STREAM->avail_in = left > kMaxChunkSize ? kMaxChunkSize : (unsigned int)left;
                left -= STREAM->avail_in;
            }
            if (STREAM->avail_out == 0) {
                STREAM->avail_out = dst_size > kMaxChunkSize ? kMaxChunkSize : (unsigned int)dst_size;
                dst_size -= STREAM->avail_out;
            }
            errcode = inflate(STREAM, Z_NO_FLUSH);
        } while (errcode == Z_OK);

        // Calculate length of output data.
        // Don't use STREAM->total_out for source buffers greater than 4GB,
        // it works if (SIZEOF_LONG > 4) only.
        *dst_len = STREAM->next_out - (unsigned char*)dst_buf;

        // Does decompression succeeded?
        if (errcode == Z_STREAM_END) {

            // Concatenated gzip?
            if (gzip_chunk_count  &&  F_ISSET(fAllowConcatenatedGZip)) {
                // Update pointers and counters to process next gzip chunk.
                // NOTE: Don't use STREAM->total_out! (see above)
                
                src       = STREAM->next_in + 8;  // skip gzip footer
                size_t processed = src - (unsigned char*)src_buf;
                src_len   = (processed > src_len) ? 0 : (src_len - processed);
                dst       = STREAM->next_out;
                dst_size += STREAM->avail_out;   // compensate for unprocessed data
            }
            else {
                // Finish processing
                src_len = 0;
            }
            inflateEnd(STREAM);
        } else {
            // Error
            inflateEnd(STREAM);
            if (!gzip_chunk_count  &&  !header_len  &&  F_ISSET(fAllowTransparentRead)) {
                // Decompression error, but transparent read is allowed
                *dst_len = (dst_size_arg < src_len) ? dst_size_arg : src_len;
                memcpy(dst_buf, src_buf, *dst_len);
                return (dst_size_arg >= src_len);
            }
            break;
        }
    } while (src_len);

    SetError(errcode, zError(errcode));
    if ( errcode != Z_STREAM_END ) {
        ERR_COMPRESS(59, FormatErrorMessage("CZipCompression::DecompressBuffer",
                                            STREAM->next_in - (unsigned char*) src_buf));
        return false;
    }
    return true;
}


long CZipCompression::EstimateCompressionBufferSize(size_t src_len)
{
#if (ZLIB_VERNUM < 0x1200)
    return -1;
#else
    size_t header_len = 0;
    int    errcode    = Z_OK;
    
    if ( F_ISSET(fWriteGZipFormat) ) {
        // Default empty GZIP header
        header_len = 10;
    }
    STREAM->zalloc = NULL;
    STREAM->zfree  = NULL;
    STREAM->opaque = NULL;
    errcode = deflateInit2_(STREAM, GetLevel(), Z_DEFLATED,
                            header_len ? -m_WindowBits : m_WindowBits,
                            m_MemLevel, m_Strategy,
                            ZLIB_VERSION, (int)sizeof(z_stream));
    if (errcode != Z_OK) {
        SetError(errcode, zError(errcode));
        return -1;
    }
    long n = (long)(deflateBound(STREAM, (unsigned long)src_len) + header_len);
    deflateEnd(STREAM);
    return n;
#endif
}


bool CZipCompression::CompressFile(const string& src_file,
                                   const string& dst_file,
                                   size_t        buf_size)
{
    CZipCompressionFile cf(GetLevel(), m_WindowBits, m_MemLevel, m_Strategy);
    cf.SetFlags(cf.GetFlags() | GetFlags());

    // Collect info about compressed file
    CZipCompression::SFileInfo info;

    // For backward compatibility -- collect file info and
    // write gzip file by default.
    s_CollectFileInfo(src_file, info);
    // Open output file
    if ( !cf.Open(dst_file, CCompressionFile::eMode_Write, &info) ) {
        SetError(cf.GetErrorCode(), cf.GetErrorDescription());
        return false;
    } 
    // Make compression
    if ( !CCompression::x_CompressFile(src_file, cf, buf_size) ) {
        if ( cf.GetErrorCode() ) {
            SetError(cf.GetErrorCode(), cf.GetErrorDescription());
        }
        cf.Close();
        return false;
    }
    // Close output file and return result
    bool status = cf.Close();
    SetError(cf.GetErrorCode(), cf.GetErrorDescription());
    return status;
}


bool CZipCompression::DecompressFile(const string& src_file,
                                     const string& dst_file,
                                     size_t        buf_size)
{
    CZipCompressionFile cf(GetLevel(), m_WindowBits, m_MemLevel, m_Strategy);
    cf.SetFlags(cf.GetFlags() | GetFlags());

    bool need_restore_attr = false;
    SFileInfo info;

    // Open compressed file, and define name of the destination file
    if ( F_ISSET(fRestoreFileAttr) ) {
        if ( !cf.Open(src_file, CCompressionFile::eMode_Read, &info) ) {
            SetError(cf.GetErrorCode(), cf.GetErrorDescription());
            return false;
        } 
        if ( !info.name.empty() ) {
            need_restore_attr = true;
        }
    } else {
        if ( !cf.Open(src_file, CCompressionFile::eMode_Read, 0) ) {
            SetError(cf.GetErrorCode(), cf.GetErrorDescription());
            return false;
        } 
    }
    // Decompress file
    if ( !CCompression::x_DecompressFile(cf, dst_file, buf_size) ) {
        if ( cf.GetErrorCode() ) {
            SetError(cf.GetErrorCode(), cf.GetErrorDescription());
        }
        cf.Close();
        return false;
    }
    // Close output file and return result
    bool status = cf.Close();
    SetError(cf.GetErrorCode(), cf.GetErrorDescription());
    // Restore time stamp if needed
    if ( status  &&  need_restore_attr ) {
        CFile(dst_file).SetTimeT(&info.mtime);
    }
    return status;
}


bool CZipCompression::DecompressFileIntoDir(const string& src_file,
                                            const string& dst_dir,
                                            size_t        buf_size)
{
    CZipCompressionFile cf(GetLevel(), m_WindowBits, m_MemLevel, m_Strategy);
    cf.SetFlags(cf.GetFlags() | GetFlags());

    bool need_restore_attr = false;
    SFileInfo info;
    string dir, name, ext;
    string dst_file;

    // Open compressed file, and define name of the destination file
    if ( F_ISSET(fRestoreFileAttr) ) {
        if ( !cf.Open(src_file, CCompressionFile::eMode_Read, &info) ) {
            SetError(cf.GetErrorCode(), cf.GetErrorDescription());
            return false;
        } 
        if ( info.name.empty() ) {
            // Header is possible broken, ignore it
            CFile::SplitPath(src_file, &dir, &name, &ext);
            dst_file = CFile::MakePath(dst_dir, name);
        } else {
            need_restore_attr = true;
            dst_file = CFile::MakePath(dst_dir, info.name);
        }
    } else {
        if ( !cf.Open(src_file, CCompressionFile::eMode_Read, 0) ) {
            SetError(cf.GetErrorCode(), cf.GetErrorDescription());
            return false;
        } 
        CFile::SplitPath(src_file, &dir, &name, &ext);
        dst_file = CFile::MakePath(dst_dir, name);
    }
    // Decompress file
    if ( !CCompression::x_DecompressFile(cf, dst_file, buf_size) ) {
        if ( cf.GetErrorCode() ) {
            SetError(cf.GetErrorCode(), cf.GetErrorDescription());
        }
        cf.Close();
        return false;
    }
    // Close output file and return result
    bool status = cf.Close();
    SetError(cf.GetErrorCode(), cf.GetErrorDescription());
    // Restore time stamp if needed
    if ( status  &&  need_restore_attr ) {
        CFile(dst_file).SetTimeT(&info.mtime);
    }
    return status;
}


string CZipCompression::FormatErrorMessage(string where, size_t pos) const
{
    string str = "[" + where + "]  " + GetErrorDescription();
    if ( !pos ) {
        pos = STREAM->total_in;
    }
    str += ";  error code = " + NStr::IntToString(GetErrorCode()) +
           ", number of processed bytes = " + NStr::SizetToString(pos);
    return str + ".";
}



//////////////////////////////////////////////////////////////////////////////
//
// CZipCompressionFile
//


CZipCompressionFile::CZipCompressionFile(
    const string& file_name, EMode mode,
    ELevel level, int window_bits, int mem_level, int strategy)
    : CZipCompression(level, window_bits, mem_level, strategy),
      m_Mode(eMode_Read), m_File(0), m_Stream(0)
{
    // For backward compatibility -- use gzip file format by default
    SetFlags(GetFlags() | fGZip);

    if ( !Open(file_name, mode) ) {
        const string smode = (mode == eMode_Read) ? "reading" : "writing";
        NCBI_THROW(CCompressionException, eCompressionFile, 
                   "[CZipCompressionFile]  Cannot open file '" + file_name +
                   "' for " + smode + ".");
    }
    return;
}


CZipCompressionFile::CZipCompressionFile(
    ELevel level, int window_bits, int mem_level, int strategy)
    : CZipCompression(level, window_bits, mem_level, strategy),
      m_Mode(eMode_Read), m_File(0), m_Stream(0)
{
    // For backward compatibility -- use gzip file format by default
    SetFlags(GetFlags() | fGZip);
    return;
}


CZipCompressionFile::~CZipCompressionFile(void)
{
    try {
        Close();
    }
    COMPRESS_HANDLE_EXCEPTIONS(92, "CZipCompressionFile::~CZipCompressionFile");
    return;
}


void CZipCompressionFile::GetStreamError(void)
{
    int     errcode;
    string  errdesc;
    if ( m_Stream->GetError(CCompressionStream::eRead, errcode, errdesc) ) {
        SetError(errcode, errdesc);
    }
}


bool CZipCompressionFile::Open(const string& file_name, EMode mode)
{
    return Open(file_name, mode, 0 /*info*/);
}


bool CZipCompressionFile::Open(const string& file_name, EMode mode, SFileInfo* info)
{
    m_Mode = mode;

    // Open a file
    if ( mode == eMode_Read ) {
        m_File = new CNcbiFstream(file_name.c_str(),
                                  IOS_BASE::in | IOS_BASE::binary);
    } else {
        m_File = new CNcbiFstream(file_name.c_str(),
                                  IOS_BASE::out | IOS_BASE::binary | IOS_BASE::trunc);
    }
    if ( !m_File->good() ) {
        Close();
        string description = string("Cannot open file '") + file_name + "'";
        SetError(-1, description.c_str());
        return false;
    }
    // Get file information
    if (mode == eMode_Read  &&  F_ISSET(fCheckFileHeader)  &&  info) {
        char buf[kMaxHeaderSize];
        m_File->read(buf, kMaxHeaderSize);
        // NOTE: 
        // m_File->seekg(0, ios_base::beg); -- do not work in all cases
        if ( m_File->rdbuf()->PUBSEEKPOS(0, ios_base::in) == CT_POS_TYPE(-1) ) {
            string description = string("Cannot read file header '") + file_name + "'";
            SetError(-1, description.c_str());
            return false;
        }
        s_CheckGZipHeader(buf, (size_t)m_File->gcount(), info);
    }

    // Create compression stream for I/O
    if ( mode == eMode_Read ) {
        CZipDecompressor* decompressor =  new CZipDecompressor(m_WindowBits, GetFlags());
        CCompressionStreamProcessor* processor = 
            new CCompressionStreamProcessor(
                decompressor, CCompressionStreamProcessor::eDelete,
                kCompressionDefaultBufSize, kCompressionDefaultBufSize);
        m_Stream = new CCompressionIOStream(*m_File, processor, 0, CCompressionStream::fOwnReader);
    } else {
        CZipCompressor* compressor = new CZipCompressor(GetLevel(), m_WindowBits, m_MemLevel, m_Strategy, GetFlags());
        if ( F_ISSET(fWriteGZipFormat)  &&  info) {
            // Enable compressor to write info information about
            // compressed file into gzip file header
            compressor->SetFileInfo(*info);
        }
        CCompressionStreamProcessor* processor = 
            new CCompressionStreamProcessor(
                compressor, CCompressionStreamProcessor::eDelete,
                kCompressionDefaultBufSize, kCompressionDefaultBufSize);
        m_Stream = new CCompressionIOStream(*m_File, 0, processor, CCompressionStream::fOwnWriter);
    }
    if ( !m_Stream->good() ) {
        Close();
        SetError(-1, "Cannot create compression stream");
        return false;
    }
    return true;
} 


long CZipCompressionFile::Read(void* buf, size_t len)
{
    LIMIT_SIZE_PARAM_LONG(len);
    LIMIT_SIZE_PARAM_STREAMSIZE(len);

    if ( !m_Stream  ||  m_Mode != eMode_Read ) {
        NCBI_THROW(CCompressionException, eCompressionFile, 
            "[CZipCompressionFile::Read]  File must be opened for reading");
    }
    if ( !m_Stream->good() ) {
        return 0;
    }
    m_Stream->read((char*)buf, len);
    // Check decompression processor status
    if ( m_Stream->GetStatus(CCompressionStream::eRead) == CCompressionProcessor::eStatus_Error ) {
        GetStreamError();
        return -1;
    }
    long nread = (long)m_Stream->gcount();
    if ( nread ) {
        return nread;
    }
    if ( m_Stream->eof() ) {
        return 0;
    }
    GetStreamError();
    return -1;
}


long CZipCompressionFile::Write(const void* buf, size_t len)
{
    if ( !m_Stream  ||  m_Mode != eMode_Write ) {
        NCBI_THROW(CCompressionException, eCompressionFile, 
            "[CZipCompressionFile::Write]  File must be opened for writing");
    }
    // Redefine standard behavior for case of writing zero bytes
    if (len == 0) {
        return 0;
    }
    LIMIT_SIZE_PARAM_LONG(len);
    LIMIT_SIZE_PARAM_STREAMSIZE(len);

    m_Stream->write((char*)buf, len);
    if ( m_Stream->good() ) {
        return (long)len;
    }
    GetStreamError();
    return -1;
}


bool CZipCompressionFile::Close(void)
{
    // Close compression/decompression stream
    if ( m_Stream ) {
        m_Stream->Finalize();
        GetStreamError();
        delete m_Stream;
        m_Stream = 0;
    }
    // Close file stream
    if ( m_File ) {
        m_File->close();
        delete m_File;
        m_File = 0;
    }
    return true;
}



//////////////////////////////////////////////////////////////////////////////
//
// CZipCompressor
//


CZipCompressor::CZipCompressor(ELevel level,  int window_bits,
                               int mem_level, int strategy, TZipFlags flags)
    : CZipCompression(level, window_bits, mem_level, strategy),
      m_CRC32(0), m_NeedWriteHeader(true)
{
    SetFlags(flags);
}


CZipCompressor::~CZipCompressor()
{
}


void CZipCompressor::SetFileInfo(const SFileInfo& info)
{
    m_FileInfo = info;
}


CCompressionProcessor::EStatus CZipCompressor::Init(void)
{
    if ( IsBusy() ) {
        // Abnormal previous session termination
        End();
    }
    // Initialize members
    Reset();
    SetBusy();

    m_CRC32 = 0;
    m_NeedWriteHeader = true;
    m_Cache.erase();

    // Initialize the compressor stream structure
    memset(STREAM, 0, sizeof(z_stream));
    // Create a compressor stream
    int errcode = deflateInit2_(STREAM, GetLevel(), Z_DEFLATED,
                                F_ISSET(fWriteGZipFormat) ? -m_WindowBits : m_WindowBits,
                                m_MemLevel, m_Strategy,
                                ZLIB_VERSION, (int)sizeof(z_stream));
    SetError(errcode, zError(errcode));
    if ( errcode == Z_OK ) {
        return eStatus_Success;
    }
    ERR_COMPRESS(60, FormatErrorMessage("CZipCompressor::Init", GetProcessedSize()));
    return eStatus_Error;
}


CCompressionProcessor::EStatus CZipCompressor::Process(
                      const char* in_buf,  size_t  in_len,
                      char*       out_buf, size_t  out_size,
                      /* out */            size_t* in_avail,
                      /* out */            size_t* out_avail)
{
    *out_avail = 0;
    if ( !out_size ) {
        return eStatus_Overflow;
    }
    // To simplify this method, limit input and output sizes,
    // the upper level code will process all necessary data
    // in the loop if necessary. 
    LIMIT_SIZE_PARAM_UINT(in_len);
    LIMIT_SIZE_PARAM_UINT(out_size);

    size_t header_len = 0;

    // Write gzip file header
    if ( F_ISSET(fWriteGZipFormat)  &&  m_NeedWriteHeader ) {
        header_len = s_WriteGZipHeader(out_buf, out_size, &m_FileInfo);
        if (!header_len) {
            SetError(-1, "Cannot write gzip header");
            ERR_COMPRESS(62, FormatErrorMessage("CZipCompressor::Process", GetProcessedSize()));
            return eStatus_Error;
        }
        m_NeedWriteHeader = false;
    }
    STREAM->next_in   = (unsigned char*)const_cast<char*>(in_buf);
    STREAM->avail_in  = (unsigned int)in_len;
    STREAM->next_out  = (unsigned char*)out_buf + header_len;
    STREAM->avail_out = (unsigned int)(out_size - header_len);

    int errcode = deflate(STREAM, Z_NO_FLUSH);
    SetError(errcode, zError(errcode));
    *in_avail  = STREAM->avail_in;
    *out_avail = out_size - STREAM->avail_out;
    IncreaseProcessedSize(in_len - *in_avail);
    IncreaseOutputSize(*out_avail);

    // If we writing in gzip file format
    if ( F_ISSET(fWriteGZipFormat) ) {
        // Update the CRC32 for processed data
        m_CRC32 = crc32(m_CRC32, (unsigned char*)in_buf,
                        (unsigned int)(in_len - *in_avail));
    }
    if ( errcode == Z_OK ) {
        return eStatus_Success;
    }
    ERR_COMPRESS(63, FormatErrorMessage("CZipCompressor::Process", GetProcessedSize()));
    return eStatus_Error;
}


CCompressionProcessor::EStatus CZipCompressor::Flush(
                      char* out_buf, size_t  out_size,
                      /* out */      size_t* out_avail)
{
    *out_avail = 0;
    if ( !out_size ) {
        return eStatus_Overflow;
    }
    // To simplify this method, limit output size,
    // the upper level code will process all necessary
    // data in the loop if necessary. 
    LIMIT_SIZE_PARAM_UINT(out_size);

    STREAM->next_in   = 0;
    STREAM->avail_in  = 0;
    STREAM->next_out  = (unsigned char*)out_buf;
    STREAM->avail_out = (unsigned int)out_size;

    int errcode = deflate(STREAM, Z_SYNC_FLUSH);
    SetError(errcode, zError(errcode));
    *out_avail = out_size - STREAM->avail_out;
    IncreaseOutputSize(*out_avail);

    if ( errcode == Z_OK  ||  errcode == Z_BUF_ERROR ) {
        if ( STREAM->avail_out == 0) {
            return eStatus_Overflow;
        }
        return eStatus_Success;
    }
    ERR_COMPRESS(64, FormatErrorMessage("CZipCompressor::Flush", GetProcessedSize()));
    return eStatus_Error;
}


CCompressionProcessor::EStatus CZipCompressor::Finish(
                      char* out_buf, size_t  out_size,
                      /* out */      size_t* out_avail)
{
    *out_avail = 0;
    if ( !out_size ) {
        return eStatus_Overflow;
    }
    // To simplify this method, limit output size,
    // the upper level code will process all necessary
    // data in the loop if necessary. 
    LIMIT_SIZE_PARAM_UINT(out_size);

    // Default behavior on empty data -- don't write header/footer
    if ( !GetProcessedSize()  &&  !F_ISSET(fAllowEmptyData) ) {
        return eStatus_EndOfData;
    }

    // Write gzip file header if not done yet
    size_t header_len = 0;
    if ( F_ISSET(fWriteGZipFormat)  &&  m_NeedWriteHeader ) {
        header_len = s_WriteGZipHeader(out_buf, out_size, &m_FileInfo);
        if (!header_len) {
            SetError(-1, "Cannot write gzip header");
            return eStatus_Overflow;
        }
        // IncreaseOutputSize()
        // -- will be called below, and it will count 'header_len'
        m_NeedWriteHeader = false;
    }

    // Finish compression
    STREAM->next_in   = 0;
    STREAM->avail_in  = 0;
    STREAM->next_out  = (unsigned char*)out_buf + header_len;
    STREAM->avail_out = (unsigned int)(out_size - header_len);

    int errcode = deflate(STREAM, Z_FINISH);
    SetError(errcode, zError(errcode));
    *out_avail = out_size - STREAM->avail_out;
    IncreaseOutputSize(*out_avail);

    switch (errcode) {
    case Z_OK:
        return eStatus_Overflow;
    case Z_STREAM_END:
        // Write .gz file footer
        if ( F_ISSET(fWriteGZipFormat) ) {
            size_t footer_len = 
                s_WriteGZipFooter(out_buf + *out_avail, STREAM->avail_out,
                                  GetProcessedSize(), m_CRC32);
            if ( !footer_len ) {
                SetError(-1, "Cannot write gzip footer");
                return eStatus_Overflow;
            }
            IncreaseOutputSize(footer_len);
            *out_avail += footer_len;
        }
        return eStatus_EndOfData;
    }
    ERR_COMPRESS(66, FormatErrorMessage("CZipCompressor::Finish", GetProcessedSize()));
    return eStatus_Error;
}


CCompressionProcessor::EStatus CZipCompressor::End(int abandon)
{
    int errcode = deflateEnd(STREAM);
    SetBusy(false);
    if (abandon) {
        // Ignore result of deflateEnd(), because it can return an error code for empty data
        return eStatus_Success;
    }
    SetError(errcode, zError(errcode));
    if ( errcode == Z_OK ) {
        return eStatus_Success;
    }
    ERR_COMPRESS(67, FormatErrorMessage("CZipCompressor::End", GetProcessedSize()));
    return eStatus_Error;
}



//////////////////////////////////////////////////////////////////////////////
//
// CZipDecompressor
//


CZipDecompressor::CZipDecompressor(int window_bits, TZipFlags flags)
    : CZipCompression(eLevel_Default, window_bits, 0, 0),
      m_NeedCheckHeader(true), m_IsGZ(false), m_SkipInput(0)
{
    SetFlags(flags);
}


CZipDecompressor::~CZipDecompressor()
{
    if ( IsBusy() ) {
        // Abnormal session termination
        End();
    }
}


CCompressionProcessor::EStatus CZipDecompressor::Init(void)
{
    // Initialize members
    Reset();
    SetBusy();
    m_NeedCheckHeader = true;
    m_IsGZ = false;
    m_SkipInput = 0;
    m_Cache.erase();
    m_Cache.reserve(kMaxHeaderSize);

    // Initialize the compressor stream structure
    memset(STREAM, 0, sizeof(z_stream));
    
    // Create a compressor stream
    int errcode = inflateInit2_(STREAM, m_WindowBits, ZLIB_VERSION, (int)sizeof(z_stream));
    SetError(errcode, zError(errcode));
    if ( errcode == Z_OK ) {
        return eStatus_Success;
    }
    ERR_COMPRESS(68, FormatErrorMessage("CZipDecompressor::Init", GetProcessedSize()));
    return eStatus_Error;
}


CCompressionProcessor::EStatus CZipDecompressor::Process(
                      const char* in_buf,  size_t  in_len,
                      char*       out_buf, size_t  out_size,
                      /* out */            size_t* in_avail,
                      /* out */            size_t* out_avail)
{
    *out_avail = 0;
    if ( !out_size ) {
        return eStatus_Overflow;
    }
    // To simplify this method, limit input and output sizes,
    // the upper level code will process all necessary data
    // in the loop if necessary. 
    LIMIT_SIZE_PARAM_UINT(in_len);
    LIMIT_SIZE_PARAM_UINT(out_size);

    // By default we consider that data is compressed
    if ( m_DecompressMode == eMode_Unknown  &&
        !F_ISSET(fAllowTransparentRead) ) {
        m_DecompressMode = eMode_Decompress;
    }

    char*  x_in_buf = const_cast<char*>(in_buf);
    size_t x_in_len = in_len;

    // If data is compressed, or the read mode is undefined yet
    if ( m_DecompressMode != eMode_TransparentRead ) {

        // Need to skip some bytes from input stream?
        // (in case of concatenated .gz files only)
        if ( m_SkipInput ) {
            // Skip from cache if present
            if ( m_Cache.size() ) {
                size_t n = min(m_Cache.size(), m_SkipInput);
                m_Cache.erase(0, n);
                m_SkipInput -= n;
                IncreaseProcessedSize(n);
            }
            // And/or from input stream also
            if ( m_SkipInput ) {
                size_t n = min(x_in_len, m_SkipInput);
                x_in_buf += n;
                x_in_len -= n;
                m_SkipInput -= n;
                IncreaseProcessedSize(n);
                if ( m_SkipInput ) {
                    // Data block is very small... and was skipped.
                    *in_avail  = x_in_len;
                    *out_avail = 0;
                    return eStatus_Success;
                }
            }
        }

        bool   from_cache   = false;
        size_t old_avail_in = 0;

        // Check file header
        if ( F_ISSET(fCheckFileHeader) ) {
            size_t header_len = 0;
            if ( m_NeedCheckHeader ) {
                if (!x_in_buf  &&  !m_Cache.size()) {
                    // Possible Flush(), but we should refill the cache
                    // to perform header check -- so, just ignore.
                    *in_avail  = 0;
                    *out_avail = 0;
                    return eStatus_Success;
                }
                if (x_in_buf  &&  m_Cache.size() < kMaxHeaderSize) {
                    size_t n = min(kMaxHeaderSize - m_Cache.size(), x_in_len);
                    m_Cache.append(x_in_buf, n);
                    x_in_buf += n;
                    x_in_len -= n;
                    if (m_Cache.size() < kMaxHeaderSize) {
                        // Data block is very small and was fully cached.
                        *in_avail  = 0;
                        *out_avail = 0;
                        return eStatus_Success;
                    }
                }
                // Check gzip header in the buffer
                header_len = s_CheckGZipHeader(m_Cache.data(), m_Cache.size());
                _ASSERT(header_len < kMaxHeaderSize);

                // If gzip header found, skip it
                if ( header_len ) {
                    m_Cache.erase(0, header_len);
                    IncreaseProcessedSize(header_len);
                    m_DecompressMode = eMode_Decompress;
                    m_IsGZ = true;
                }
                // Reinit decompression stream
                inflateEnd(STREAM);
                int errcode = inflateInit2_(STREAM,
                                            m_IsGZ ? -m_WindowBits : m_WindowBits,
                                            ZLIB_VERSION,
                                            (int)sizeof(z_stream));
                SetError(errcode, zError(errcode));
                if ( errcode != Z_OK ) {
                    return eStatus_Error;
                }
                // Already skipped, or we don't have header here
                m_NeedCheckHeader = false;
            }
        }

        // Prepare STREAM for decompressing
        if ( m_Cache.size() ) {
            // Possible, we have some unprocessed data in the cache
            STREAM->next_in   = (unsigned char*)(m_Cache.data());
            STREAM->avail_in  = (unsigned int)m_Cache.size();
            STREAM->next_out  = (unsigned char*)out_buf;
            STREAM->avail_out = (unsigned int)out_size;
            from_cache        = true;
            old_avail_in      = STREAM->avail_in; // = m_Cache.size()
        } else {
            STREAM->next_in   = (unsigned char*)x_in_buf;
            STREAM->avail_in  = (unsigned int)x_in_len;
            STREAM->next_out  = (unsigned char*)out_buf;
            STREAM->avail_out = (unsigned int)out_size;
        }

        // Try to decompress data
        int errcode = inflate(STREAM, Z_SYNC_FLUSH);

        if ( m_DecompressMode == eMode_Unknown ) {
            // The flag fAllowTransparentRead is set
            _ASSERT(F_ISSET(fAllowTransparentRead));
            // Determine decompression mode for following operations
            if (errcode == Z_OK  ||  errcode == Z_STREAM_END) {
                m_DecompressMode = eMode_Decompress;
            } else {
                m_DecompressMode = eMode_TransparentRead;
            }
        }
        if ( m_DecompressMode == eMode_Decompress ) {
            SetError(errcode, zError(errcode));

            // Concatenated file? Try to process next .gz chunk, if present
            if ((errcode == Z_STREAM_END)  &&  m_IsGZ) {
                // Skip .gz file footer (8 bytes)
                if (STREAM->avail_in < 8) {
                    m_SkipInput = 8 - STREAM->avail_in;
                    STREAM->avail_in = 0;
                } else {
                    STREAM->avail_in -= 8;
                }
                if ( F_ISSET(fAllowConcatenatedGZip) ) {
                    m_NeedCheckHeader = true;
                    errcode = Z_OK;
                }
            }
            // Update count of processed data
            if ( from_cache ) {
                m_Cache.erase(0, old_avail_in - STREAM->avail_in);
                *in_avail = x_in_len;
                IncreaseProcessedSize(old_avail_in - STREAM->avail_in);
            } else {
                *in_avail = STREAM->avail_in;
                IncreaseProcessedSize(x_in_len - *in_avail);
                x_in_len = *in_avail;
            }
            // In case of concatenated .gz files:
            // Possible, we already skipped some bytes from cache, 
            // and it should be empty now. If needed, try to skip some
            // bytes from the input stream also.
            if ( m_SkipInput ) {
                _ASSERT(m_Cache.size() == 0);
                size_t n = min(x_in_len, m_SkipInput);
                if ( n ) {
                    x_in_len -= n;
                    m_SkipInput -= n;
                    *in_avail = x_in_len;
                    IncreaseProcessedSize(n);
                }
            }
            *out_avail = out_size - STREAM->avail_out;
            IncreaseOutputSize(*out_avail);

            // Analyze decompressor status
            switch (errcode) {
            case Z_OK:
                if ( from_cache  &&  
                     STREAM->avail_in > 0  &&  *out_avail == 0) {
                     return m_NeedCheckHeader ? eStatus_Repeat : eStatus_Overflow;
                }
                return eStatus_Success;
            case Z_STREAM_END:
                return eStatus_EndOfData;
            }
            ERR_COMPRESS(70, FormatErrorMessage("CZipDecompressor::Process", GetProcessedSize()));
            return eStatus_Error;
        }
        /* else: eMode_ThansparentRead (see below) */
    }

    // Transparent read

    _ASSERT(m_DecompressMode == eMode_TransparentRead);
    size_t total = 0;
    if ( m_Cache.size() ) {
        total = min(m_Cache.size(), out_size);
        memcpy(out_buf, m_Cache.data(), total);
        m_Cache.erase(0, total);
        out_size -= total;
    }
    if (x_in_buf  &&  x_in_len  &&  out_size)  {
        size_t n = min(x_in_len, out_size);
        memcpy(out_buf + total, x_in_buf, n);
        total += n;
        x_in_len -= n;
    }
    *in_avail  = x_in_len;
    *out_avail = total;
    IncreaseProcessedSize(total);
    IncreaseOutputSize(total);

    return eStatus_Success;
}


CCompressionProcessor::EStatus CZipDecompressor::Flush(
                      char*   out_buf,
                      size_t  out_size,
                      size_t* out_avail)
{
    // Do not check here on eMode_Unknown. It will be processed below.
    size_t in_avail;
    return Process(0, 0, out_buf, out_size, &in_avail, out_avail);
}


CCompressionProcessor::EStatus CZipDecompressor::Finish(
                      char*   out_buf,
                      size_t  out_size,
                      size_t* out_avail)
{
    if (m_DecompressMode == eMode_TransparentRead) {
        return eStatus_EndOfData;
    }
    // Do not check here on eMode_Unknown. It will be processed below.
    size_t in_avail;
    // Process only if we have some data -- otherwise zlib return error
    CCompressionProcessor::EStatus status = eStatus_Success;
    if ( GetProcessedSize() || m_Cache.size() ) {
        status = Process(0, 0, out_buf, out_size, &in_avail, out_avail);
    } else {
        if ( !F_ISSET(fAllowEmptyData) ) {
            return eStatus_Error;
        }
    }
    if (status == eStatus_Success  &&  *out_avail == 0) {
        // Possible case with .gz files only (check on concatenated files)
        return eStatus_EndOfData;
    }
    return status;
}


CCompressionProcessor::EStatus CZipDecompressor::End(int abandon)
{
    int errcode = inflateEnd(STREAM);
    SetBusy(false);
    if ( abandon ||
         m_DecompressMode == eMode_TransparentRead   ||
         errcode == Z_OK ) {
        return eStatus_Success;
    }
    ERR_COMPRESS(71, FormatErrorMessage("CZipDecompressor::End", GetProcessedSize()));
    return eStatus_Error;
}


//////////////////////////////////////////////////////////////////////////////
//
// Global functions
//

void g_GZip_ScanForChunks(CNcbiIstream& is, IChunkHandler& handler)
{
    typedef IChunkHandler::TPosition TPos;

    // Use our own total counters to avoid 4GB limit 
    TPos     total_in  = 0;       // Offset in input compressed data
    TPos     total_out = 0;       // Offset in output decompressed data
    z_stream strm;                // Compressed stream structure
    int      ret = Z_STREAM_END;  // zlib return status, init with Z_STREAM_END to get a (0,0)
    bool     initialized = false;


    // Default buffer size
    size_t in_size  = kCompressionDefaultBufSize;
    size_t out_size = kCompressionDefaultBufSize * 2;

    // Allocate buffers
    AutoArray<unsigned char> in_buf_arr(in_size);
    unsigned char* in_buf = in_buf_arr.get();
    if ( !in_buf ) {
        NCBI_THROW(CCoreException, eNullPtr, kEmptyStr);
    }
    AutoArray<unsigned char> out_buf_arr(out_size);
    unsigned char* out_buf = out_buf_arr.get();
    if ( !out_buf ) {
        NCBI_THROW(CCoreException, eNullPtr, kEmptyStr);
    }

    try {
        IChunkHandler::EAction action = IChunkHandler::eAction_Continue;
        // Process all decompressed data in the input stream
        while ( is  &&  action != IChunkHandler::eAction_Stop) {
            // Get some compressed data
            is.read((char*)in_buf, in_size);
            size_t nread = (size_t)is.gcount();
            if ( !nread ) {
                break;
            }
            // Process all data in the buffer
            strm.next_in  = in_buf;
            strm.avail_in = (unsigned int)nread;
            do {
                // Next gzip-file?
                if (ret == Z_STREAM_END) {
                    // Save current position
                    action = handler.OnChunk(total_in, total_out);
                    if (action == IChunkHandler::eAction_Stop) {
                        // Stop scanning
                        break;
                    }
                    // (Re)Initialize inflate
                    strm.zalloc = NULL;
                    strm.zfree  = NULL;
                    strm.opaque = NULL;
                    ret = inflateInit2(&strm, 15+16 /* max windowbits + automatic gzip header decoding */); /* NCBI_FAKE_WARNING */
                    if (ret != Z_OK) {
                        throw "inflateInit2() failed: " + string(zError(ret));
                    }
                    initialized = true;
                }
                // We don't need uncompressed data -- discard it
                strm.next_out  = out_buf;
                strm.avail_out = (unsigned int)out_size;

                // Decompress
                ret = inflate(&strm, Z_SYNC_FLUSH);
                if (ret != Z_OK  &&  ret != Z_STREAM_END ) {
                    // Error
                    throw "inflate() failed: " + string(zError(ret));
                }
                // Increase counters
                total_out += (out_size - strm.avail_out);
                total_in  += (nread - strm.avail_in);
                nread = strm.avail_in;
                // If found end of compressed stream -- cleanup
                if (ret == Z_STREAM_END) {
                    inflateEnd(&strm);
                    initialized = false;
                }
            } while (strm.avail_in != 0);
        }
        if ( initialized ) {
            inflateEnd(&strm);
        }
    }
    // Cleanup
    catch (string& e) {
        if ( initialized ) {
            inflateEnd(&strm);
        }
        NCBI_THROW(CCompressionException, eCompression, e);
    }
}


END_NCBI_SCOPE
