/*  $Id: macro_biodata_iter.cpp 45050 2020-05-18 17:58:56Z asztalos $
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
 * Authors:  Andrea Asztalos
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbimisc.hpp>

#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/util/seq_loc_util.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/valid/Comment_rule.hpp>
#include <objects/seq/seq_macros.hpp>
#include <objects/seqset/seqset_macros.hpp>
#include <util/line_reader.hpp>

#include <objtools/writers/write_util.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_change_align.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_change_seq_entry.hpp>
#include <gui/objutils/cmd_change_bioseq_set.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <gui/objutils/cmd_del_seq_align.hpp>
#include <gui/objutils/cmd_del_bioseq_set.hpp>
#include <gui/objutils/cmd_del_bioseq.hpp>
#include <gui/objutils/macro_util.hpp>
#include <gui/objutils/macro_field_resolve.hpp>
#include <gui/objutils/macro_biodata_iter.hpp>
#include <gui/objutils/macro_ex.hpp>



/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)
USING_SCOPE(objects);


bool IMacroBioDataIter::GetChoiceType(const string& field_name, string& choice_type) const
{
    CMQueryNodeValue::TObs nodes;
    CObjectInfo oi = m_CurIterOI;

    if (!GetFieldsByName(&nodes, oi, field_name) || nodes.size() != 1)
        return false;

    oi = nodes.front().field;
    if (oi.GetTypeFamily() == eTypeFamilyPointer) {
        oi = oi.GetPointedObject();
    }

    if (oi.GetTypeFamily() != eTypeFamilyChoice)
        return false;

    CObjectTypeInfoCV cv = oi.GetCurrentChoiceVariant();
    choice_type = cv.GetAlias();
    return true;
}


void IMacroBioDataIter::RunCommand(CRef<CCmdComposite> cmd, CMacroCmdComposite* cmd_composite)
{
    _ASSERT(cmd_composite);
    if (cmd) {
        cmd->Execute();
        cmd_composite->AddCommand(*cmd);
    }
}

void IMacroBioDataIter::x_Init()
{
    if (!IsEnd()) {
        x_SetDefaultObjectInfo();
        m_DeleteObject = false;
        m_Modified = false;
    }
}

void IMacroBioDataIter::SetTableToApply(const string& file_name, size_t col,
    const string& delimiter, bool merge_del,
    bool split_firstcol, bool convert_multi, bool merge_firstcols)
{
    if (file_name.empty()) {
        LOG_POST(Error << "Applying table macro has empty table name");
        return;
    }
    // it can only handle one table at a time
    if (m_TableToApply.m_Name.empty()) {
        m_TableToApply.m_Name = file_name;
        m_TableToApply.m_Delimiter = delimiter;
        if (m_TableToApply.m_Delimiter == "\\t")
            m_TableToApply.m_Delimiter = "\t";
        m_TableToApply.m_SplitFlags = (merge_del) ? NStr::fSplit_MergeDelimiters : 0;
        m_TableToApply.m_SplitFirstCol = split_firstcol;
        m_TableToApply.m_ConvertMulti = convert_multi;
        m_TableToApply.m_MergeFirstCols = merge_firstcols;

        bool set = m_TableToApply.SetTableToApply(file_name, col);
        if (!set) {
            NCBI_THROW(CException, eUnknown, "Unknown error occurred while reading the table");
        }
    }
}

const string& IMacroBioDataIter::GetTableName() const
{
    return m_TableToApply.m_Name;
}

string IMacroBioDataIter::GetValueFromTable(size_t col)
{
    string value;
    bool found = m_TableToApply.GetValueFromTable(col, value);
    if (!found) {
        NCBI_THROW(CException, eUnknown, "Value from table in column "
            + NStr::NumericToString(col) + " was not found");
    }
    return value;
}

bool IMacroBioDataIter::STableToApply::SetTableToApply(const string& file_name, size_t col)
{
    x_InitAndCheck(file_name, col);

    CRef<ILineReader> line_reader(ILineReader::New(m_Name));

    string line;
    unsigned nr = 0;
    while (!line_reader->AtEOF()) {
        line_reader->ReadLine();
        line = line_reader->GetCurrentLine();
        if (line.empty())
            continue;

        if (m_ConvertMulti) {
            NMacroUtil::ConvertMultiSpaces(line, m_Delimiter);
        }
        vector<string> cols;
        NStr::SplitByPattern(line, m_Delimiter, cols, m_SplitFlags);

        auto token_it = find_if(cols.begin(), cols.end(), [](const string& elem) { return !elem.empty(); });
        if (token_it == cols.end()) {
            continue;
        }

        string value = cols[col];
        if (m_MergeFirstCols && col == 0) {
            value = cols[0] + "/" + cols[1];
            cols.erase(cols.begin() + 1);
        }

        if (m_SplitFirstCol) {
            if (col < 2) {
                string first, second;
                NStr::SplitInTwo(cols[0], " ", first, second);
                if (col == 0) {
                    value = first;
                }
                else if (col == 1) {
                    value = second;
                }
            }
            else {
                value = cols[col - 1];
            }
        }

        auto it = m_Index.find(value);
        if (it == m_Index.end()) {
            m_Index.emplace(value, nr);
            m_State.emplace_back(value, false);
        }
        else {
            auto ident_it = m_IdenticalItems.find(value);
            if (ident_it != m_IdenticalItems.end()) {
                ident_it->second++;
            }
            else {
                m_IdenticalItems.emplace(value, 2);
            }
        }
        ++nr;
    }

    if (!m_IdenticalItems.empty()) {
        string msg = "Table contains entries that appear multiple times:\n";
        for (auto& it : m_IdenticalItems) {
            msg += "'" + it.first + "' " + NStr::IntToString(it.second) + " times\n";
        }
        NCBI_THROW(CException, eUnknown, msg);
    }

    return (nr == m_Index.size());
}

void IMacroBioDataIter::STableToApply::x_InitAndCheck(const string& file_name, size_t col)
{
    m_Index.clear();
    m_IdenticalItems.clear();

    CRef<ILineReader> line_reader(ILineReader::New(m_Name));
    if (line_reader.Empty()) {
        ThrowCMacroExecException(DIAG_COMPILE_INFO, CMacroExecException::eFileNotFound,
            "Could not open table file: '" + m_Name, nullptr);
    }

    // read the first line, to check that the specified column number is correct
    if (!line_reader->AtEOF()) {
        line_reader->ReadLine();
        string line = line_reader->GetCurrentLine();
        if (m_ConvertMulti) {
            NMacroUtil::ConvertMultiSpaces(line, m_Delimiter);
        }

        vector<string> cols;
        NStr::SplitByPattern(line, m_Delimiter, cols, m_SplitFlags);
        _ASSERT(!cols.empty());
        m_NrCols = cols.size();
        if (m_MergeFirstCols)
            m_NrCols--;

        if (m_SplitFirstCol) {
            string first, second;
            NStr::SplitInTwo(cols[0], " ", first, second);
            if (!first.empty() && !second.empty())
                m_NrCols++;
        }
    }

    if (col >= m_NrCols) {
        NCBI_THROW(CException, eUnknown, "Table has only " + NStr::NumericToString(m_NrCols) + " columns");
    }
}

bool IMacroBioDataIter::STableToApply::GetValueFromTable(size_t col, string& value)
{
    size_t pos = m_CurrentPos->second;
    _ASSERT(col < m_NrCols);

    auto col_it = find_if(m_TableValues.begin(), m_TableValues.end(),
        [&col](const pair<size_t, TTableValues>& elem) { return elem.first == col; });

    if (col_it == m_TableValues.end()) {
        x_CacheColFromTable(col);
    }

    auto it = m_TableValues.begin();
    bool found = false;
    for (; !found && it != m_TableValues.end(); ++it) {
        if (col == it->first) {
            auto& values = it->second;
            value = values[pos];
            found = true;
        }
    }

    return found;
}

bool IMacroBioDataIter::STableToApply::x_CacheColFromTable(size_t col)
{
    if (col >= m_NrCols) {
        NCBI_THROW(CException, eUnknown, "Table has only " + NStr::NumericToString(m_NrCols) + " columns");
    }

    CRef<ILineReader> line_reader(ILineReader::New(m_Name));

    STableToApply::TTableValues values;  // stores the values from the col(th) column
    string line;

    while (!line_reader->AtEOF()) {
        line_reader->ReadLine();
        line = line_reader->GetCurrentLine();
        if (line.empty())
            continue;

        if (m_ConvertMulti) {
            NMacroUtil::ConvertMultiSpaces(line, m_Delimiter);
        }

        vector<string> cols;
        NStr::SplitByPattern(line, m_Delimiter, cols, m_SplitFlags);

        if (m_MergeFirstCols) {
            cols[0] = cols[0] + "/" + cols[1];
            cols.erase(cols.begin() + 1);
        }

        if (m_SplitFirstCol) {
            if (col < 2) {
                string first, second;
                NStr::SplitInTwo(cols[0], " ", first, second);
                if (col == 0) {
                    values.push_back(first);
                }
                else if (col == 1) {
                    values.push_back(second);
                }
            }
            else {
                values.push_back(cols[col - 1]);
            }
        }
        else {
            values.push_back(cols[col]);
        }
    }

    m_TableValues.push_back(make_pair(col, values));
    return (line_reader->GetLineNumber() == values.size());
}

bool IMacroBioDataIter::FindInTable(const CMQueryNodeValue::TObs& objs)
{
    bool found = false;
    vector<string> values_from_data;
    for (auto it = objs.begin(); it != objs.end() && !found; ++it) {
        CMQueryNodeValue::TObs objects;
        NMacroUtil::GetPrimitiveObjectInfos(objects, *it);
        string value;
        for (auto iter = objects.begin(); iter != objects.end() && !found; ++iter) {
            CObjectInfo oi = iter->field;
            if (oi.GetPrimitiveValueType() == ePrimitiveValueString) {
                value = oi.GetPrimitiveValueString();
            }
            else if (oi.GetPrimitiveValueType() == ePrimitiveValueInteger) {
                Int8 nr = oi.GetPrimitiveValueInt8();
                value = NStr::Int8ToString(nr);
            }

            else if (oi.GetPrimitiveValueType() == ePrimitiveValueEnum) {
                try {
                    value = oi.GetPrimitiveValueString();
                }
                catch (const CException&) {
                    value = NStr::NumericToString(oi.GetPrimitiveValueInt4());
                }
            }
            auto found_it = m_TableToApply.m_Index.find(value);
            if (found_it != m_TableToApply.m_Index.end()) {
                m_TableToApply.m_CurrentPos = found_it;
                m_TableToApply.m_State[m_TableToApply.m_CurrentPos->second].second = true;
                found = true;
            }
            else {
                values_from_data.push_back(value);
            }
        }
    }

    if (!values_from_data.empty()) {
        string msg = "These values were not found in the table:\n";
        for (auto& it : values_from_data) {
            msg += it + "\n";
        }
        LOG_POST(Info << msg); // add as a function report
    }

    return found;
}

bool IMacroBioDataIter::FindInTable(const vector<string>& strs)
{
    bool found = false;
    for (auto&& it : strs) {
        auto found_it = m_TableToApply.m_Index.find(it);
        if (found_it != m_TableToApply.m_Index.end()) {
            m_TableToApply.m_CurrentPos = found_it;
            m_TableToApply.m_State[m_TableToApply.m_CurrentPos->second].second = true;
            found = true;
            break;
        }
    }
    return found;
}

string IMacroBioDataIter::GetReport()
{
    string report;
    if (!m_TableToApply.m_Name.empty()) {
        report = "\nThere were NO matches for the following values present in the table:";
        for (auto& it : m_TableToApply.m_State) {
            if (!it.second) {
                report += "\n" + it.first;
            }
        }
        if (NStr::EndsWith(report, "in the table:"))
            report.resize(0);
    }
    return report;
}

//////////////////////////////////////////////////////////////////////////////
/// CMacroBioData_FeatIterBase
///
CMacroBioData_FeatIterBase::CMacroBioData_FeatIterBase(const CSeq_entry_Handle& entry,
    const SAnnotSelector& sel)
    : IMacroBioDataIter(entry), m_FeatIter(entry, sel)
{
}

CMacroBioData_FeatIterBase::CMacroBioData_FeatIterBase(
    const CBioseq_Handle& bsh, const SAnnotSelector& sel, const CSeq_loc& loc)
    : IMacroBioDataIter(bsh.GetSeq_entry_Handle()), m_FeatIter(bsh.GetScope(), loc, sel)
{
}

IMacroBioDataIter& CMacroBioData_FeatIterBase::Begin(void)
{
    m_FeatIter.Rewind();
    if (m_FeatIter) {
        m_FirstFeat = m_FeatIter->GetOriginalSeq_feat();
    }
    x_Init();
    return *this;
}

IMacroBioDataIter& CMacroBioData_FeatIterBase::Next(void)
{
    ++m_FeatIter;
    x_Init();
    return *this;
}

bool CMacroBioData_FeatIterBase::IsEnd(void) const
{
    if (m_FeatIter) {
        return false;
    }
    return true;
}

bool CMacroBioData_FeatIterBase::IsBegin(void)
{
    return (m_FirstFeat == m_FeatIter->GetOriginalSeq_feat());
}

size_t CMacroBioData_FeatIterBase::GetCount() const
{
    return m_FeatIter.GetSize();
}


SConstScopedObject CMacroBioData_FeatIterBase::GetScopedObject() const
{
    SConstScopedObject sobject;

    CConstRef<CSeq_feat> seqfeat = m_FeatIter->GetSeq_feat();
    sobject.object.Reset(seqfeat.GetPointer());

    CRef<CScope> pscope(&(m_FeatIter->GetScope()));
    sobject.scope = pscope;

    return sobject;
}


string CMacroBioData_FeatIterBase::GetBestDescr() const
{
    string subtype = CSeqFeatData::SubtypeValueToName(m_FeatIter->GetData().GetSubtype());

    CConstRef<CSeq_feat> seqfeat = m_FeatIter->GetSeq_feat();
    CSeq_loc loc;
    loc.Assign(seqfeat->GetLocation());
    sequence::ChangeSeqLocId(&loc, true, &(m_FeatIter->GetScope()));

    string label;
    loc.GetLabel(&label);
    subtype.append(" " + label);
    return subtype;
}


void CMacroBioData_FeatIterBase::BuildEditedObject()
{
    m_OrigFeat.Reset(&(*m_FeatIter->GetSeq_feat()));
    m_EditedFeat.Reset(new CSeq_feat);
    m_EditedFeat->Assign(*m_OrigFeat);

    CObjectInfo oi(m_EditedFeat, m_EditedFeat->GetThisTypeInfo());
    m_CurIterOI = oi;
}

void CMacroBioData_FeatIterBase::RunEditCommand(CMacroCmdComposite* cmd_composite)
{
    _ASSERT(m_OrigFeat);

    CSeq_feat_Handle orig_fh = m_Seh.GetScope().GetSeq_featHandle(*(m_OrigFeat.GetPointerOrNull()));
    CRef<CCmdChangeSeq_feat> cmd(new CCmdChangeSeq_feat(orig_fh, *m_EditedFeat.GetNCPointerOrNull()));
    cmd->Execute();
    cmd_composite->AddCommand(*cmd);
}

void CMacroBioData_FeatIterBase::SetToDelete(bool intent)
{
    m_DeleteObject = intent;
    if (IsBegin()) {
        m_ProductToCDS.clear();
        GetProductToCDSMap(m_Seh.GetScope(), m_ProductToCDS);
    }
}

void CMacroBioData_FeatIterBase::RunDeleteCommand(CMacroCmdComposite* cmd_composite)
{
    CSeq_feat_Handle fh = m_FeatIter->GetSeq_feat_Handle();
    bool remove_proteins(true); // this should be changed to TRUE
    CRef<CCmdComposite> del_cmd = GetDeleteFeatureCommand(fh, remove_proteins, m_ProductToCDS);
    del_cmd->Execute();
    cmd_composite->AddCommand(*del_cmd);

    Next();
}

CBioseq_Handle CMacroBioData_FeatIterBase::GetBioseqHandle() const
{
    try {
        return GetBioseqForSeqFeat(m_FeatIter->GetOriginalFeature(), m_Seh.GetScope());
    }
    catch (const CException& err)
    {
        NCBI_RETHROW2(err, CMacroDataException, CException::eUnknown,
            "The feature iterator failed to obtain a bioseq handle.",
            CConstRef<CSerialObject>(&(m_FeatIter->GetLocation())));
    }
}

void CMacroBioData_FeatIterBase::x_SetDefaultObjectInfo()
{
    const CSeq_feat* const_seq_feat = &(*m_FeatIter->GetSeq_feat());
    CSeq_feat* seq_feat = const_cast<CSeq_feat*>(const_seq_feat);
    CObjectInfo oi(seq_feat, seq_feat->GetThisTypeInfo());
    m_CurIterOI = oi;
}

void CMacroBioData_FeatIterBase::SetCreateGeneralIdFlag()
{
    m_CreateGeneralId = false;
    CScope::TTSE_Handles vec_tse;
    m_Seh.GetScope().GetAllTSEs(vec_tse, CScope::eAllTSEs);
    if (!vec_tse.empty()) {
        m_CreateGeneralId = edit::IsGeneralIdProtPresent(vec_tse.front());
    }
}

//////////////////////////////////////////////////////////////////////////////
/// CMacroBioData_FeatIntervalIter
///
CMacroBioData_FeatIntervalIter::CMacroBioData_FeatIntervalIter(const CBioseq_Handle& bsh,
    const SAnnotSelector& sel, const SFeatInterval& feat_interval)
    : IMacroBioDataIter(bsh.GetSeq_entry_Handle()), m_FeatInterval(feat_interval)
{
    CRef<CSeq_loc> loc = bsh.GetRangeSeq_loc(m_FeatInterval.m_Range.GetFrom(), m_FeatInterval.m_Range.GetTo());
    if (loc) {
        CFeat_CI feat_it(bsh.GetScope(), *loc, sel);
        m_FeatIter = feat_it;
    }
}

IMacroBioDataIter& CMacroBioData_FeatIntervalIter::Begin(void)
{
    m_FeatIter.Rewind();
    x_StepToNextFeat();
    if (m_FeatIter) {
        m_FirstFeat = m_FeatIter->GetOriginalSeq_feat();
    }
    x_Init();
    return *this;
}

IMacroBioDataIter& CMacroBioData_FeatIntervalIter::Next(void)
{
    ++m_FeatIter;
    x_StepToNextFeat();
    x_Init();
    return *this;
}

bool CMacroBioData_FeatIntervalIter::IsEnd(void) const
{
    if (m_FeatIter) {
        return false;
    }
    return true;
}

bool CMacroBioData_FeatIntervalIter::IsBegin(void)
{
    return (m_FirstFeat == m_FeatIter->GetOriginalSeq_feat());
}

void CMacroBioData_FeatIntervalIter::x_StepToNextFeat()
{
    if ((m_FeatInterval.left_closed == m_FeatInterval.right_closed) &&
        (m_FeatInterval.left_closed == true)) {
        // do not adjust the iterator
        return;
    }

    auto range = m_FeatInterval.m_Range;
    bool left_closed = m_FeatInterval.left_closed;
    bool right_closed = m_FeatInterval.right_closed;

    auto good_feat = [&range, left_closed, right_closed](const CSeq_loc& feat_loc)
    {
        auto feat_start = feat_loc.GetStart(eExtreme_Positional);
        auto feat_stop = feat_loc.GetStop(eExtreme_Positional);
        bool good = true; // assume that it belongs to this interval

        if (left_closed == false) {
            if (feat_start <= range.GetFrom()) {
                good = false;
            }
        }
        if (right_closed == false) {
            if (feat_stop >= range.GetTo()) {
                good = false;
            }
        }
        return good;
    };

    while (m_FeatIter && !good_feat(m_FeatIter->GetOriginalFeature().GetLocation())) {
        ++m_FeatIter;
    }
}


SConstScopedObject CMacroBioData_FeatIntervalIter::GetScopedObject() const
{
    SConstScopedObject sobject;

    CConstRef<CSeq_feat> seqfeat = m_FeatIter->GetSeq_feat();
    sobject.object.Reset(seqfeat.GetPointer());

    CRef<CScope> pscope(&(m_FeatIter->GetScope()));
    sobject.scope = pscope;

    return sobject;
}

string CMacroBioData_FeatIntervalIter::GetBestDescr() const
{
    string subtype = CSeqFeatData::SubtypeValueToName(m_FeatIter->GetData().GetSubtype());

    CConstRef<CSeq_feat> seqfeat = m_FeatIter->GetSeq_feat();
    CSeq_loc loc;
    loc.Assign(seqfeat->GetLocation());
    sequence::ChangeSeqLocId(&loc, true, &(m_FeatIter->GetScope()));

    string label;
    loc.GetLabel(&label);
    subtype.append(" " + label);
    return subtype;
}

void CMacroBioData_FeatIntervalIter::BuildEditedObject()
{
    m_OrigFeat.Reset(&(*m_FeatIter->GetSeq_feat()));
    m_EditedFeat.Reset(new CSeq_feat);
    m_EditedFeat->Assign(*m_OrigFeat);

    CObjectInfo oi(m_EditedFeat, m_EditedFeat->GetThisTypeInfo());
    m_CurIterOI = oi;
}

CBioseq_Handle CMacroBioData_FeatIntervalIter::GetBioseqHandle() const
{
    try {
        return GetBioseqForSeqFeat(m_FeatIter->GetOriginalFeature(), m_Seh.GetScope());
    }
    catch (const CException& err)
    {
        NCBI_RETHROW2(err, CMacroDataException, CException::eUnknown,
            "The feature iterator failed to obtain a bioseq handle.",
            CConstRef<CSerialObject>(&(m_FeatIter->GetLocation())));
    }
}

void CMacroBioData_FeatIntervalIter::x_SetDefaultObjectInfo()
{
    const CSeq_feat* const_seq_feat = &(*m_FeatIter->GetSeq_feat());
    CSeq_feat* seq_feat = const_cast<CSeq_feat*>(const_seq_feat);
    CObjectInfo oi(seq_feat, seq_feat->GetThisTypeInfo());
    m_CurIterOI = oi;
}

//////////////////////////////////////////////////////////////////////////////
/// CMacroBioData_BioSourceIter
///
CMacroBioData_BioSourceIter::CMacroBioData_BioSourceIter(const CSeq_entry_Handle& entry)
    : IMacroBioDataIter(entry), m_CollDateType(eCollDateType_NotSet)
{
}

IMacroBioDataIter& CMacroBioData_BioSourceIter::Begin(void)
{
    m_SeqIter = CBioseq_CI(m_Seh, CSeq_inst::eMol_na);
    m_OrigSource = ConstRef(x_GetBioSource());
    while (m_OrigSource.IsNull() && m_SeqIter) {
        m_SrcFeatIter = CFeat_CI(*m_SeqIter, SAnnotSelector(CSeqFeatData::eSubtype_biosrc));
        if (m_SrcFeatIter) {
            m_OrigSource = ConstRef(&m_SrcFeatIter->GetData().GetBiosrc());
        }

        if (m_OrigSource.IsNull()) {
            if (++m_SeqIter) {
                m_OrigSource = ConstRef(x_GetBioSource());
            }
        }
    }

    m_FirstSource = m_OrigSource;

    x_Init();
    return *this;
}

IMacroBioDataIter& CMacroBioData_BioSourceIter::Next(void)
{
    m_OrigSource.Reset();
    while (m_OrigSource.IsNull() && m_SeqIter) {
        if (m_SrcFeatIter) {
            if (++m_SrcFeatIter) {
                m_OrigSource = ConstRef(&m_SrcFeatIter->GetData().GetBiosrc());
            }
        }
        else {
            m_SrcFeatIter = CFeat_CI(*m_SeqIter, SAnnotSelector(CSeqFeatData::eSubtype_biosrc));
            if (m_SrcFeatIter) {
                m_OrigSource = ConstRef(&m_SrcFeatIter->GetData().GetBiosrc());
            }
        }

        if (m_OrigSource.IsNull()) {
            if (++m_SeqIter) {
                m_OrigSource = ConstRef(x_GetBioSource());
            }
        }
    }

    x_Init();
    return *this;
}

const CBioSource* CMacroBioData_BioSourceIter::x_GetBioSource()
{
    if (!m_SeqIter) return nullptr;
    {{
        CSeqdesc_CI desc(*m_SeqIter, CSeqdesc::e_Source);
        if (desc) {
            m_SrcDescrIter = desc;
            return &m_SrcDescrIter->GetSource();
        }
    }}
    {{
        CSeqdesc_CI desc(m_SeqIter->GetTopLevelEntry(), CSeqdesc::e_Source);
        if (desc) {
            m_SrcDescrIter = desc;
            return &m_SrcDescrIter->GetSource();
        }
     }}
    return nullptr;
}

bool CMacroBioData_BioSourceIter::IsEnd(void) const
{
    return m_OrigSource.IsNull();
}

bool CMacroBioData_BioSourceIter::IsBegin(void)
{
    return (m_FirstSource == m_OrigSource);
}

SConstScopedObject CMacroBioData_BioSourceIter::GetScopedObject() const
{
    SConstScopedObject sobject;
    sobject.object.Reset(m_OrigSource);

    CRef<CScope> pscope(&(m_Seh.GetScope()));
    sobject.scope = pscope;

    return sobject;
}

string CMacroBioData_BioSourceIter::GetBestDescr() const
{
    string seq_id;
    CWriteUtil::GetBestId(
        CSeq_id_Handle::GetHandle(*(m_SeqIter->GetCompleteBioseq()->GetFirstId())),
        m_Seh.GetScope(), seq_id);

    return string("Biosource ").append(seq_id);
}

void CMacroBioData_BioSourceIter::BuildEditedObject()
{
    m_EditedSource.Reset(new CBioSource);
    m_EditedSource->Assign(*m_OrigSource);

    CObjectInfo oi(m_EditedSource, m_EditedSource->GetThisTypeInfo());
    m_CurIterOI = oi;
}

void CMacroBioData_BioSourceIter::RunEditCommand(CMacroCmdComposite* cmd_composite)
{
    CIRef<IEditCommand> cmd;
    if (m_SrcFeatIter) {
        CSeq_feat_Handle orig_fh = m_SrcFeatIter->GetSeq_feat_Handle();
        CRef<CSeq_feat> new_feat(new CSeq_feat);
        new_feat->Assign(m_SrcFeatIter->GetOriginalFeature());
        new_feat->SetData().SetBiosrc(m_EditedSource.GetNCObject());
        cmd.Reset(new CCmdChangeSeq_feat(orig_fh, *new_feat));
    }
    else if (m_SrcDescrIter) {
        CRef<CSeqdesc> new_desc(new CSeqdesc);
        new_desc->SetSource(m_EditedSource.GetNCObject());
        cmd.Reset(new CCmdChangeSeqdesc(m_SrcDescrIter.GetSeq_entry_Handle(), *m_SrcDescrIter, *new_desc));
    }
    if (cmd) {
        cmd->Execute();
        cmd_composite->AddCommand(*cmd);
    }
}

void CMacroBioData_BioSourceIter::RunDeleteCommand(CMacroCmdComposite* cmd_composite)
{
    if (m_SrcFeatIter) {
        CSeq_feat_Handle fh = m_SrcFeatIter->GetSeq_feat_Handle();
        bool remove_proteins(false);
        CRef<CCmdComposite> del_cmd = GetDeleteFeatureCommand(fh, remove_proteins);
        del_cmd->Execute();
        cmd_composite->AddCommand(*del_cmd);
        Next();
    }
    else {
        CRef<CCmdDelDesc> del_cmd(new CCmdDelDesc(m_SrcDescrIter.GetSeq_entry_Handle(), *m_SrcDescrIter));
        if (del_cmd) {
            del_cmd->Execute();
            cmd_composite->AddCommand(*del_cmd);
        }
        Next();
    }
}

CBioseq_Handle CMacroBioData_BioSourceIter::GetBioseqHandle() const
{
    return *m_SeqIter;
}

void CMacroBioData_BioSourceIter::x_SetDefaultObjectInfo()
{
    CBioSource* biosource = const_cast<CBioSource*>(m_OrigSource.GetPointer());
    CObjectInfo oi(biosource, biosource->GetThisTypeInfo());
    m_CurIterOI = oi;
}

bool CMacroBioData_BioSourceIter::IsDescriptor() const
{
    return !IsFeature();
}

bool CMacroBioData_BioSourceIter::IsFeature() const
{
    return (m_SrcFeatIter) ? true : false;
}



//////////////////////////////////////////////////////////////////////////////
/// CMacroBioData_MolInfoIter
///
CMacroBioData_MolInfoIter::CMacroBioData_MolInfoIter(const CSeq_entry_Handle& entry)
    : CMacroBioData_SeqdescIter(entry)
{
}

IMacroBioDataIter& CMacroBioData_MolInfoIter::Begin(void)
{
    m_SeqIter = CBioseq_CI(m_Seh);
    m_OrigMolinfo = ConstRef(x_GetMolInfo());
    while (m_OrigMolinfo.IsNull() && m_SeqIter) {
        if (++m_SeqIter) {
            m_OrigMolinfo = ConstRef(x_GetMolInfo());
        }
    }

    m_FirstMolinfo = m_OrigMolinfo;

    x_Init();
    return *this;
}

IMacroBioDataIter& CMacroBioData_MolInfoIter::Next(void)
{
    m_OrigMolinfo.Reset();
    while (m_OrigMolinfo.IsNull() && m_SeqIter) {
        if (++m_SeqIter) {
            m_OrigMolinfo = ConstRef(x_GetMolInfo());
        }
    }

    x_Init();
    return *this;
}

const CMolInfo* CMacroBioData_MolInfoIter::x_GetMolInfo()
{
    if (!m_SeqIter) return nullptr;
    CSeqdesc_CI desc(*m_SeqIter, CSeqdesc::e_Molinfo);
    if (desc) {
        m_DescIter = desc;
        return &m_DescIter->GetMolinfo();
    }
    return nullptr;
}

bool CMacroBioData_MolInfoIter::IsEnd(void) const
{
    return m_OrigMolinfo.IsNull();
}

bool CMacroBioData_MolInfoIter::IsBegin(void)
{
    return (m_FirstMolinfo == m_OrigMolinfo);
}

SConstScopedObject CMacroBioData_MolInfoIter::GetScopedObject() const
{
    SConstScopedObject sobject;
    sobject.object.Reset(m_OrigMolinfo);

    CRef<CScope> pscope(&(m_Seh.GetScope()));
    sobject.scope = pscope;

    return sobject;
}

string CMacroBioData_MolInfoIter::GetBestDescr() const
{
    const string& seq_id = x_GetSeqId();
    return string("MolInfo ").append(seq_id);
}

void CMacroBioData_MolInfoIter::BuildEditedObject()
{
    m_EditedMolinfo.Reset(new CMolInfo);
    m_EditedMolinfo->Assign(*m_OrigMolinfo);

    CObjectInfo oi(m_EditedMolinfo, m_EditedMolinfo->GetThisTypeInfo());
    m_CurIterOI = oi;
}

void CMacroBioData_MolInfoIter::RunEditCommand(CMacroCmdComposite* cmd_composite)
{
    CRef<CSeqdesc> new_desc(new CSeqdesc);
    new_desc->SetMolinfo(m_EditedMolinfo.GetNCObject());
    CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(m_DescIter.GetSeq_entry_Handle(), *m_DescIter, *new_desc));
    if (cmd) {
        cmd->Execute();
        cmd_composite->AddCommand(*cmd);
    }
}

void CMacroBioData_MolInfoIter::RunDeleteCommand(CMacroCmdComposite* cmd_composite)
{
    CRef<CCmdDelDesc> del_cmd(new CCmdDelDesc(m_DescIter.GetSeq_entry_Handle(), *m_DescIter));
    if (del_cmd) {
        del_cmd->Execute();
        cmd_composite->AddCommand(*del_cmd);
    }
    Next();
}

void CMacroBioData_MolInfoIter::x_SetDefaultObjectInfo()
{
    CMolInfo* molinfo = const_cast<CMolInfo*>(m_OrigMolinfo.GetPointer());
    CObjectInfo oi(molinfo, molinfo->GetThisTypeInfo());
    m_CurIterOI = oi;
}


//////////////////////////////////////////////////////////////////////////////
/// CMacroBioData_PubdescIter
///
CMacroBioData_PubdescIter::CMacroBioData_PubdescIter(const CSeq_entry_Handle& entry)
    : IMacroBioDataIter(entry)
{
}

IMacroBioDataIter& CMacroBioData_PubdescIter::Begin(void)
{
    m_SeqIter = CBioseq_CI(m_Seh, CSeq_inst::eMol_na);
    m_PubDescrIter = CSeqdesc_CI(*m_SeqIter, CSeqdesc::e_Pub);
    if (m_PubDescrIter) {
        m_OrigPubdesc = ConstRef(&m_PubDescrIter->GetPub());
    }
    while (m_OrigPubdesc.IsNull() && m_SeqIter) {
        m_PubFeatIter = CFeat_CI(*m_SeqIter, SAnnotSelector(CSeqFeatData::eSubtype_pub));
        if (m_PubFeatIter) {
            m_OrigPubdesc = ConstRef(&m_PubFeatIter->GetData().GetPub());
        }

        if (m_OrigPubdesc.IsNull()) {
            if (++m_SeqIter) {
                m_PubDescrIter = CSeqdesc_CI(*m_SeqIter, CSeqdesc::e_Pub);
                if (m_PubDescrIter) {
                    m_OrigPubdesc = ConstRef(&m_PubDescrIter->GetPub());
                }
            }
        }
    }

    m_FirstPubdesc = m_OrigPubdesc;

    if (m_FirstPubdesc.IsNull() && !m_SeqSubmit.IsNull()) {
        if (m_SubmitBlockIter.IsNull()) {
            if (m_SeqSubmit->IsSetSub() && m_SeqSubmit->GetSub().IsSetCit()) {
                m_SubmitBlockIter.Reset(new CSubmitblockIter(m_SeqSubmit));
            }
        }
        else {
            m_SubmitBlockIter->MarkProcessed();
        }
    }

    x_Init();
    return *this;
}

IMacroBioDataIter& CMacroBioData_PubdescIter::Next(void)
{
    CConstRef<CPubdesc> current_pubdesc(m_OrigPubdesc);
    m_OrigPubdesc.Reset();
    while (m_OrigPubdesc.IsNull() && m_SeqIter) {
        if (m_PubDescrIter) {
            if (++m_PubDescrIter) {
                m_OrigPubdesc = ConstRef(&m_PubDescrIter->GetPub());
            }
        }

        if (m_OrigPubdesc.IsNull()) {
            if (m_PubFeatIter) {
                if (++m_PubFeatIter) {
                    m_OrigPubdesc = ConstRef(&m_PubFeatIter->GetData().GetPub());
                }
            }
            else {
                m_PubFeatIter = CFeat_CI(*m_SeqIter, SAnnotSelector(CSeqFeatData::eSubtype_pub));
                if (m_PubFeatIter) {
                    m_OrigPubdesc = ConstRef(&m_PubFeatIter->GetData().GetPub());
                }
            }
        }

        if (m_OrigPubdesc.IsNull()) {
            if (++m_SeqIter) {
                CSeqdesc_CI new_iter = CSeqdesc_CI(*m_SeqIter, CSeqdesc::e_Pub);
                if (new_iter) {
                    if (current_pubdesc.GetPointer() != &new_iter->GetPub()) {
                        m_PubDescrIter = new_iter;
                        m_OrigPubdesc = ConstRef(&m_PubDescrIter->GetPub());
                    }
                }
            }
        }
    }

    if (m_OrigPubdesc.IsNull() && !m_SeqSubmit.IsNull()) {
        if (m_SubmitBlockIter.IsNull()) {
            if (m_SeqSubmit->IsSetSub() && m_SeqSubmit->GetSub().IsSetCit()) {
                m_SubmitBlockIter.Reset(new CSubmitblockIter(m_SeqSubmit));
            }
        }
        else {
            m_SubmitBlockIter->MarkProcessed();
        }

    }

    x_Init();
    return *this;
}

bool CMacroBioData_PubdescIter::IsEnd(void) const
{
    if (m_OrigPubdesc.IsNull()) {
        return (m_SubmitBlockIter) ? m_SubmitBlockIter->Processed() : true;
    }
    return false;
}

bool CMacroBioData_PubdescIter::IsBegin(void)
{
    if (m_FirstPubdesc) {
        return (m_FirstPubdesc == m_OrigPubdesc);
    }
    return true;
}

SConstScopedObject CMacroBioData_PubdescIter::GetScopedObject() const
{
    SConstScopedObject sobject;
    CRef<CScope> pscope(&(m_Seh.GetScope()));
    sobject.scope = pscope;

    if (m_OrigPubdesc) {
        sobject.object.Reset(m_OrigPubdesc);
    }
    else if (m_SubmitBlockIter) {
        sobject.object.Reset(&m_SubmitBlockIter->GetOrigSubmitBlock());
    }
    return sobject;
}

string CMacroBioData_PubdescIter::GetBestDescr() const
{
    if (m_SubmitBlockIter) {
        return "Submitter block: ";
    }

    string seq_id;
    CWriteUtil::GetBestId(CSeq_id_Handle::GetHandle(*(m_SeqIter->GetCompleteBioseq()->GetFirstId())),
        m_Seh.GetScope(), seq_id);

    return string("Publication ").append(seq_id);
}

void CMacroBioData_PubdescIter::BuildEditedObject()
{
    if (m_OrigPubdesc) {
        m_EditedPubdesc.Reset(new CPubdesc);
        m_EditedPubdesc->Assign(*m_OrigPubdesc);

        CObjectInfo oi(m_EditedPubdesc, m_EditedPubdesc->GetThisTypeInfo());
        m_CurIterOI = oi;
    }
    else if (m_SubmitBlockIter) {
        CRef<CSubmit_block> edited_subblock = m_SubmitBlockIter->BuildEditedObject();
        CObjectInfo oi(edited_subblock, edited_subblock->GetThisTypeInfo());
        m_CurIterOI = oi;
    }
}

void CMacroBioData_PubdescIter::RunEditCommand(CMacroCmdComposite* cmd_composite)
{
    if (m_OrigPubdesc) {
        CIRef<IEditCommand> cmd;

        if (m_PubFeatIter) {
            CSeq_feat_Handle orig_fh = m_PubFeatIter->GetSeq_feat_Handle();
            CRef<CSeq_feat> new_feat(new CSeq_feat);
            new_feat->Assign(m_PubFeatIter->GetOriginalFeature());
            new_feat->SetData().SetPub(m_EditedPubdesc.GetNCObject());
            cmd.Reset(new CCmdChangeSeq_feat(orig_fh, *new_feat));
        }
        else if (m_PubDescrIter) {
            CRef<CSeqdesc> new_desc(new CSeqdesc);
            new_desc->SetPub(m_EditedPubdesc.GetNCObject());
            cmd.Reset(new CCmdChangeSeqdesc(m_PubDescrIter.GetSeq_entry_Handle(), *m_PubDescrIter, *new_desc));
        }
        if (cmd) {
            cmd->Execute();
            cmd_composite->AddCommand(*cmd);
        }
    }
    else if (m_SubmitBlockIter) {
        const CSubmit_block& orig_block = m_SubmitBlockIter->GetOrigSubmitBlock();
        CRef<CChangeSubmitBlockCommand> cmd(new CChangeSubmitBlockCommand());
        CObject* actual = (CObject*)(&orig_block);
        cmd->Add(actual, CConstRef<CObject>(&m_SubmitBlockIter->GetEditedSubmitBlock()));

        cmd->Execute();
        cmd_composite->AddCommand(*cmd);
        m_SubmitBlockIter->MarkProcessed();
    }
}

void CMacroBioData_PubdescIter::RunDeleteCommand(CMacroCmdComposite* cmd_composite)
{
    if (m_PubFeatIter) {
        CSeq_feat_Handle fh = m_PubFeatIter->GetSeq_feat_Handle();
        bool remove_proteins(false);
        CRef<CCmdComposite> del_cmd = GetDeleteFeatureCommand(fh, remove_proteins);
        if (del_cmd) {
            del_cmd->Execute();
            cmd_composite->AddCommand(*del_cmd);
        }
        Next();
    }
    else if (m_OrigPubdesc) {
        CRef<CCmdDelDesc> del_cmd(new CCmdDelDesc(m_PubDescrIter.GetSeq_entry_Handle(), *m_PubDescrIter));
        Next();
        if (del_cmd) {
            del_cmd->Execute();
            cmd_composite->AddCommand(*del_cmd);
        }
    }
}

CBioseq_Handle CMacroBioData_PubdescIter::GetBioseqHandle() const
{
    return *m_SeqIter;
}

void CMacroBioData_PubdescIter::x_SetDefaultObjectInfo()
{
    if (m_OrigPubdesc) {
        CPubdesc* pubdesc = const_cast<CPubdesc*>(m_OrigPubdesc.GetPointer());
        CObjectInfo oi(pubdesc, pubdesc->GetThisTypeInfo());
        m_CurIterOI = oi;
    }
    else if (m_SubmitBlockIter) {
        const CSubmit_block& const_block = m_SubmitBlockIter->GetOrigSubmitBlock();
        CSubmit_block* block = const_cast<CSubmit_block*>(&const_block);
        CObjectInfo oi(block, block->GetThisTypeInfo());
        m_CurIterOI = oi;
    }
}

bool CMacroBioData_PubdescIter::IsDescriptor() const
{
    return (m_PubDescrIter) ? true : false;
}

bool CMacroBioData_PubdescIter::IsFeature() const
{
    return (m_PubFeatIter) ? true : false;
}


// CSubmitblockIter 
CMacroBioData_PubdescIter::CSubmitblockIter::CSubmitblockIter(CConstRef<CSeq_submit> seqsubmit)
{
    _ASSERT(seqsubmit);
    _ASSERT(seqsubmit->GetSub().IsSetCit());
    m_OrigSubmitBlock.Reset(&seqsubmit->GetSub());
}

CRef<CSubmit_block> CMacroBioData_PubdescIter::CSubmitblockIter::BuildEditedObject()
{
    m_EditedSubmitBlock.Reset(new CSubmit_block);
    m_EditedSubmitBlock->Assign(*m_OrigSubmitBlock);
    return m_EditedSubmitBlock;

}

///////////////////////////////////////////////////////////////////////////////
/// CMacroBioData_UserObjectIter
///
CMacroBioData_UserObjectIter::CMacroBioData_UserObjectIter(const CSeq_entry_Handle& entry)
    : CMacroBioData_SeqdescIter(entry)
{
}

IMacroBioDataIter& CMacroBioData_UserObjectIter::Begin()
{
    m_SeqIter = CBioseq_CI(m_Seh);
    m_DescIter = CSeqdesc_CI(*m_SeqIter, CSeqdesc::e_User);
    if (m_DescIter) {
        m_OrigUserObj = ConstRef(&m_DescIter->GetUser());
    }

    while (m_OrigUserObj.IsNull() && m_SeqIter) {
        if (++m_SeqIter) {
            m_DescIter = CSeqdesc_CI(*m_SeqIter, CSeqdesc::e_User);
            if (m_DescIter) {
                m_OrigUserObj = ConstRef(&m_DescIter->GetUser());
            }
        }
    }

    m_FirstUserObj = m_OrigUserObj;

    x_Init();
    return *this;
}

IMacroBioDataIter& CMacroBioData_UserObjectIter::Next()
{
    m_OrigUserObj.Reset();
    while (m_OrigUserObj.IsNull() && m_SeqIter) {
        if (++m_DescIter) {
            m_OrigUserObj = ConstRef(&m_DescIter->GetUser());
        }

        if (m_OrigUserObj.IsNull()) {
            if (++m_SeqIter) {
                m_DescIter = CSeqdesc_CI(*m_SeqIter, CSeqdesc::e_User);
                if (m_DescIter) {
                    m_OrigUserObj = ConstRef(&m_DescIter->GetUser());
                }
            }
        }
    }

    x_Init();
    return *this;
}

bool CMacroBioData_UserObjectIter::IsEnd() const
{
    return m_OrigUserObj.IsNull();
}

bool CMacroBioData_UserObjectIter::IsBegin()
{
    return (m_FirstUserObj == m_OrigUserObj);
}

SConstScopedObject CMacroBioData_UserObjectIter::GetScopedObject() const
{
    SConstScopedObject sobject;
    sobject.object.Reset(m_OrigUserObj);

    CRef<CScope> pscope(&(m_Seh.GetScope()));
    sobject.scope = pscope;

    return sobject;
}

string CMacroBioData_UserObjectIter::GetBestDescr() const
{
    const string& seq_id = x_GetSeqId();
    return string("User object ").append(seq_id);
}

void CMacroBioData_UserObjectIter::BuildEditedObject()
{
    m_EditedUserObj.Reset(new CUser_object);
    m_EditedUserObj->Assign(*m_OrigUserObj);

    CObjectInfo oi(m_EditedUserObj, m_EditedUserObj->GetThisTypeInfo());
    m_CurIterOI = oi;
}

void CMacroBioData_UserObjectIter::RunEditCommand(CMacroCmdComposite* cmd_composite)
{
    CRef<CSeqdesc> new_desc(new CSeqdesc);
    new_desc->SetUser(m_EditedUserObj.GetNCObject());
    CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(m_DescIter.GetSeq_entry_Handle(), *m_DescIter, *new_desc));
    if (cmd) {
        cmd->Execute();
        cmd_composite->AddCommand(*cmd);
    }
}

void CMacroBioData_UserObjectIter::RunDeleteCommand(CMacroCmdComposite* cmd_composite)
{
    CRef<CCmdDelDesc> del_cmd(new CCmdDelDesc(m_DescIter.GetSeq_entry_Handle(), *m_DescIter));
    Next();
    if (del_cmd) {
        del_cmd->Execute();
        cmd_composite->AddCommand(*del_cmd);
    }
}

void CMacroBioData_UserObjectIter::x_SetDefaultObjectInfo()
{
    CUser_object* user = const_cast<CUser_object*>(m_OrigUserObj.GetPointer());
    CObjectInfo oi(user, user->GetThisTypeInfo());
    m_CurIterOI = oi;
}


//////////////////////////////////////////////////////////////////////////////
/// CMacroBioData_StructCommentIter
///
CMacroBioData_StructCommentIter::CMacroBioData_StructCommentIter(const CSeq_entry_Handle& entry)
    : CMacroBioData_UserObjectIter(entry)
{
}

IMacroBioDataIter& CMacroBioData_StructCommentIter::Begin()
{
    m_SeqIter = CBioseq_CI(m_Seh, CSeq_inst::eMol_na);
    m_DescIter = CSeqdesc_CI(*m_SeqIter, CSeqdesc::e_User);
    x_StepToNextStructComment();

    while (m_OrigUserObj.IsNull() && m_SeqIter) {
        if (++m_SeqIter) {
            m_DescIter = CSeqdesc_CI(*m_SeqIter, CSeqdesc::e_User);
            x_StepToNextStructComment();
        }
    }

    m_FirstUserObj = m_OrigUserObj;

    x_Init();
    return *this;
}

IMacroBioDataIter& CMacroBioData_StructCommentIter::Next()
{
    m_OrigUserObj.Reset();
    while (m_OrigUserObj.IsNull() && m_SeqIter) {
        if (m_DescIter) {
            ++m_DescIter;
            x_StepToNextStructComment();
        }

        if (m_OrigUserObj.IsNull()) {
            if (++m_SeqIter) {
                m_DescIter = CSeqdesc_CI(*m_SeqIter, CSeqdesc::e_User);
                x_StepToNextStructComment();
            }
        }
    }

    x_Init();
    return *this;
}

string CMacroBioData_StructCommentIter::GetBestDescr() const
{
    const string& seq_id = x_GetSeqId();
    return string("Structured comment ").append(seq_id);
}

void CMacroBioData_StructCommentIter::x_StepToNextStructComment()
{
    while (m_DescIter) {
        if (CComment_rule::IsStructuredComment(m_DescIter->GetUser())) {
            m_OrigUserObj = ConstRef(&m_DescIter->GetUser());
            break;
        }
        ++m_DescIter;
    }
}


//////////////////////////////////////////////////////////////////////////////
/// CMacroBioData_DBLinkIter
///
CMacroBioData_DBLinkIter::CMacroBioData_DBLinkIter(const CSeq_entry_Handle& entry)
    : CMacroBioData_UserObjectIter(entry)
{
}

IMacroBioDataIter& CMacroBioData_DBLinkIter::Begin()
{
    m_SeqIter = CBioseq_CI(m_Seh, CSeq_inst::eMol_na);
    m_OrigUserObj = ConstRef(x_GetDBLink());
    while (m_OrigUserObj.IsNull() && m_SeqIter) {
        ++m_SeqIter;
        m_OrigUserObj = ConstRef(x_GetDBLink());
    }

    m_FirstUserObj = m_OrigUserObj;

    x_Init();
    return *this;
}

IMacroBioDataIter& CMacroBioData_DBLinkIter::Next()
{
    m_OrigUserObj.Reset();
    while (m_OrigUserObj.IsNull() && m_SeqIter) {
        ++m_SeqIter;
        m_OrigUserObj = ConstRef(x_GetDBLink());
    }

    x_Init();
    return *this;
}

string CMacroBioData_DBLinkIter::GetBestDescr() const
{
    const string& seq_id = x_GetSeqId();
    return string("DBLink ").append(seq_id);
}

const objects::CUser_object* CMacroBioData_DBLinkIter::x_GetDBLink()
{
    if (!m_SeqIter) return nullptr;
    for (CSeqdesc_CI desc_it(*m_SeqIter, CSeqdesc::e_User); desc_it; ++desc_it) {
        if (desc_it->GetUser().GetType().IsStr()
            && desc_it->GetUser().GetType().GetStr() == "DBLink") {
            m_DescIter = desc_it;
            return &desc_it->GetUser();
        }
    }
    return nullptr;
}


//////////////////////////////////////////////////////////////////////////////
/// CMacroBioData_SeqdescIter
///
CMacroBioData_SeqdescIter::CMacroBioData_SeqdescIter(const CSeq_entry_Handle& entry)
    : IMacroBioDataIter(entry)
{
}

IMacroBioDataIter& CMacroBioData_SeqdescIter::Begin(void)
{
    m_SeqIter = CBioseq_CI(m_Seh);
    m_DescIter = CSeqdesc_CI(*m_SeqIter);
    if (m_DescIter) {
        m_OrigSeqdesc = ConstRef(&(*m_DescIter));
    }
    while (m_OrigSeqdesc.IsNull() && m_SeqIter) {
        if (++m_SeqIter) {
            m_DescIter = CSeqdesc_CI(*m_SeqIter);
            if (m_DescIter) {
                m_OrigSeqdesc = ConstRef(&(*m_DescIter));
            }
        }
    }

    m_FirstSeqdesc = m_OrigSeqdesc;

    x_Init();
    return *this;
}

IMacroBioDataIter& CMacroBioData_SeqdescIter::Next(void)
{
    m_OrigSeqdesc.Reset();
    while (m_OrigSeqdesc.IsNull() && m_SeqIter) {
        if (++m_DescIter) {
            m_OrigSeqdesc = ConstRef(&(*m_DescIter));
        }

        if (m_OrigSeqdesc.IsNull()) {
            if (++m_SeqIter) {
                m_DescIter = CSeqdesc_CI(*m_SeqIter);
                if (m_DescIter) {
                    m_OrigSeqdesc = ConstRef(&(*m_DescIter));
                }
            }
        }
    }

    x_Init();
    return *this;
}

bool CMacroBioData_SeqdescIter::IsEnd(void) const
{
    return m_OrigSeqdesc.IsNull();
}

bool CMacroBioData_SeqdescIter::IsBegin(void)
{
    return (m_FirstSeqdesc == m_OrigSeqdesc);
}

SConstScopedObject CMacroBioData_SeqdescIter::GetScopedObject() const
{
    SConstScopedObject sobject;
    sobject.object.Reset(m_OrigSeqdesc);

    CRef<CScope> pscope(&(m_Seh.GetScope()));
    sobject.scope = pscope;

    return sobject;
}

string CMacroBioData_SeqdescIter::x_GetSeqId() const
{
    string seq_id;
    CWriteUtil::GetBestId(
        CSeq_id_Handle::GetHandle(*(m_SeqIter->GetCompleteBioseq()->GetFirstId())),
        m_Seh.GetScope(), seq_id);
    return seq_id;
}

string CMacroBioData_SeqdescIter::GetBestDescr() const
{
    const string& seq_id = x_GetSeqId();
    return string("Descriptor ").append(seq_id);
}

void CMacroBioData_SeqdescIter::BuildEditedObject()
{
    m_EditedSeqdesc.Reset(new CSeqdesc);
    m_EditedSeqdesc->Assign(*m_OrigSeqdesc);

    CObjectInfo objInfo(m_EditedSeqdesc, m_EditedSeqdesc->GetThisTypeInfo());
    m_CurIterOI = objInfo;
}

void CMacroBioData_SeqdescIter::RunEditCommand(CMacroCmdComposite* cmd_composite)
{
    CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(m_DescIter.GetSeq_entry_Handle(), *m_OrigSeqdesc, *m_EditedSeqdesc));
    if (cmd) {
        cmd->Execute();
        cmd_composite->AddCommand(*cmd);
    }
}

void CMacroBioData_SeqdescIter::RunDeleteCommand(CMacroCmdComposite* cmd_composite)
{
    CRef<CCmdDelDesc> del_cmd(new CCmdDelDesc(m_DescIter.GetSeq_entry_Handle(), *m_OrigSeqdesc));
    Next();
    if (del_cmd) {
        del_cmd->Execute();
        cmd_composite->AddCommand(*del_cmd);
    }
}

CBioseq_Handle CMacroBioData_SeqdescIter::GetBioseqHandle() const
{
    return *m_SeqIter;
}

void CMacroBioData_SeqdescIter::x_SetDefaultObjectInfo()
{
    CSeqdesc* seqdesc = const_cast<CSeqdesc*>(m_OrigSeqdesc.GetPointer());
    CObjectInfo objInfo(seqdesc, seqdesc->GetThisTypeInfo());
    m_CurIterOI = objInfo;
}

//////////////////////////////////////////////////////////////////////////////
/// CMacroBioData_SeqAlignIter
///
CMacroBioData_SeqAlignIter::CMacroBioData_SeqAlignIter(const CSeq_entry_Handle& entry)
    : IMacroBioDataIter(entry), m_AlignIter(entry, SAnnotSelector(CSeq_annot::C_Data::e_Align))
{
}

IMacroBioDataIter& CMacroBioData_SeqAlignIter::Begin(void)
{
    m_AlignIter.Rewind();
    x_Init();
    return *this;
}

IMacroBioDataIter& CMacroBioData_SeqAlignIter::Next(void)
{
    ++m_AlignIter;
    x_Init();
    return *this;
}

bool CMacroBioData_SeqAlignIter::IsEnd(void) const
{
    if (m_AlignIter) {
        return false;
    }
    return true;
}

bool CMacroBioData_SeqAlignIter::IsBegin(void)
{
    CAlign_CI align_it(m_AlignIter);
    align_it.Rewind();
    if (&(*m_AlignIter) == &(*align_it))
        return true;
    return false;

}

SConstScopedObject CMacroBioData_SeqAlignIter::GetScopedObject() const
{
    SConstScopedObject sobject;

    const CSeq_align& align = *m_AlignIter;
    sobject.object.Reset(&align);

    CRef<CScope> pscope(&(m_Seh.GetScope()));
    sobject.scope = pscope;

    return sobject;
}

string CMacroBioData_SeqAlignIter::GetBestDescr() const
{
    return "Alignment";
}

void CMacroBioData_SeqAlignIter::BuildEditedObject()
{
    m_OrigSeqalign.Reset(&(*m_AlignIter));
    m_EditedSeqalign.Reset(new CSeq_align);
    m_EditedSeqalign->Assign(*m_OrigSeqalign);

    CObjectInfo objInfo(m_EditedSeqalign, m_EditedSeqalign->GetThisTypeInfo());
    m_CurIterOI = objInfo;
}

void CMacroBioData_SeqAlignIter::RunEditCommand(CMacroCmdComposite* cmd_composite)
{
    CRef<CCmdChangeAlign> chgAlign(new CCmdChangeAlign(m_AlignIter.GetSeq_align_Handle(), *m_EditedSeqalign.GetNCPointerOrNull()));
    chgAlign->Execute();
    cmd_composite->AddCommand(*chgAlign);
}

void CMacroBioData_SeqAlignIter::RunDeleteCommand(CMacroCmdComposite* cmd_composite)
{
    CSeq_align_Handle ah = m_AlignIter.GetSeq_align_Handle();
    CRef<CCmdDelSeq_align> del_cmd(new CCmdDelSeq_align(ah));
    del_cmd->Execute();
    cmd_composite->AddCommand(*del_cmd);

    Next();
}

void CMacroBioData_SeqAlignIter::x_SetDefaultObjectInfo()
{
    const CSeq_align* const_seqalign = &(*m_AlignIter);
    CSeq_align* seqalign = const_cast<CSeq_align*>(const_seqalign);
    CObjectInfo objInfo(seqalign, seqalign->GetThisTypeInfo());
    m_CurIterOI = objInfo;
}


//////////////////////////////////////////////////////////////////////////////
/// CMacroBioData_SeqIter
///
CMacroBioData_SeqIter::CMacroBioData_SeqIter(const CSeq_entry_Handle& entry)
    : IMacroBioDataIter(entry)
{
}

CMacroBioData_SeqIter* CMacroBioData_SeqIter::s_MakeSeqIterator(const objects::CSeq_entry_Handle& entry, objects::CSeq_inst::EMol mol_type)
{
    switch (mol_type) {
    case CSeq_inst::eMol_na:
        return new CMacroBioData_SeqNAIter(entry);
    case CSeq_inst::eMol_aa:
        return new CMacroBioData_SeqAAIter(entry);
    default:
        return new CMacroBioData_SeqIter(entry);
    }
    return nullptr;
}

IMacroBioDataIter& CMacroBioData_SeqIter::Begin(void)
{
    m_SeqIter = CBioseq_CI(m_Seh);
    x_Init();
    return *this;
}

IMacroBioDataIter& CMacroBioData_SeqIter::Next(void)
{
    ++m_SeqIter;
    x_Init();
    return *this;
}

bool CMacroBioData_SeqIter::IsEnd(void) const
{
    return (!m_SeqIter);
}

bool CMacroBioData_SeqIter::IsBegin(void)
{
    CBioseq_CI seq_it(m_Seh);
    return (*m_SeqIter == *seq_it);
}

SConstScopedObject CMacroBioData_SeqIter::GetScopedObject() const
{
    SConstScopedObject sobject;

    const CBioseq& bseq = *m_SeqIter->GetCompleteBioseq();
    sobject.object.Reset(&bseq);

    CRef<CScope> pscope(&(m_Seh.GetScope()));
    sobject.scope = pscope;

    return sobject;
}


string CMacroBioData_SeqIter::GetBestDescr() const
{
    const CBioseq& bseq = *m_SeqIter->GetCompleteBioseq();
    CBioseq_Handle bsh = m_Seh.GetScope().GetBioseqHandle(bseq);
    string best_id;
    CWriteUtil::GetBestId(bsh.GetAccessSeq_id_Handle(), m_Seh.GetScope(), best_id);
    return best_id;
}

void CMacroBioData_SeqIter::BuildEditedObject()
{
    // build editable Bioseq/Seq-entry:
    m_OrigSeq.Reset(m_SeqIter->GetCompleteBioseq().GetPointer());
    m_EditedSeq.Reset(new CBioseq);
    m_EditedSeq->Assign(*m_OrigSeq);

    m_EditedEntry.Reset(new CSeq_entry);
    m_EditedEntry->SetSeq(*m_EditedSeq);

    CObjectInfo objInfo(m_EditedSeq, m_EditedSeq->GetThisTypeInfo());
    m_CurIterOI = objInfo;
}

void CMacroBioData_SeqIter::RunEditCommand(CMacroCmdComposite* cmd_composite)
{
    const CBioseq& bseq = *m_SeqIter->GetCompleteBioseq();
    CBioseq_Handle bsh = m_Seh.GetScope().GetBioseqHandle(bseq);
    CSeq_entry_Handle b_seh = bsh.GetSeq_entry_Handle();

    CIRef<IEditCommand> chg_seq(new CCmdChangeSeqEntry(b_seh, m_EditedEntry));
    chg_seq->Execute();
    cmd_composite->AddCommand(*chg_seq);
}

void CMacroBioData_SeqIter::RunDeleteCommand(CMacroCmdComposite* cmd_composite)
{
    const CBioseq& bseq = *m_SeqIter->GetCompleteBioseq();
    CBioseq_Handle bsh = m_Seh.GetScope().GetBioseqHandle(bseq);

    Next();
    CRef<CCmdDelBioseq> del_cmd(new CCmdDelBioseq(bsh));
    del_cmd->Execute();
    cmd_composite->AddCommand(*del_cmd);
    // when protein sequences are deleted, this command does not renormalize the nuc-prot set
}

CBioseq_Handle CMacroBioData_SeqIter::GetBioseqHandle(void) const
{
    return *m_SeqIter;
}

void CMacroBioData_SeqIter::x_SetDefaultObjectInfo()
{
    const CBioseq* const_bseq = m_SeqIter->GetCompleteBioseq().GetPointerOrNull();
    CBioseq* bseq = const_cast<CBioseq*>(const_bseq);
    CObjectInfo oi(bseq, bseq->GetThisTypeInfo());
    m_CurIterOI = oi;
}


CMacroBioData_SeqNAIter::CMacroBioData_SeqNAIter(const CSeq_entry_Handle& entry)
    : CMacroBioData_SeqIter(entry)
{
}

IMacroBioDataIter& CMacroBioData_SeqNAIter::Begin(void)
{
    m_SeqIter = CBioseq_CI(m_Seh, CSeq_inst::eMol_na);
    x_Init();
    return *this;
}

bool CMacroBioData_SeqNAIter::IsBegin(void)
{
    CBioseq_CI seq_it(m_Seh, CSeq_inst::eMol_na);
    // check if handles point to the same bioseq
    return (*m_SeqIter == *seq_it);
}


CMacroBioData_SeqAAIter::CMacroBioData_SeqAAIter(const CSeq_entry_Handle& entry)
    : CMacroBioData_SeqIter(entry)
{
}

IMacroBioDataIter& CMacroBioData_SeqAAIter::Begin(void)
{
    m_SeqIter = CBioseq_CI(m_Seh, CSeq_inst::eMol_aa);
    x_Init();
    return *this;
}

bool CMacroBioData_SeqAAIter::IsBegin(void)
{
    CBioseq_CI seq_it(m_Seh, CSeq_inst::eMol_aa);
    // check if handles point to the same bioseq
    return (*m_SeqIter == *seq_it);
}



/////////////////////////////////////////////////////////////////
/// CMacroBioData_SeqSetIter- iterates over all types of sets
///
CMacroBioData_SeqSetIter::CMacroBioData_SeqSetIter(const CSeq_entry_Handle& entry)
    : IMacroBioDataIter(entry), m_SeqSetIter(entry, CSeq_entry_CI::fRecursive | CSeq_entry_CI::fIncludeGivenEntry, CSeq_entry::e_Set)
{
}

IMacroBioDataIter& CMacroBioData_SeqSetIter::Begin(void)
{
    CSeq_entry_CI local_iter(m_Seh, CSeq_entry_CI::fRecursive | CSeq_entry_CI::fIncludeGivenEntry, CSeq_entry::e_Set);
    m_SeqSetIter = local_iter;
    x_Init();
    return *this;
}

IMacroBioDataIter& CMacroBioData_SeqSetIter::Next(void)
{
    ++m_SeqSetIter;
    x_Init();
    return *this;
}

bool CMacroBioData_SeqSetIter::IsEnd(void) const
{
    return (!m_SeqSetIter);
}

bool CMacroBioData_SeqSetIter::IsBegin(void)
{
    CSeq_entry_CI set_iter(m_Seh, CSeq_entry_CI::fRecursive | CSeq_entry_CI::fIncludeGivenEntry, CSeq_entry::e_Set);
    return (m_SeqSetIter->GetCompleteSeq_entry().GetPointerOrNull() == set_iter->GetCompleteSeq_entry().GetPointerOrNull());
}

SConstScopedObject CMacroBioData_SeqSetIter::GetScopedObject() const
{
    SConstScopedObject sobject;

    CConstRef<CSeq_entry> entry = m_SeqSetIter->GetCompleteSeq_entry();
    _ASSERT(entry->IsSet());
    const CBioseq_set& set = entry->GetSet();
    sobject.object.Reset(&set);

    CRef<CScope> pscope(&(m_SeqSetIter->GetScope()));
    sobject.scope = pscope;

    return sobject;
}

string CMacroBioData_SeqSetIter::GetBestDescr() const
{
    string label = kEmptyStr;
    m_SeqSetIter->GetCompleteSeq_entry()->GetLabel(&label, CSeq_entry::eType);
    return label;
}

void CMacroBioData_SeqSetIter::BuildEditedObject()
{
    CConstRef<CSeq_entry> entry = m_SeqSetIter->GetCompleteSeq_entry();
    _ASSERT(entry->IsSet());
    m_OrigSeqSet.Reset(&entry->GetSet());
    m_EditedSeqSet.Reset(new CBioseq_set);
    m_EditedSeqSet->Assign(*m_OrigSeqSet);

    CObjectInfo objInfo(m_EditedSeqSet, m_EditedSeqSet->GetThisTypeInfo());
    m_CurIterOI = objInfo;
}

void CMacroBioData_SeqSetIter::RunEditCommand(CMacroCmdComposite* cmd_composite)
{
    CConstRef<CSeq_entry> entry = m_SeqSetIter->GetCompleteSeq_entry();
    _ASSERT(entry->IsSet());
    CBioseq_set_Handle bssh = (*m_SeqSetIter).GetScope().GetBioseq_setHandle(entry->GetSet());
    CRef<CCmdChangeBioseqSet> chg_cmd(new CCmdChangeBioseqSet(bssh, *m_EditedSeqSet.GetNCPointerOrNull()));
    chg_cmd->Execute();
    cmd_composite->AddCommand(*chg_cmd);
}

void CMacroBioData_SeqSetIter::RunDeleteCommand(CMacroCmdComposite* cmd_composite)
{
    CConstRef<CSeq_entry> entry = m_SeqSetIter->GetCompleteSeq_entry();
    _ASSERT(entry->IsSet());
    CBioseq_set_Handle bssh = (*m_SeqSetIter).GetScope().GetBioseq_setHandle(entry->GetSet());
    CRef<CCmdDelBioseqSet> del_cmd(new CCmdDelBioseqSet(bssh));
    del_cmd->Execute();
    cmd_composite->AddCommand(*del_cmd);

    Next();
}

void CMacroBioData_SeqSetIter::x_SetDefaultObjectInfo()
{
    CConstRef<CSeq_entry> entry = m_SeqSetIter->GetCompleteSeq_entry();
    _ASSERT(entry->IsSet());
    const CBioseq_set& c_set = entry->GetSet();
    CBioseq_set& set = const_cast<CBioseq_set&>(c_set);
    CObjectInfo oi(&set, set.GetThisTypeInfo());
    m_CurIterOI = oi;
}

/////////////////////////////////////////////////////////////////
/// CMacroBioData_TSEntryIter
///
CMacroBioData_TSEntryIter::CMacroBioData_TSEntryIter(const CSeq_entry_Handle& entry)
    : IMacroBioDataIter(entry), m_EntryIter(entry, CSeq_entry_CI::fIncludeGivenEntry)
{
}


IMacroBioDataIter& CMacroBioData_TSEntryIter::Begin(void)
{
    CSeq_entry_CI local_iter(m_Seh, CSeq_entry_CI::fIncludeGivenEntry);
    m_EntryIter = local_iter;
    x_Init();
    return *this;
}

IMacroBioDataIter& CMacroBioData_TSEntryIter::Next(void)
{
    ++m_EntryIter;
    x_Init();
    return *this;
}

bool CMacroBioData_TSEntryIter::IsEnd(void) const
{
    if (m_EntryIter && m_EntryIter.GetDepth() == 0) {
        return false;
    }
    return true;
}

bool CMacroBioData_TSEntryIter::IsBegin(void)
{
    CSeq_entry_CI entry_iter(m_Seh, CSeq_entry_CI::fIncludeGivenEntry);
    if (m_EntryIter->GetCompleteSeq_entry().GetPointerOrNull() ==
        entry_iter->GetCompleteSeq_entry().GetPointerOrNull()) {
        return true;
    }
    return false;
}

SConstScopedObject CMacroBioData_TSEntryIter::GetScopedObject() const
{
    SConstScopedObject sobject;

    CConstRef<CSeq_entry> entry = m_EntryIter->GetCompleteSeq_entry();
    sobject.object.Reset(&(*entry));

    CRef<CScope> pscope(&(m_EntryIter->GetScope()));
    sobject.scope = pscope;

    return sobject;
}

string CMacroBioData_TSEntryIter::GetBestDescr() const
{
    return kEmptyStr;
}

void CMacroBioData_TSEntryIter::BuildEditedObject()
{
    CConstRef<CSeq_entry> orig_entry = m_EntryIter->GetCompleteSeq_entry();
    m_OrigTSE.Reset(&(*orig_entry));

    m_EditedTSE.Reset(new CSeq_entry());
    m_EditedTSE->Assign(*orig_entry);

    CObjectInfo objInfo(m_EditedTSE, m_EditedTSE->GetThisTypeInfo());
    m_CurIterOI = objInfo;
}

void CMacroBioData_TSEntryIter::RunEditCommand(CMacroCmdComposite* cmd_composite)
{
    CRef<CCmdChangeSeqEntry> chg_cmd(new CCmdChangeSeqEntry(*m_EntryIter, m_EditedTSE));
    chg_cmd->Execute();
    cmd_composite->AddCommand(*chg_cmd);
}

void CMacroBioData_TSEntryIter::RunDeleteCommand(CMacroCmdComposite* cmd_composite)
{
    // do nothing
}


void CMacroBioData_TSEntryIter::x_SetDefaultObjectInfo()
{
    CConstRef<CSeq_entry> c_entry = m_EntryIter->GetCompleteSeq_entry();
    CSeq_entry& entry = const_cast<CSeq_entry&>(*c_entry);
    CObjectInfo oi(&entry, entry.GetThisTypeInfo());
    m_CurIterOI = oi;
}

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */
