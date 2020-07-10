/*  $Id: remote_app.cpp 584886 2019-04-18 16:59:46Z sadyrovr $
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
 * Authors:  Maxim Didenko, Dmitry Kazimirov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <connect/services/grid_rw_impl.hpp>
#include <connect/services/remote_app.hpp>
#include <connect/services/error_codes.hpp>

#include <corelib/ncbifile.hpp>


#define NCBI_USE_ERRCODE_X   ConnServ_Remote

BEGIN_NCBI_SCOPE

//////////////////////////////////////////////////////////////////////////////
//
inline CNcbiOstream& WriteStrWithLen(CNcbiOstream& os, const string& str)
{
    os << str.size() << ' ' << str;
    return os;
}

inline CNcbiIstream& ReadStrWithLen(CNcbiIstream& is, string& str)
{
    string::size_type len;
    if (!is.good()) return is;
    is >> len;
    if (!is.good()) return is;
    vector<char> buf(len+1);
    is.read(&buf[0], len+1);
    str.assign(buf.begin()+1, buf.end());
    return is;
}

//////////////////////////////////////////////////////////////////////////////
//

CBlobStreamHelper::~CBlobStreamHelper()
{
    try {
        Reset();
    } NCBI_CATCH_ALL_X(14, "CBlobStreamHelper::~CBlobStreamHelper()");
}

CNcbiOstream& CBlobStreamHelper::GetOStream(const string& fname /*= ""*/,
    EStdOutErrStorageType type /*= eBlobStorage*/,
    size_t max_inline_size /*= kMaxBlobInlineSize*/)
{
    if (!m_GridWrite.stream) {
        _ASSERT(!m_GridRead.stream);

        m_GridWrite(m_Storage, max_inline_size, *m_Data);
        *m_GridWrite.stream << (int) type << " ";
        WriteStrWithLen(*m_GridWrite.stream, fname);
        if (!fname.empty() && type == eLocalFile) {
            m_GridWrite.stream.reset(new CNcbiOfstream(fname.c_str()));
            m_GridWrite.writer.reset();
            if (!m_GridWrite.stream->good()) {
                NCBI_THROW(CFileException, eRelativePath,
                    "Cannot open " + fname + " for output");
            }
            m_GridWrite.stream->exceptions(IOS_BASE::badbit | IOS_BASE::failbit);
        }
    }
    return *m_GridWrite.stream;
}

int CBlobStreamHelper::x_GetTypeAndName(CNcbiIstream& istream,
        string& name)
{
    int res = eBlobStorage;
    if (istream.good()) istream >> res;
    if (istream.good()) ReadStrWithLen(istream, name);
    return res;
}

CNcbiIstream& CBlobStreamHelper::GetIStream(string* fname /*= NULL*/,
    EStdOutErrStorageType* type /*= NULL*/)
{
    if (!m_GridRead.stream) {
        _ASSERT(!m_GridWrite.stream);

        m_GridRead(m_Storage, *m_Data, m_DataSize);
        string name;
        int tmp = (int)eBlobStorage;
        try {
            tmp = x_GetTypeAndName(*m_GridRead.stream, name);
        } catch (...) {
            if (!m_GridRead.stream->eof()) {
                string msg =
                        "Job output does not match remote_app output format";
                ERR_POST_X(1, msg);
                m_GridRead.stream.reset(new CNcbiIstrstream(msg.c_str()));
            }
            return *m_GridRead.stream.get();
        }

        if (fname) *fname = name;
        if (type) *type = (EStdOutErrStorageType)tmp;
        if (!name.empty() && (EStdOutErrStorageType)tmp == eLocalFile) {
            m_GridRead.stream.reset(new CNcbiIfstream(name.c_str()));
            if (m_GridRead.stream->good()) {
                m_GridRead.stream->exceptions(IOS_BASE::badbit | IOS_BASE::failbit);
            } else {
                string msg = "Can not open " + name;
                msg += " for reading";
                ERR_POST_X(2, msg);
                m_GridRead.stream.reset(new CNcbiIstrstream(msg.c_str()));
            }
        }
    }
    return *m_GridRead.stream;
}

void CBlobStreamHelper::Reset()
{
    m_GridRead.Reset();
    m_GridWrite.Reset(true);
}
//////////////////////////////////////////////////////////////////////////////
//

