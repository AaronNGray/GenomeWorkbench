/*  $Id: table_annot_data_source.cpp 43695 2019-08-14 19:16:30Z katargir $
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
 * Authors: Bob Falk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/loaders/table_annot_data_source.hpp>
#include <gui/widgets/loaders/table_import_column.hpp>
#include <corelib/ncbistre.hpp>

#include <objects/seqtable/Seq_table.hpp>
#include <objects/seqtable/SeqTable_column.hpp>
#include <objects/seqtable/SeqTable_column_info.hpp>
#include <objects/seqtable/SeqTable_multi_data.hpp>
#include <objects/seqtable/CommonString_table.hpp>
#include <objects/seqtable/SeqTable_single_data.hpp>
#include <objects/seqtable/SeqTable_sparse_index.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_point.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/variation/Variation.hpp>
#include <objects/variation/VariantPlacement.hpp>
#include <objects/seqfeat/VariantProperties.hpp>
#include <objmgr/object_manager.hpp>

#include <objtools/readers/idmapper.hpp>
#include <gui/objutils/snp_gui.hpp>
#include <gui/objutils/gencoll_svc.hpp>


#include <math.h>
#include <sstream>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*****************************************************************************/
/*************************** CTableAnnotDataSource::STableLocation ***********/

string CTableAnnotDataSource::STableLocation::GetColumnInfo() const
{
    string colinfo;

    if (m_IdCol == -1)
        colinfo += "ID: missing ";
    else
        colinfo += "ID: " + NStr::NumericToString(m_IdCol);

    if (m_StartPosCol != -1)
        colinfo += " Start Pos: " + NStr::NumericToString(m_StartPosCol);

    if (m_StopPosCol != -1)
        colinfo += " Stop Pos: " + NStr::NumericToString(m_StopPosCol);

    if (m_LengthCol != -1)
        colinfo += " Length: " + NStr::NumericToString(m_LengthCol);

    if (m_StrandCol != -1)
        colinfo += " Strand: " + NStr::NumericToString(m_StrandCol);

    if (m_GenotypeCol != -1)
        colinfo += " Genotype: " + NStr::NumericToString(m_GenotypeCol);

    if (m_DataRegionCol != -1)
        colinfo += " Data Region: " + NStr::NumericToString(m_DataRegionCol);

    return colinfo;
}

/*****************************************************************************/
/*************************** CTableAnnotDataSource **************************/


CTableAnnotDataSource::CTableAnnotDataSource()
{
    m_AnnotContainer.Reset(new CSeq_annot());
}

void CTableAnnotDataSource::Clear()
{
}


string CTableAnnotDataSource::GetField(size_t row, size_t col) const
{
    return "";
}

vector<CTableAnnotDataSource::STableLocation>
CTableAnnotDataSource::FindLocations(string& msg, bool strand_required)
{
    vector<STableLocation> locations;

    msg = "";
    bool missing_assembly = false;
    bool is_rsid = false;

    // Since there may be multiple locations on a single
    // row, we need to have rules for when one location ends and
    // another one starts.  Since we want to allow rows with a single
    // id (and possibly strand) and multiple start/stop/length's, we
    // decide that a new location should be saved whenever:
    // 1. a new id is encountered
    // 2. a new start position is encountered
    // 3. The last column is reached
    if (!m_AnnotContainer->IsSetData()) {
        msg = "SeqTable not found in SeqAnnot";
        LOG_POST(Error << msg);
        return locations;
    }
    CSeq_annot::TData& table_container = m_AnnotContainer->SetData();

    if (!table_container.IsSeq_table()) {
        msg = "SeqAnnot does not contain a SeqTable";
        LOG_POST(Error << msg);
        return locations;
    }
    CSeq_table& table = table_container.SetSeq_table();
    CSeq_table::TColumns& cols = table.SetColumns();

    CAnnotdesc::TUser* column_meta_info = x_GetColumnMetaInfo();
    if (column_meta_info == NULL)
        return locations;

    bool prev_is_rsid = false;
    int prev_id_idx = -1;
    int prev_start_idx = -1;
    int prev_end_idx = -1;
    int prev_strand_idx = -1;
    int prev_length_idx = -1;
    int prev_data_region_idx = -1;
    int prev_genotype_idx = -1;

    // Iterate over columns looking for possible locations
    for (size_t idx=0; idx<cols.size(); ++idx) {

        CSeqTable_column_info& header = cols[idx]->SetHeader();

        string field_name;
        string assembly;
        CTableImportColumn::eDataType col_type = CTableImportColumn::eUndefined;

        if (idx < column_meta_info->GetData().size()) {
            const CUser_field& field_meta_info = column_meta_info->GetData()[idx].GetObject();
            field_name = x_GetMetaInfoTag(field_meta_info.GetData().GetStr(), "xtype");
            assembly = x_GetMetaInfoTag(field_meta_info.GetData().GetStr(),
                "genome_assembly");
        }

        if (header.GetField_id() == CSeqTable_column_info::eField_id_location ||
            header.GetField_id() == CSeqTable_column_info::eField_id_location_id ||
            header.GetField_id() == CSeqTable_column_info::eField_id_location_gi ||
            header.GetField_id() == CSeqTable_column_info::eField_id_id_local) {
                // Chromosomes need an assembly otherwise we can't use them
                if (CTableImportColumn::GetDataTypeFromString(field_name) ==
                    CTableImportColumn::eChromosome) {
                        if (assembly != "") {
                            col_type = CTableImportColumn::eUnspecifiedID;
                        }
                        else {
                            msg = "Assembly not provided for chromosome";
                            LOG_POST(Error << msg);
                            // If there is no assembly, set the type to a non-id type so that it is not
                            // seen as an id field when/if the table is converted to a feature table.
                            //header.SetField_id(CSeqTable_column_info::eField_id_comment);
                            missing_assembly = true;
                        }
                }
                else {
                    // acccept any other kind of id here, so just call it unspecified
                    col_type = CTableImportColumn::eUnspecifiedID;
                }

                // SNP/Variation ids have locations that we will look up rather than using
                // the value in the table
                is_rsid = false;
                const CUser_field& id_meta_info = column_meta_info->GetData()[idx].GetObject();
                if (id_meta_info.IsSetData() && id_meta_info.GetData().IsStr())
                    is_rsid = x_GetMetaInfoTag(id_meta_info.GetData().GetStr(), "xtype") == "Rsid";
        }
        else if (header.GetField_id() == CSeqTable_column_info::eField_id_data_region) {
                col_type = CTableImportColumn::eDataRegion;
        }
        else if (CTableImportColumn::GetDataTypeFromString(field_name) ==
                 CTableImportColumn::eStartPosition) {
            col_type = CTableImportColumn::eStartPosition;
        }
        else if (CTableImportColumn::GetDataTypeFromString(field_name) ==
                 CTableImportColumn::eStopPosition) {
            col_type = CTableImportColumn::eStopPosition;
        }
        else if (CTableImportColumn::GetDataTypeFromString(field_name) ==
                 CTableImportColumn::eLength) {
            col_type = CTableImportColumn::eLength;
        }
        else if (CTableImportColumn::GetDataTypeFromString(field_name) ==
                 CTableImportColumn::eStrand) {
            col_type = CTableImportColumn::eStrand;
        }
        else if (CTableImportColumn::GetDataTypeFromString(field_name) ==
                 CTableImportColumn::eGenotype) {
            col_type = CTableImportColumn::eGenotype;
        }

        // Is the current set of fields sufficient to create a
        // location?  Since there may be multiple locations, new ones
        // are created when a new start position or ID column is
        // encountered (or after the last column, handled at function end)
        // Note that rsids (snips) have implied locations (we can look it up)
        // so that other fields are not needed
        if ((col_type == CTableImportColumn::eUnspecifiedID ||
             col_type == CTableImportColumn::eStartPosition) &&
            (prev_id_idx >= 0 && (prev_is_rsid ||   // we have an id
            (prev_start_idx > prev_id_idx &&   // we have a start position
            (prev_end_idx > prev_start_idx || // we have an end position or length
             prev_length_idx > prev_start_idx) &&
            (prev_strand_idx > prev_id_idx || // we have a strand if required
             strand_required == false))) )) {

                // The strand may be -1 and either length OR stop position
                // is not negative 1.   Dataregion may also be -1 (undefined)
                STableLocation fc(prev_id_idx, prev_start_idx, prev_end_idx,
                    prev_length_idx, prev_strand_idx, prev_data_region_idx, prev_genotype_idx, prev_is_rsid);
                locations.push_back(fc);

                // Before a new location is created, a new start position
                // and stop position OR length must be found.  The ID and
                // strand may be reused.
                //prev_id_idx = -1;
                //prev_strand_idx = -1;
                prev_start_idx = -1;
                prev_end_idx = -1;
                prev_length_idx = -1;
                prev_data_region_idx = -1;
                prev_genotype_idx = -1;
        }

        if (col_type == CTableImportColumn::eUnspecifiedID) {
            prev_id_idx = (int)idx;
            prev_is_rsid = is_rsid;
        }
        else if (col_type == CTableImportColumn::eStartPosition) {
            prev_start_idx = (int)idx;
        }
        else if (col_type == CTableImportColumn::eStopPosition) {
            prev_end_idx = (int)idx;
        }
        else if (col_type == CTableImportColumn::eLength) {
            prev_length_idx = (int)idx;
        }
        else if (col_type == CTableImportColumn::eStrand) {
            // If there are multiple locs they can use a strand field before
            // the id or start position, but prioritize the closest strand after
            // the current start position. (every loc has a new start position)
            if (prev_strand_idx == -1 || prev_strand_idx < prev_start_idx)
                prev_strand_idx = (int)idx;
        }
        else if (col_type == CTableImportColumn::eDataRegion) {
            prev_data_region_idx = (int)idx;
        }
        else if (col_type == CTableImportColumn::eGenotype) {
            prev_genotype_idx = (int)idx;
        }
    }

    // We are at the end of the columns.  Can we form another location
    // with the data we have?:
    if ((prev_id_idx >= 0) && (prev_is_rsid ||
       (prev_start_idx > prev_id_idx &&
        (prev_end_idx > prev_start_idx || prev_length_idx > prev_id_idx) &&
        (prev_strand_idx > prev_id_idx || strand_required == false))) ) {
            // The strand may be -1 and either length OR stop position
            // is not negative 1.
            STableLocation fc(prev_id_idx, prev_start_idx, prev_end_idx,
                prev_length_idx, prev_strand_idx, prev_data_region_idx, prev_genotype_idx, prev_is_rsid);
            locations.push_back(fc);
    }

    // If no locations are found, return a user-friendly message as to why.
    if (locations.size() == 0)  {
        if (prev_id_idx == -1) {
            if (missing_assembly)
                msg = "Assembly not provided for chromosome";
            else
                msg = "Missing ID column";
        }
        else {
            if (prev_start_idx == -1)
                msg = "Start Position column not defined";

            if (prev_end_idx == -1 && prev_length_idx == -1) {
                if (msg != "")
                    msg += " and neither a Stop Position nor Length column was given";
                else
                    msg = "Neither a Stop Position nor Length column was defined";
            }

            if (strand_required && prev_strand_idx == -1) {
                if (msg == "") {
                    msg = "Strand column not defined";
                }
                else {
                    msg += " Also, no Strand column was identified.";
                }
            }
        }
    }

    return locations;
}

void CTableAnnotDataSource::x_LogErr(const string& logstr, string& errstr, int& err_count, int row)
{
    if (errstr != "")
        errstr += " | ";
    errstr += logstr;
    if (++err_count < 100) {
        LOG_POST("Invalid row " << row+1 << " " << logstr);
    }
}

struct CompareSNPResults {
    CompareSNPResults(const string& rsid) : m_Rsid(rsid) {}

    bool operator()(const  NSNPWebServices::TSNPSearchCompoundResult& rhs) const { return m_Rsid == rhs.first; }

    string m_Rsid;
};

