#ifndef GUI_OBJUTILS___TABLE_QUERY_EXEC__HPP
#define GUI_OBJUTILS___TABLE_QUERY_EXEC__HPP

/*  $Id: table_query_exec.hpp 39021 2017-07-20 16:37:55Z falkrb $
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
 * Authors: Robert Falk
 *
 * File Description: 
 * Header file for classes needed to implement query execution.
 *
 */

#include <gui/gui_export.h>

#include <objmgr/scope.hpp>

#include <util/qparse/query_exec.hpp>
#include <gui/objutils/table_data.hpp>
#include <gui/objutils/macro_query_exec.hpp>

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// class CTableQueryExec
///
/// Subclass of CQueryExec that will execute a query against a data source derived
/// from ITableData.
///  
///  
class NCBI_GUIOBJUTILS_EXPORT CTableQueryExec : public CMacroQueryExec
{
public:
    CTableQueryExec(ITableData& td);

    virtual ~CTableQueryExec() {}

    /// Set current node for query execution
    //void SetNode(CPhyloTreeNode *node) { m_EvalNode = node; }
    void SetRow(unsigned int idx) {}

    /// @name Find and convert feature-list values in individual nodes
    /// @{    
    /// Search for bool 'identifier' in feature list and return if found
    virtual bool ResolveIdentifier(const std::string& identifier, 
                                   bool& value);    

    /// Search for integer 'identifier' in feature list and return if found
    virtual bool ResolveIdentifier(const std::string& identifier, 
                                   Int8& value);

    /// Search for float 'identifier' in feature list and return if found
    virtual bool ResolveIdentifier(const std::string& identifier, 
                                   double& value);

    /// Search for string 'identifier' in feature list and return if found
    virtual bool ResolveIdentifier(const std::string& identifier, 
                                   std::string& value);
    /// Following functions look up field using column number
    virtual bool ResolveIdentifier(const TFieldID& id, bool& value);
    virtual bool ResolveIdentifier(const TFieldID& id, Int8& value);
    virtual bool ResolveIdentifier(const TFieldID& id, double& value);
    virtual bool ResolveIdentifier(const TFieldID& id, std::string& value);
    /// @}

    /// Get column number for identifier or return TFieldID(-1)
    virtual TFieldID GetIdentifier(const std::string& identifier);
    
    /// Search for 'identifier' in dictionary and return true if it exists. 
    /// This does not mean that the value is defined for a particular node.
    virtual bool HasIdentifier(const std::string& identifier);

    /// Return one of eIntConst, eBoolConst, eFloatConst, eString, or eNotSet.
    virtual CQueryParseNode::EType IdentifierType(const std::string& identifier); 

    /// Move to the first row for eval, return false if table empty
    virtual void EvalStart();
    /// Move to the next row for eval, return false if table size < m_EvalRow+1
    virtual bool EvalNext(CQueryParseTree& qtree);
    virtual bool EvalComplete();

    std::vector<size_t> GetTableSelected() const { return m_Selected; }
    
protected:


    /// return index of col_name or (unsigned int)-1 if it is not found
    unsigned int x_GetColumnIdx(const std::string& col_name);

    /// current table row for evaluation purposes
    unsigned int m_EvalRow;

    /// pointer to data including data dictionary (columm names)
    CIRef<ITableData> m_TableData;

    /// currently selected rows from table
    std::vector<size_t> m_Selected;
};



END_NCBI_SCOPE


#endif  // GUI_OBJUTILS___TABLE_QUERY_EXEC__HPP

