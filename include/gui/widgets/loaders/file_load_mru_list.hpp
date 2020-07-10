#ifndef GUI_WIDGETS___LOADERS___FILE_LOAD_MRU_LIST__HPP
#define GUI_WIDGETS___LOADERS___FILE_LOAD_MRU_LIST__HPP

/*  $Id: file_load_mru_list.hpp 28938 2013-09-17 22:00:07Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <wx/string.h>

#include <gui/utils/mru_list.hpp>

BEGIN_NCBI_SCOPE

class CFileLoadDescriptor
{
public:
    CFileLoadDescriptor(
		            const wxString& fileName,
                    const wxString& fileLoaderLabel,
                    const string& fileLoaderId)
                    : m_FileName(fileName),
                      m_FileLoaderLabel(fileLoaderLabel),
                      m_FileLoaderId(fileLoaderId) {}

    bool operator==(const CFileLoadDescriptor& fd) 
    { 
        return ((m_FileName == fd.m_FileName) &&
                (m_FileLoaderId == fd.m_FileLoaderId));
    }

    wxString GetFileName() const { return m_FileName; }
    wxString GetFileLoaderLabel() const { return m_FileLoaderLabel; }
    string   GetFileLoaderId() const { return m_FileLoaderId; }
private:
    wxString m_FileName;
    wxString m_FileLoaderLabel;
    string   m_FileLoaderId;
};

class CFileLoadMRUList : public CTimeMRUList<CFileLoadDescriptor>
{
public:
	typedef CTimeMRUList<CFileLoadDescriptor> TParent;
	CFileLoadMRUList(int max_size = 10) : TParent(max_size) {}

	void LoadFromStrings(const vector<string>& values);
	void SaveToStrings(vector<string>& values) const;

private:
	string  m_RegPath;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS___LOADERS___FILE_LOAD_MRU_LIST__HPP