bool CTableAnnotDataSource::AddSeqLoc(const STableLocation& fc,
                                      int loc_number,
                                      ICanceled* call)
{
    // Get User data from Annot Descriptor this is the Properties data
    // structure, where each entry is a combination of column number
    // and field properties, e.g.
    // Column.1  &xtype=StartPosition &one_based=true
    CAnnotdesc::TUser* column_meta_info = x_GetColumnMetaInfo();
    if (column_meta_info == NULL)
        return false;

    // Now get the actual columns (which have headers with descriptive
    // info as well - as much as we currently need for finding features
    // actually)
    if (!m_AnnotContainer->IsSetData()) {
        LOG_POST("Error - SeqTable not found in SeqAnnot");
        return false;
    }
    CSeq_annot::TData& table_container = m_AnnotContainer->SetData();

    if (!table_container.IsSeq_table()) {
        LOG_POST("Error - SeqAnnot does not contain a SeqTable");
        return false;
    }
    CSeq_table& table = table_container.SetSeq_table();
    CSeq_table::TColumns& cols = table.SetColumns();

    const CUser_field& id_meta_info = column_meta_info->GetData()[fc.m_IdCol].GetObject();

    bool start_one_based = true;
    const CUser_field& start_meta_info = column_meta_info->GetData()[fc.m_StartPosCol].GetObject();
    if (start_meta_info.IsSetData() && start_meta_info.GetData().IsStr())
        start_one_based = x_GetMetaInfoTag(start_meta_info.GetData().GetStr(), "one_based") == "true";

    bool stop_one_based = true;
    if (fc.m_StopPosCol >= 0) {
        const CUser_field& stop_meta_info = column_meta_info->GetData()[fc.m_StopPosCol].GetObject();
        if (stop_meta_info.IsSetData() && stop_meta_info.GetData().IsStr())
            stop_one_based = x_GetMetaInfoTag(stop_meta_info.GetData().GetStr(), "one_based") == "true";
    }
    else {
        // we are using length instead of a 'to' position.  Stop will be
        // start + len, so if start is one-based, stop will be to.
        stop_one_based = start_one_based;
    }

    //
    /// Chromosomes need to be re-mapped using the assembly
    CIdMapper* mapper = NULL;
    string id_meta_info_str;

    bool is_chromosome = false;
    if (id_meta_info.IsSetData() && id_meta_info.GetData().IsStr())
        id_meta_info_str = id_meta_info.GetData().GetStr();

    string column_type = x_GetMetaInfoTag(id_meta_info_str, "xtype");
    if (CTableImportColumn::GetDataTypeFromString(column_type) ==
        CTableImportColumn::eChromosome) {
        string assembly;
        string meta_info;

        assembly = x_GetMetaInfoTag(id_meta_info_str, "genome_assembly");

        if (!assembly.empty() ) {
            // used for initializing CIdMapperGCAssembly; having a full assembly seems to be inevitable here
            CRef<CGC_Assembly> assm = CGencollSvc::GetInstance()->GetGCAssembly(assembly, true, "Gbench");
            if (assm) {
                CRef<CObjectManager> obj_mgr = CObjectManager::GetInstance();
                CRef<CScope> scope(new CScope(*obj_mgr));
                scope->AddDefaults();
                CIdMapperGCAssembly::EAliasMapping alias =
                    assm->IsRefSeq() ? CIdMapperGCAssembly::eRefSeqAcc : CIdMapperGCAssembly::eGenBankAcc;
                mapper = new CIdMapperGCAssembly(*scope, *assm, alias);
                is_chromosome = true;
            }
        }
    }

    //
    //  Add a new column for the SeqLoc
    CRef<CSeqTable_column_info> cinfo((new CSeqTable_column_info()));

    string label = "Column." + NStr::NumericToString(cols.size() + 1);
    string properties = string("&xtype=SeqLoc") + " &derived_field=true";
    column_meta_info->AddField(label, properties);

    cinfo->SetField_id(CSeqTable_column_info_Base::eField_id_location);
    cinfo->SetTitle(string("Location ") + NStr::NumericToString(loc_number));

    CRef< CSeqTable_column > column(new CSeqTable_column());
    column->SetHeader(*cinfo);
    CRef<CSeqTable_multi_data> data(new CSeqTable_multi_data());
    data->Select(CSeqTable_multi_data_Base::e_Loc);

    // If there are errors we can put them in an error column
    // at the end of conversion, if no errors have occured, the column
    // will not be added to the table. Columns with errors will also
    // be marked as disabled to keep the table valid.
    CRef< CSeqTable_column > error_column;
    {
        CRef<CSeqTable_column_info> cinfo(new CSeqTable_column_info());
        cinfo->SetTitle("Error Messages");
        cinfo->SetField_id(CSeqTable_column_info_Base::eField_id_comment);

        error_column.Reset(new CSeqTable_column());
        error_column->SetHeader(*cinfo);
        CRef<CSeqTable_multi_data> data(new CSeqTable_multi_data());
        data->Select(CSeqTable_multi_data_Base::e_String);
        error_column->SetData(data.GetNCObject());

        // I think our table viewer is not handling sparse data...
        //CSeqTable_sparse_index* si = new CSeqTable_sparse_index();
        //si->Select(CSeqTable_sparse_index_Base::e_Indexes);
        //error_column->SetSparse(*si);
    }

    // if call is asynchronous, return if/when cancelled.
    if (call != NULL &&
        call->IsCanceled())
        return false;

    // Iterate over all rows and attempt to add a loccation for each row.  If there
    // is an error, add a null location for that row.
    int err_count = 0;
    for (size_t row=0; row<cols[fc.m_IdCol]->GetData().GetSize(); ++row) {
        int from = 0;
        int to = 0;

        bool null_seqloc = false;
        string logstr;
        string err_str;

        CRef<CSeq_loc> location( new CSeq_loc );
        if (!cols[fc.m_StartPosCol]->TryGetInt(row, from)) {
            logstr = "cannot convert start position to an integer";
            err_str = logstr;
            if (++err_count < 100) {
                LOG_POST("Invalid row " << row+1 << " " << logstr);
            }
            null_seqloc = true;
        }
        else {
            if (start_one_based) {
                if (from < 1) {
                    null_seqloc = true;
                    logstr = "start column is one-based but value is < 1";
                    if (err_str != "")
                        err_str += " | ";
                    err_str += logstr;
                    if (++err_count < 100) {
                        LOG_POST("Invalid row " << row+1 << " " << logstr);
                    }
                }
                else {
                    from -= 1;
                    cols[fc.m_StartPosCol]->SetData().SetInt()[row] = from;
                }
            }
            else if (from < 0) {
                logstr = "start column is < 0";
                if (err_str != "")
                    err_str += " | ";
                err_str += logstr;
                null_seqloc = true;
                if (++err_count < 100) {
                    LOG_POST("Invalid row " << row+1 << " " << logstr);
                }
            }
        }

        if (fc.m_StopPosCol >= 0) {
            if (!cols[fc.m_StopPosCol]->TryGetInt(row, to)) {
                logstr = "cannot convert stop position to an integer";
                if (err_str != "")
                    err_str += " | ";
                err_str += logstr;
                if (++err_count < 100) {
                    LOG_POST("Invalid row " << row+1 << " " << logstr);
                }
                null_seqloc = true;
            }
            else {
                if (stop_one_based) {
                    if (to < 1) {
                        logstr = "stop column is one-based but value is < 1";
                    if (err_str != "")
                        err_str += " | ";
                    err_str += logstr;
                        null_seqloc = true;

                        if (++err_count < 100) {
                            LOG_POST("Invalid row " << row+1 << " " << logstr);
                        }
                    }
                    else {
                        to -= 1;
                        cols[fc.m_StopPosCol]->SetData().SetInt()[row] = to;
                    }
                }
                else if (to < 0) {
                    logstr = "stop column is < 0";
                    if (err_str != "")
                        err_str += " | ";
                    err_str += logstr;
                    null_seqloc = true;

                    if (++err_count < 100) {
                        LOG_POST("Invalid row " << row+1 << " " << logstr);
                    }
                }
            }
        }
        else if (fc.m_LengthCol >= 0) {
            int len = 0;
            if (!cols[fc.m_LengthCol]->TryGetInt(row, len)) {
                logstr = "cannot convert length to an integer";
                if (err_str != "")
                    err_str += " | ";
                err_str += logstr;
                if (++err_count < 100) {
                    LOG_POST("Invalid row " << row+1 << " " << logstr);
                }
                null_seqloc = true;
            }
            to = from + len;
        }

        ENa_strand strand_e = eNa_strand_unknown;

        if (fc.m_StrandCol != -1) {
            if (cols[fc.m_StrandCol]->GetData().IsString()) {
                string strand = *cols[fc.m_StrandCol]->GetStringPtr(row);
                strand_e = x_GetStrand(strand);

                // Invalid strand string - log an error and set the strand type based
                // on the to and from positions.
                if (strand_e == eNa_strand_unknown) {
                    if (from > to) {
                        strand_e = eNa_strand_minus;
                        std::swap(from, to);
                    }
                    else {
                        strand_e = eNa_strand_plus;
                    }

                    if (++err_count < 100) {
                        logstr = " strand identifier: " + strand +
                            " must be +,-,pos, or neg";
                        if (err_str != "")
                            err_str += " | ";
                        err_str += logstr;
                        LOG_POST("Invalid row " << row+1 << " " << logstr);
                    }
                }
            }
            else if (cols[fc.m_StrandCol]->GetData().IsInt()) {
                int strand_int;
                if (cols[fc.m_StrandCol]->TryGetInt(row, strand_int)) {
                    // As an integer, strand must match values defined for ENa_strand:
                    if ((strand_int >= 0 && strand_int <= 4) || strand_int==255) {
                        strand_e = ENa_strand(strand_int);
                    }
                    else {
                        // Invalid strand number - guess strand based on
                        // relative start and stop positions.
                        if (from > to) {
                            strand_e = eNa_strand_minus;
                            std::swap(from, to);
                        }
                        else {
                            strand_e = eNa_strand_plus;
                        }

                        if (++err_count < 100) {
                            logstr = "Integer strand value: " + NStr::IntToString(strand_int) +
                                " not valid";
                            if (err_str != "")
                                err_str += " | ";
                            err_str += logstr;
                            LOG_POST(logstr << " in row: " << row+1);
                        }
                    }

                }
                else {
                    null_seqloc = true;
                    logstr = "Unable to retrieve strand value";
                    if (err_str != "")
                        err_str += " | ";
                    err_str += logstr;
                    if (++err_count < 100)
                        LOG_POST(logstr << " in row: " << row+1);
                }
            }
        }

        /// From and to are the same - single position
        if (from == to) {
            location->SetPnt().SetPoint(from);
        }
        else {
            location->SetInt().SetFrom(from);
            location->SetInt().SetTo(to);
        }


        // Set the strand automatically based on whether to position > from position
        if (fc.m_StrandCol == -1) {
            strand_e = (to >= from) ? eNa_strand_plus : eNa_strand_minus;

            // from and to on minus strand should still have (to < from)
            if (strand_e == eNa_strand_minus) {
                location->SetInt().SetFrom(to);
                location->SetInt().SetTo(from);
            }
        }

        location->SetStrand(strand_e);

        // Create the id - could be chromosome that needs mapping, an ID or a GI.  If
        // it is a GI it may be saved in a column of type int instead of ID.
        if (null_seqloc) {
            location->SetNull();
        }
        else if (!is_chromosome) {
            CConstRef<CSeq_id>  seq_id;

            if (cols[fc.m_IdCol]->GetData().IsInt()) {
                int gid = cols[fc.m_IdCol]->GetData().GetInt()[row];

                try {
                    seq_id.Reset(new CSeq_id(CSeq_id_Base::e_Gi, gid));
                    location->SetId(seq_id.GetObject());
                }
                catch(CException& ex) {
                    null_seqloc = true;
                    logstr = "Error constructing seq-id from GI " +
                             NStr::IntToString(gid) + " | " + ex.GetMsg();
                    if (err_str != "")
                        err_str += " | ";
                    err_str += logstr;
                    if (++err_count < 100) {
                        LOG_POST(logstr << " in row: " << row+1);
                    }
                    location->SetNull();
                }
            }
            else {
                try {
                    seq_id = cols[fc.m_IdCol]->GetSeq_id(row);
                    if (seq_id) {
                        location->SetId(seq_id.GetObject());
                    }
                }
                catch(CException& e) {
                    null_seqloc = true;
                    logstr = "Exception getting seq-id " + e.GetMsg();
                    if (err_str != "")
                        err_str += " | ";
                    err_str += logstr;
                    if (++err_count < 100)
                        LOG_POST(logstr << " in row: " << row+1);
                    location->SetNull();
                }
            }
        }
        else {
            CRef<CSeq_id>  seq_id;

            string idstr = *cols[fc.m_IdCol]->GetStringPtr(row);

            // Mitochondrial - MT??
            size_t idx = idstr.find_first_of("0123456789XYxy");
            if (idx != string::npos) {
                size_t chr_len = idstr.size()-idx;

                size_t end_idx = idstr.find_first_not_of("0123456789XYxy", idx);
                if (end_idx != string::npos)
                    chr_len = end_idx-idx;

                string chr_num = idstr.substr(idx, chr_len);
                try {
                    // seq-id starts as local since I don't know how else we could
                    // set it using a chromosome number
                    seq_id.Reset(new CSeq_id(CSeq_id_Base::e_Local, chr_num));
                    CSeq_id_Handle handle = mapper->Map(CSeq_id_Handle::GetHandle(*seq_id));

                    if (handle.IsGi()) {
                        TGi gi = handle.GetGi();
                        seq_id->SetGi(gi);
                        //seq_id.Reset(new CSeq_id(CSeq_id_Base::e_Gi, gi));
                    }
                    else if (!handle.GetSeqId().IsNull()) {
                        CConstRef<CSeq_id> cseq_id = handle.GetSeqId();

                        // This is roundabout (seq-id to string to seq-id) but
                        // it works
                        string seqid_str = cseq_id->GetSeqIdString();
                        //seq_id.Reset(new CSeq_id(seqid_str));
                        seq_id->Set(seqid_str);
                    }
                    else {
                        // Error - can't create an id - a null one will be saved
                        null_seqloc = true;
                        logstr = "Unable to generate seq-id from id: " + idstr;
                        if (err_str != "")
                            err_str += " | ";
                        err_str += logstr;
                        if (++err_count < 100) {
                            LOG_POST("Invalid row " << row+1 << " " << logstr);
                        }
                    }
                }
                catch(CStringException&) {
                    // Parse error
                    null_seqloc = true;
                    logstr = "Unable to parse valid chromosome number from string: " + idstr;
                    if (err_str != "")
                        err_str += " | ";
                    err_str += logstr;
                    if (++err_count < 100) {
                        LOG_POST("Invalid row " << row+1 << " " << logstr);
                    }
                }
                catch (CException& ex) {
                    // Error (e.g. objmgr):
                    null_seqloc = true;
                    logstr = "Error creating id for: " + idstr + " : " + ex.GetMsg();
                    if (err_str != "")
                        err_str += " | ";
                    err_str += logstr;
                    if (++err_count < 100) {
                        LOG_POST("Invalid row " << row+1 << " " << logstr);
                    }
                }
            }

            if (seq_id && !null_seqloc) {
                location->SetId(seq_id.GetObject());
            }
            else {
                // Any errors with seq-id result in the null location being
                // created and added here.
                location->SetNull();
            }
        }

        // if call is asynchronous, return if/when cancelled.
        if (call != NULL &&
            call->IsCanceled())
            return false;

        data->SetLoc().push_back(location);

        // Add error (or blank if no error for this row)
        error_column->SetData().SetString().push_back(err_str);
    }

    delete mapper;

    column->SetData(*data);
    table.SetColumns().push_back(column);

    if (err_count > 0) {
        table.SetColumns().push_back(error_column);
        string label = error_column->GetHeader().GetTitle();
        string properties = string("&xtype=") +
            CTableImportColumn::GetStringFromDataType(CTableImportColumn::eUnspecifiedText) +
            " &derived_field=true";
        column_meta_info->AddField(label, properties);
    }

    return true;
}