CAtomicCounter CRemoteAppRequest::sm_DirCounter;

const string kLocalFSSign = "LFS";

CRemoteAppRequest::~CRemoteAppRequest()
{
    try {
        Reset();
    } NCBI_CATCH_ALL_X(15, "CRemoteAppRequest::~CRemoteAppRequest()");
}

void CRemoteAppRequest::Send(CNcbiOstream& os)
{
    m_StdIn.Reset();
    typedef map<string,string> TFmap;
    TFmap file_map;
    ITERATE(TFiles, it, GetFileNames()) {
        const string& fname = it->first;
        if (it->second == eLocalFile) {
            file_map[fname] = kLocalFSSign;
            continue;
        }
        CFile file(fname);
        string blobid;
        if (!file.Exists()) {
            LOG_POST_X(3, Warning << "File :\"" << fname << "\" does not exist.");
            continue;
        }
        if (NStr::Find(GetCmdLine(), fname) == NPOS) {
            LOG_POST_X(4, Warning << "File :\"" << fname << "\" is not found in cmdline. Skipping.");
            continue;
        }

        CNcbiIfstream inf(fname.c_str());
        if (inf.good()) {
            unique_ptr<CNcbiOstream> of(GetNetCacheAPI().CreateOStream(blobid));
            *of << inf.rdbuf();
            file_map[fname] = blobid;
        }
    }

    WriteStrWithLen(os, GetCmdLine());
    WriteStrWithLen(os, m_InBlobIdOrData);

    os << file_map.size() << ' ';
    ITERATE(TFmap, itf, file_map) {
        WriteStrWithLen(os, itf->first);
        WriteStrWithLen(os, itf->second);
    }
    WriteStrWithLen(os, m_StdOutFileName);
    WriteStrWithLen(os, m_StdErrFileName);
    os << (int)m_StorageType << " ";
    os << GetAppRunTimeout() << " ";
    os << (int)m_ExlusiveMode;
    Reset();
}

static void s_ReplaceArg( vector<string>& args, const string& old_fname,
                          const string& new_fname)
{
    for(vector<string>::iterator it = args.begin();
        it != args.end(); ++it) {
        string& arg = *it;
        SIZE_TYPE pos = NStr::Find(arg, old_fname);
        if (pos == NPOS)
            return;
        if ( (pos == 0 || !isalnum((unsigned char)arg[pos-1]) )
             && pos + old_fname.size() == arg.size())
            arg = NStr::Replace(arg, old_fname, new_fname);
    }
}

bool CRemoteAppRequest::x_Deserialize(CNcbiIstream& is, TStoredFiles* files)
{
    // Partial deserialization doesn't create working dir and deserialize files,
    // but fills the "files" map with deserialized filenames and blob IDs.
    const bool partial_deserialization = files;

    if (partial_deserialization)
        files->clear();

    Reset();

    string cmdline;
    ReadStrWithLen(is, cmdline);
    SetCmdLine(cmdline);
    ReadStrWithLen(is, m_InBlobIdOrData);

    int fcount = 0;
    vector<string> args;
    if (!is.good()) return false;
    is >> fcount;
    if ( fcount > 0 && !partial_deserialization) {
        TokenizeCmdLine(GetCmdLine(), args);
        x_CreateWDir();
    }

    for( int i = 0; i < fcount; ++i) {
        string blobid, fname;
        ReadStrWithLen(is, fname);
        ReadStrWithLen(is, blobid);
        if (!is.good()) return false;

        const bool is_blob = blobid != kLocalFSSign;
        if (partial_deserialization) {
            files->insert(make_pair(fname, is_blob ? blobid : kEmptyStr));
        } else if (is_blob) {
            string nfname = GetWorkingDir() + CDirEntry::GetPathSeparator()
                + blobid;
            CNcbiOfstream of(nfname.c_str());
            if (of.good()) {
                unique_ptr<CNcbiIstream> blob_is(GetNetCacheAPI().GetIStream(blobid));
                of << blob_is->rdbuf();
                blob_is.reset();
                s_ReplaceArg(args, fname, nfname);
            }
        }
    }
    if ( fcount > 0 && !partial_deserialization) {
        SetCmdLine(JoinCmdLine(args));
    }

    ReadStrWithLen(is, m_StdOutFileName);
    ReadStrWithLen(is, m_StdErrFileName);
    if (!is.good()) return false;
    int tmp;
    is >> tmp;
    m_StorageType = (EStdOutErrStorageType)tmp;
    if (!is.good()) return false;
    is >> tmp; SetAppRunTimeout(tmp);
    if (!is.good()) return false;
    is >> tmp;
    m_ExlusiveMode = tmp != 0;
    return !is.fail();
}

