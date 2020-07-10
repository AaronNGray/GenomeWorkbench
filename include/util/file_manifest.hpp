#ifndef UTIL___FILE_MANIFEST_HPP__
#define UTIL___FILE_MANIFEST_HPP__

/*  $Id: file_manifest.hpp 574980 2018-11-21 14:24:48Z ucko $
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
 * Authors:  Cheinan Marks
 *
 * File Description:
 *   C++ class to handle GPipe manifest files.  These are action node
 *   newline-separated, one column file path manifests.  Do not confuse
 *   this manifest with the queue XML manifest in gpipe/objects/manifest.
 */


#include <corelib/ncbiobj.hpp>
#include <corelib/ncbiexpt.hpp>
#include <vector>
#include <string>
#include <iterator>


BEGIN_NCBI_SCOPE

class CArgValue;

class NCBI_XUTIL_EXPORT CFileManifest
{
public:
    CFileManifest( const string & manifest_path );

    /// Convenience constructor, to read a manifest from a command
    /// line argument's value. This avoids the user having to choose
    /// amongst AsString(), which is correct, and AsInputFile(), which
    /// isn't supported (as there is no constructor taking a stream
    /// as input).
    CFileManifest( const CArgValue& manifest_path );

    /// Check the manifest for consistency and open all the referenced files.
    /// WARNING:  This method throws an exception on failure!  Be prepared to
    /// catch it!
    /// Success is indicated by a return without an exception.
    void Validate() const;   /// throws CManifestException

    /// Returns the manifest file path, not the files referenced in the
    /// manifest.
    string GetPath() const { return m_ManifestPath; }

    /// Returns all the file paths referenced by the manifest.
    vector<string> GetAllFilePaths() const;

    /// Returns the first file path in the manifest.  Throws if there are more
    /// than one file.
    string GetSingleFilePath() const;

    /// Write a list of files to a manifest.  Will overwrite any previous data.
    void WriteManyFilePaths( const vector<string> & file_paths );

protected:
    void x_Init();

private:
    string m_ManifestPath;
};


class NCBI_XUTIL_EXPORT CManifestException : public CException
{
public:
    enum EErrCode {
        eEmptyManifestName
        , eCantOpenManifestForRead
        , eCantOpenManifestForWrite
        , eCantOpenFileInManifest
        , eMoreThanOneFile
        , eInvalidFileFormat
        , eInvalidFilePath
    };

    virtual const char* GetErrCodeString() const override
    {
        switch (GetErrCode()) {
            case eEmptyManifestName: 
                return "The manifest filename was empty.";
            case eCantOpenManifestForRead: 
                return "Unable to open the manifest for reading.";
            case eCantOpenManifestForWrite: 
                return "Unable to open the manifest for writing.";
            case eCantOpenFileInManifest: 
                return "Unable to open a file in the manifest.";
            case eInvalidFileFormat: 
                return "Invalid manifest format: must be 1 or 2 columns";
            case eInvalidFilePath: 
                return "Invalid file path: must not contain spaces, "
                       "quotes, or escapes";
            default:     
                return CException::GetErrCodeString();
        }
    }

    NCBI_EXCEPTION_DEFAULT( CManifestException, CException );
};


/// Iterator to walk the files in the manifest.  Ignores comment (start with #)
/// and blank lines.  Filters out the first column in two column manifests.
template <class TString>
class CBasicManifest_CI : public iterator<input_iterator_tag, TString, 
                                          ptrdiff_t, const TString *, 
                                          const TString &>
{
public:
    typedef typename TString::value_type        TChar;
    typedef typename TString::traits_type       TTraits;
    typedef basic_istream<TChar, TTraits>  TIStream;

    CBasicManifest_CI() : m_InputStream( NULL ) {}
    CBasicManifest_CI( TIStream & input_stream )
        : m_InputStream( &input_stream ),
          m_NextValue( x_GetNextValue() )
    {
        ++*this;
    }

    const TString& operator*() const
    {
        _ASSERT( m_InputStream );
        return m_Value;
    }
    const TString* operator->() const
    {
        _ASSERT( m_InputStream );
        return &m_Value;
    }

    CBasicManifest_CI<TString> & operator++()
    {
        _ASSERT( m_InputStream );
        if ( m_InputStream ) {
            m_Value = m_NextValue;

            if ( m_InputStream->fail() ) {
                m_InputStream = NULL;
            } else {
                m_NextValue = x_GetNextValue();
            }
        }

        return *this;
    }

    CBasicManifest_CI<TString> operator++( int )
    {
        CBasicManifest_CI<TString> prev( *this );
        ++*this;

        return prev;
    }

    bool operator!=( const CBasicManifest_CI<TString> & other ) const
    {
        return m_InputStream != other.m_InputStream;
    }

    bool operator==( const CBasicManifest_CI<TString> & other ) const
    {
        return !(*this != other);
    }

private:
    TIStream *  m_InputStream;
    TString     m_Value;
    TString     m_NextValue;

    string x_GetNextValue()
    {
        string next_value;

        do {
            getline( *m_InputStream, next_value );
        } while ( *m_InputStream && 
                  ( next_value.empty() || ( next_value[0] == '#' ) ) );

        //  Filter out the first column of two column manifests
        string::size_type  first_tab = next_value.find_first_of( '\t' );
        string::size_type  last_tab = next_value.find_last_of( '\t' );
        if (first_tab != last_tab) {
            string error_string = "More than 2 columns in: " + next_value;
            NCBI_THROW( CManifestException, eInvalidFileFormat, error_string );
        }
        if ( last_tab != string::npos ) {
            next_value.erase( 0, last_tab + 1 );
            LOG_POST( Warning << "Two column manifest was found." );
        }

        string::size_type  bad_char_pos = next_value.find_first_of(" '`\"\\");
        if ( bad_char_pos != string::npos ) {
            string error_string = "Invalid character at position " +
                                    NStr::NumericToString(bad_char_pos + 1) +
                                    ": " +
                                    next_value.substr(0, bad_char_pos) +
                                    ">>>" + next_value[bad_char_pos] + "<<<" +
                                    next_value.substr(bad_char_pos + 1);
            NCBI_THROW( CManifestException, eInvalidFilePath, error_string );
        }
        return  next_value;
    }
};

typedef CBasicManifest_CI<string> CManifest_CI;



END_NCBI_SCOPE

#endif  //  UTIL___FILE_MANIFEST_HPP__