bool CTableAnnotDataSource::AddSnpSeqLoc(const STableLocation& fc,
                                         int loc_number,
                                         ICanceled* call)
{
    // Get User data from Annot Descriptor this is the Properties data
    // structure, where each entry is a combination of column number
    // and field properties, e.g.
    // Column.1  &xtype=StartPosition &one_based=true
    CAnnotdesc::TUser* column_meta_info = x_GetColumnMetaInfo();
    if (column_meta_info == NULL)
        return false;

    // Now get the actual columns (which have headers with descriptive
    // info as well - as much as we currently need for finding features
    // actually)
    if (!m_AnnotContainer->IsSetData()) {
        LOG_POST("Error - SeqTable not found in SeqAnnot");
        return false;
    }
    CSeq_annot::TData& table_container = m_AnnotContainer->SetData();

    if (!table_container.IsSeq_table()) {
        LOG_POST("Error - SeqAnnot does not contain a SeqTable");
        return false;
    }
    CSeq_table& table = table_container.SetSeq_table();
    CSeq_table::TColumns& cols = table.SetColumns();

    const CUser_field& id_meta_info = column_meta_info->GetData()[fc.m_IdCol].GetObject();

    //
    /// Chromosomes need to be re-mapped using the assembly
    //CIdMapperConfig* mapper = NULL;
    string id_meta_info_str;

    bool has_assembly = false;
    string assembly;

    if (id_meta_info.IsSetData() && id_meta_info.GetData().IsStr())
        id_meta_info_str = id_meta_info.GetData().GetStr();

    string column_type = x_GetMetaInfoTag(id_meta_info_str, "xtype");
    if (CTableImportColumn::GetDataTypeFromString(column_type) ==
        CTableImportColumn::eRsid) {
        string meta_info;

        assembly = x_GetMetaInfoTag(id_meta_info_str, "genome_assembly");

        if (!assembly.empty() ) {
            // Don't need the assembly mapper since we are just going to pass the assembly name to
            // the SNP search function
            has_assembly = true;
        }
    }

    //
    //  Add a new column for the SeqLoc
    CRef<CSeqTable_column_info> cinfo((new CSeqTable_column_info()));

    string label = "Column." + NStr::NumericToString(cols.size() + 1);
    string properties = string("&xtype=SeqLoc") + " &derived_field=true";
    column_meta_info->AddField(label, properties);

    cinfo->SetField_id(CSeqTable_column_info_Base::eField_id_location);
    cinfo->SetTitle(string("Location ") + NStr::NumericToString(loc_number));

    CRef< CSeqTable_column > column(new CSeqTable_column());
    column->SetHeader(*cinfo);
    CRef<CSeqTable_multi_data> data(new CSeqTable_multi_data());
    data->Select(CSeqTable_multi_data_Base::e_Loc);

    // If there are errors we can put them in an error column
    // at the end of conversion, if no errors have occured, the column
    // will not be added to the table. Columns with errors will also
    // be marked as disabled to keep the table valid.
    CRef< CSeqTable_column > error_column;
    {
        CRef<CSeqTable_column_info> cinfo(new CSeqTable_column_info());
        cinfo->SetTitle("Error Messages");
        cinfo->SetField_id(CSeqTable_column_info_Base::eField_id_comment);

        error_column.Reset(new CSeqTable_column());
        error_column->SetHeader(*cinfo);
        CRef<CSeqTable_multi_data> data(new CSeqTable_multi_data());
        data->Select(CSeqTable_multi_data_Base::e_String);
        error_column->SetData(data.GetNCObject());
    }

    // if call is asynchronous, return if/when cancelled.
    if (call != NULL &&
        call->IsCanceled())
        return false;

    size_t rsid_search_size = 100;

    // Visit all rows in the table and add any columns needed to create a
    // complete feature.  If there are no columns to add/update, skip
    // this step.
    int err_count = 0;

    CRef<CObjectManager> om(CObjectManager::GetInstance());
    CRef<CScope> scope(new CScope(*om));
    scope->AddDefaults();

    size_t row = 0;
    size_t search_row = 0;
    size_t current_size = 0;
    size_t row_count = cols[fc.m_IdCol]->GetData().GetSize();
    while (search_row < row_count) {

        // could resize rsids to begin with (rsids have predictable lengths...)
        string rsids = "";
        for (current_size=0; search_row < row_count && current_size<rsid_search_size; ++current_size, ++search_row) {
            rsids += *cols[fc.m_IdCol]->GetStringPtr(search_row);
            rsids += ",";
        }
        // remove last comma
        rsids = rsids.substr(0, rsids.length()-1);

        // Query for current set of rsids
        NSNPWebServices::TSNPSearchCompoundResultList SNPSearchResultList;
        try {
            // assembly may be blank.
            NSNPWebServices::Search(rsids, assembly, SNPSearchResultList);
        }
        catch (CException& e) {
            LOG_POST("Exception while searching SNP database: " + e.GetMsg());
            // If there is an error getting a group of ids continually reduce the batch
            // size until there is only 1 id retrieved and then if there is still an
            // error process it as an error
            if (rsid_search_size > 2) {
                search_row = row;
                rsid_search_size = rsid_search_size/2;
                continue;
            }
        }
        catch(...) {
            LOG_POST("Unidentified exception while searching SNP database");
            // If there is an error getting a group of ids continually reduce the batch
            // size until there is only 1 id retrieved and then if there is still an
            // error process it as an error
            if (rsid_search_size > 2) {
                search_row = row;
                rsid_search_size = rsid_search_size/2;
                continue;
            }
        }

        // process the current set of rows. Results in result list should be in same order
        // as the rows, but we will search the result set if there is a mismatch.
        NSNPWebServices::TSNPSearchCompoundResultList::iterator current_iter = SNPSearchResultList.begin();

        for (; row < search_row; ++row) {

            string logstr;
            string errstr = "";

            CRef<CSeq_loc> location( new CSeq_loc );

            bool found = false;
            string rsid = *cols[fc.m_IdCol]->GetStringPtr(row);

            // See if current snp query result matches current row. If not, search for current
            // row rsid in entire result set. (usually they should match)
            NSNPWebServices::TSNPSearchCompoundResultList::iterator iter = current_iter;
            ++current_iter;

            if (iter->first != rsid) {
                CompareSNPResults pred(rsid);
                iter = std::find_if(SNPSearchResultList.begin(), SNPSearchResultList.end(), pred);
            }

            if (iter == SNPSearchResultList.end()) {
                x_LogErr("No matches for given rsid", errstr, err_count, row);
            }
            else if ((*iter).second.size() == 0) {
                x_LogErr("No matches for rsid and assembly: " + assembly,
                    errstr, err_count, row);
            }
            else {
                const NSNPWebServices::TSNPSearchResultList::value_type& first_result = (*iter).second.front();

                // CVariation used as a search result can have one and only one placement
                if (first_result->CanGetPlacements() == 0) {
                    x_LogErr("Unexpected absence of placements in SNP Search Result", errstr, err_count, row);
                }
                else {
                    const CVariation::TPlacements& placements(first_result->GetPlacements());
                    if (placements.size() == 0) {
                        x_LogErr("Unexpected number of placements (0) in SNP Search Result", errstr, err_count, row);
                    }
                    else {
                        location->Assign(placements.front()->GetLoc());

                        found = true;
                    }
                }
            }


            if (!found || errstr != "") {
                location->SetNull();
            }


            data->SetLoc().push_back(location);

            // Will add blanks too for cols w/o errors
            error_column->SetData().SetString().push_back(errstr);

            // if call is asynchronous, return if/when cancelled.
            if (call != NULL &&
                call->IsCanceled())
                return false;
        }
    }

    column->SetData(*data);
    table.SetColumns().push_back(column);

    if (err_count > 0) {
        table.SetColumns().push_back(error_column);
        string label = error_column->GetHeader().GetTitle();
        string properties = string("&xtype=") +
            CTableImportColumn::GetStringFromDataType(CTableImportColumn::eUnspecifiedText) +
            " &derived_field=true";
        column_meta_info->AddField(label, properties);
    }

    return true;
}

bool CTableAnnotDataSource::CreateFeature(const STableLocation& fc,
                                          ICanceled* call)
{

    // Get User data from Annot Descriptor this is the Properties data
    // structure, where each entry is a combination of column number
    // and field properties, e.g.
    // Column.1  &xtype=StartPosition &one_based=true
    CAnnotdesc::TUser* column_meta_info = x_GetColumnMetaInfo();
    if (column_meta_info == NULL)
        return false;

    // Now get the actual columns (which have headers with descriptive
    // info as well - as much as we currently need for finding features
    // actually)
    if (!m_AnnotContainer->IsSetData()) {
        LOG_POST("Error - SeqTable not found in SeqAnnot");
        return false;
    }
    CSeq_annot::TData& table_container = m_AnnotContainer->SetData();

    if (!table_container.IsSeq_table()) {
        LOG_POST("Error - SeqAnnot does not contain a SeqTable");
        return false;
    }
    CSeq_table& table = table_container.SetSeq_table();
    CSeq_table::TColumns& cols = table.SetColumns();

    /// To make the table a valid source for features first need to set its type
    /// and subtype to indicate that the rows represent region features.
    table.SetFeat_type(CSeqFeatData_Base::e_Region);
    table.SetFeat_subtype(CSeqFeatData::eSubtype_region);

    const CSeqTable_column_info& id_header = cols[fc.m_IdCol]->GetHeader();

    // Can only have one ID field if the table is a feature table -search for any others here
    // and switch them to comment fields.  Also, no comment fields may be of type int or real.
    for (size_t col_num=0; col_num<cols.size(); ++col_num) {
        if (col_num != fc.m_IdCol &&
            col_num != fc.m_LengthCol &&
            col_num != fc.m_StartPosCol &&
            col_num != fc.m_StopPosCol &&
            col_num != fc.m_StrandCol) {

            // Can only have 1 id field in region feature so set others to comment
            CSeqTable_column_info& header =  cols[col_num]->SetHeader();
            if (header.GetField_id() == CSeqTable_column_info::eField_id_location ||
                header.GetField_id() == CSeqTable_column_info::eField_id_location_id ||
                header.GetField_id() == CSeqTable_column_info::eField_id_location_gi ||
                header.GetField_id() == CSeqTable_column_info::eField_id_id_local) {
                    header.SetField_id(CSeqTable_column_info::eField_id_comment);
            }
            // Can't have Numeric fields other than start, stop length.  In table loader
            // integer fields not being used in locations have id type as comment and data
            // type as int. switch data type to text.
            else if (cols[col_num]->GetData().Which() == CSeqTable_multi_data_Base::e_Int &&
                     header.GetField_id() == CSeqTable_column_info::eField_id_comment) {

                         // Get copy current data for this integer field
                         CSeqTable_multi_data_Base::TInt col_data = cols[col_num]->GetData().GetInt();
                         // delete field data and set new type to string
                         cols[col_num]->SetData().Select(CSeqTable_multi_data_Base::e_String, eDoResetVariant);
                         // copy int data to new field, converting it to string
                         for (size_t row=0; row<col_data.size(); ++row) {
                             string val;
                             try {
                                val = NStr::IntToString(col_data[row]);
                             }
                             catch (CException&) { }

                             cols[col_num]->SetData().SetString().push_back(val);
                         }
                         // done - we converted the int field to a string field
                         // (becuase int fields can't be comment fields - in
                         // region features)
            }
            else if (cols[col_num]->GetData().Which() == CSeqTable_multi_data_Base::e_Real &&
                     header.GetField_id() == CSeqTable_column_info::eField_id_comment) {

                         // Get copy current data for this integer field
                         CSeqTable_multi_data_Base::TReal col_data = cols[col_num]->GetData().GetReal();
                         // delete field data and set new type to string
                         cols[col_num]->SetData().Select(CSeqTable_multi_data_Base::e_String, eDoResetVariant);
                         // copy int data to new field, converting it to string
                         for (size_t row=0; row<col_data.size(); ++row) {
                             string val;
                             try {
                                val = NStr::DoubleToString(col_data[row]);
                             }
                             catch (CException&) { }

                             cols[col_num]->SetData().SetString().push_back(val);
                         }
                         // done - we converted the real field to a string field
                         // (becuase real fields can't be comment fields - in
                         // region features)
            }
        }
    }

    bool start_one_based = true;
    CUser_field& start_meta_info = column_meta_info->SetData()[fc.m_StartPosCol].GetObject();
    if (start_meta_info.IsSetData() && start_meta_info.GetData().IsStr())   {
        start_one_based = x_GetMetaInfoTag(start_meta_info.GetData().GetStr(), "one_based") == "true";

        // To make a feature table we need 0-based indices, so before we update
        // below change the meta-data to indicate it is no longer 1-based:
        if (start_one_based) {
            string meta_info_str = start_meta_info.GetData().GetStr();
            x_UpdateMetaInfoTag(meta_info_str, "one_based", "false");
            start_meta_info.SetData().SetStr(meta_info_str);
        }
    }

    bool stop_one_based = true;
    if (fc.m_StopPosCol >= 0) {
        CUser_field& stop_meta_info = column_meta_info->SetData()[fc.m_StopPosCol].GetObject();
        if (stop_meta_info.IsSetData() && stop_meta_info.GetData().IsStr())
            stop_one_based = x_GetMetaInfoTag(stop_meta_info.GetData().GetStr(), "one_based") == "true";

        // To make a feature table we need 0-based indices, so before we update
        // below change the meta-data to indicate it is no longer 1-based:
        if (stop_one_based) {
            string meta_info_str = stop_meta_info.GetData().GetStr();
            x_UpdateMetaInfoTag(meta_info_str, "one_based", "false");
            stop_meta_info.SetData().SetStr(meta_info_str);
        }
    }
    else {
        // we are using length instead of a 'to' position.  Stop will be
        // start + len, so if start is one-based, stop will be to.
        stop_one_based = start_one_based;
    }

    /// Chromosomes need to be re-mapped using the accession
    CIdMapper* mapper = NULL;
    CRef< CSeqTable_column > xform_ids_column;

    const CUser_field& id_meta_info = column_meta_info->GetData()[fc.m_IdCol].GetObject();

    string column_type = x_GetMetaInfoTag(id_meta_info.GetData().GetStr(), "xtype");
    if (CTableImportColumn::GetDataTypeFromString(column_type) ==
        CTableImportColumn::eChromosome) {
        string assembly;
        string meta_info;

        if (id_meta_info.IsSetData() && id_meta_info.GetData().IsStr())
            meta_info = id_meta_info.GetData().GetStr();

        assembly = x_GetMetaInfoTag(meta_info, "genome_assembly");

        if (!assembly.empty() ) {
            // used for initializing CIdMapperGCAssembly; having a full assembly seems to be inevitable here
            CRef<CGC_Assembly> assm = CGencollSvc::GetInstance()->GetGCAssembly(assembly, true, "Gbench");
            if (assm) {
                CRef<CObjectManager> obj_mgr = CObjectManager::GetInstance();
                CRef<CScope> scope(new CScope(*obj_mgr));
                scope->AddDefaults();
                CIdMapperGCAssembly::EAliasMapping alias =
                    assm->IsRefSeq() ? CIdMapperGCAssembly::eRefSeqAcc : CIdMapperGCAssembly::eGenBankAcc;
                mapper = new CIdMapperGCAssembly(*scope, *assm, alias);
            }

            // Create a new column to put the mapped ids into
            CRef<CSeqTable_column_info> cinfo(new CSeqTable_column_info());
            cinfo->SetTitle(id_header.GetTitle());
            cinfo->SetField_id(CSeqTable_column_info_Base::eField_id_location_id);

            xform_ids_column.Reset(new CSeqTable_column());
            xform_ids_column->SetHeader(*cinfo);
            CRef<CSeqTable_multi_data> data(new CSeqTable_multi_data());
            data->Select(CSeqTable_multi_data_Base::e_Id);
            xform_ids_column->SetData(data.GetNCObject());
        }
    }

    // Do we need a new strand column?  For features it has to be numeric.  If
    // there is no strand column, create one based on start>stop or start<stop.
    CRef< CSeqTable_column > xform_strand_column;
    if (fc.m_StrandCol == -1 ||
        cols[fc.m_StrandCol]->GetData().IsString()) {
        CRef<CSeqTable_column_info> cinfo(new CSeqTable_column_info());

        cinfo->SetField_id(CSeqTable_column_info_Base::eField_id_location_strand);
        if (fc.m_StrandCol != -1) {
            const CSeqTable_column_info& strand_header =
                cols[fc.m_StrandCol]->GetHeader();
            cinfo->SetTitle(strand_header.GetTitle());
        }
        else {
            cinfo->SetTitle(string("Column.") +
                NStr::NumericToString(cols.size() + 1));
        }

        xform_strand_column.Reset(new CSeqTable_column());
        xform_strand_column->SetHeader(*cinfo);
        CRef<CSeqTable_multi_data> data(new CSeqTable_multi_data());
        data->Select(CSeqTable_multi_data_Base::e_Int);
        xform_strand_column->SetData(data.GetNCObject());
    }

    //If there is not already a data-region column, we need to add one.
    CRef< CSeqTable_column > data_region_column;
    if (fc.m_DataRegionCol == -1) {
        CRef<CSeqTable_column_info> cinfo(new CSeqTable_column_info());
        cinfo->SetTitle("Data Region Col");
        cinfo->SetField_id(CSeqTable_column_info_Base::eField_id_data_region);

        data_region_column.Reset(new CSeqTable_column());
        data_region_column->SetHeader(*cinfo);
        CRef<CSeqTable_multi_data> data(new CSeqTable_multi_data());
        data->Select(CSeqTable_multi_data_Base::e_String);
        data_region_column->SetData(data.GetNCObject());
    }

    // Need to have a stop-position column (not just a length
    // column) for features
    CRef< CSeqTable_column > stop_position_column;
    if (fc.m_StopPosCol == -1 && fc.m_LengthCol >= 0) {
        CRef<CSeqTable_column_info> cinfo(new CSeqTable_column_info());
        cinfo->SetTitle("Stop Column");
        cinfo->SetField_id(CSeqTable_column_info_Base::eField_id_location_to);

        stop_position_column.Reset(new CSeqTable_column());
        stop_position_column->SetHeader(*cinfo);
        CRef<CSeqTable_multi_data> data(new CSeqTable_multi_data());
        data->Select(CSeqTable_multi_data_Base::e_Int);
        stop_position_column->SetData(data.GetNCObject());
    }

    // If there are errors we can put them in an error column
    // at the end of conversion, if no errors have occured, the column
    // will not be added to the table. Columns with errors will also
    // be marked as disabled to keep the table valid.
    bool errors_occured = false;
    CRef< CSeqTable_column > error_column;
    {
        CRef<CSeqTable_column_info> cinfo(new CSeqTable_column_info());
        cinfo->SetTitle("Error Messages");
        cinfo->SetField_id(CSeqTable_column_info_Base::eField_id_comment);

        error_column.Reset(new CSeqTable_column());
        error_column->SetHeader(*cinfo);
        CRef<CSeqTable_multi_data> data(new CSeqTable_multi_data());
        data->Select(CSeqTable_multi_data_Base::e_String);
        error_column->SetData(data.GetNCObject());

        // I think our table viewer is not handling sparse data...
        //CSeqTable_sparse_index* si = new CSeqTable_sparse_index();
        //si->Select(CSeqTable_sparse_index_Base::e_Indexes);
        //error_column->SetSparse(*si);
    }

    CRef< CSeqTable_column > disabled_column;
    {
        CRef<CSeqTable_column_info> cinfo(new CSeqTable_column_info());
        cinfo->SetField_name("disabled");

        disabled_column.Reset(new CSeqTable_column());
        disabled_column->SetHeader(*cinfo);
        CSeqTable_single_data *sd = new CSeqTable_single_data();
        sd->SetBit(true);
        disabled_column->SetDefault(*sd);

        CSeqTable_sparse_index* si = new CSeqTable_sparse_index();
        si->Select(CSeqTable_sparse_index_Base::e_Indexes);
        disabled_column->SetSparse(*si);
    }


    if (call != NULL &&
        call->IsCanceled())
        return false;

    // Visit all rows in the table and add any columns needed to create a
    // complete feature.  If there are no columns to add/update, skip
    // this step.
    if (!xform_strand_column.IsNull() ||
        !stop_position_column.IsNull() ||
        !data_region_column.IsNull() ||
        !(mapper == NULL) ||
        start_one_based ||
        stop_one_based) {

            int err_count = 0;
            for (size_t row=0; row<cols[fc.m_IdCol]->GetData().GetSize(); ++row) {

                int from = 0;
                int to = 0;
                string logstr;
                string errstr = "";

                if (!cols[fc.m_StartPosCol]->TryGetInt(row, from)) {
                    logstr = " cannot convert start position to an integer";
                    if (++err_count < 100) {
                        LOG_POST("Invalid row " << row+1 << logstr);
                    }
                }
                else {
                    if (start_one_based) {
                        if (from < 1) {
                            logstr = " start column is one-based but value is < 1";
                            if (++err_count < 100) {
                                LOG_POST("Invalid row " << row+1 << logstr);
                            }
                        }

                        from -= 1;
                        cols[fc.m_StartPosCol]->SetData().SetInt()[row] = from;
                    }
                    else if (from < 0) {
                        logstr = " start column is < 0";
                        if (++err_count < 100) {
                            LOG_POST("Invalid row " << row+1 << logstr);
                        }
                    }
                }

                errstr = logstr;

                // Inserting a stop position column if the original table had only
                // a start and length column (stop = start + length)
                if (!stop_position_column.IsNull()) {
                    int len = 0;
                    // if 'from' not retrieved above get it from start column
                    if (!cols[fc.m_LengthCol]->TryGetInt(row, len)) {
                        logstr = " cannot convert length to an integer";
                        if (errstr != "") errstr += " | ";
                        errstr += logstr;

                        if (++err_count < 100) {
                            LOG_POST("Invalid row " << row+1 << logstr);
                        }
                        len = 0;
                    }

                    if (len < 0) {
                        logstr = " length column is < 0";
                        if (errstr != "") errstr += " | ";
                        errstr += logstr;

                        if (++err_count < 100) {
                            LOG_POST("Invalid row " << row+1 << logstr);
                        }
                        len = 0;
                    }

                    to = from + len;

                    stop_position_column->SetData().SetInt().push_back(to);
                }
                else {
                    // Get stop column, check if value is valid and update it
                    // to be 0-based if needed.
                    if (!cols[fc.m_StopPosCol]->TryGetInt(row, to)) {
                        logstr = " cannot convert stop position to an integer";
                        if (errstr != "") errstr += " | ";
                        errstr += logstr;

                        if (++err_count < 100) {
                            LOG_POST("Invalid row " << row+1 << logstr);
                        }
                    }
                    else {
                        if (stop_one_based) {
                            if (to < 1) {
                                logstr = " stop column is one-based but value is < 1";
                                if (errstr != "") errstr += " | ";
                                errstr += logstr;

                                if (++err_count < 100) {
                                    LOG_POST("Invalid row " << row+1 << logstr);
                                }
                            }

                            to -= 1;
                            cols[fc.m_StopPosCol]->SetData().SetInt()[row] = to;
                        }
                        else if (to < 0) {
                            logstr = " stop column is < 0";
                            if (errstr != "") errstr += " | ";
                            errstr += logstr;

                            if (++err_count < 100) {
                                LOG_POST("Invalid row " << row+1 << logstr);
                            }
                        }
                    }
                }


                // Convert the strand column to a numeric as required for
                // making seqtable features.
                if (!xform_strand_column.IsNull()) {
                    ENa_strand strand_e;
                    if (fc.m_StrandCol != -1) {
                        string strand = *cols[fc.m_StrandCol]->GetStringPtr(row);
                        strand_e = x_GetStrand(strand);

                        // Invalid strand string - log an error.
                        if (strand_e == eNa_strand_unknown) {
                            logstr = string(" error - strand: \"") + strand + "\" not valid";
                            if (errstr != "") errstr += " | ";
                            errstr += logstr;

                            if (++err_count < 100)
                                LOG_POST("Invalid row " << row+1 << logstr);
                        }
                    }
                    else {
                        // Infer strand from start and stop position.  These
                        // have already been retrieved above
                        // If to>from assume positive strand, otherwise negative.
                        if (to >= from) {
                            strand_e = eNa_strand_plus;
                        }
                        else {
                            // If a negative strand is inferred, switch the 'from' and 'to'
                            // values so that 'to' is greater than 'from':
                            strand_e = eNa_strand_minus;
                            int tmp = to;
                            to = from;
                            from = tmp;
                        }

                        cols[fc.m_StartPosCol]->SetData().SetInt()[row] = from;

                        // Update the stop column which may have been in the
                        // original table or may be a generated column we are
                        // filling based on the length field.
                        if (!stop_position_column.IsNull())
                            stop_position_column->SetData().SetInt().push_back(to);
                        else
                            cols[fc.m_StopPosCol]->SetData().SetInt()[row] = to;


                    }
                    xform_strand_column->SetData().SetInt().push_back((int)strand_e);
                }

                // Add a (generated) data-region column if none was previously
                // specified.
                if (!data_region_column.IsNull()) {
                    string region_name = "Region " + NStr::IntToString((int)row);
                    data_region_column->SetData().SetString().push_back(region_name);
                }

                if (mapper != NULL) {
                    // Handle chromosomes of the forms:
                    // "1", "20", "chr20", "X", "y", "Chr[x,y,X,Y], "20|text..."
                    string idstr = *cols[fc.m_IdCol]->GetStringPtr(row);
                    size_t idx = idstr.find_first_of("0123456789XYxy");

                    CRef<CSeq_id>  seq_id;
                    if (idx != string::npos) {
                        size_t chr_len = idstr.size()-idx;

                        size_t end_idx = idstr.find_first_not_of("0123456789XYxy", idx);
                        if (end_idx != string::npos)
                            chr_len = end_idx-idx;

                        string chr_num = idstr.substr(idx, chr_len);
                        try {
                            seq_id.Reset(new CSeq_id(CSeq_id_Base::e_Local, chr_num));
                            CSeq_id_Handle handle = mapper->Map(CSeq_id_Handle::GetHandle(*seq_id));

                            if (handle.IsGi()) {
                                TGi gi = handle.GetGi();
                                seq_id.Reset(new CSeq_id(CSeq_id_Base::e_Gi, gi));
                                xform_ids_column->SetData().SetId().push_back(seq_id);
                            }
                            else if (!handle.GetSeqId().IsNull()) {
                                CConstRef<CSeq_id> cseq_id = handle.GetSeqId();

                                // This is roundabout (seq-id to string to seq-id) but
                                // it works.
                                string seqid_str = cseq_id->GetSeqIdString();
                                seq_id.Reset(new CSeq_id(seqid_str));

                                xform_ids_column->SetData().SetId().push_back(seq_id);
                            }
                            else {
                                // Error - something didn't work...
                                seq_id.Reset(new CSeq_id());
                                xform_ids_column->SetData().SetId().push_back(seq_id);

                                logstr = " Unable to generate seq-id from id: " + idstr;
                                if (errstr != "") errstr += " | ";
                                errstr += logstr;

                                if (++err_count < 100) {
                                    LOG_POST("Invalid row " << row+1 << logstr);
                                }
                            }
                        }
                        catch(CStringException&) {
                            // // Error - chromosome didn't work
                            seq_id.Reset(new CSeq_id());
                            xform_ids_column->SetData().SetId().push_back(seq_id);

                            logstr = " Unable to parse valid chromosome number from string: " + idstr;
                            if (errstr != "") errstr += " | ";
                            errstr += logstr;

                            if (++err_count < 100) {
                                LOG_POST("Invalid row " << row+1 << logstr);
                            }
                        }
                        catch (CException& ex) {
                            // Error - chromosome didn't work
                            seq_id.Reset(new CSeq_id());
                            xform_ids_column->SetData().SetId().push_back(seq_id);

                            logstr = string(" Error id: ") + idstr + " - " + ex.GetMsg();
                            if (errstr != "") errstr += " | ";
                            errstr += logstr;

                            if (++err_count < 100) {
                                LOG_POST("Invalid row " << row+1 << logstr);
                            }
                        }
                    }
                    else {
                        // Could not identify as a chromosome - try to create id with
                        // unmodified string.
                        try {
                            seq_id.Reset(new CSeq_id(idstr));
                        }
                        catch (CException&) {
                            seq_id.Reset(new CSeq_id());
                        }
                        xform_ids_column->SetData().SetId().push_back(seq_id);
                    }
                }

                // will only add the column at the end if 1 or more errors
                // occured.
                if (errstr != "") {
                    errors_occured = true;
                    disabled_column->SetSparse().SetIndexes().push_back(row);

                    // our viewer doesn't currently handle sparse indices with data
                    //error_column->SetSparse().SetIndexes().push_back(row);
                }
                // Will add blanks too for cols w/o errors
                error_column->SetData().SetString().push_back(errstr);

                // if call is asynchronous, return if/when cancelled.
                if (call != NULL &&
                    call->IsCanceled())
                    return false;
            }
    }

    delete mapper;


    /// Add any new columns that were required to make a valid feature:
    // (strand, data region , stop position and ID)
    if (!xform_strand_column.IsNull()) {
        if (fc.m_StrandCol != -1) {
            table.SetColumns().erase(table.SetColumns().begin() + fc.m_StrandCol);
            table.SetColumns().insert(table.SetColumns().begin() + fc.m_StrandCol,
                xform_strand_column);
        }
        else {
            table.SetColumns().push_back(xform_strand_column);
            string label = xform_strand_column->GetHeader().GetTitle();
            string properties = string("&xtype=") +
                CTableImportColumn::GetStringFromDataType(CTableImportColumn::eStrand) +
                " &derived_field=true";
            column_meta_info->AddField(label, properties);
        }

    }

    if (!data_region_column.IsNull()) {
        table.SetColumns().push_back(data_region_column);

        string label = data_region_column->GetHeader().GetTitle();
        string properties = string("&xtype=") +
            CTableImportColumn::GetStringFromDataType(CTableImportColumn::eDataRegion) +
            " &derived_field=true";
        column_meta_info->AddField(label, properties);
    }

    if (!stop_position_column.IsNull()) {
        table.SetColumns().push_back(stop_position_column);

        string label = stop_position_column->GetHeader().GetTitle();
        string properties = string("&xtype=") +
            CTableImportColumn::GetStringFromDataType(CTableImportColumn::eStopPosition) +
            " &derived_field=true";
        column_meta_info->AddField(label, properties);
    }

    if (!xform_ids_column.IsNull()) {
        table.SetColumns().erase(table.SetColumns().begin() + fc.m_IdCol);
        table.SetColumns().insert(table.SetColumns().begin() + fc.m_IdCol,
            xform_ids_column);
    }

    if (errors_occured) {
        table.SetColumns().push_back(error_column);
        string label = error_column->GetHeader().GetTitle();
        string properties = string("&xtype=") +
            CTableImportColumn::GetStringFromDataType(CTableImportColumn::eUnspecifiedText) +
            " &derived_field=true";
        column_meta_info->AddField(label, properties);

        table.SetColumns().push_back(disabled_column);
        label = "disabled";
        properties = string("&xtype=disabled_indices") +
            //CTableImportColumn::GetStringFromDataType(CTableImportColumn::eUndefined) +
            " &derived_field=true";
        column_meta_info->AddField(label, properties);
    }

    return true;
}

