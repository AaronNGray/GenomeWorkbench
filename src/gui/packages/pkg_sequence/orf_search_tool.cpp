/*  $Id: orf_search_tool.cpp 39744 2017-10-31 21:12:13Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/core/data_mining_service.hpp>
#include "orf_search_tool.hpp"

#include <corelib/ncbiapp.hpp>
#include <corelib/ncbifile.hpp>
#include <corelib/ncbireg.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/sys_path.hpp>

#include <objects/seqfeat/Cdregion.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seq/Seq_annot.hpp>

#include <gui/objutils/label.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/widgets/wx/ui_tool_registry.hpp>

#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/utils/extension_impl.hpp>

#include <objmgr/util/sequence.hpp>

#include <objects/seqfeat/Genetic_code.hpp>
#include <objects/seqfeat/Genetic_code_table.hpp>
#include <algo/sequence/orf.hpp>
#include <objects/taxon1/taxon1.hpp>


#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/srchctrl.h>
#include <wx/filename.h>
#include <wx/dir.h>


#define ID_COMBOBOX 10003
#define ID_TEXT             11414
#define ID_COMBOBOX_UPDATE  10111

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const char* kOrfGencode  = "OrfGencode";
static const char* kOrfStartOrf = "OrfStartOrf";
static const char* kOrfMinPairs = "OrfMinPairs";
static const char* kOrfFilters = "OrfFilters";

static const wxChar* kKozakFile = wxT("kozak.ini");

COrfSearchForm::TKozakList COrfSearchForm::m_Kozak;

/*
static string skColumnNames[] = {//"Label",
    "Start", "Stop", "ORF Length",
    "Begining Codon", "Terminal Codon", "Translated Sequence",
    "Nucleotide Sequence", "Context"
};
*/

class COrfSearchTableModel : public CObjectListTableModel
{
public:
    COrfSearchTableModel() : m_ColorCol( -1 ) {}

    //virtual int GetNumExtraColumns() const;
    virtual wxColor GetBackgroundColor( int row ) const;

    virtual int GetNumColumns() const;

    void SetColorColumn( int col ) { m_ColorCol = col; }

private:
    int m_ColorCol;
};
/*
int COrfSearchTableModel::GetNumExtraColumns() const
{
    // we can use just the first column from the set of standard ones
    // ie "Label"
    return 1;
}
*/
wxColor COrfSearchTableModel::GetBackgroundColor( int row ) const
{
    if( m_ColorCol < 0 ) return wxColor();

    int color = m_ObjectList->GetInteger( m_ColorCol, row );

    switch( color ){
    case 1:
        return wxColor(wxT("#D5EDF5"));
    case 2:
        return wxColor(wxT("#D5EDF5"));
    default:
        return wxColor();
    }
}

int COrfSearchTableModel::GetNumColumns() const
{
    // we reduce number of columns, effectively 
    // making one column hidden from existence
    return CObjectListTableModel::GetNumColumns() - 1;
}


///////////////////////////////////////////////////////////////////////////////
/// COrfSearchTool
COrfSearchTool::COrfSearchTool()
{
    m_Conversions.options.clear();
    m_Conversions.options.push_back(CSeqFeatData::eSubtype_cdregion);
    m_Conversions.options.push_back(CSeqFeatData::eSubtype_mat_peptide_aa);
    m_Conversions.options.push_back(CSeqFeatData::eSubtype_sig_peptide_aa);
    m_Conversions.options.push_back(CSeqFeatData::eSubtype_mat_peptide);
    m_Conversions.options.push_back(CSeqFeatData::eSubtype_sig_peptide);
    m_Conversions.options.push_back(CSeqFeatData::eSubtype_transit_peptide);
    m_Conversions.options.push_back(CSeqFeatData::eSubtype_prot);
    m_Conversions.options.push_back(CSeqFeatData::eSubtype_preprotein);

    m_Conversions.selected = CSeqFeatData::eSubtype_cdregion;

    m_Filters.filters.push_back(pair<string,string>("Show only positive strand results", 
        "Strand='+'"));
    
    m_Filters.filters.push_back(pair<string,string>("Show only negative strand results",
        "Strand='-'"));

    m_Filters.filters.push_back(pair<string,string>("Show only longest ORFs",
        "LongestORFs"));
    m_Filters.selected.push_back(2);

    //m_Filters.selected.push_back(0);

}

