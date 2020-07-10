/*  $Id: feat_table_ds.cpp 40280 2018-01-19 17:54:35Z katargir $
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
 * Authors:  Mike DiCuccio, Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/feat_table/feat_table_ds.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>

#include <gui/utils/app_job.hpp>
#include <gui/utils/app_job_impl.hpp>
#include <gui/utils/event_translator.hpp>

#include <objmgr/feat_ci.hpp>
#include <objmgr/util/feature.hpp>
#include <objmgr/util/sequence.hpp>

#include <objects/seqloc/Seq_bond.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_loc_equiv.hpp>
#include <objects/seqloc/Seq_point.hpp>


BEGIN_NCBI_SCOPE

USING_SCOPE(objects);


static int s_CountIntervals( const CSeq_loc& loc ){

    switch( loc.Which() ){
    case CSeq_loc::e_Null:
    case CSeq_loc::e_Empty:
    case CSeq_loc::e_Whole:
    case CSeq_loc::e_Int:
    case CSeq_loc::e_Pnt:
    case CSeq_loc::e_Bond:
    case CSeq_loc::e_Feat:
        return 1;

    case CSeq_loc::e_Packed_int:
        return loc.GetPacked_int().Get().size();

    case CSeq_loc::e_Packed_pnt:
        return loc.GetPacked_pnt().GetPoints().size();

    case CSeq_loc::e_Mix:
        {{
            int ivals = 0;
            ITERATE( CSeq_loc::TMix::Tdata, iter, loc.GetMix().Get() ){
                ivals += s_CountIntervals(**iter);
            }
            return ivals;
        }}

    default:
        {{
            int size = 0;
            for( CSeq_loc_CI iter( loc );  iter;  ++iter ){
                ++size;
            }
            return size;
        }}
    }
}


/////////////////////////////////////////////////////////////////////////////
///
/// AppJobs for background loading and preparation
///
class CFeatureRetrievalResult : public CObject
{
public:
    vector<CFeatTableDS::SFeatureRow> m_Features;
};


/////////////////////////////////////////////////////////////////////////////
///
///  Alignment-based data source
///
class CFeatureRetrievalJob : public CJobCancelable
{
public:
    CFeatureRetrievalJob( const CSerialObject& obj, CScope& scope, SAnnotSelector& sel );

    /// @name IAppJob implementation
    /// @{
    virtual EJobState                   Run();
    virtual CConstIRef<IAppJobProgress> GetProgress();
    virtual CRef<CObject>               GetResult();
    virtual CConstIRef<IAppJobError>    GetError();
    virtual string                      GetDescr() const;
    /// @}

protected:

    CConstRef<CSerialObject> m_Object;
    CRef<CScope> m_Scope;
    SAnnotSelector m_Sel;

    /// for status reporting
    size_t m_TotalComparisons;
    size_t m_CompletedComparisons;

    // for job results/status
    CRef<CAppJobError> m_Error;
    CRef<CFeatureRetrievalResult> m_Result;
};


/////////////////////////////////////////////////////////////////////////////

CFeatureRetrievalJob::CFeatureRetrievalJob(
    const CSerialObject& obj, CScope& scope, SAnnotSelector& sel
)
    : m_Object(&obj)
    , m_Scope(&scope)
    , m_Sel(sel)
    , m_TotalComparisons(1)
    , m_CompletedComparisons(0)
{
}


CConstIRef<IAppJobProgress> CFeatureRetrievalJob::GetProgress()
{
    CRef<CAppJobProgress> p( new CAppJobProgress() );
    p->SetNormDone( float(m_CompletedComparisons) / float(m_TotalComparisons) );
    return CConstIRef<IAppJobProgress>( p.GetPointer() );
}


CRef<CObject> CFeatureRetrievalJob::GetResult()
{
    return CRef<CObject>( m_Result.GetPointer() );
}


CConstIRef<IAppJobError> CFeatureRetrievalJob::GetError()
{
    return CConstIRef<IAppJobError>( m_Error.GetPointer() );
}


string CFeatureRetrievalJob::GetDescr() const
{
    return string( "CFeatureRetrievalJob" );
}


CFeatureRetrievalJob::EJobState CFeatureRetrievalJob::Run()
{
    m_Error.Reset( NULL );
    m_Result.Reset( new CFeatureRetrievalResult() );

    try {
        const CSerialObject* object = m_Object.GetPointer();
        const CSeq_id* seq_id = NULL;

        CFeat_CI feat_iter;

        for(;;){
            const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(object);
            if( annot ){
                feat_iter = CFeat_CI( m_Scope->GetSeq_annotHandle( *annot ), m_Sel );
                break;
            }

            const CSeq_loc* seq_loc = dynamic_cast<const CSeq_loc*>(object);
            if( seq_loc ){
                feat_iter = CFeat_CI( *m_Scope, *seq_loc, m_Sel );
                seq_id = seq_loc->GetId();

                break;
            }

            seq_id = dynamic_cast<const CSeq_id*>(object);
            if( seq_id ){
                feat_iter = CFeat_CI( m_Scope->GetBioseqHandle( *seq_id ), m_Sel );
                break;
            }

            break;
        }

        m_Result -> m_Features.reserve( feat_iter.GetSize() );

        /// now, find features to be loaded
        for( int i = 0; feat_iter && !IsCanceled(); ++feat_iter ){
            const CMappedFeat& mapped = *feat_iter;
            const CSeq_loc& mapped_loc = mapped.GetLocation();
            if( mapped_loc.GetId() != NULL ){
                CFeatTableDS::SFeatureRow proxy( i++, mapped );
                proxy.FillIn();

                m_Result->m_Features.push_back( proxy );

            } else if( seq_id != NULL ){
                CSeq_loc::TRange range;
                for( CSeq_loc_CI ci( mapped_loc ); ci; ++ci ){
                    const CSeq_id& part_id = ci.GetSeq_id();
                    if( !sequence::IsSameBioseq( *seq_id, part_id, m_Scope ) ){
                        continue;
                    }

                    range += ci.GetRange();
                }

                CFeatTableDS::SFeatureRow proxy;
                proxy.row_idx = i++;
                proxy.FillIn( mapped, range );

                m_Result->m_Features.push_back( proxy );

            } else {
                CSeq_loc::TRange range;
                for( CSeq_loc_CI ci( mapped_loc ); ci; ++ci ){
                    range = ci.GetRange();

                    CFeatTableDS::SFeatureRow proxy;
                    proxy.row_idx = i++;
                    proxy.FillIn( mapped, range );

                    m_Result->m_Features.push_back( proxy );
                }
            }
        }
    } catch( CException& ex ){
        LOG_POST(Error << "Feature Table retrieval:" << ex.what() );

        m_Error.Reset( new CAppJobError( ex.what() ) );

        return eFailed;
    }

    return IsCanceled() ? eCanceled : eCompleted;
}


/////////////////////////////////////////////////////////////////////////////
// CFeatTableDS implementation
void CFeatTableDS::SFeatureRow::FillIn( const objects::CMappedFeat& map_feat )
{
    if( filledIn ){
        return;
    }

    const CSeq_feat& feat = map_feat.GetOriginalFeature();
    CScope& scope = map_feat.GetAnnot().GetScope();
    CLabel::GetLabel( feat, &label, CLabel::eDefault, &scope );
    feature::GetLabel( feat, &type, feature::fFGL_Type, &scope );

    CSeq_loc::TRange range = map_feat.GetLocation().GetTotalRange();
    from = (int) range.GetFrom() +1;
    to = (int) range.GetTo() +1;
    length = (int) range.GetLength();
    partial = false;

    ENa_strand naStrand = sequence::GetStrand( map_feat.GetLocation() );
    switch( naStrand ){
    case eNa_strand_unknown:
        strand = "?";
        break;
    case eNa_strand_plus:
        strand = "+";
        break;
    case eNa_strand_minus:
        strand = "-";
        break;
    case eNa_strand_both:
        strand = "+-";
        break;
    case eNa_strand_both_rev:
        strand = "+- (rev)";
        break;
    case eNa_strand_other:
        strand = "other";
        break;
    }

    if( map_feat.IsSetProduct() ){
        const CSeq_loc& loc = map_feat.GetProduct();
        CLabel::GetLabel( loc, &product, CLabel::eDefault, &scope );
    }

    intervals = s_CountIntervals( map_feat.GetLocation() );

    filledIn = true;
}

void CFeatTableDS::SFeatureRow::FillIn( const CMappedFeat& map_feat, CSeq_loc::TRange& range )
{
    if( filledIn ){
        return;
    }

    const CSeq_feat& feat = map_feat.GetOriginalFeature();
    CScope& scope = map_feat.GetAnnot().GetScope();
    CLabel::GetLabel( feat, &label, CLabel::eDefault, &scope );
    feature::GetLabel( feat, &type, feature::fFGL_Type, &scope );

    from = (int) range.GetFrom() +1;
    to = (int) range.GetTo() +1;
    length = (int) range.GetLength();
    partial = true;

    ENa_strand naStrand = sequence::GetStrand( map_feat.GetLocation() );
    switch( naStrand ){
    case eNa_strand_unknown:
        strand = "?";
        break;
    case eNa_strand_plus:
        strand = "+";
        break;
    case eNa_strand_minus:
        strand = "-";
        break;
    case eNa_strand_both:
        strand = "+-";
        break;
    case eNa_strand_both_rev:
        strand = "+- (rev)";
        break;
    case eNa_strand_other:
        strand = "other";
        break;
    }

    if( map_feat.IsSetProduct() ){
        const CSeq_loc& loc = map_feat.GetProduct();
        CLabel::GetLabel( loc, &product, CLabel::eDefault, &scope );
    }

    intervals = s_CountIntervals( map_feat.GetLocation() );

    filledIn = true;
}



CFeatTableDS::CFeatTableDS()
    : m_ActiveJob( -1 )
{
}

CFeatTableDS::~CFeatTableDS()
{
    Clear();
}

void CFeatTableDS::Init( const CSerialObject& obj, objects::CScope& scope,  const objects::SAnnotSelector* sel)
{
    m_Object.Reset( &obj );
    m_Scope.Reset( &scope );

    if( sel == NULL ){
        m_Selector = CSeqUtils::GetAnnotSelector( CSeq_annot::TData::e_Ftable );
        m_Selector
            .SetExcludeExternal( true )
            .ExcludeFeatSubtype( CSeqFeatData::eSubtype_variation )
            //.SetResolveNone()
            .SetAdaptiveDepth()
        ;
    } else {
        m_Selector = *sel;
    }

    Update();
}

void CFeatTableDS::SetSelector( const SAnnotSelector& sel )
{
    m_Selector = sel;

    Update();
}

void CFeatTableDS::Update()
{
    CRef<CFeatureRetrievalJob> job( new CFeatureRetrievalJob( *m_Object, *m_Scope, m_Selector ) );
    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();

    ClearCurrentJob();

    try {
        m_ActiveJob = disp.StartJob( *job, "ObjManagerEngine", *this, -1, true );

    } catch( CAppJobException& e ){
        LOG_POST( Error << "CFeatTableDS::Update(): Failed to start job: " << e.GetMsg() );
        LOG_POST( Error << e.ReportAll() );
    }
}

void CFeatTableDS::ClearCurrentJob()
{
    if( m_ActiveJob != -1 ){
        /// delete any active jobs
        CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
        try {
            disp.DeleteJob( m_ActiveJob );

        } catch( CAppJobException& e ){
            switch( e.GetErrCode() ) {
            case CAppJobException::eUnknownJob:
            case CAppJobException::eEngine_UnknownJob:
                /// this is fine - job probably already finished
                break;

            default:
                // something wrong
                LOG_POST( Error << "CFeatTableDS::ClearCurrentJob(): " << e.GetMsg() );
                LOG_POST( Error << e.ReportAll() );
            }
        }

        m_ActiveJob = -1;
    }
}

void CFeatTableDS::Clear()
{
    ClearCurrentJob();

    m_FeatureRows.clear();

    x_UpdateTypeHash();
    x_FireDataChanged();
    x_NotifyObserversNow();
}

void CFeatTableDS::x_UpdateTypeHash()
{
    m_TypeHash.clear();
    ITERATE( vector<SFeatureRow>, it, m_FeatureRows ){
        m_TypeHash[it->type] = "";
    }
}

void CFeatTableDS::GetTypeNames( vector<string>& names ) const
{
    ITERATE( TTypeHash, it, m_TypeHash ){
        names.push_back( it->first );
    }
}

BEGIN_EVENT_MAP( CFeatTableDS, CEventHandler )
    ON_EVENT(CAppJobNotification, CAppJobNotification::eStateChanged, &CFeatTableDS::OnJobNotification)
END_EVENT_MAP()


void CFeatTableDS::OnJobNotification( CEvent* evt )
{
    x_OnJobNotification( evt );
}


void CFeatTableDS::x_OnJobNotification( CEvent* evt )
{
    CAppJobNotification* notif = dynamic_cast<CAppJobNotification*>( evt );
    _ASSERT( notif );
    if( !notif ){
        return;
    }

    int job_id = notif -> GetJobID();
    if( job_id != -1 && job_id != m_ActiveJob ){
        return;
    }

    switch( notif -> GetState() ){
    case IAppJob::eCompleted:
        {{
            CRef<CObject> res_obj = notif -> GetResult();
            if( !res_obj.IsNull() ){
                CFeatureRetrievalResult* result =
                    dynamic_cast<CFeatureRetrievalResult*>( &*res_obj )
                ;
                m_FeatureRows.clear();
                m_FeatureRows.swap( result -> m_Features );

                x_UpdateTypeHash();
                x_FireDataChanged();
            }
        }}

        m_ActiveJob = -1;
        x_NotifyObserversNow();
        break;

    case IAppJob::eFailed:
    case IAppJob::eCanceled:
        m_ActiveJob = -1;
        x_NotifyObserversNow();
        break;

    default:
        break;
    }
}

const CFeatTableDS::SFeatureRow& CFeatTableDS::GetFeatureRow( size_t row ) const
{
    _ASSERT( (int)row < GetNumRows() );

    return m_FeatureRows[row];
}


const CSeq_feat& CFeatTableDS::GetFeature( size_t row ) const
{
    return GetFeatureRow( row ).feat.GetOriginalFeature();
}

void CFeatTableDS::x_NotifyObserversNow()
{
    Send(CRef<CEvent>(new CFeatTableEvent()));
}


////////////////////////////////////////////////////
// Part of ITableModel implementation

static wxString s_FeatTableColNames[] = {
    wxT( "#" ),
    wxT("Label"),
    wxT("Type"),
    wxT("Start"),
    wxT("Stop"),
    wxT("Length"),
    wxT("Strand"),
    wxT("Product"),
    wxT("Intervals")
};
static const int s_FeatTableNumCols = sizeof(s_FeatTableColNames) / sizeof(wxString);


int CFeatTableDS::GetNumColumns() const
{
    return s_FeatTableNumCols;
}

wxString CFeatTableDS::GetColumnName( int aColIx ) const
{
    _ASSERT( aColIx < GetNumColumns() );

    if( aColIx < s_FeatTableNumCols ){
        return s_FeatTableColNames[aColIx];

    } else {
        return CwxAbstractTableModel::GetColumnName( aColIx );
    }
}

wxString CFeatTableDS::GetColumnType( int aColIx ) const
{
    _ASSERT( aColIx < GetNumColumns() );

    switch( aColIx ){
    case eLabel:
    case eType:
    case eStrand:
    case eProduct:
        return wxT("string");
    case eNum:
    case eFrom:
    case eTo:
    case eLength:
    case eIntervals:
        return wxT("int");
    default:
        return wxT("char");
    }
}

int CFeatTableDS::GetNumRows() const
{
    return m_FeatureRows.size();
}

wxVariant CFeatTableDS::GetValueAt( int row, int col ) const
{
    _ASSERT( row < (int)GetNumRows() );

    wxString rv;
    switch( col ){
    case eNum:
        return row+1;
    case eLabel:
        return ToWxString( m_FeatureRows[row].label );

    case eType:
        rv = ToWxString( m_FeatureRows[row].type );
        if( m_FeatureRows[row].partial ){
            rv += wxT(" (partial)");
        }
        return rv;
    
    case eFrom:
        return m_FeatureRows[row].from;
    case eTo:
        return m_FeatureRows[row].to;
    case eLength:
        return m_FeatureRows[row].length;
    case eStrand:
        return ToWxString( m_FeatureRows[row].strand );
    case eProduct:
        return ToWxString( m_FeatureRows[row].product );
    case eIntervals:
        return m_FeatureRows[row].intervals;
    default:
        break;
    }

    return ToWxString( kEmptyStr );
}

SFeatureFilter::SFeatureFilter()
    : m_Disabled( false )
    , m_Label( wxEmptyString ), m_Types(), m_FromLoc( -1 ), m_ToLoc( -1 ), m_InclLoc( false )
    , m_LengthLoc( -1 ), m_Product( wxEmptyString ), m_NoProduct( false )
{
}

bool SFeatureFilter:: operator()( const IRowModel& aRow ) const
{
    if( m_Disabled ){
        return true;
    }

    if( !m_Label.empty() ){
        wxString line =
            aRow.GetValueAtColumn( CFeatTableDS::eLabel ).GetString().Upper()
        ;

        size_t ix = line.Find( m_Label.Upper() );
        if( ix == wxNOT_FOUND ){
            return false;
        }
    }

    if( !m_Types.IsEmpty() ){
        wxString line =
            aRow.GetValueAtColumn( CFeatTableDS::eType ).GetString()
        ;

        size_t ix = m_Types.Index( line, false, false );
        if( ix == wxNOT_FOUND ){
            return false;
        }
    }

    size_t from = aRow.GetValueAtColumn( CFeatTableDS::eFrom ).GetLong();
    size_t to = aRow.GetValueAtColumn( CFeatTableDS::eTo ).GetLong();

    if( m_InclLoc ){
        if(
            !( (m_FromLoc < 0 || (size_t)m_FromLoc < from) 
            && (m_ToLoc < 0 || to < (size_t)m_ToLoc) )
        ){
            return false;
        }
    } else {
        if(
            !( (m_FromLoc < 0 || (size_t)m_FromLoc < to)
            && (m_ToLoc < 0 || from < (size_t)m_ToLoc) )
        ){
            return false;
        }
    }

    if( m_LengthLoc > 0 ){
        if( m_LengthLoc != aRow.GetValueAtColumn( CFeatTableDS::eLength ).GetLong() ){
            return false;
        }
    }

    wxString prod =
        aRow.GetValueAtColumn( CFeatTableDS::eProduct ).GetString().Upper()
    ;

    if( m_NoProduct ){
        if( !prod.empty() ){
            return false;
        }
    } else if( !m_Product.empty() ){
        if( prod.empty() ){
            return false;
        }

        size_t ix = prod.Find( m_Product.Upper() );
        if( ix == wxNOT_FOUND ){
            return false;
        }
    }

    return true;
}

END_NCBI_SCOPE