bool CTableAnnotDataSource::CreateSnpFeature(const STableLocation& fc,
                                             ICanceled* call)
{
    // Get User data from Annot Descriptor this is the Properties data
    // structure, where each entry is a combination of column number
    // and field properties, e.g.
    // Column.1  &xtype=StartPosition &one_based=true
    CAnnotdesc::TUser* column_meta_info = x_GetColumnMetaInfo();
    if (column_meta_info == NULL)
        return false;

    // Now get the actual columns (which have headers with descriptive
    // info as well - as much as we currently need for finding features
    // actually)
    if (!m_AnnotContainer->IsSetData()) {
        LOG_POST("Error - SeqTable not found in SeqAnnot");
        return false;
    }
    CSeq_annot::TData& table_container = m_AnnotContainer->SetData();

    if (!table_container.IsSeq_table()) {
        LOG_POST("Error - SeqAnnot does not contain a SeqTable");
        return false;
    }
    CSeq_table& table = table_container.SetSeq_table();
    CSeq_table::TColumns& cols = table.SetColumns();

    /// To make the table a valid source for features first need to set its type
    /// and subtype to indicate that the rows represent region features.
    table.SetFeat_type(CSeqFeatData_Base::e_Region);
    table.SetFeat_subtype(CSeqFeatData::eSubtype_region);

    // Can only have one ID field if the table is a feature table.
    // Also, no comment fields may be of type int or real.  So since we are adding
    // all the feature fields (start/stop/strand/id) set any other numeric or id fields
    // to comments.
    for (size_t col_num=0; col_num<cols.size(); ++col_num) {
        // Can only have 1 id field in region feature so set others to comment
        CSeqTable_column_info& header =  cols[col_num]->SetHeader();
        if (header.GetField_id() == (int)CSeqTable_column_info::eField_id_location ||
            header.GetField_id() == (int)CSeqTable_column_info::eField_id_location_id ||
            header.GetField_id() == (int)CSeqTable_column_info::eField_id_location_gi ||
            header.GetField_id() == (int)CSeqTable_column_info::eField_id_id_local) {
                header.SetField_id(CSeqTable_column_info::eField_id_comment);
        }
        // Can't have Numeric fields other than start, stop length.  In table loader
        // integer fields not being used in locations have id type as comment and data
        // type as int. switch data type to text.
        else if (cols[col_num]->GetData().Which() == CSeqTable_multi_data_Base::e_Int &&
            header.GetField_id() == CSeqTable_column_info::eField_id_comment) {

                // Get copy current data for this integer field
                CSeqTable_multi_data_Base::TInt col_data = cols[col_num]->GetData().GetInt();
                // delete field data and set new type to string
                cols[col_num]->SetData().Select(CSeqTable_multi_data_Base::e_String, eDoResetVariant);
                // copy int data to new field, converting it to string
                for (size_t row=0; row<col_data.size(); ++row) {
                    string val;
                    try {
                        val = NStr::IntToString(col_data[row]);
                    }
                    catch (CException&) { }

                    cols[col_num]->SetData().SetString().push_back(val);
                }
                // done - we converted the int field to a string field
                // (becuase int fields can't be comment fields - in
                // region features)
        }
        else if (cols[col_num]->GetData().Which() == CSeqTable_multi_data_Base::e_Real &&
            header.GetField_id() == CSeqTable_column_info::eField_id_comment) {

                // Get copy current data for this integer field
                CSeqTable_multi_data_Base::TReal col_data = cols[col_num]->GetData().GetReal();
                // delete field data and set new type to string
                cols[col_num]->SetData().Select(CSeqTable_multi_data_Base::e_String, eDoResetVariant);
                // copy int data to new field, converting it to string
                for (size_t row=0; row<col_data.size(); ++row) {
                    string val;
                    try {
                        val = NStr::DoubleToString(col_data[row]);
                    }
                    catch (CException&) { }

                    cols[col_num]->SetData().SetString().push_back(val);
                }
                // done - we converted the real field to a string field
                // (becuase real fields can't be comment fields - in
                // region features)
        }
    }

    /// Chromosomes need to be re-mapped using the accession
    bool has_assembly = false;
    string assembly;

    const CUser_field& id_meta_info = column_meta_info->GetData()[fc.m_IdCol].GetObject();

    string column_type = x_GetMetaInfoTag(id_meta_info.GetData().GetStr(), "xtype");
    if (CTableImportColumn::GetDataTypeFromString(column_type) ==
        CTableImportColumn::eRsid) {
        string meta_info;

        if (id_meta_info.IsSetData() && id_meta_info.GetData().IsStr())
            meta_info = id_meta_info.GetData().GetStr();

        assembly = x_GetMetaInfoTag(meta_info, "genome_assembly");

        if (!assembly.empty() ) {
            has_assembly = true;
        }
    }

    // Add id column for seq-id of molecule on which rsid is found
    CRef< CSeqTable_column > seqid_column;
    {
        CRef<CSeqTable_column_info> cinfo(new CSeqTable_column_info());

        cinfo->SetField_id(CSeqTable_column_info_Base::eField_id_location_id);
        if (fc.m_StrandCol != -1) {
            const CSeqTable_column_info& strand_header =
                cols[fc.m_StrandCol]->GetHeader();
            cinfo->SetTitle(strand_header.GetTitle());
        }
        else {
            cinfo->SetTitle(string("Seq-ID"));
        }

        seqid_column.Reset(new CSeqTable_column());
        seqid_column->SetHeader(*cinfo);
        CRef<CSeqTable_multi_data> data(new CSeqTable_multi_data());
        data->Select(CSeqTable_multi_data_Base::e_Id);
        seqid_column->SetData(data.GetNCObject());
    }

    // Do we need a new strand column?  For features it has to be numeric.  If
    // there is no strand column, create one based on start>stop or start<stop.
    CRef< CSeqTable_column > xform_strand_column;
    if (fc.m_StrandCol == -1 ||
        cols[fc.m_StrandCol]->GetData().IsString()) {
        CRef<CSeqTable_column_info> cinfo(new CSeqTable_column_info());

        cinfo->SetField_id(CSeqTable_column_info_Base::eField_id_location_strand);
        if (fc.m_StrandCol != -1) {
            const CSeqTable_column_info& strand_header =
                cols[fc.m_StrandCol]->GetHeader();
            cinfo->SetTitle(strand_header.GetTitle());
        }
        else {
            cinfo->SetTitle(string("Strand"));
        }

        xform_strand_column.Reset(new CSeqTable_column());
        xform_strand_column->SetHeader(*cinfo);
        CRef<CSeqTable_multi_data> data(new CSeqTable_multi_data());
        data->Select(CSeqTable_multi_data_Base::e_Int);
        xform_strand_column->SetData(data.GetNCObject());
    }

    //If there is not already a data-region column, we need to add one.
    CRef< CSeqTable_column > data_region_column;
    if (fc.m_DataRegionCol == -1) {
        CRef<CSeqTable_column_info> cinfo(new CSeqTable_column_info());
        cinfo->SetTitle("Data Region Col");
        cinfo->SetField_id(CSeqTable_column_info_Base::eField_id_data_region);

        data_region_column.Reset(new CSeqTable_column());
        data_region_column->SetHeader(*cinfo);
        CRef<CSeqTable_multi_data> data(new CSeqTable_multi_data());
        data->Select(CSeqTable_multi_data_Base::e_String);
        data_region_column->SetData(data.GetNCObject());
    }

    // Need to have a stop-position column (not just a length
    // column) for features
    CRef< CSeqTable_column > stop_position_column;
    if (fc.m_StopPosCol == -1) {
        CRef<CSeqTable_column_info> cinfo(new CSeqTable_column_info());
        cinfo->SetTitle("Stop Column");
        cinfo->SetField_id(CSeqTable_column_info_Base::eField_id_location_to);

        stop_position_column.Reset(new CSeqTable_column());
        stop_position_column->SetHeader(*cinfo);
        CRef<CSeqTable_multi_data> data(new CSeqTable_multi_data());
        data->Select(CSeqTable_multi_data_Base::e_Int);
        stop_position_column->SetData(data.GetNCObject());
    }
    CRef< CSeqTable_column > start_position_column;
    if (fc.m_StartPosCol == -1) {
        CRef<CSeqTable_column_info> cinfo(new CSeqTable_column_info());
        cinfo->SetTitle("Start Column");
        cinfo->SetField_id(CSeqTable_column_info_Base::eField_id_location_from);

        start_position_column.Reset(new CSeqTable_column());
        start_position_column->SetHeader(*cinfo);
        CRef<CSeqTable_multi_data> data(new CSeqTable_multi_data());
        data->Select(CSeqTable_multi_data_Base::e_Int);
        start_position_column->SetData(data.GetNCObject());
    }

    // If there are errors we can put them in an error column
    // at the end of conversion, if no errors have occured, the column
    // will not be added to the table. Columns with errors will also
    // be marked as disabled to keep the table valid.
    bool errors_occured = false;
    CRef< CSeqTable_column > error_column;
    {
        CRef<CSeqTable_column_info> cinfo(new CSeqTable_column_info());
        cinfo->SetTitle("Error Messages");
        cinfo->SetField_id(CSeqTable_column_info_Base::eField_id_comment);

        error_column.Reset(new CSeqTable_column());
        error_column->SetHeader(*cinfo);
        CRef<CSeqTable_multi_data> data(new CSeqTable_multi_data());
        data->Select(CSeqTable_multi_data_Base::e_String);
        error_column->SetData(data.GetNCObject());
    }

    CRef< CSeqTable_column > disabled_column;
    {
        CRef<CSeqTable_column_info> cinfo(new CSeqTable_column_info());
        cinfo->SetField_name("disabled");

        disabled_column.Reset(new CSeqTable_column());
        disabled_column->SetHeader(*cinfo);
        CSeqTable_single_data *sd = new CSeqTable_single_data();
        sd->SetBit(true);
        disabled_column->SetDefault(*sd);

        CSeqTable_sparse_index* si = new CSeqTable_sparse_index();
        si->Select(CSeqTable_sparse_index_Base::e_Indexes);
        disabled_column->SetSparse(*si);
    }


    if (call != NULL &&
        call->IsCanceled())
        return false;

    size_t rsid_search_size = 100;

    // Visit all rows in the table and add any columns needed to create a
    // complete feature.  If there are no columns to add/update, skip
    // this step.
    int err_count = 0;

    CRef<CObjectManager> om(CObjectManager::GetInstance());
    CRef<CScope> scope(new CScope(*om));
    scope->AddDefaults();


    // We have an outer loop to retrieve the snp information from the snp database in
    // groups of 'rsid_search_size' at a time since thats more efficient, and then an
    // inner loop to process those results one at a time.
    size_t row = 0;
    size_t search_row = 0;
    size_t current_size = 0;
    size_t row_count = cols[fc.m_IdCol]->GetData().GetSize();
    while (search_row < row_count) {

        // could resize rsids to begin with (rsids have predictable lengths...)
        string rsids = "";
        for (current_size=0; search_row < row_count && current_size<rsid_search_size; ++current_size, ++search_row) {
            rsids += *cols[fc.m_IdCol]->GetStringPtr(search_row);
            rsids += ",";
        }
        // remove last comma
        rsids = rsids.substr(0, rsids.length()-1);

        // Query for current set of rsids
        NSNPWebServices::TSNPSearchCompoundResultList SNPSearchResultList;
        try {
            // assembly may be blank.
            NSNPWebServices::Search(rsids, assembly, SNPSearchResultList);
        }
        catch (CException& e) {
            LOG_POST("Exception while searching SNP database: " + e.GetMsg());
            // If there is an error getting a group of ids continually reduce the batch
            // size until there is only 1 id retrieved and then if there is still an
            // error process it as an error
            if (rsid_search_size > 2) {
                search_row = row;
                rsid_search_size = rsid_search_size/2;
                continue;
            }
        }
        catch(...) {
            LOG_POST("Unidentified exception while searching SNP database");
            // If there is an error getting a group of ids continually reduce the batch
            // size until there is only 1 id retrieved and then if there is still an
            // error process it as an error
            if (rsid_search_size > 2) {
                search_row = row;
                rsid_search_size = rsid_search_size/2;
                continue;
            }
        }

        // process the current set of rows. Results in result list should be in same order
        // as the rows, but we will search the result set if there is a mismatch.
        NSNPWebServices::TSNPSearchCompoundResultList::iterator current_iter = SNPSearchResultList.begin();

        for (; row < search_row; ++row) {

            int from = 0;
            int to = 0;
            string logstr;
            string errstr = "";

            bool found = false;
            string rsid = *cols[fc.m_IdCol]->GetStringPtr(row);

            // See if current snp query result matches current row. If not, search for current
            // row rsid in entire result set. (usually they should match)
            NSNPWebServices::TSNPSearchCompoundResultList::iterator iter = current_iter;
            ++current_iter;

            if (iter->first != rsid) {
                CompareSNPResults pred(rsid);
                iter = std::find_if(SNPSearchResultList.begin(), SNPSearchResultList.end(), pred);
            }

            if (iter == SNPSearchResultList.end()) {
                x_LogErr("No matches for given rsid", errstr, err_count, row);
            }
            else if ((*iter).second.size() == 0) {
                x_LogErr("No matches for rsid and assembly: " + assembly,
                    errstr, err_count, row);
            }
            else {
                const NSNPWebServices::TSNPSearchResultList::value_type& first_result = (*iter).second.front();

                // CVariation used as a search result can have one and only one placement
                if (first_result->CanGetPlacements() == 0) {
                    x_LogErr("Unexpected absence of placements in SNP Search Result", errstr, err_count, row);
                }
                else {
                    const CVariation::TPlacements& placements(first_result->GetPlacements());
                    if (placements.size() == 0) {
                        x_LogErr("Unexpected number of placements (0) in SNP Search Result", errstr, err_count, row);
                    }
                    else {
                        const CSeq_id* id = placements.front()->GetLoc().GetId();
                        CRef<CSeq_id> rid(new CSeq_id());
                        rid->Assign(*id);

                        seqid_column->SetData().SetId().push_back(rid);

                        from = placements.front()->GetLoc().GetStart(eExtreme_Positional);
                        start_position_column->SetData().SetInt().push_back(from);

                        to = placements.front()->GetLoc().GetStop(eExtreme_Positional);
                        stop_position_column->SetData().SetInt().push_back(to);

                        xform_strand_column->SetData().SetInt().push_back(placements.front()->GetLoc().GetStrand());
                        found = true;
                    }
                }
            }

            if (!found) {
                CRef<CSeq_id> empty_id(new CSeq_id());

                seqid_column->SetData().SetId().push_back(empty_id);
                start_position_column->SetData().SetInt().push_back(0);
                stop_position_column->SetData().SetInt().push_back(0);
                xform_strand_column->SetData().SetInt().push_back(eNa_strand_unknown);
            }

            // Add a (generated) data-region column if none was previously
            // specified.
            if (!data_region_column.IsNull()) {
                string region_name = "Region " + NStr::IntToString((int)row);
                data_region_column->SetData().SetString().push_back(region_name);
            }

            // will only add the column at the end if 1 or more errors
            // occured.
            if (errstr != "") {
                errors_occured = true;
                disabled_column->SetSparse().SetIndexes().push_back(row);
            }
            // Will add blanks too for cols w/o errors
            error_column->SetData().SetString().push_back(errstr);

            // if call is asynchronous, return if/when cancelled.
            if (call != NULL &&
                call->IsCanceled())
                return false;
        }
    }

    /// Add any new columns that were required to make a valid feature:
    // (strand, data region , stop position and ID)
    if (!seqid_column.IsNull()) {
        table.SetColumns().push_back(seqid_column);
        string label = seqid_column->GetHeader().GetTitle();
        string properties = string("&xtype=") +
            CTableImportColumn::GetStringFromDataType(CTableImportColumn::eLocationID) +
            " &derived_field=true";
        column_meta_info->AddField(label, properties);
    }

    if (!xform_strand_column.IsNull()) {
        if (fc.m_StrandCol != -1) {
            table.SetColumns().erase(table.SetColumns().begin() + fc.m_StrandCol);
            table.SetColumns().insert(table.SetColumns().begin() + fc.m_StrandCol,
                xform_strand_column);
        }
        else {
            table.SetColumns().push_back(xform_strand_column);
            string label = xform_strand_column->GetHeader().GetTitle();
            string properties = string("&xtype=") +
                CTableImportColumn::GetStringFromDataType(CTableImportColumn::eStrand) +
                " &derived_field=true";
            column_meta_info->AddField(label, properties);
        }

    }

    if (!data_region_column.IsNull()) {
        table.SetColumns().push_back(data_region_column);

        string label = data_region_column->GetHeader().GetTitle();
        string properties = string("&xtype=") +
            CTableImportColumn::GetStringFromDataType(CTableImportColumn::eDataRegion) +
            " &derived_field=true";
        column_meta_info->AddField(label, properties);
    }

    if (!start_position_column.IsNull()) {
        table.SetColumns().push_back(start_position_column);

        string label = start_position_column->GetHeader().GetTitle();
        string properties = string("&xtype=") +
            CTableImportColumn::GetStringFromDataType(CTableImportColumn::eStartPosition) +
            " &derived_field=true";
        column_meta_info->AddField(label, properties);
    }

    if (!stop_position_column.IsNull()) {
        table.SetColumns().push_back(stop_position_column);

        string label = stop_position_column->GetHeader().GetTitle();
        string properties = string("&xtype=") +
            CTableImportColumn::GetStringFromDataType(CTableImportColumn::eStopPosition) +
            " &derived_field=true";
        column_meta_info->AddField(label, properties);
    }

    if (errors_occured) {
        table.SetColumns().push_back(error_column);
        string label = error_column->GetHeader().GetTitle();
        string properties = string("&xtype=") +
            CTableImportColumn::GetStringFromDataType(CTableImportColumn::eUnspecifiedText) +
            " &derived_field=true";
        column_meta_info->AddField(label, properties);

        table.SetColumns().push_back(disabled_column);
        label = "disabled";
        properties = string("&xtype=disabled_indices") +
            //CTableImportColumn::GetStringFromDataType(CTableImportColumn::eUndefined) +
            " &derived_field=true";
        column_meta_info->AddField(label, properties);
    }

    return true;
}