string COrfSearchTool::GetName() const
{
    static string name("Open Reading Frames Search");
    return name;
}


IUITool* COrfSearchTool::Clone() const
{
    return new COrfSearchTool();
}


string COrfSearchTool::GetDescription() const
{
    return "";
}


CIRef<IDMSearchForm> COrfSearchTool::CreateSearchForm()
{
    CIRef<IDMSearchForm> form(new COrfSearchForm(*this));
    return form;
}


bool COrfSearchTool::IsCompatible(IDataMiningContext* context)
{
    ISeqLocSearchContext* sl_ctx =
        dynamic_cast<ISeqLocSearchContext*>(context);
    return sl_ctx != NULL;
}


CRef<CSearchJobBase> COrfSearchTool::x_CreateJob(IDMSearchQuery& query)
{
    CRef<CSearchJobBase> job;
    COrfSearchQuery* f_query = dynamic_cast<COrfSearchQuery*>(&query);
    if(f_query) {
        job.Reset(new COrfSearchJob(*f_query, COrfSearchForm::m_Kozak));
    }
    return job;
}



string  COrfSearchTool::GetExtensionIdentifier() const
{
    return "search_tool::orf_search_tool";
}

string  COrfSearchTool::GetExtensionLabel() const
{
    return "Datamining Tool - Open Reading Frames search";
}

IDMSearchTool::TUIToolFlags COrfSearchTool::GetFlags(void)
{
    return (IDMSearchTool::eFilter | IDMSearchTool::eCache);
}



///////////////////////////////////////////////////////////////////////////////
/// COrfSearchQuery

COrfSearchQuery::COrfSearchQuery(TScopedLocs& locs,
                                         const string& gc,
                                         const string&  so,
                                         const string&  mp )
:   CSearchQueryBase(locs),
    m_Gencode(gc),
    m_StartOrf(so),
    m_MinPairs(mp)
{
}

string COrfSearchQuery::ToString() const
{
    string s;
    s.append("genecode="); 
    s.append(m_Gencode);
    s.append(" StartOrf=");
    s.append(m_StartOrf);
    s.append(" MinPairs=");
    s.append(m_MinPairs);
    return s;
}

///////////////////////////////////////////////////////////////////////////////
/// COrfSearchForm


COrfSearchForm::COrfSearchForm(COrfSearchTool& tool)
:   m_Tool(&tool),
    m_GencodeCombo(NULL),
    m_StartorfCombo(NULL),
    m_BasepairsText(NULL)
{
}


COrfSearchForm::~COrfSearchForm()
{
}

void COrfSearchForm::Create()
{
}

void COrfSearchForm::Init()
{
    CSearchFormBase::Init();

    static bool kozakInitialized = false;
    if (!kozakInitialized) {
        x_UpdateKozak();
        kozakInitialized = true;
    }
}

void COrfSearchForm::x_LoadSettings(const CRegistryReadView& view)
{
    m_Gencode  = view.GetString(kOrfGencode);
    m_StartOrf = view.GetString(kOrfStartOrf);
    m_MinPairs = view.GetString(kOrfMinPairs);

    view.GetIntVec(kOrfFilters, m_Tool->SetFilters().selected);
}


void COrfSearchForm::x_SaveSettings(CRegistryWriteView view) const
{
    if (m_GencodeCombo) {
        view.Set(kOrfGencode, ToStdString( m_GencodeCombo->GetStringSelection() ));
    }
    if (m_StartorfCombo) {
        view.Set(kOrfStartOrf, ToStdString( m_StartorfCombo->GetStringSelection() ));
    }
    if (m_BasepairsText) {
        view.Set(kOrfMinPairs, ToStdString( m_BasepairsText->GetValue() ));
    }

    view.Set(kOrfFilters, m_Tool->GetFilters().selected);
}