void CRemoteAppRequest::Reset()
{
    m_CmdLine = "";
    m_Files.clear();
    m_AppRunTimeout = 0;
    x_RemoveWDir();

    m_StdIn.Reset();
    m_InBlobIdOrData = "";
    m_StdInDataSize = 0;
    m_ExlusiveMode = false;
}

void CRemoteAppRequest::x_CreateWDir()
{
    if (!m_TmpDirName.empty())
        return;
    m_TmpDirName = m_TmpDirPath + NStr::NumericToString(sm_DirCounter.Add(1));
    CDir wdir(m_TmpDirName);
    if (wdir.Exists())
        wdir.Remove();
    CDir(m_TmpDirName).CreatePath();
}

void CRemoteAppRequest::x_RemoveWDir()
{
    if (m_TmpDirName.empty())
        return;
    CDir dir(m_TmpDirName);
    if (dir.Exists())
        dir.Remove();
    m_TmpDirName = "";
}


//////////////////////////////////////////////////////////////////////////////
//

CRemoteAppRequest::CRemoteAppRequest(
        CNetCacheAPI::TInstance storage, size_t max_inline_size) :
    m_NetCacheAPI(storage),
    m_AppRunTimeout(0),
    m_TmpDirPath(CDir::GetCwd() + CDirEntry::GetPathSeparator()),
    m_StdIn(storage, m_InBlobIdOrData, m_StdInDataSize),
    m_StdInDataSize(0),
    m_StorageType(eBlobStorage),
    m_ExlusiveMode(false),
    m_MaxInlineSize(max_inline_size)
{
}

CRemoteAppResult::~CRemoteAppResult()
{
    try {
        Reset();
    } NCBI_CATCH_ALL_X(16, "CRemoteAppResult::~CRemoteAppResult()");
}

void CRemoteAppResult::Serialize(CNcbiOstream& os)
{
    m_StdOut.Reset();
    m_StdErr.Reset();
    WriteStrWithLen(os, m_OutBlobIdOrData);
    WriteStrWithLen(os, m_ErrBlobIdOrData);
    os << GetRetCode();
}
void CRemoteAppResult::Receive(CNcbiIstream& is)
{
    Reset();
    ReadStrWithLen(is, m_OutBlobIdOrData);
    ReadStrWithLen(is, m_ErrBlobIdOrData);
    int ret = -1; is >> ret; SetRetCode(ret);
}

void CRemoteAppResult::Reset()
{
    m_RetCode = -1;

    m_OutBlobIdOrData = "";
    m_OutBlobSize = 0;
    m_StdOut.Reset();

    m_ErrBlobIdOrData = "";
    m_ErrBlobSize = 0;
    m_StdErr.Reset();

    m_StdOutFileName = "";
    m_StdErrFileName = "";
    m_StorageType = eBlobStorage;
}


void TokenizeCmdLine(const string& cmdline, vector<string>& args)
{
    if (!cmdline.empty()) {
        string arg;

        for (size_t i = 0; i < cmdline.size();) {
            if (cmdline[i] == ' ') {
                if (!arg.empty()) {
                    args.push_back(arg);
                    arg.erase();
                }
                i++;
                continue;
            }
            if (cmdline[i] == '\'' || cmdline[i] == '"') {
                char quote = cmdline[i];
                while( ++i < cmdline.size() && cmdline[i] != quote )
                    arg += cmdline[i];
                args.push_back(arg);
                arg.erase();
                ++i;
                continue;
            }
            arg += cmdline[i++];
        }
        if (!arg.empty())
            args.push_back(arg);
    }
}


string JoinCmdLine(const vector<string>& args)
{
    string cmd_line;

    for (vector<string>::const_iterator it = args.begin();
            it != args.end(); ++it) {
        if (it != args.begin())
            cmd_line += ' ';

        if (it->find(" ") != string::npos)
            cmd_line += '\"' + *it + '\"';
        else
            cmd_line += *it;
    }
    return cmd_line;
}


END_NCBI_SCOPE