bool CTableAnnotDataSource::CreateSnps(const STableLocation& fc,
                                       ICanceled* call)
{
    // Get User data from Annot Descriptor this is the Properties data
    // structure, where each entry is a combination of column number
    // and field properties, e.g.
    // Column.1  &xtype=StartPosition &one_based=true
    CAnnotdesc::TUser* column_meta_info = x_GetColumnMetaInfo();
    if (column_meta_info == NULL)
        return false;

    // Now get the actual columns (which have headers with descriptive
    // info as well - as much as we currently need for finding features
    // actually)
    if (!m_AnnotContainer->IsSetData()) {
        LOG_POST("Error - SeqTable not found in SeqAnnot");
        return false;
    }
    CSeq_annot::TData& table_container = m_AnnotContainer->SetData();

    if (!table_container.IsSeq_table()) {
        LOG_POST("Error - SeqAnnot does not contain a SeqTable");
        return false;
    }
    CSeq_table& table = table_container.SetSeq_table();
    CSeq_table::TColumns& cols = table.SetColumns();

    /// To make the table a valid source for features first need to set its type
    /// and subtype to indicate that the rows represent region features.
    table.SetFeat_type(CSeqFeatData_Base::e_Imp);
    table.SetFeat_subtype(CSeqFeatData::eSubtype_variation);

    const CUser_field& id_meta_info = column_meta_info->GetData()[fc.m_IdCol].GetObject();

    // Can only have one ID field if the table is a feature table.
    // Also, no comment fields may be of type int or real.  So since we are adding
    // all the feature fields (start/stop/strand/id) set any other numeric or id fields
    // to comments.
    for (size_t col_num=0; col_num<cols.size(); ++col_num) {
        // Can only have 1 id field in region feature so set others to comment
        CSeqTable_column_info& header =  cols[col_num]->SetHeader();
        if (header.GetField_id() == (int)CSeqTable_column_info::eField_id_location ||
            header.GetField_id() == (int)CSeqTable_column_info::eField_id_location_id ||
            header.GetField_id() == (int)CSeqTable_column_info::eField_id_location_gi ||
            header.GetField_id() == (int)CSeqTable_column_info::eField_id_id_local) {
                header.SetField_id(CSeqTable_column_info::eField_id_comment);
        }
        // Can't have Numeric fields other than start, stop length.  In table loader
        // integer fields not being used in locations have id type as comment and data
        // type as int. switch data type to text.
        else if (cols[col_num]->GetData().Which() == CSeqTable_multi_data_Base::e_Int &&
            header.GetField_id() == CSeqTable_column_info::eField_id_comment) {

                // Get copy current data for this integer field
                CSeqTable_multi_data_Base::TInt col_data = cols[col_num]->GetData().GetInt();
                // delete field data and set new type to string
                cols[col_num]->SetData().Select(CSeqTable_multi_data_Base::e_String, eDoResetVariant);
                // copy int data to new field, converting it to string
                for (size_t row=0; row<col_data.size(); ++row) {
                    string val;
                    try {
                        val = NStr::IntToString(col_data[row]);
                    }
                    catch (CException&) { }

                    cols[col_num]->SetData().SetString().push_back(val);
                }
                // done - we converted the int field to a string field
                // (becuase int fields can't be comment fields - in
                // region features)
        }
        else if (cols[col_num]->GetData().Which() == CSeqTable_multi_data_Base::e_Real &&
            header.GetField_id() == CSeqTable_column_info::eField_id_comment) {

                // Get copy current data for this integer field
                CSeqTable_multi_data_Base::TReal col_data = cols[col_num]->GetData().GetReal();
                // delete field data and set new type to string
                cols[col_num]->SetData().Select(CSeqTable_multi_data_Base::e_String, eDoResetVariant);
                // copy int data to new field, converting it to string
                for (size_t row=0; row<col_data.size(); ++row) {
                    string val;
                    try {
                        val = NStr::DoubleToString(col_data[row]);
                    }
                    catch (CException&) { }

                    cols[col_num]->SetData().SetString().push_back(val);
                }
                // done - we converted the real field to a string field
                // (becuase real fields can't be comment fields - in
                // region features)
        }
    }

    /// Chromosomes need to be re-mapped using the accession
    bool has_assembly = false;
    string assembly;

    string column_type = x_GetMetaInfoTag(id_meta_info.GetData().GetStr(), "xtype");
    if (CTableImportColumn::GetDataTypeFromString(column_type) ==
        CTableImportColumn::eRsid) {
        string meta_info;

        // Change the field name and type to match requirements for dbsnp field
        CSeqTable_column_info& header =  cols[fc.m_IdCol]->SetHeader();
        header.ResetField_id();
        header.SetField_name("D.dbSNP");

        if (id_meta_info.IsSetData() && id_meta_info.GetData().IsStr())
            meta_info = id_meta_info.GetData().GetStr();

        assembly = x_GetMetaInfoTag(meta_info, "genome_assembly");

        if (!assembly.empty() ) {
            has_assembly = true;
        }
    }

    // Add data-imp column which represents variations
    CRef< CSeqTable_column > variation_column;
    {
        CRef<CSeqTable_column_info> cinfo(new CSeqTable_column_info());

        cinfo->SetField_id(CSeqTable_column_info_Base::eField_id_data_imp_key);
        cinfo->SetTitle(string("Var"));

        variation_column.Reset(new CSeqTable_column());
        variation_column->SetHeader(*cinfo);

        CRef<CSeqTable_single_data> default_data(new CSeqTable_single_data());
        default_data->Select(CSeqTable_single_data::e_String);
        default_data->SetString("variation");
        variation_column->SetDefault(*default_data);
    }

    // Add id column for seq-id of molecule on which rsid is found
    CRef< CSeqTable_column > seqid_column;
    {
        CRef<CSeqTable_column_info> cinfo(new CSeqTable_column_info());

        cinfo->SetField_id(CSeqTable_column_info_Base::eField_id_location_id);
        if (fc.m_StrandCol != -1) {
            const CSeqTable_column_info& strand_header =
                cols[fc.m_StrandCol]->GetHeader();
            cinfo->SetTitle(strand_header.GetTitle());
        }
        else {
            cinfo->SetTitle(string("Seq-ID"));
        }

        seqid_column.Reset(new CSeqTable_column());
        seqid_column->SetHeader(*cinfo);
        CRef<CSeqTable_multi_data> data(new CSeqTable_multi_data());
        data->Select(CSeqTable_multi_data_Base::e_Id);
        seqid_column->SetData(data.GetNCObject());
    }

    CRef< CSeqTable_column > start_position_column;
    if (fc.m_StartPosCol == -1) {
        CRef<CSeqTable_column_info> cinfo(new CSeqTable_column_info());
        cinfo->SetTitle("Start Column");
        cinfo->SetField_id(CSeqTable_column_info_Base::eField_id_location_from);

        start_position_column.Reset(new CSeqTable_column());
        start_position_column->SetHeader(*cinfo);
        CRef<CSeqTable_multi_data> data(new CSeqTable_multi_data());
        data->Select(CSeqTable_multi_data_Base::e_Int);
        start_position_column->SetData(data.GetNCObject());
    }

    CRef< CSeqTable_column > replace_column1;
    {
        CRef<CSeqTable_column_info> cinfo(new CSeqTable_column_info());
        cinfo->SetTitle("Replace 1");
        cinfo->SetField_name("Q.replace");

        replace_column1.Reset(new CSeqTable_column());
        replace_column1->SetHeader(*cinfo);
        CRef<CSeqTable_multi_data> data(new CSeqTable_multi_data());
        data->Select(CSeqTable_multi_data_Base::e_Common_string);
        replace_column1->SetData(data.GetNCObject());
    }


    CRef< CSeqTable_column > replace_column2;
    {
        CRef<CSeqTable_column_info> cinfo(new CSeqTable_column_info());
        cinfo->SetTitle("Replace 2");
        cinfo->SetField_name("Q.replace");

        replace_column2.Reset(new CSeqTable_column());
        replace_column2->SetHeader(*cinfo);
        CRef<CSeqTable_multi_data> data(new CSeqTable_multi_data());
        data->Select(CSeqTable_multi_data_Base::e_Common_string);
        replace_column2->SetData(data.GetNCObject());
    }


    // If there are errors we can put them in an error column
    // at the end of conversion, if no errors have occured, the column
    // will not be added to the table. Columns with errors will also
    // be marked as disabled to keep the table valid.
    bool errors_occured = false;
    CRef< CSeqTable_column > error_column;
    {
        CRef<CSeqTable_column_info> cinfo(new CSeqTable_column_info());
        cinfo->SetTitle("Error Messages");
        cinfo->SetField_id(CSeqTable_column_info_Base::eField_id_comment);

        error_column.Reset(new CSeqTable_column());
        error_column->SetHeader(*cinfo);
        CRef<CSeqTable_multi_data> data(new CSeqTable_multi_data());
        data->Select(CSeqTable_multi_data_Base::e_String);
        error_column->SetData(data.GetNCObject());
    }

    CRef< CSeqTable_column > disabled_column;
    {
        CRef<CSeqTable_column_info> cinfo(new CSeqTable_column_info());
        cinfo->SetField_name("disabled");

        disabled_column.Reset(new CSeqTable_column());
        disabled_column->SetHeader(*cinfo);
        CSeqTable_single_data *sd = new CSeqTable_single_data();
        sd->SetBit(true);
        disabled_column->SetDefault(*sd);

        CSeqTable_sparse_index* si = new CSeqTable_sparse_index();
        si->Select(CSeqTable_sparse_index_Base::e_Indexes);
        disabled_column->SetSparse(*si);
    }


    if (call != NULL &&
        call->IsCanceled())
        return false;

    size_t rsid_search_size = 100;

    // Visit all rows in the table and add any columns needed to create a
    // complete feature.  If there are no columns to add/update, skip
    // this step.
    int err_count = 0;

    CRef<CObjectManager> om(CObjectManager::GetInstance());
    CRef<CScope> scope(new CScope(*om));
    scope->AddDefaults();

    size_t row = 0;
    size_t search_row = 0;
    size_t current_size = 0;
    size_t row_count = cols[fc.m_IdCol]->GetData().GetSize();
    while (search_row < row_count) {

        // could resize rsids to begin with (rsids have predictable lengths...)
        string rsids = "";
        for (current_size=0; search_row < row_count && current_size<rsid_search_size; ++current_size, ++search_row) {
            rsids += *cols[fc.m_IdCol]->GetStringPtr(search_row);
            rsids += ",";
        }
        // remove last comma
        rsids = rsids.substr(0, rsids.length()-1);

        // Query for current set of rsids
        NSNPWebServices::TSNPSearchCompoundResultList SNPSearchResultList;
        try {
            // assembly may be blank.
            NSNPWebServices::Search(rsids, assembly, SNPSearchResultList);
        }
        catch (CException& e) {
            LOG_POST("Exception while searching SNP database: " + e.GetMsg());
            // If there is an error getting a group of ids continually reduce the batch
            // size until there is only 1 id retrieved and then if there is still an
            // error process it as an error
            if (rsid_search_size > 2) {
                search_row = row;
                rsid_search_size = rsid_search_size/2;
                continue;
            }
        }
        catch(...) {
            LOG_POST("Unidentified exception while searching SNP database");
            // If there is an error getting a group of ids continually reduce the batch
            // size until there is only 1 id retrieved and then if there is still an
            // error process it as an error
            if (rsid_search_size > 2) {
                search_row = row;
                rsid_search_size = rsid_search_size/2;
                continue;
            }
        }

        // process the current set of rows. Results in result list should be in same order
        // as the rows, but we will search the result set if there is a mismatch.
        NSNPWebServices::TSNPSearchCompoundResultList::iterator current_iter = SNPSearchResultList.begin();

        for (; row < search_row; ++row) {

            int from = 0;
            string logstr;
            string errstr = "";

            bool found = false;
            string rsid = *cols[fc.m_IdCol]->GetStringPtr(row);

            // See if current snp query result matches current row. If not, search for current
            // row rsid in entire result set. (usually they should match)
            NSNPWebServices::TSNPSearchCompoundResultList::iterator iter = current_iter;
            ++current_iter;

            if (iter->first != rsid) {
                CompareSNPResults pred(rsid);
                iter = std::find_if(SNPSearchResultList.begin(), SNPSearchResultList.end(), pred);
            }

            if (iter == SNPSearchResultList.end()) {
                x_LogErr("No matches for given rsid", errstr, err_count, row);
            }
            else if ((*iter).second.size() == 0) {
                x_LogErr("No matches for rsid and assembly: " + assembly,
                    errstr, err_count, row);
            }
            else {
                const NSNPWebServices::TSNPSearchResultList::value_type& first_result = (*iter).second.front();

                // CVariation used as a search result can have one and only one placement
                if (first_result->CanGetPlacements() == 0) {
                    x_LogErr("Unexpected absence of placements in SNP Search Result", errstr, err_count, row);
                }
                else {
                    const CVariation::TPlacements& placements(first_result->GetPlacements());
                    if (placements.size() == 0) {
                        x_LogErr("Unexpected number of placements (0) in SNP Search Result", errstr, err_count, row);
                    }
                    else {
                        const CSeq_id* id = placements.front()->GetLoc().GetId();
                        CRef<CSeq_id> rid(new CSeq_id());
                        rid->Assign(*id);

                        seqid_column->SetData().SetId().push_back(rid);

                        from = placements.front()->GetLoc().GetStart(eExtreme_Positional);
                        start_position_column->SetData().SetInt().push_back(from);

                        found = true;
                    }
                }
            }

            if (fc.m_GenotypeCol != -1) {
                string geno_str = *cols[fc.m_GenotypeCol]->GetStringPtr(row);
                string val1 = "-";
                string val2 = "-";

                if (geno_str.length() >= 1)
                    val1 = geno_str[0];
                if (geno_str.length() >= 2)
                    val2 = geno_str[0];

                {
                    CCommonString_table_Base::TStrings& strs = replace_column1->SetData().SetCommon_string().SetStrings();
                    CCommonString_table_Base::TStrings::iterator iter = std::find(strs.begin(), strs.end(), val1);
                    if (iter != strs.end()) {
                        size_t idx = iter-strs.begin();
                        replace_column1->SetData().SetCommon_string().SetIndexes().push_back(idx);
                    }
                    else {
                        strs.push_back(val1);
                        replace_column1->SetData().SetCommon_string().SetIndexes().push_back(strs.size()-1);
                    }
                }

                {
                    CCommonString_table_Base::TStrings& strs = replace_column2->SetData().SetCommon_string().SetStrings();
                    CCommonString_table_Base::TStrings::iterator iter = std::find(strs.begin(), strs.end(), val2);
                    if (iter != strs.end()) {
                        size_t idx = iter-strs.begin();
                        replace_column2->SetData().SetCommon_string().SetIndexes().push_back(idx);
                    }
                    else {
                        strs.push_back(val2);
                        replace_column2->SetData().SetCommon_string().SetIndexes().push_back(strs.size()-1);
                    }
                }
            }

            if (!found) {
                CRef<CSeq_id> empty_id(new CSeq_id());

                seqid_column->SetData().SetId().push_back(empty_id);
                start_position_column->SetData().SetInt().push_back(0);
            }

            // will only add the column at the end if 1 or more errors
            // occured.
            if (errstr != "") {
                errors_occured = true;
                disabled_column->SetSparse().SetIndexes().push_back(row);

                // our viewer doesn't currently handle sparse indices with data
                //error_column->SetSparse().SetIndexes().push_back(row);
            }
            // Will add blanks too for cols w/o errors
            error_column->SetData().SetString().push_back(errstr);

            // if call is asynchronous, return if/when cancelled.
            if (call != NULL &&
                call->IsCanceled())
                return false;
        }
    }


    /// Add any new columns that were required to make a valid feature:
    // (strand, data region , stop position and ID)
    if (!variation_column.IsNull()) {
        table.SetColumns().push_back(variation_column);
        string label = variation_column->GetHeader().GetTitle();
        string properties = string("&xtype=") +
            CTableImportColumn::GetStringFromDataType(CTableImportColumn::eUnspecifiedText) +
            " &derived_field=true";
        column_meta_info->AddField(label, properties);
    }

    if (!seqid_column.IsNull()) {
        table.SetColumns().push_back(seqid_column);
        string label = seqid_column->GetHeader().GetTitle();
        string properties = string("&xtype=") +
            CTableImportColumn::GetStringFromDataType(CTableImportColumn::eLocationID) +
            " &derived_field=true";
        column_meta_info->AddField(label, properties);
    }

    if (!start_position_column.IsNull()) {
        table.SetColumns().push_back(start_position_column);

        string label = start_position_column->GetHeader().GetTitle();
        string properties = string("&xtype=") +
            CTableImportColumn::GetStringFromDataType(CTableImportColumn::eStartPosition) +
            " &derived_field=true";
        column_meta_info->AddField(label, properties);
    }


    if (!replace_column1.IsNull()) {
        table.SetColumns().push_back(replace_column1);

        string label = replace_column1->GetHeader().GetTitle();
        string properties = string("&xtype=") +
            CTableImportColumn::GetStringFromDataType(CTableImportColumn::eGenotype) +
            " &derived_field=true";
        column_meta_info->AddField(label, properties);
    }

    if (!replace_column1.IsNull()) {
        table.SetColumns().push_back(replace_column1);

        string label = replace_column1->GetHeader().GetTitle();
        string properties = string("&xtype=") +
            CTableImportColumn::GetStringFromDataType(CTableImportColumn::eGenotype) +
            " &derived_field=true";
        column_meta_info->AddField(label, properties);
    }

    if (errors_occured) {
        table.SetColumns().push_back(error_column);
        string label = error_column->GetHeader().GetTitle();
        string properties = string("&xtype=") +
            CTableImportColumn::GetStringFromDataType(CTableImportColumn::eUnspecifiedText) +
            " &derived_field=true";
        column_meta_info->AddField(label, properties);

        table.SetColumns().push_back(disabled_column);
        label = "disabled";
        properties = string("&xtype=disabled_indices") +
            //CTableImportColumn::GetStringFromDataType(CTableImportColumn::eUndefined) +
            " &derived_field=true";
        column_meta_info->AddField(label, properties);
    }

    // Container description must be SNP to make snp features visible in Genome Workbench
    CAnnotdesc* adesc = new CAnnotdesc();
    adesc->SetName() = "SNP";
    m_AnnotContainer->SetDesc().Set().push_back(CRef<CAnnotdesc>(adesc));


    return true;
}