IDMSearchTool* COrfSearchForm::x_GetTool()
{
    return m_Tool.GetPointer();
}

static string sAutoSelect = "Auto Select";

wxSizer *  COrfSearchForm::GetWidget(wxWindow * parent)
{
    if (!m_Sizer) {
        wxBoxSizer * vSz = new wxBoxSizer(wxVERTICAL);
        m_Sizer = vSz;

        wxFlexGridSizer * sz = new wxFlexGridSizer(0, 4, 0, 0);
        sz->AddGrowableCol(1);
        sz->AddGrowableCol(3);

        m_Sizer->Add(sz, 0, wxGROW|wxALL, 0);


        sz->Add(new wxStaticText( parent, wxID_STATIC,
                                  wxT("Search Context:"),
                                  wxDefaultPosition, wxDefaultSize, 0 ),
            0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

        m_DbCombo = new wxChoice(parent, ID_COMBOBOX,
                                   wxDefaultPosition, wxDefaultSize,
                                   0, (const wxString*)NULL);

        sz->Add(m_DbCombo,1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

        // Genetic code combo
        sz->Add(new wxStaticText( parent, wxID_STATIC, wxT("Genetic Code:"),
                                  wxDefaultPosition, wxDefaultSize, 0 ),
                0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
        m_GencodeCombo = new  wxChoice(parent, ID_COMBOBOX_UPDATE,
                                         wxDefaultPosition, wxDefaultSize,
                                         0, (const wxString*)NULL);
        const CGenetic_code_table& code_table = CGen_code_table::GetCodeTable();
        const CGenetic_code_table::Tdata& codes = code_table.Get();
        m_GencodeCombo->Append(ToWxString(sAutoSelect));
        ITERATE (CGenetic_code_table::Tdata, code, codes) {
            m_GencodeCombo->Append(ToWxString((*code)->GetName()));
        }
        if (!m_Gencode.empty()) {
            m_GencodeCombo->SetStringSelection(ToWxString(m_Gencode));
        } else {
            m_GencodeCombo->Select(0);
        }
        sz->Add(m_GencodeCombo,1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

        // min number of basepairs
        sz->Add(new wxStaticText( parent, wxID_STATIC,
                                  wxT("Min Number of Basepairs:"),
                                  wxDefaultPosition, wxDefaultSize, 0 ),
                0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
        m_BasepairsText = new wxTextCtrl(parent, wxID_STATIC, wxT("100"),
                                         wxDefaultPosition, wxDefaultSize, 0 );
        if (!m_MinPairs.empty()) {
            m_BasepairsText->SetValue(ToWxString(m_MinPairs));
        } else {
            m_BasepairsText->SetValue(wxT("100"));
        }
        sz->Add(m_BasepairsText,1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);


        // Start ORFS combo
        sz->Add(new wxStaticText( parent, wxID_STATIC, wxT("Start ORFs At:"),
                                  wxDefaultPosition, wxDefaultSize, 0 ),
                0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
        m_StartorfCombo = new  wxChoice(parent, wxID_STATIC,
                                          wxDefaultPosition, wxDefaultSize,
                                          0, (const wxString*)NULL);
        m_StartorfCombo->Append(wxT("ATG Only"));
        m_StartorfCombo->Append(wxT("Any Sense Codon"));

        if (!m_StartOrf.empty()) {
            m_StartorfCombo->SetStringSelection(ToWxString(m_StartOrf));
        } else {
            m_StartorfCombo->Select(0);
        }
        sz->Add(m_StartorfCombo,1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    }
    return m_Sizer;
}


void COrfSearchForm::Update()
{
    UpdateContextCombo(m_DbCombo);
}


void COrfSearchForm::x_UpdateKozak(void)
{
    string kozakDir;

    m_Kozak.clear();

    CNcbiApplication* app = CNcbiApplication::Instance();
    _ASSERT(app);
    CNcbiRegistry& registry = app->GetConfig();

    if ( (kozakDir = registry.Get("Patterns", "PatternPath")).empty() ) {
        registry.Set("Patterns", "PatternPath", "<std>, <home>",
                     CNcbiRegistry::ePersistent, " default external_path");
    }
    kozakDir = registry.Get("Patterns", "PatternPath");

    list<string> paths;
    NStr::Split(kozakDir, ", \t\n\r", paths, NStr::fSplit_Tokenize);

    ITERATE (list<string>, iter, paths) {
        wxString dir_name;
        if (*iter == "<std>"  ||  *iter == "<home>") {
            dir_name = CSysPath::ResolvePath(wxString::FromAscii(iter->c_str()), wxT("etc/patterns"));
        } else {
            dir_name = CSysPath::ResolvePath(wxString::FromAscii(iter->c_str()), wxEmptyString);
        }
        if ( dir_name.empty() ) {
            continue;
        }

        wxFileName fname(dir_name, wxEmptyString);
        if ( !fname.DirExists(dir_name) ) {
            continue;
        }

        wxDir dir(dir_name);
        wxString filename;

        bool cont = dir.GetFirst(&filename, kKozakFile, wxDIR_FILES);
        while ( cont ) {
            fname.SetFullName(filename);
            wxString full_path = fname.GetFullPath();

            CNcbiIfstream reg_stream(full_path.fn_str());
            CNcbiRegistry patterns(reg_stream);

             // iterate over patterns
            list<string> pat_ids;
            patterns.EnumerateSections(&pat_ids);
            string fName="";

            ITERATE (list<string>, pat_id, pat_ids) {
                if (*pat_id == "-") {
                    fName = patterns.Get("-", "name");
                    // this for only one pattern
                    /*
                    if (!fName.empty()) {
                        m_FileList[fName].first = full_path;
                    } 
                    */
                }
                else {
                    string pattern = patterns.Get(*pat_id, "pattern");
                    string desc = patterns.Get(*pat_id, "description");
                    int codon_pos = patterns.GetInt(*pat_id, "codon_pos", 3, 0, IRegistry::eReturn);
                    m_Kozak.push_back(TKozakPattern(desc, pattern, codon_pos));                        
                }
            }
            cont = dir.GetNext(&filename);
        }
    }
}



CIRef<IDMSearchQuery> COrfSearchForm::ConstructQuery()
{
     m_SeqLocContexts.clear();
     for( size_t i = 0;  i < m_Contexts.size();  i++ )   {
         if (m_DbCombo->GetSelection() != 0  &&
             m_DbCombo->GetSelection() != int(i + 1)) {
             continue;
         }
         ISeqLocSearchContext* seq_ctx =
            dynamic_cast<ISeqLocSearchContext*>(m_Contexts[i]);
         m_SeqLocContexts.push_back(seq_ctx);
      }

    _ASSERT(m_SeqLocContexts.size());

    /// accumulate locations from all selected contexts
    TScopedLocs sc_locs;//(m_SeqLocContexts.size());
    for( size_t i = 0;  i < m_SeqLocContexts.size();  i++ ) {
        ISeqLocSearchContext* sl_ctx = m_SeqLocContexts[i];
        _ASSERT(sl_ctx);
        

        TConstScopedObjects cso;
        sl_ctx->GetMultiple(cso);

        if (!cso.empty()) {
            ITERATE (TConstScopedObjects, obj, cso) {            
                CRef<CSeq_loc> loc(new CSeq_loc());
                const CSeq_loc * in_loc = 
                    dynamic_cast<const CSeq_loc*>(obj->object.GetPointer());

                // ignore loc if range mode turned off
                if (m_bRange) {
                    loc.Reset(const_cast<CSeq_loc*>(in_loc));
                }
                else {
                    CRef<CSeq_id> id(new CSeq_id());
                    id->Assign(*(in_loc->GetId()));
                    loc->SetWhole(*id);
                }

                COrfSearchQuery::SScopedLoc sl;
                string label;
                loc->GetLabel(&label);
                sl.m_Loc = loc;
                sl.m_Scope = obj->scope;
                sl.m_ContextName = sl_ctx->GetDMContextName() + " (" + label + ")";

                sc_locs.push_back(sl);
            }
        }
        else if (!sl_ctx->GetSearchLoc().IsNull()){
            COrfSearchQuery::SScopedLoc sl;
            
            CRef<CSeq_loc> searchLoc = sl_ctx->GetSearchLoc();

            if (m_bRange) {
                sl.m_Loc = searchLoc;
            }
            else {
                CRef<CSeq_id> id(new CSeq_id());
                id->Assign(*(searchLoc->GetId()));
                sl.m_Loc = new CSeq_loc();
                sl.m_Loc->SetWhole(*id);
            }

            sl.m_Scope = sl_ctx->GetSearchScope();            
            sl.m_ContextName = sl_ctx->GetDMContextName();
            sc_locs.push_back(sl);
        }
    }

    CIRef<IDMSearchQuery> ref
        (new COrfSearchQuery(sc_locs,
                             ToStdString(m_GencodeCombo->GetStringSelection()),
                             ToStdString(m_StartorfCombo->GetStringSelection()),
                             ToStdString(m_BasepairsText->GetValue())));
    return ref;
}



///////////////////////////////////////////////////////////////////////////////
/// COrfSearchJob

COrfSearchJob::COrfSearchJob(COrfSearchQuery& query, const COrfSearchForm::TKozakList& kozakList)
    : m_Query( &query )
    , m_ColorCol( -1 )
    , m_KozakList(kozakList)
{
}


bool COrfSearchJob::x_ValidateParams()
{
    m_Error.Reset();

    if( m_Query->GetScopedLocs().empty()) {
        m_Error = new CAppJobError("Invalid input parameters - no search context specified.");
    }

    return m_Error ? false : true;
}


IAppJob::EJobState COrfSearchJob::x_DoSearch()
{
    EJobState res_state = IAppJob::eFailed;
    TScopedLocs& locs   = m_Query->GetScopedLocs();

    vector<string> allowable_starts;
    if (m_Query->GetStartOrf().find("ATG")!=string::npos) {
        allowable_starts.push_back("ATG");
    }

    bool searchForward = true, searchReverse = true;
    bool bLongestORFs = false;

    ITERATE(vector<int>, iflt, m_Filters.selected) {
        string expr = m_Filters.filters[*iflt].second;
        if (expr.find("Strand=") == 0 && expr.size() > 7) {
            string value = expr.substr(7);
            if (value == "'+'")
                searchReverse = false;
            else if (value == "'-'")
                searchForward = false;
        }
        else if (expr == "LongestORFs")
            bLongestORFs = true;;
    }

    if (!searchForward && !searchReverse) 
        searchForward = searchReverse = true;

    string gc = m_Query->GetGencode();
    int gc_id = 1;
    if (gc != sAutoSelect)
        gc_id = x_DecodeGeneticCode(gc);

    ITERATE (TScopedLocs, it, locs) {
        TScopedLoc& sc_loc = const_cast<TScopedLoc&>(*it);
        CSeq_loc& loc = *sc_loc.m_Loc;
        CScope& scope = *sc_loc.m_Scope;

        string locLabel;

        if (IsCanceled())
            return eCanceled;

        // find the best ID for this bioseq
        try {
            locLabel.clear();
            CLabel::GetLabel(loc, &locLabel, CLabel::eDefault, &scope);

            CBioseq_Handle handle = scope.GetBioseqHandle(loc);
            if (gc == sAutoSelect) {
                CScope::TSeq_id_Handles handles;
                CSeq_id_Handle seq_id_h = 
                    sequence::GetId(handle, sequence::eGetId_Canonical);
                handles.push_back(seq_id_h);
                CScope::TTaxIds taxid_vector = scope.GetTaxIds(handles);
                int tax_id = 0;
                if (taxid_vector.size() > 0) {
                    tax_id = taxid_vector[0];
                    if (tax_id == -1) {
                        gc_id = 1;
                    } else {
                        CTaxon1 tax_client;
                        if (tax_client.Init()) {
                            const ITaxon1Node* node = NULL;
                            tax_client.LoadNode(tax_id, &node);
                            if (node) gc_id = node->GetGC(); 
                        }
                    }
                }
            }

            multimap<int, int> pos_feats, neg_feats;
            CFeat_CI it(scope, loc, SAnnotSelector(CSeqFeatData::e_Cdregion));
            for (; it; ++it) {
                CSeq_loc::TRange range = it->GetLocation().GetTotalRange();
                pos_feats.insert(multimap<int, int>::value_type((int)(range.GetTo()), (int)(range.GetFrom())));
                neg_feats.insert(multimap<int, int>::value_type((int)(range.GetFrom()), (int)(range.GetTo())));
            }

            // get sequence vector
            CSeqVector vec(loc, scope, CBioseq_Handle::eCoding_Ncbi);
            CSeqVector k_vec(loc, scope, CBioseq_Handle::eCoding_Iupac);

            // place to store orfs
            vector< CRef<CSeq_loc> > orfs;

            // find some ORFs
            COrf::FindOrfs(vec, orfs,
                           NStr::StringToInt(m_Query->GetMinPairs()),
                           gc_id,
                           allowable_starts,
                           bLongestORFs);
            sort(orfs.begin(), orfs.end());

            NON_CONST_ITERATE (vector< CRef<CSeq_loc> >, it2, orfs) {
                if (IsCanceled())
                    return eCanceled;

                (**it2).SetId(sequence::GetId(loc, &scope));
                *it2 = CSeqUtils::RemapChildToParent(loc, **it2);
                ENa_strand strand = (**it2).GetStrand();
                //if(strand != eNa_strand_plus  &&  strand != eNa_strand_minus

                int start = (int)(**it2).GetStart(eExtreme_Positional);
                int stop = (int)(**it2).GetStop(eExtreme_Positional);
                int bioseqLength = (int)handle.GetBioseqLength();

                CRef<CSeq_loc> loc_new(new CSeq_loc());

                int start1, stop1;

                start1 = max (start - 6, 0);
                stop1  = min(bioseqLength - 1, stop + 6);

                loc_new->SetInt().SetFrom(start1);
                loc_new->SetInt().SetTo (stop1);
                loc_new->SetInt().SetStrand(strand);
                loc_new->SetId(*(**it2).GetId());

                string seq_iupac;
                CSeqVector seqv(*loc_new, scope,  CBioseq_Handle::eCoding_Ncbi);
                seqv.SetCoding(CSeq_data::e_Iupacna);
                seqv.GetSeqData(0, seqv.size(), seq_iupac);

                int delta = (strand == eNa_strand_minus) ? stop1 - stop : start - start1;

                string s1 = seq_iupac.substr(0, delta);
                string s2 = seq_iupac.substr(delta, 3);
                string s3 = seq_iupac.substr(delta + 3, 2);
                string start_context = NStr::ToLower(s1) +  NStr::ToUpper(s2) + NStr::ToLower(s3);

                seq_iupac = seq_iupac.substr(delta, stop - start + 1);

                // do not create a feature here
                string seq_tran = "";
                CRef<CSeq_id> this_id(const_cast<CSeq_id*>(&sequence::GetId(**it2, &scope)));
                CRef<CSeq_feat> feat(new CSeq_feat());
                feat->SetExp_ev(CSeq_feat::eExp_ev_not_experimental);
                feat->SetData().SetCdregion().SetOrf(true);  // just an ORF
                // they're all frame 1 in this sense of 'frame'
                feat->SetData().SetCdregion().SetFrame(CCdregion::eFrame_one);
                feat->SetTitle("Open reading frame");

                // set up the location
                feat->SetLocation(const_cast<CSeq_loc&>(**it2));
                if (this_id) {
                    feat->SetLocation().SetId(*this_id);
                }

                CSeqTranslator::Translate(*feat, scope, seq_tran, true, true);

                string kozakSignal      = "None";
                int    kozakPosition    = -1;
                bool   bMinus = (strand == eNa_strand_minus);
                string strStrand = bMinus ? "-" : "+";

                if ((bMinus && !searchReverse) || (!bMinus && !searchForward))
                    continue;

                bool cdsExtension = false;

                if (bMinus) {
                    multimap<int, int>::const_iterator it2;
                    it2 = neg_feats.find(start);
                    while (it2 != neg_feats.end() && it2->first == start) {
                        if (stop > it2->second) {
                            cdsExtension = true;
                            break;
                        }
                        ++it2;
                    }
                }
                else {
                    multimap<int, int>::const_iterator it2;
                    it2 = pos_feats.find(stop);
                    while (it2 != pos_feats.end() && it2->first == stop) {
                        if (start < it2->second) {
                            cdsExtension = true;
                            break;
                        }
                        ++it2;
                    }
                }

                ITERATE (COrfSearchForm::TKozakList, koz, m_KozakList){                     
                    int codon_pos = koz->m_Codon_pos;
                    if (codon_pos > start)
                        continue;

                    loc_new->SetInt().SetFrom(start - codon_pos);
                    loc_new->SetInt().SetTo (min(bioseqLength - 1, start + 4));
                    loc_new->SetInt().SetStrand(strand);
                    loc_new->SetId(*(**it2).GetId());

                    string tmp;
                    CSeqVector seqv2(*loc_new, scope,  CBioseq_Handle::eCoding_Ncbi);
                    seqv2.SetCoding(CSeq_data::e_Iupacna);
                    seqv2.GetSeqData(0, seqv2.size(), tmp);

                    CRegexp rexp(koz->m_Regexp, CRegexp::fCompile_ignore_case);
                    string match = rexp.GetMatch(tmp);
                    if (!match.empty()) {
                        kozakPosition = start - codon_pos;                            
                        kozakSignal   = koz->m_Name;
                        break;
                    }
                }

                if (cdsExtension)
                    kozakSignal += " [CDS Extension]";

                int color = 0; //cdsExtension ? 1 : 0;
                //if (color && (kozakSignal == "Strong" || kozakSignal == "Optimal"))
                    //color = 2;
            
                // adding to grid
                x_AddToResults(**it2,
                                //*feat,
                                scope,
                                (int)(*it2)->GetTotalRange().GetFrom()+1,
                                (int)(*it2)->GetTotalRange().GetTo()+1,
                                strStrand,
                                (int)(*it2)->GetTotalRange().GetLength(),
                                max<int>(0, seq_tran.length() - 1),
                                seq_iupac.substr(0, 3),
                                seq_iupac.substr(seq_iupac.length()-3),
                                seq_tran,
                                seq_iupac,
                                start_context,
                                sc_loc.m_ContextName,
                                kozakSignal,
                                kozakPosition,
                                color);
            }
            res_state = IAppJob::eCompleted;
        } NCBI_CATCH("Error processing location \"" + locLabel + "\".");
    }

    return res_state;
}


void COrfSearchJob::x_SetupColumns( CObjectList& obj_list )
{
	obj_list.ClearObjectLabels();
	obj_list.AddObjectLabel( "Label", CLabel::eContent );

    obj_list.AddColumn( CObjectList::eInteger, "Start" );
    obj_list.AddColumn( CObjectList::eInteger, "Stop" );
    obj_list.AddColumn( CObjectList::eString,  "Strand" );
    obj_list.AddColumn( CObjectList::eInteger, "ORF Length" );
    obj_list.AddColumn( CObjectList::eInteger, "Protein Length" );
    obj_list.AddColumn( CObjectList::eString, "Begining Codon" );
    obj_list.AddColumn( CObjectList::eString, "Terminal Codon" );
    obj_list.AddColumn( CObjectList::eString, "Translated Sequence" );
    obj_list.AddColumn( CObjectList::eString, "Start Context" );
    obj_list.AddColumn( CObjectList::eString, "Nucleotide Sequence" );
    obj_list.AddColumn( CObjectList::eString, "Kozak Signal" );
    obj_list.AddColumn( CObjectList::eInteger, "Kozak Location" );
    obj_list.AddColumn( CObjectList::eString, "Context" );

    m_ColorCol = obj_list.AddColumn( CObjectList::eInteger, "Color" );
}


// figure out the id of the genetic code the user wants
int COrfSearchJob::x_DecodeGeneticCode(const string& s)
{
    const CGenetic_code_table& code_table = CGen_code_table::GetCodeTable();
    const CGenetic_code_table::Tdata& codes = code_table.Get();
    ITERATE (CGenetic_code_table::Tdata, code, codes) {
        if ((*code)->GetName() == s) {
            return (*code)->GetId();
        }
    }
    // if we got here, nothing matched
    NCBI_THROW(CException, eUnknown,
               "CAlgoPlugin_FindOrfs: no genetic code matched " + s);
}


void COrfSearchJob::x_AddToResults(CObject& obj, CScope& scope,
                                   int   start,
                                   int   stop,
                                   const string& strand,
                                   int   length,
                                   int   p_length,
                                   const string& start_cdn,
                                   const string& stop_cdn,
                                   const string& translated_seq,
                                   const string& nucleotide_seq,
                                   const string& start_context,
                                   const string& ctx_name,
                                   const string& kozak,
                                   int   k_loc,
                                   int   color)
{
    static const int kUpdateIncrement = 250;

    /// adding the result to the Accumulator
    int row = m_AccList.AddRow(&obj, &scope);
    int col = 0;
    // it should be integer, but object list widget throws asserts
    // even with properly setuped to CObjectList::eInteger type and SetInteger used...
    m_AccList.SetInteger(col++, row, start);
    m_AccList.SetInteger(col++, row, stop);
    m_AccList.SetString(col++, row, strand);
    m_AccList.SetInteger(col++, row, length);
    m_AccList.SetInteger(col++, row, p_length);
    m_AccList.SetString(col++, row, start_cdn);
    m_AccList.SetString(col++, row, stop_cdn);
    m_AccList.SetString(col++, row, translated_seq);
    m_AccList.SetString(col++, row, start_context);
    m_AccList.SetString(col++, row, nucleotide_seq);
    m_AccList.SetString(col++, row, kozak);
    m_AccList.SetInteger(col++, row, k_loc + 1);

    m_AccList.SetString(col++, row, ctx_name);
    m_AccList.SetInteger(col++, row, color);

    int count = m_AccList.GetNumRows();
    if(count >= kUpdateIncrement)   {
        // time to update the Result
        CMutexGuard Guard(m_Mutex);

        /// transfer results from Accumulator to m_TempResult
        m_TempResult->GetObjectList()->Append(m_AccList);
        m_AccList.ClearRows();

        /// update progress string
        m_ProgressStr = NStr::IntToString(count, NStr::fWithCommas) + " ORF";
        if (count != 1) {
            m_ProgressStr += "'s";
        }
        m_ProgressStr += " found.";
    }
}

CObjectListTableModel* COrfSearchJob::x_GetNewOLTModel() const
{
    COrfSearchTableModel* model = new COrfSearchTableModel();
    model->SetColorColumn( m_ColorCol );

    return model;
}

END_NCBI_SCOPE
