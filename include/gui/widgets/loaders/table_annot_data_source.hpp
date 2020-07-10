#ifndef GUI_CORE___TABLE_ANNOT_DATA_SOURCE__HPP
#define GUI_CORE___TABLE_ANNOT_DATA_SOURCE__HPP

/*  $Id: table_annot_data_source.hpp 30332 2014-04-30 19:22:24Z falkrb $
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
 * Authors:  Bob Falk
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <util/icanceled.hpp>

#include <objects/seq/Seq_annot.hpp>
#include <objects/seq/Annotdesc.hpp>
#include <objects/seqloc/Na_strand_.hpp>

#include <gui/gui_export.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


///////////////////////////////////////////////////////////////////////////////
/// CAnnotDataSource - 
///
/// This class wraps a CSeq_annot that contains a CSeq_table.  The class 
/// has features for finding fields in the table that respresent locations
/// and then either adding those as location columns or converting the fields
/// as needed to create region fetures.
class  NCBI_GUIWIDGETS_LOADERS_EXPORT  CTableAnnotDataSource : public CObject
{
public:
    struct NCBI_GUIWIDGETS_LOADERS_EXPORT STableLocation {
        STableLocation()
            : m_IdCol(-1)
            , m_StartPosCol(-1)
            , m_StopPosCol(-1)
            , m_StrandCol(-1)
            , m_LengthCol(-1)
            , m_DataRegionCol(-1)
            , m_GenotypeCol(-1)
            , m_IsRsid(false) {}

        /// Some of these may be -1 (e.g. stop OR length, and data region)
        STableLocation(int col, int start, int stop, int length, int strand, int dr, int gt, bool rsid)
            : m_IdCol(col)
            , m_StartPosCol(start)
            , m_StopPosCol(stop)
            , m_StrandCol(strand)
            , m_LengthCol(length)
            , m_DataRegionCol(dr)
            , m_GenotypeCol(gt)
            , m_IsRsid(rsid) {}

        /// Return informational string as to which columns are selected
        string GetColumnInfo() const;

        int m_IdCol;
        int m_StartPosCol;
        int m_StopPosCol;
        int m_StrandCol;
        int m_LengthCol;
        // data region is needed for creating seq-features from table rows
        int m_DataRegionCol;
        // genotype column needed for creating snp features
        int m_GenotypeCol;
        // Snip/variation ids imply a specific location so will not need start/stop fields
        bool m_IsRsid;
    };

public:
    /// ctor
    CTableAnnotDataSource();

    /// clears all columns rows and delimiters
    void Clear();

    void Init() {}

 

    /// return total number of rows read
    size_t GetNumRows() const { return 0; }
    
    /// return a specific field from a specific row, based on current table type
    /// and delimiter
    string GetField(size_t row, size_t col) const;

    /// Find colums that can be combined to form locations based on column info
    /// If no locations are found, missing info is given in msg.
    vector<STableLocation> FindLocations(string& msg, bool strand_required);

    /// Add a location to the table based on the specified columns
    bool AddSeqLoc(const STableLocation& fc, int loc_number, ICanceled* cancel=NULL);

    /// Add a location to the table using the rsid (snp/variation) ids location from the snp db
    bool AddSnpSeqLoc(const STableLocation& fc, int loc_number, ICanceled* cancel=NULL);

    /// Create a feature using specified columns
    bool CreateFeature(const STableLocation& fc, ICanceled* cancel=NULL);

    /// Create a region feature for each snp using specified snp (rsid) column
    bool CreateSnpFeature(const STableLocation& fc, ICanceled* cancel=NULL);

    /// Create snips (features that show variations) for each row
    bool CreateSnps(const STableLocation& fc, ICanceled* cancel=NULL);

    /// Remove any seqloc columns (can be used to undo the AddSeqLoc actions)
    void RemoveSeqLocs();

    CRef<CSeq_annot> GetContainer() { return m_AnnotContainer; }

    /// Write seqtable to file 'fname'.  If write_extended_form is not
    /// true a seq-table will be written, otherwise a seq-annot with meta-
    /// information will be written with an embedded seq-table. 
    void WriteAsn(const string& fname);
 
protected:

    /// Get meta information stored in User_Data in m_AnnotContainer
    CAnnotdesc::TUser* x_GetColumnMetaInfo();

    /// Return enumerated strand type based on string
    ENa_strand x_GetStrand(string strand);

    /// Log an error (but stop logging if error count gets high
    void x_LogErr(const string& logstr, string& errstr, int& err_count, int row);

    /// Search string 'meta_string' for the value assigned to 'tag_name', e.g.
    /// GetMetaInfoTag("&genome_assembly=GRCh37.p5 &one_based=true",
    ///                "genome_assembly") == "GRCh37.p5"
    string x_GetMetaInfoTag(const string& meta_string, const string& tag_name);

    /// Change the value for 'tag_name' in 'meta_string' to 'new_value'. 
    /// returns true on change, false on not-found.
    bool x_UpdateMetaInfoTag(string& meta_string, 
                             const string& tag_name,
                             const string& new_value);

    CRef<CSeq_annot> m_AnnotContainer;
};

END_NCBI_SCOPE


#endif  // GUI_CORE___TABLE_ANNOT_DATA_SOURCE__HPP