void CTableAnnotDataSource::RemoveSeqLocs()
{
    if (!m_AnnotContainer->IsSetData()) {
        LOG_POST(Error <<  "SeqTable not found in SeqAnnot");
        return;
    }
    CSeq_annot::TData& table_container = m_AnnotContainer->SetData();

    if (!table_container.IsSeq_table()) {
        LOG_POST(Error << "SeqAnnot does not contain a SeqTable");
        return;
    }
    CSeq_table& table = table_container.SetSeq_table();
    CSeq_table::TColumns& cols = table.SetColumns();


    CAnnotdesc::TUser* column_meta_info = x_GetColumnMetaInfo();
    if (column_meta_info == NULL)
        return;

    // Iterate over columns looking for seq-loc fields and erase them.
    for (int idx=(int)cols.size()-1; idx>=0; --idx) {
        const CSeqTable_column_info& header = cols[(size_t)idx]->GetHeader();
        if (header.GetField_id() == CSeqTable_column_info_Base::eField_id_location)  {

            // delete the column from the vector of columns.  Since we are
            // iterating backwards, it will not effect our iteration
            table.SetColumns().erase(table.SetColumns().begin() + idx);
            column_meta_info->SetData().erase(column_meta_info->SetData().begin() + idx);
        }
    }
}

void CTableAnnotDataSource::WriteAsn(const string& fname)
{
    if (m_AnnotContainer.IsNull())
        return;

    ofstream ofs(fname.c_str());

    try {
        ofs << MSerial_AsnText << *m_AnnotContainer;
    }
    catch (CException& c) {
        LOG_POST(Error <<  "Error writing asn: " << c.GetMsg());
    }
}

ENa_strand CTableAnnotDataSource::x_GetStrand(string strand) {

    if (strand == "+" || NStr::ToLower(strand) == "pos")
        return eNa_strand_plus;
    else if (strand == "-" || NStr::ToLower(strand) == "neg")
        return eNa_strand_minus;
    else {
        return eNa_strand_unknown;
    }
}

string CTableAnnotDataSource::x_GetMetaInfoTag(const string& meta_string,
                                               const string& tag_name)
{
    string value;

    string tag = "&" + tag_name + "=";

    size_t tag_idx = meta_string.find(tag);
    if (tag_idx != string::npos) {
        // End of value is the start of the next value qualifier (this allows
        // embedded blanks in values)
        size_t tag_end_idx = meta_string.find_first_of("&", tag_idx+1);
        size_t start_idx = tag_idx + string(tag).length();

        if (tag_end_idx == string::npos) {
            value = meta_string.substr(start_idx, meta_string.length()-start_idx);
        }
        else {
            value = meta_string.substr(start_idx, tag_end_idx-start_idx);
        }

        value = NStr::TruncateSpaces(value);
    }

    return value;
}

bool CTableAnnotDataSource::x_UpdateMetaInfoTag(string& meta_string,
                                                const string& tag_name,
                                                const string& new_value)
{
    string value;

    string tag = "&" + tag_name + "=";

    size_t tag_idx = meta_string.find(tag);
    if (tag_idx != string::npos) {
        // End of value is the start of the next value qualifier (this allows
        // embedded blanks in values)
        size_t tag_end_idx = meta_string.find_first_of("&", tag_idx+1);
        size_t start_idx = tag_idx + string(tag).length();

        string trim_value = NStr::TruncateSpaces(new_value);

        if (tag_end_idx == string::npos) {
            meta_string.erase(start_idx, meta_string.length()-start_idx);
            meta_string += trim_value;
        }
        else {
            meta_string.erase(start_idx, tag_end_idx-start_idx);
            // insert value in previous value position and end with a blank
            meta_string.insert(start_idx, trim_value + " ");
        }

        return true;
    }

    return false;
}

CAnnotdesc::TUser*
CTableAnnotDataSource::x_GetColumnMetaInfo()
{
    if (!m_AnnotContainer->CanGetDesc()) {
        LOG_POST("Error - annot-desc not initialized.");
        return NULL;
    }

    CSeq_annot::TDesc& annot_desc = m_AnnotContainer->SetDesc();
    if (!annot_desc.CanGet()) {
        LOG_POST("Error - annot-desc list not initialized.");
        return NULL;
    }


    CAnnot_descr_Base::Tdata& desc_list_data = annot_desc.Set();
    CAnnot_descr_Base::Tdata::iterator tditer = desc_list_data.begin();

    for (; tditer!=desc_list_data.end(); ++tditer) {
        if ( (*tditer)->IsUser() )
            break;
    }

    if (tditer == desc_list_data.end()) {
        LOG_POST("Error - User data not found in annot-desc");
        return NULL;
    }

    // Get User data from Annot Descriptor this is the Properties data
    // structure, where each entry is a combination of column number
    // and field properties, e.g.
    // Column.1  &xtype=StartPosition &one_based=true
    CAnnotdesc::TUser& column_meta_info = (*tditer)->SetUser();

    return &column_meta_info;
}

END_NCBI_SCOPE
