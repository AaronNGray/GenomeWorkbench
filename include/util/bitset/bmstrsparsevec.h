#ifndef BMSTRSPARSEVEC__H__INCLUDED__
#define BMSTRSPARSEVEC__H__INCLUDED__
/*
Copyright(c) 2002-2017 Anatoliy Kuznetsov(anatoliy_kuznetsov at yahoo.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

For more information please visit:  http://bitmagic.io
*/

/*! \file bmstrsparsevec.h
    \brief string sparse vector based on bit-transposed matrix
*/

#include <stddef.h>
#include "bmconst.h"

#ifndef BM_NO_STL
#include <stdexcept>
#endif

#ifndef BM__H__INCLUDED__
// BitMagic utility headers do not include main "bm.h" declaration 
// #include "bm.h" or "bm64.h" explicitly 
# error missing include (bm.h or bm64.h)
#endif

#include "bmtrans.h"
#include "bmalgo.h"
#include "bmbuffer.h"
#include "bmbmatrix.h"
#include "bmdef.h"

namespace bm
{

/*!
   \brief sparse vector for strings with compression using bit transposition method
 
   Initial string is bit-transposed into bit-planes so collection may use less
   memory due to prefix sum (GAP) compression in bit-plains.
 
   @ingroup sv
*/
template<typename CharType, typename BV, unsigned MAX_STR_SIZE>
class str_sparse_vector : public base_sparse_vector<CharType, BV, MAX_STR_SIZE>
{
public:
    typedef BV                                       bvector_type;
    typedef bvector_type*                            bvector_type_ptr;
    typedef const bvector_type*                      bvector_type_const_ptr;
    typedef CharType                                 value_type;
    typedef typename bvector_type::size_type         size_type;
    typedef typename BV::allocator_type              allocator_type;
    typedef typename bvector_type::allocation_policy allocation_policy_type;
    typedef typename bvector_type::enumerator        bvector_enumerator_type;
    typedef typename allocator_type::allocator_pool_type allocator_pool_type;
    typedef bm::basic_bmatrix<BV>                    bmatrix_type;
    typedef base_sparse_vector<CharType, BV, MAX_STR_SIZE> parent_type;
    
    /*! Statistical information about  memory allocation details. */
    struct statistics : public bv_statistics
    {};
    
    enum octet_plains
    {
        sv_octet_plains = MAX_STR_SIZE
    };
    
    typedef
    bm::heap_matrix<unsigned char,
                    MAX_STR_SIZE, // ROWS
                    256,          // COLS
                    typename bvector_type::allocator_type>
                                    plain_octet_matrix_type;

    struct is_remap_support { enum trait { value = true }; };
    struct is_rsc_support { enum trait { value = false }; };

    /**
         Reference class to access elements via common [] operator
         @ingroup sv
    */
    class const_reference
    {
    public:
        const_reference(const str_sparse_vector<CharType, BV, MAX_STR_SIZE>& str_sv,
                  size_type idx) BMNOEXCEPT
        : str_sv_(str_sv), idx_(idx)
        {}
        
        operator const value_type*() const BMNOEXCEPT
        {
            str_sv_.get(idx_, buf_, MAX_STR_SIZE);
            return &(buf_[0]);
        }

        bool operator==(const const_reference& ref) const BMNOEXCEPT
                                { return bool(*this) == bool(ref); }
        bool is_null() const BMNOEXCEPT { return str_sv_.is_null(idx_); }
    private:
        const str_sparse_vector<CharType, BV, MAX_STR_SIZE>& str_sv_;
        size_type                                            idx_;
        mutable CharType                                    buf_[MAX_STR_SIZE];
    };

    /**
         Reference class to access elements via common [] operator
         @ingroup sv
    */
    class reference
    {
    public:
        reference(str_sparse_vector<CharType, BV, MAX_STR_SIZE>& str_sv,
                  size_type idx) BMNOEXCEPT
        : str_sv_(str_sv), idx_(idx)
        {}
        
        operator const value_type*() const BMNOEXCEPT
        {
            str_sv_.get(idx_, buf_, MAX_STR_SIZE);
            return &(buf_[0]);
        }

        reference& operator=(const reference& ref)
        {
            // TO DO: implement element copy bit by bit
            str_sv_.set(idx_, (const value_type*)ref);
            return *this;
        }

        reference& operator=(const value_type* str)
        {
            str_sv_.set(idx_, str);
            return *this;
        }
        bool operator==(const reference& ref) const BMNOEXCEPT
                                { return bool(*this) == bool(ref); }
        bool is_null() const BMNOEXCEPT { return str_sv_.is_null(idx_); }
    private:
        str_sparse_vector<CharType, BV, MAX_STR_SIZE>& str_sv_;
        size_type                                      idx_;
        mutable CharType                               buf_[MAX_STR_SIZE];
    };
    
    /**
        Const iterator to do quick traverse of the sparse vector.
     
        Implementation uses buffer for decoding so, competing changes
        to the original vector may not match the iterator returned values.
     
        This iterator keeps an operational buffer of transposed elements,
        so memory footprint is not negligable.
     
        @ingroup sv
    */
    class const_iterator
    {
    public:
    friend class str_sparse_vector;
#ifndef BM_NO_STL
        typedef std::input_iterator_tag  iterator_category;
#endif
        typedef str_sparse_vector<CharType, BV, MAX_STR_SIZE> str_sparse_vector_type;
        typedef str_sparse_vector_type*                       str_sparse_vector_type_ptr;
        typedef typename str_sparse_vector_type::value_type   value_type;
        typedef typename str_sparse_vector_type::size_type    size_type;
        typedef typename str_sparse_vector_type::bvector_type bvector_type;
        typedef typename bvector_type::allocator_type         allocator_type;
        typedef typename allocator_type::allocator_pool_type  allocator_pool_type;
        
        typedef long long                   difference_type;
        typedef CharType*                   pointer;
        typedef CharType*&                  reference;
    public:
        const_iterator() BMNOEXCEPT;
        const_iterator(const str_sparse_vector_type* sv) BMNOEXCEPT;
        const_iterator(const str_sparse_vector_type* sv, size_type pos) BMNOEXCEPT;
        const_iterator(const const_iterator& it) BMNOEXCEPT;
        
        bool operator==(const const_iterator& it) const BMNOEXCEPT
                                { return (pos_ == it.pos_) && (sv_ == it.sv_); }
        bool operator!=(const const_iterator& it) const BMNOEXCEPT
                                { return ! operator==(it); }
        bool operator < (const const_iterator& it) const BMNOEXCEPT
                                { return pos_ < it.pos_; }
        bool operator <= (const const_iterator& it) const BMNOEXCEPT
                                { return pos_ <= it.pos_; }
        bool operator > (const const_iterator& it) const BMNOEXCEPT
                                { return pos_ > it.pos_; }
        bool operator >= (const const_iterator& it) const BMNOEXCEPT
                                { return pos_ >= it.pos_; }

        /// \brief Get current position (value)
        const value_type* operator*() const BMNOEXCEPT { return this->value(); }

        /// \brief Advance to the next available value
        const_iterator& operator++() BMNOEXCEPT
            { this->advance(); return *this; }

        /// \brief Advance to the next available value
        const_iterator& operator++(int) BMNOEXCEPT
            { const_iterator tmp(*this);this->advance(); return tmp; }


        /// \brief Get current position (value)
        const value_type* value() const BMNOEXCEPT;

        /// \brief Get NULL status
        bool is_null() const BMNOEXCEPT { return sv_->is_null(this->pos_); }

        /// Returns true if iterator is at a valid position
        bool valid() const BMNOEXCEPT { return pos_ != bm::id_max; }

        /// Invalidate current iterator
        void invalidate() BMNOEXCEPT { pos_ = bm::id_max; }

        /// Current position (index) in the vector
        size_type pos() const BMNOEXCEPT { return pos_; }

        /// re-position to a specified position
        void go_to(size_type pos) BMNOEXCEPT;

        /// advance iterator forward by one
        void advance() BMNOEXCEPT;

    protected:
        typedef bm::heap_matrix<CharType,
                        1024,         // ROWS: number of strings in one batch
                        MAX_STR_SIZE, // COLS
                        allocator_type> buffer_matrix_type;

    private:
        const str_sparse_vector_type*     sv_;      ///!< ptr to parent
        mutable size_type                 pos_;     ///!< Position
        mutable buffer_matrix_type        buf_matrix_; ///!< decode value buffer
        mutable size_type                 pos_in_buf_; ///!< buffer position
    };


    /**
        Back insert iterator implements buffered insert, faster than generic
        access assignment.
     
        Limitations for buffered inserter:
        1. Do not use more than one inserter (into one vector) at the same time
        2. Use method flush() at the end to send the rest of accumulated buffer
        flush is happening automatically on destruction, but if flush produces an
        exception (for whatever reason) it will be an exception in destructor.
        As such, explicit flush() is safer way to finilize the sparse vector load.

        @ingroup sv
    */
    class back_insert_iterator
    {
    public:
#ifndef BM_NO_STL
        typedef std::output_iterator_tag  iterator_category;
#endif
        typedef str_sparse_vector<CharType, BV, MAX_STR_SIZE>  str_sparse_vector_type;
        typedef str_sparse_vector_type*                        str_sparse_vector_type_ptr;
        typedef typename str_sparse_vector_type::value_type    value_type;
        typedef typename str_sparse_vector_type::size_type     size_type;
        typedef typename str_sparse_vector_type::bvector_type  bvector_type;
        typedef typename bvector_type::allocator_type          allocator_type;
        typedef typename allocator_type::allocator_pool_type   allocator_pool_type;

        typedef void difference_type;
        typedef void pointer;
        typedef void reference;
        
    public:
        back_insert_iterator() BMNOEXCEPT;
        back_insert_iterator(str_sparse_vector_type* sv) BMNOEXCEPT;
        back_insert_iterator(const back_insert_iterator& bi) BMNOEXCEPT;
        
        back_insert_iterator& operator=(const back_insert_iterator& bi)
        {
            BM_ASSERT(bi.empty());
            this->flush(); sv_ = bi.sv_;
            return *this;
        }

        ~back_insert_iterator();
        
        /** push value to the vector */
        back_insert_iterator& operator=(const value_type* v)
            { this->add(v); return *this; }


        /** push value to the vector */
        template<typename StrType>
        back_insert_iterator& operator=(const StrType& v)
        {
            this->add(v.c_str()); return *this; // TODO: avoid c_str()
        }

        /** noop */
        back_insert_iterator& operator*() { return *this; }
        /** noop */
        back_insert_iterator& operator++() { return *this; }
        /** noop */
        back_insert_iterator& operator++( int ) { return *this; }
        
        /** add value to the container*/
        void add(const value_type* v);
        
        /** add NULL (no-value) to the container */
        void add_null();
        
        /** add a series of consequitve NULLs (no-value) to the container */
        void add_null(size_type count);

        /** return true if insertion buffer is empty */
        bool empty() const BMNOEXCEPT;
        
        /** flush the accumulated buffer */
        void flush();
    protected:
        typedef typename bvector_type::block_idx_type     block_idx_type;

        /** add value to the buffer without changing the NULL vector
            @param v - value to push back
            @return index of added value in the internal buffer
            @internal
        */
        size_type add_value(const value_type* v);

    private:
        enum buf_size_e
        {
            n_buf_size = 1024 * 8
        };
        typedef bm::heap_matrix<CharType,
                        n_buf_size,   // ROWS: number of strings in one batch
                        MAX_STR_SIZE, // COLS
                        allocator_type> buffer_matrix_type;

    private:
        str_sparse_vector_type*  sv_;          ///!< pointer on the parent vector
        bvector_type*            bv_null_;     ///!< not NULL vector pointer
        buffer_matrix_type       buf_matrix_;  ///!< value buffer
        size_type                pos_in_buf_;  ///!< buffer position
        block_idx_type           prev_nb_;     ///!< previous block added
    };


public:

    /*!
        \brief Sparse vector constructor
     
        \param null_able - defines if vector supports NULL values flag
            by default it is OFF, use bm::use_null to enable it
        \param ap - allocation strategy for underlying bit-vectors
        Default allocation policy uses BM_BIT setting (fastest access)
        \param bv_max_size - maximum possible size of underlying bit-vectors
        Please note, this is NOT size of svector itself, it is dynamic upper limit
        which should be used very carefully if we surely know the ultimate size
        \param alloc - allocator for bit-vectors
     
        \sa bvector<>
        \sa bm::bvector<>::allocation_policy
        \sa bm::startegy
    */
    str_sparse_vector(bm::null_support null_able = bm::no_null,
                      allocation_policy_type ap = allocation_policy_type(),
                      size_type bv_max_size = bm::id_max,
                      const allocator_type&   alloc  = allocator_type());

    /*! copy-ctor */
    str_sparse_vector(const str_sparse_vector& str_sv);
    
    /*! copy assignmment operator */
    str_sparse_vector<CharType, BV, MAX_STR_SIZE>& operator = (
                const str_sparse_vector<CharType, BV, MAX_STR_SIZE>& str_sv)
    {
        if (this != &str_sv)
            parent_type::copy_from(str_sv);
        remap_flags_ = str_sv.remap_flags_;
        remap_matrix1_ = str_sv.remap_matrix1_;
        remap_matrix2_ = str_sv.remap_matrix2_;
        return *this;
    }
#ifndef BM_NO_CXX11
    /*! move-ctor */
    str_sparse_vector(str_sparse_vector<CharType, BV, MAX_STR_SIZE>&& str_sv) BMNOEXCEPT
    {
        parent_type::swap(str_sv);
        remap_flags_ = str_sv.remap_flags_;
        remap_matrix1_.swap(str_sv.remap_matrix1_);
        remap_matrix2_.swap(str_sv.remap_matrix2_);
    }

    /*! move assignmment operator */
    str_sparse_vector<CharType, BV, MAX_STR_SIZE>& operator =
            (str_sparse_vector<CharType, BV, MAX_STR_SIZE>&& str_sv) BMNOEXCEPT
    {
        if (this != &str_sv)
        {
            this->swap(str_sv);
        }
        return *this;
    }
#endif

public:

    // ------------------------------------------------------------
    /*! @name String element access */
    ///@{

    /** \brief Operator to get write access to an element  */
    reference operator[](size_type idx) { return reference(*this, idx); }

    /** \brief Operator to get read access to an element  */
    const_reference operator[](size_type idx) const
                                    { return const_reference(*this, idx); }

    /*!
        \brief set specified element with bounds checking and automatic resize
        \param idx  - element index (vector auto-resized if needs to)
        \param str  - string to set (zero terminated)
    */
    void set(size_type idx, const value_type* str);

    /*!
        \brief set NULL status for the specified element
        Vector is resized automatically
        \param idx  - element index (vector auto-resized if needs to)
    */
    void set_null(size_type idx);

    
    /*!
        \brief insert the specified element
        \param idx  - element index (vector auto-resized if needs to)
        \param str  - string to set (zero terminated)
    */
    void insert(size_type idx, const value_type* str);


    /*!
        \brief insert STL string
        \param idx  - element index (vector auto-resized if needs to)
        \param str  - STL string to set
    */
    template<typename StrType>
    void insert(size_type idx, const StrType& str)
    {
        this->insert(idx, str.c_str()); // TODO: avoid c_str()
    }

    /*!
        \brief erase the specified element
        \param idx  - element index
    */
    void erase(size_type idx);

    /*!
        \brief get specified element
     
        \param idx  - element index
        \param str  - string buffer
        \param buf_size - string buffer size
     
        @return string length
    */
    size_type get(size_type idx,
                 value_type* str, size_type buf_size) const BMNOEXCEPT;
    
    /*!
        \brief set specified element with bounds checking and automatic resize
     
        This is an equivalent of set() method, but templetized to be
        more compatible with the STL std::string and the likes
     
        \param idx  - element index (vector auto-resized if needs to)
        \param str  - input string
                      expected an STL class with size() support,
                      like basic_string<> or vector<char>
    */
    template<typename StrType>
    void assign(size_type idx, const StrType& str)
    {
        if (idx >= this->size())
            this->size_ = idx+1;

        size_type str_size = size_type(str.size());
        size_type sz = size_type((str_size < MAX_STR_SIZE) ? str_size : MAX_STR_SIZE-1);
        if (!sz)
        {
            this->clear_value_plains_from(0, idx);
            return;
        }
        unsigned i = 0;
        for (; i < sz; ++i)
        {
            CharType ch = str[i];
            if (remap_flags_) // compressional re-mapping is in effect
            {
                unsigned char remap_value = remap_matrix2_.get(i, unsigned(ch));
                BM_ASSERT(remap_value);
                ch = CharType(remap_value);
            }
            this->bmatr_.set_octet(idx, i, (unsigned char)ch);
            if (!ch)
                break;
        } // for i
        if (idx > sz)
            return;
        this->bmatr_.set_octet(idx, sz, 0);
        this->clear_value_plains_from(unsigned(sz*8+1), idx);
    }
    
    /*!
        \brief push back a string
        \param str  - string to set
                    (STL class with size() support, like basic_string)
    */
    template<typename StrType>
    void push_back(const StrType& str) { assign(this->size_, str); }
    
    /*!
        \brief push back a string (zero terminated)
        \param str  - string to set
    */
    void push_back(const value_type* str) { set(this->size_, str); }


    /*!
        \brief get specified string element
        Template method expects an STL-compatible type basic_string<>
        \param idx  - element index (vector auto-resized if needs to)
        \param str  - string to get [out]
    */
    template<typename StrType>
    void get(size_type idx, StrType& str) const
    {
        str.clear();
        for (unsigned i = 0; i < MAX_STR_SIZE; ++i)
        {
            CharType ch = CharType(this->bmatr_.get_octet(idx, i));
            if (ch == 0)
                break;
            if (remap_flags_)
            {
                const unsigned char* remap_row = remap_matrix1_.row(i);
                unsigned char remap_value = remap_row[unsigned(ch)];
                BM_ASSERT(remap_value);
                if (!remap_value) // unknown dictionary element
                {
                    throw_bad_value(0);
                    break;
                }
                ch = CharType(remap_value);
            }
            str.push_back(ch);
        } // for i
    }

    /*! Swap content */
    void swap(str_sparse_vector& str_sv) BMNOEXCEPT;

    ///@}
    
    // ------------------------------------------------------------
    /*! @name Element comparison functions       */
    ///@{

    /**
        \brief Compare vector element with argument lexicographically
     
        NOTE: for a re-mapped container, input string may have no correct
        remapping, in this case we have an ambiguity
        (we know it is not equal (0) but LT or GT?).
        Behavior is undefined.
     
        \param idx - vactor element index
        \param str - argument to compare with
     
        \return 0 - equal, < 0 - vect[i] < str, >0 otherwise
    */
    int compare(size_type idx, const value_type* str) const BMNOEXCEPT;
    
    
    /**
        \brief Find size of common prefix between two vector elements in octets
        \return size of common prefix
    */
    unsigned common_prefix_length(size_type idx1, size_type idx2) const BMNOEXCEPT;

    ///@}


    // ------------------------------------------------------------
    /*! @name Clear                                              */
    ///@{

    /*! \brief resize to zero, free memory */
    void clear() BMNOEXCEPT;

    /*!
        \brief clear range (assign bit 0 for all plains)
        \param left  - interval start
        \param right - interval end (closed interval)
        \param set_null - set cleared values to unassigned (NULL)
    */
    str_sparse_vector<CharType, BV, MAX_STR_SIZE>&
        clear_range(size_type left, size_type right, bool set_null = false)
    {
        parent_type::clear_range(left, right, set_null);
        return *this;
    }


    ///@}

    
    // ------------------------------------------------------------
    /*! @name Size, etc       */
    ///@{

    /*! \brief return size of the vector
        \return size of sparse vector
    */
    size_type size() const { return this->size_; }
    
    /*! \brief return true if vector is empty
        \return true if empty
    */
    bool empty() const { return (size() == 0); }
    
    /*! \brief resize vector
        \param sz - new size
    */
    void resize(size_type sz) { parent_type::resize(sz); }
    
    /*! \brief get maximum string length capacity
        \return maximum string length sparse vector can take
    */
    static size_type max_str() { return sv_octet_plains; }
    
    /*! \brief get effective string length used in vector
        Calculate and returns efficiency, how close are we
        to the reserved maximum.
        \return current string length maximum
    */
    size_type effective_max_str() const BMNOEXCEPT;
    
    /*! \brief get effective string length used in vector
        \return current string length maximum
    */
    size_type effective_vector_max() const { return effective_max_str(); }
    ///@}


    // ------------------------------------------------------------
    /*! @name Memory optimization/compression                    */
    ///@{

    /*!
        \brief run memory optimization for all vector plains
        \param temp_block - pre-allocated memory block to avoid unnecessary re-allocs
        \param opt_mode - requested compression depth
        \param stat - memory allocation statistics after optimization
    */
    void optimize(
       bm::word_t* temp_block = 0,
       typename bvector_type::optmode opt_mode = bvector_type::opt_compress,
       typename str_sparse_vector<CharType, BV, MAX_STR_SIZE>::statistics* stat = 0);

    /*!
        @brief Calculates memory statistics.

        Function fills statistics structure containing information about how
        this vector uses memory and estimation of max. amount of memory
        bvector needs to serialize itself.

        @param st - pointer on statistics structure to be filled in.

        @sa statistics
    */
    void calc_stat(
        struct str_sparse_vector<CharType, BV, MAX_STR_SIZE>::statistics* st
        ) const BMNOEXCEPT;
    
    
    ///@}

    // ------------------------------------------------------------
    /*! @name Iterator access */
    //@{

    /** Provide const iterator access to container content  */
    const_iterator begin() const BMNOEXCEPT;

    /** Provide const iterator access to the end    */
    const_iterator end() const BMNOEXCEPT { return const_iterator(this, bm::id_max); }

    /** Get const_itertor re-positioned to specific element
    @param idx - position in the sparse vector
    */
    const_iterator get_const_iterator(size_type idx) const BMNOEXCEPT
        { return const_iterator(this, idx); }
    
     /** Provide back insert iterator
    Back insert iterator implements buffered insertion, which is faster, than random access
    or push_back
    */
    back_insert_iterator get_back_inserter()
        { return back_insert_iterator(this); }

    ///@}



    // ------------------------------------------------------------
    /*! @name Various traits                                     */
    ///@{
    
    /** \brief trait if sparse vector is "compressed" (false)
    */
    static
    bool is_compressed() BMNOEXCEPT { return false; }

    ///@}

    // ------------------------------------------------------------
    /*! @name remapping, succinct utilities
        Remapping implements reduction of dit-depth thus improves
        search performance. Remapping limits farther modifications
        of sparse vector.
    */
    ///@{
    
    /**
        Get remapping status (true|false)
    */
    bool is_remap() const BMNOEXCEPT { return remap_flags_ != 0; }
    
    /**
        Build remapping profile and load content from another sparse vector
        \param str_sv - source sparse vector (assumed it is not remapped)
    */
    void remap_from(const str_sparse_vector& str_sv);

    /*!
        Calculate flags which octets are present on each byte-plain.
        @internal
    */
    void calc_octet_stat(plain_octet_matrix_type& octet_matrix) const BMNOEXCEPT;

    static
    void build_octet_remap(
                plain_octet_matrix_type& octet_remap_matrix1,
                plain_octet_matrix_type& octet_remap_matrix2,
                const plain_octet_matrix_type& octet_occupancy_matrix);
    /*!
        remap string from external (ASCII) system to matrix internal code
        @return true if remapping was ok, false if found incorrect value
                for the plain
        @internal
    */
    static
    bool remap_tosv(value_type*  BMRESTRICT      sv_str,
                    size_type                    buf_size,
                    const value_type* BMRESTRICT str,
                    const plain_octet_matrix_type& BMRESTRICT octet_remap_matrix2
                    ) BMNOEXCEPT;
    
    /*!
        remap string from external (ASCII) system to matrix internal code
        @internal
    */
    bool remap_tosv(value_type*       sv_str,
                    size_type         buf_size,
                    const value_type* str) const BMNOEXCEPT
    {
        return remap_tosv(sv_str, buf_size, str, remap_matrix2_);
    }
    /*!
        remap string from internal code to external (ASCII) system
        @return true if remapping was ok, false if found incorrect value
                for the plain
        @internal
    */
    static
    bool remap_fromsv(
            value_type*   BMRESTRICT     str,
            size_type                    buf_size,
            const value_type* BMRESTRICT sv_str,
            const plain_octet_matrix_type& BMRESTRICT octet_remap_matrix1
            ) BMNOEXCEPT;
    /*!
        re-calculate remap matrix2 based on matrix1
        @internal
    */
    void recalc_remap_matrix2();

    ///@}
    
    // ------------------------------------------------------------
    /*! @name Export content to C-style                          */
    ///@{
    
    /**
        \brief Bulk export strings to a C-style matrix of chars
     
        \param cmatr  - dest matrix (bm::heap_matrix)
        \param idx_from - index in the sparse vector to export from
        \param dec_size - decoding size (matrix column allocation should match)
        \param zero_mem - set to false if target array is pre-initialized
                          with 0s to avoid performance penalty
     
        \return number of actually exported elements (can be less than requested)
    */
    template<typename CharMatrix>
    size_type decode(CharMatrix& cmatr,
                     size_type   idx_from, size_type  dec_size,
                     bool        zero_mem = true) const
    {
        BM_ASSERT(cmatr.is_init());
        if (zero_mem)
            cmatr.set_zero();
        
        size_type rows = size_type(cmatr.rows());
        BM_ASSERT(cmatr.cols() >= MAX_STR_SIZE);
        size_type max_sz = this->size() - idx_from;
        if (max_sz < dec_size)
            dec_size = max_sz;
        if (rows < dec_size)
            dec_size = rows;
        if (!dec_size)
            return dec_size;
        
        for (unsigned i = 0; i < MAX_STR_SIZE; ++i)
        {
            unsigned bi = 0;
            for (unsigned k = i * 8; k < (i * 8) + 8; ++k, ++bi)
            {
                const bvector_type* bv = this->bmatr_.get_row(k);
                if (!bv)
                    continue;
                value_type mask = value_type(1u << bi);
                typename bvector_type::enumerator en(bv, idx_from);
                for ( ;en.valid(); ++en )
                {
                    size_type idx = *en - idx_from;
                    if (idx >= dec_size)
                        break;
                    typename CharMatrix::value_type* str = cmatr.row(idx);
                    str[i] |= mask;
                } // for en
            } // for k
        } // for i
        
        if (remap_flags_)
        {
            for (unsigned i = 0; i < dec_size; ++i)
            {
                typename CharMatrix::value_type* str = cmatr.row(i);
                remap_matrix1_.remapz(str);
            } // for i
        }
        return dec_size;
    }

    /**
        \brief Bulk import of strings from a C-style matrix of chars

        \param cmatr  - source matrix (bm::heap_matrix)
                        [in/out] parameter gets modified(corrupted)
                        in the process
        \param idx_from - destination index in the sparse vector
        \param imp_size - import size (number or rows to import)
    */
    template<typename CharMatrix>
    void import(CharMatrix& cmatr, size_type idx_from, size_type imp_size)
    {
        if (!imp_size)
            return;
        if (idx_from < this->size_) // in case it touches existing elements
        {
            // clear all plains in the range to provide corrrect import of 0 values
            this->clear_range(idx_from, idx_from + imp_size - 1);
        }
        import_no_check(cmatr, idx_from, imp_size);
    }

    /**
        \brief Bulk push-back import of strings from a C-style matrix of chars

        \param cmatr  - source matrix (bm::heap_matrix)
                        [in/out] parameter gets modified(corrupted)
                        in the process
        \param imp_size - import size (number or rows to import)
    */
    template<typename CharMatrix>
    void import_back(CharMatrix& cmatr, size_type imp_size)
    {
        if (!imp_size)
            return;
        import_no_check(cmatr, this->size(), imp_size);
    }


    ///@}

    // ------------------------------------------------------------
    /*! @name Merge, split, partition data                        */
    ///@{

    /**
        @brief copy range of values from another sparse vector

        Copy [left..right] values from the source vector,
        clear everything outside the range.

        \param sv - source vector
        \param left  - index from in losed diapason of [left..right]
        \param right - index to in losed diapason of [left..right]
        \param splice_null - "use_null" copy range for NULL vector or
                             do not copy it
    */
    void copy_range(const str_sparse_vector<CharType, BV, MAX_STR_SIZE>& sv,
                    size_type left, size_type right,
                    bm::null_support splice_null = bm::use_null);

    ///@}

    // ------------------------------------------------------------

    /*! \brief syncronize internal structures */
    void sync(bool force);

    /*!
        \brief check if another sparse vector has the same content and size
     
        \param sv        - sparse vector for comparison
        \param null_able - flag to consider NULL vector in comparison (default)
                           or compare only value content plains
     
        \return true, if it is the same
    */
    bool equal(const str_sparse_vector<CharType, BV, MAX_STR_SIZE>& sv,
               bm::null_support null_able = bm::use_null) const BMNOEXCEPT;

    /**
        \brief find position of compressed element by its rank
    */
    static
    bool find_rank(size_type rank, size_type& pos) BMNOEXCEPT;
    
    /**
        \brief size of sparse vector (may be different for RSC)
    */
    size_type effective_size() const BMNOEXCEPT { return size(); }

protected:

    /// @internal
    template<typename CharMatrix>
    void import_no_check(CharMatrix& cmatr,
                         size_type idx_from, size_type imp_size,
                         bool set_not_null = true)
    {
        BM_ASSERT (cmatr.is_init());
        
        unsigned max_str_size = 0;
        {
            for (unsigned j = 0; j < imp_size; ++j)
            {
                typename CharMatrix::value_type* str = cmatr.row(j);
                unsigned i;
                for (i = 0; i < MAX_STR_SIZE; ++i)
                {
                    value_type ch = str[i];
                    if (!ch)
                    {
                        max_str_size = (i > max_str_size) ? i : max_str_size;
                        break;
                    }
                    if (remap_flags_) // re-mapping is in effect
                    {
                        unsigned char remap_value = remap_matrix2_.get(i, unsigned(ch));
                        BM_ASSERT(remap_value);
                        if (!remap_value) // unknown dictionary element
                            throw_bad_value(0);
                        str[i] = CharType(remap_value);
                    }
                } // for i
                if (i == MAX_STR_SIZE)
                    max_str_size = i;
            } // for j
        }
        
        size_type bit_list[CharMatrix::n_rows];
        for (unsigned i = 0; i < max_str_size; ++i)
        {
            for (unsigned bi = 0; bi < 8; ++bi)
            {
                unsigned n_bits = 0;
                value_type mask = value_type(1u << bi);
                for (size_type j = 0; j < imp_size; ++j)
                {
                    typename CharMatrix::value_type* str = cmatr.row(j);
                    value_type ch = str[i];
                    if (!ch)
                        continue;
                    if (ch & mask)
                    {
                        bit_list[n_bits++] = idx_from + j;
                        str[i] ^= mask;
                    }
                } // for j
                if (n_bits) // set transposed bits to the target plain
                {
                    unsigned plain = i*8 + bi;
                    bvector_type* bv = this->bmatr_.get_row(plain);
                    if (!bv)
                    {
                        bv = this->bmatr_.construct_row(plain);
                        bv->init();
                    }
                    bv->set(&bit_list[0], n_bits, BM_SORTED);
                }
            } // for k
        } // for i
        
        size_type idx_to = idx_from + imp_size - 1;
        if (set_not_null)
        {
            bvector_type* bv_null = this->get_null_bvect();
            if (bv_null)
                bv_null->set_range(idx_from, idx_to);
        }
        if (idx_to >= this->size())
            this->size_ = idx_to+1;

    }

    // ------------------------------------------------------------
    /*! @name Errors and exceptions                              */
    ///@{

    /**
        \brief throw range error
        \internal
    */
    static
    void throw_range_error(const char* err_msg);

    /**
        \brief throw domain error
        \internal
    */
    static
    void throw_bad_value(const char* err_msg);

    ///@}

    /*! \brief set value without checking boundaries */
    void set_value(size_type idx, const value_type* str);

    /*! \brief set value without checking boundaries or support of NULL */
    void set_value_no_null(size_type idx, const value_type* str);

    /*! \brief insert value without checking boundaries */
    void insert_value(size_type idx, const value_type* str);

    /*! \brief insert value without checking boundaries or support of NULL */
    void insert_value_no_null(size_type idx, const value_type* str);


    size_type size_internal() const { return size(); }
    void resize_internal(size_type sz) { resize(sz); }

    size_t remap_size() const { return remap_matrix1_.get_buffer().size(); }
    const unsigned char* get_remap_buffer() const
                { return remap_matrix1_.get_buffer().buf(); }
    unsigned char* init_remap_buffer()
    {
        remap_matrix1_.init();
        return remap_matrix1_.get_buffer().data();
    }
    void set_remap() { remap_flags_ = 1; }

protected:

    bool resolve_range(size_type from, size_type to,
                       size_type* idx_from, size_type* idx_to) const
    {
        *idx_from = from; *idx_to = to; return true;
    }

protected:
    template<class SVect> friend class sparse_vector_serializer;
    template<class SVect> friend class sparse_vector_deserializer;
    
protected:
    unsigned                 remap_flags_;   ///< remapping status
    plain_octet_matrix_type  remap_matrix1_; ///< octet remap table 1
    plain_octet_matrix_type  remap_matrix2_; ///< octet remap table 2
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------


template<class CharType, class BV, unsigned MAX_STR_SIZE>
str_sparse_vector<CharType, BV, MAX_STR_SIZE>::str_sparse_vector(
        bm::null_support null_able,
        allocation_policy_type  ap,
        size_type               bv_max_size,
        const allocator_type&   alloc)
: parent_type(null_able, ap, bv_max_size, alloc),
  remap_flags_(0)
{}


//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
str_sparse_vector<CharType, BV, MAX_STR_SIZE>::str_sparse_vector(
                                        const str_sparse_vector& str_sv)
: parent_type(str_sv),
  remap_flags_(str_sv.remap_flags_),
  remap_matrix1_(str_sv.remap_matrix1_),
  remap_matrix2_(str_sv.remap_matrix2_)
{}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void str_sparse_vector<CharType, BV, MAX_STR_SIZE>::swap(
                                str_sparse_vector& str_sv) BMNOEXCEPT
{
    parent_type::swap(str_sv);
    bm::xor_swap(remap_flags_, str_sv.remap_flags_);
    remap_matrix1_.swap(str_sv.remap_matrix1_);
    remap_matrix2_.swap(str_sv.remap_matrix2_);
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void str_sparse_vector<CharType, BV, MAX_STR_SIZE>::set(
                                size_type idx, const value_type* str)
{
    if (idx >= this->size())
        this->size_ = idx+1;
    set_value(idx, str);
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void str_sparse_vector<CharType, BV, MAX_STR_SIZE>::insert(
                                size_type idx, const value_type* str)
{
    if (idx >= this->size())
    {
        this->size_ = idx+1;
        set_value(idx, str);
        return;
    }
    insert_value(idx, str);
    this->size_++;
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void str_sparse_vector<CharType, BV, MAX_STR_SIZE>::erase(size_type idx)
{
    BM_ASSERT(idx < this->size_);
    if (idx >= this->size_)
        return;
    this->erase_column(idx);
    this->size_--;
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void str_sparse_vector<CharType, BV, MAX_STR_SIZE>::set_null(size_type idx)
{
    bvector_type* bv_null = this->get_null_bvect();
    if (bv_null)
        bv_null->clear_bit_no_check(idx);
    if (idx >= this->size_)
    {
        this->size_ = idx + 1;
    }
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void str_sparse_vector<CharType, BV, MAX_STR_SIZE>::set_value(
                                size_type idx, const value_type* str)
{
    set_value_no_null(idx, str);
    bvector_type* bv_null = this->get_null_bvect();
    if (bv_null)
        bv_null->set_bit_no_check(idx);
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void str_sparse_vector<CharType, BV, MAX_STR_SIZE>::set_value_no_null(
                                size_type idx, const value_type* str)
{
    for (unsigned i = 0; i < MAX_STR_SIZE; ++i)
    {
        CharType ch = str[i];
        if (!ch)
        {
            this->clear_value_plains_from(i*8, idx);
            return;
        }
        
        if (remap_flags_) // compressional re-mapping is in effect
        {
            unsigned char remap_value = remap_matrix2_.get(i, unsigned(ch));
            BM_ASSERT(remap_value);
            if (!remap_value) // unknown dictionary element
            {
                this->clear_value_plains_from(i*8, idx);
                return;
            }
            ch = CharType(remap_value);
        }
        this->bmatr_.set_octet(idx, i, (unsigned char)ch);
    } // for i
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void str_sparse_vector<CharType, BV, MAX_STR_SIZE>::insert_value(
                                    size_type idx, const value_type* str)
{
    insert_value_no_null(idx, str);
    this->insert_null(idx, true);
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void str_sparse_vector<CharType, BV, MAX_STR_SIZE>::insert_value_no_null(
                                        size_type idx, const value_type* str)
{
    for (unsigned i = 0; i < MAX_STR_SIZE; ++i)
    {
        CharType ch = str[i];
        if (!ch)
        {
            this->insert_clear_value_plains_from(i*8, idx);
            return;
        }
        
        if (remap_flags_) // compressional re-mapping is in effect
        {
            unsigned char remap_value = remap_matrix2_.get(i, unsigned(ch));
            BM_ASSERT(remap_value);
            if (!remap_value) // unknown dictionary element
            {
                this->insert_clear_value_plains_from(i*8, idx);
                return;
            }
            ch = CharType(remap_value);
        }
        this->bmatr_.insert_octet(idx, i, (unsigned char)ch);
    } // for i
}


//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
typename str_sparse_vector<CharType, BV, MAX_STR_SIZE>::size_type
str_sparse_vector<CharType, BV, MAX_STR_SIZE>::get(
            size_type idx, value_type* str, size_type buf_size) const BMNOEXCEPT
{
    size_type i = 0;
    for (; i < MAX_STR_SIZE; ++i)
    {
        if (i < buf_size)
            str[i] = 0;
        else
            break;
        CharType ch = CharType(this->bmatr_.get_octet(idx, i));
        if (ch == 0)
        {
            str[i] = 0;
            break;
        }
        str[i] = ch;
    } // for i
    if (remap_flags_)
    {
        remap_matrix1_.remap(str, i);
    }
    return i;
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void str_sparse_vector<CharType, BV, MAX_STR_SIZE>::optimize(
      bm::word_t* temp_block,
      typename bvector_type::optmode opt_mode,
      typename str_sparse_vector<CharType, BV, MAX_STR_SIZE>::statistics* st)
{
    typename bvector_type::statistics stbv;
    parent_type::optimize(temp_block, opt_mode, &stbv);
    
    if (st)
        st->add(stbv);
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void str_sparse_vector<CharType, BV, MAX_STR_SIZE>::calc_stat(
    struct str_sparse_vector<CharType, BV, MAX_STR_SIZE>::statistics* st
    ) const BMNOEXCEPT
{
    BM_ASSERT(st);
    typename bvector_type::statistics stbv;
    parent_type::calc_stat(&stbv);
    
    st->reset();
    
    st->bit_blocks += stbv.bit_blocks;
    st->gap_blocks += stbv.gap_blocks;
    st->ptr_sub_blocks += stbv.ptr_sub_blocks;
    st->bv_count += stbv.bv_count;
    st->max_serialize_mem += stbv.max_serialize_mem + 8;
    st->memory_used += stbv.memory_used;
    st->gap_cap_overhead += stbv.gap_cap_overhead;
    
    size_t remap_mem_usage = sizeof(remap_flags_);
    remap_mem_usage += remap_matrix1_.get_buffer().mem_usage();
    remap_mem_usage += remap_matrix2_.get_buffer().mem_usage();

    st->memory_used += remap_mem_usage;
    if (remap_flags_) // use of remapping requires some extra storage
    {
        st->max_serialize_mem += remap_mem_usage;
    }
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
int str_sparse_vector<CharType, BV, MAX_STR_SIZE>::compare(
                     size_type idx,
                     const value_type* str) const BMNOEXCEPT
{
    BM_ASSERT(str);
    int res = 0;
    for (unsigned i = 0; i < MAX_STR_SIZE; ++i)
    {
        CharType ch = str[i];
        if (remap_flags_ && ch)
        {
            unsigned char remap_value = remap_matrix2_.get(i, unsigned(ch));
            if (!remap_value) // unknown dictionary element
            {
                throw_bad_value(0);
            }
            ch = CharType(remap_value);
        }

        res = this->bmatr_.compare_octet(idx, i, ch);
        if (res || !ch)
            break;
    } // for
    return res;
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
unsigned str_sparse_vector<CharType, BV, MAX_STR_SIZE>::common_prefix_length(
                                size_type idx1, size_type idx2) const BMNOEXCEPT
{
    unsigned i = 0;
    for (; i < MAX_STR_SIZE; ++i)
    {
        CharType ch1 = CharType(this->bmatr_.get_octet(idx1, i));
        CharType ch2 = CharType(this->bmatr_.get_octet(idx2, i));
        if (!ch1 || !ch2)
        {
            if (i) 
                --i;
            break;
        }
        if (ch1 != ch2)
        {
            break;
        }
    } // for

    return i;
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
bool 
str_sparse_vector<CharType, BV, MAX_STR_SIZE>::find_rank(
                                                size_type rank,
                                                size_type& pos) BMNOEXCEPT
{
    BM_ASSERT(rank);
    pos = rank - 1;
    return true;
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
typename str_sparse_vector<CharType, BV, MAX_STR_SIZE>::size_type
str_sparse_vector<CharType, BV, MAX_STR_SIZE>::effective_max_str()
                                                        const BMNOEXCEPT
{
    for (int i = MAX_STR_SIZE-1; i >= 0; --i)
    {
        unsigned octet_plain = unsigned(i) * unsigned(sizeof(CharType)) * 8;
        for (unsigned j = 0; j < sizeof(CharType) * 8; ++j)
        {
            if (this->bmatr_.row(octet_plain+j))
                return unsigned(i)+1;
        } // for j
    } // for i
    return 0;
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void str_sparse_vector<CharType, BV, MAX_STR_SIZE>::calc_octet_stat(
                    plain_octet_matrix_type& octet_matrix) const BMNOEXCEPT
{
    octet_matrix.init();
    octet_matrix.set_zero();
    
    size_type size = this->size();
    
    for (unsigned i = 0; i < MAX_STR_SIZE; ++i)
    {
        unsigned char* row = octet_matrix.row(i);
        
        // TODO: optimize partial transposition
        for (size_type j = 0; j < size; ++j)
        {
            unsigned char ch = this->bmatr_.get_octet(j, i);
            unsigned k = ch;
            if (k)
                row[k] = 1;
        } // for j
    } // for i
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void str_sparse_vector<CharType, BV, MAX_STR_SIZE>::build_octet_remap(
            plain_octet_matrix_type& octet_remap_matrix1,
            plain_octet_matrix_type& octet_remap_matrix2,
            const plain_octet_matrix_type& octet_occupancy_matrix)
{
    octet_remap_matrix1.init();
    octet_remap_matrix1.set_zero();
    octet_remap_matrix2.init();
    octet_remap_matrix2.set_zero();

    for (unsigned i = 0; i < octet_occupancy_matrix.rows(); ++i)
    {
        const unsigned char* row = octet_occupancy_matrix.row(i);
        unsigned char* remap_row1 = octet_remap_matrix1.row(i);
        unsigned char* remap_row2 = octet_remap_matrix2.row(i);
        unsigned count = 1;
        for (unsigned j = 1; j < octet_occupancy_matrix.cols(); ++j)
        {
            if (row[j]) // octet is present
            {
                // set two remapping table values
                remap_row1[count] = (unsigned char)j;
                remap_row2[j] = (unsigned char)count;
                ++count;
                BM_ASSERT(count < 256);
            }
        } // for j
    } // for i
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void str_sparse_vector<CharType, BV, MAX_STR_SIZE>::recalc_remap_matrix2()
{
    BM_ASSERT(remap_flags_);
    
    remap_matrix2_.init();
    remap_matrix2_.set_zero();
    
    for (unsigned i = 0; i < remap_matrix1_.rows(); ++i)
    {
        const unsigned char* remap_row1 = remap_matrix1_.row(i);
              unsigned char* remap_row2 = remap_matrix2_.row(i);
        for (unsigned j = 1; j < remap_matrix1_.cols(); ++j)
        {
            if (remap_row1[j])
            {
                unsigned count = remap_row1[j];
                remap_row2[count] = (unsigned char)j;
                BM_ASSERT(count < 256);
            }
        } // for j
    } // for i
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
bool str_sparse_vector<CharType, BV, MAX_STR_SIZE>::remap_tosv(
       value_type*   BMRESTRICT     sv_str,
       size_type                    buf_size,
       const value_type* BMRESTRICT str,
       const plain_octet_matrix_type& BMRESTRICT octet_remap_matrix2) BMNOEXCEPT
{
    for (unsigned i = 0; i < buf_size; ++i)
    {
        CharType ch = str[i];
        if (ch == 0)
        {
            sv_str[i] = ch;
            break;
        }
        const unsigned char* remap_row = octet_remap_matrix2.row(i);
        unsigned char remap_value = remap_row[unsigned(ch)];
        if (!remap_value) // unknown dictionary element
        {
            return false;
        }
        sv_str[i] = CharType(remap_value);
    } // for i
    return true;
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
bool str_sparse_vector<CharType, BV, MAX_STR_SIZE>::remap_fromsv(
         value_type* BMRESTRICT str,
         size_type         buf_size,
         const value_type* BMRESTRICT sv_str,
         const plain_octet_matrix_type& BMRESTRICT octet_remap_matrix1
         ) BMNOEXCEPT
{
    for (unsigned i = 0; i < buf_size; ++i)
    {
        CharType ch = sv_str[i];
        if (ch == 0)
        {
            str[i] = ch;
            break;
        }
        const unsigned char* remap_row = octet_remap_matrix1.row(i);
        unsigned char remap_value = remap_row[unsigned(ch)];
        if (!remap_value) // unknown dictionary element
        {
            return false;
        }
        str[i] = CharType(remap_value);
    } // for i
    return true;
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void
str_sparse_vector<CharType, BV, MAX_STR_SIZE>::remap_from(const str_sparse_vector& str_sv)
{
    if (str_sv.is_remap())
    {
        *this = str_sv;
        return;
    }
    this->clear();
    if (str_sv.empty()) // no content to remap
    {
        return;
    }
    
    plain_octet_matrix_type omatrix; // occupancy map
    str_sv.calc_octet_stat(omatrix);
    
    str_sv.build_octet_remap(remap_matrix1_, remap_matrix2_, omatrix);
    remap_flags_ = 1; // turn ON remapped mode
    
    const unsigned buffer_size = 1024 * 8;
    
    typedef bm::heap_matrix<CharType,
                    buffer_size,         // ROWS: number of strings in one batch
                    MAX_STR_SIZE,        // COLS
                    allocator_type> remap_buffer_type;
    
    remap_buffer_type cmatr(true);
    for (size_type i = 0; true; )
    {
        size_type dsize = str_sv.decode(cmatr, i, buffer_size, true);
        if (!dsize)
            break;
        this->import(cmatr, i, dsize);
        i += dsize;
    } // for i
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void str_sparse_vector<CharType, BV, MAX_STR_SIZE>::sync(bool /*force*/)
{
    if (remap_flags_)
    {
        recalc_remap_matrix2();
    }
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
bool str_sparse_vector<CharType, BV, MAX_STR_SIZE>::equal(
                const str_sparse_vector<CharType, BV, MAX_STR_SIZE>& sv,
                bm::null_support null_able) const BMNOEXCEPT
{
    // at this point both vectors should have the same remap settings
    // to be considered "equal".
    // Strictly speaking this is incorrect, because re-map and non-remap
    // vectors may have the same content

    if (remap_flags_ != sv.remap_flags_)
        return false;
    if (remap_flags_)
    {
        bool b;
        b = remap_matrix1_.get_buffer().equal(sv.remap_matrix1_.get_buffer());
        if (!b)
            return b;
        b = remap_matrix2_.get_buffer().equal(sv.remap_matrix2_.get_buffer());
        if (!b)
            return b;
    }
    return parent_type::equal(sv, null_able);
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void str_sparse_vector<CharType, BV, MAX_STR_SIZE>::copy_range(
                const str_sparse_vector<CharType, BV, MAX_STR_SIZE>& sv,
                size_type left, size_type right,
                bm::null_support splice_null)
{
    if (left > right)
        bm::xor_swap(left, right);
    this->clear();

    remap_flags_ = sv.remap_flags_;
    remap_matrix1_ = sv.remap_matrix1_;
    remap_matrix2_ = sv.remap_matrix2_;

    this->copy_range_plains(sv, left, right, splice_null);
    this->resize(sv.size());
}


//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
typename str_sparse_vector<CharType, BV, MAX_STR_SIZE>::const_iterator
str_sparse_vector<CharType, BV, MAX_STR_SIZE>::begin() const BMNOEXCEPT
{
    typedef typename
        str_sparse_vector<CharType, BV, MAX_STR_SIZE>::const_iterator it_type;
    return it_type(this);
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void str_sparse_vector<CharType, BV, MAX_STR_SIZE>::clear() BMNOEXCEPT
{
    parent_type::clear();
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void str_sparse_vector<CharType, BV, MAX_STR_SIZE>::throw_range_error(
                                                           const char* err_msg)
{
#ifndef BM_NO_STL
    throw std::range_error(err_msg);
#else
    BM_ASSERT_THROW(false, BM_ERR_RANGE);
#endif
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void str_sparse_vector<CharType, BV, MAX_STR_SIZE>::throw_bad_value(
                                                           const char* err_msg)
{
#ifndef BM_NO_STL
    if (!err_msg)
        err_msg = "Unknown/incomparable dictionary character";
    throw std::domain_error(err_msg);
#else
    BM_ASSERT_THROW(false, BM_BAD_VALUE);
#endif
}


//---------------------------------------------------------------------
//
//---------------------------------------------------------------------


template<class CharType, class BV, unsigned MAX_STR_SIZE>
str_sparse_vector<CharType, BV, MAX_STR_SIZE>::const_iterator::const_iterator() BMNOEXCEPT
: sv_(0), pos_(bm::id_max), pos_in_buf_(~size_type(0))
{}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
str_sparse_vector<CharType, BV, MAX_STR_SIZE>::const_iterator::const_iterator(
   const str_sparse_vector<CharType, BV, MAX_STR_SIZE>::const_iterator& it) BMNOEXCEPT
: sv_(it.sv_), pos_(it.pos_), pos_in_buf_(~size_type(0))
{}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
str_sparse_vector<CharType, BV, MAX_STR_SIZE>::const_iterator::const_iterator(
    const str_sparse_vector<CharType, BV, MAX_STR_SIZE>* sv) BMNOEXCEPT
: sv_(sv), pos_(sv->empty() ? bm::id_max : 0), pos_in_buf_(~size_type(0))
{}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
str_sparse_vector<CharType, BV, MAX_STR_SIZE>::const_iterator::const_iterator(
    const str_sparse_vector<CharType, BV, MAX_STR_SIZE>* sv,
    typename str_sparse_vector<CharType, BV, MAX_STR_SIZE>::size_type pos) BMNOEXCEPT
: sv_(sv), pos_(pos >= sv->size() ? bm::id_max : pos), pos_in_buf_(~size_type(0))
{}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
const typename str_sparse_vector<CharType, BV, MAX_STR_SIZE>::value_type*
str_sparse_vector<CharType, BV, MAX_STR_SIZE>::const_iterator::value() const BMNOEXCEPT
{
    BM_ASSERT(sv_);
    BM_ASSERT(this->valid());
    if (pos_in_buf_ == ~size_type(0))
    {
        if (!buf_matrix_.is_init())
            buf_matrix_.init();
        pos_in_buf_ = 0;
        size_type d = sv_->decode(buf_matrix_, pos_, buffer_matrix_type::n_rows);
        if (!d)
        {
            pos_ = bm::id_max;
            return 0;
        }
    }
    return buf_matrix_.row(pos_in_buf_);
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void
str_sparse_vector<CharType, BV, MAX_STR_SIZE>::const_iterator::go_to(
   typename str_sparse_vector<CharType, BV, MAX_STR_SIZE>::size_type pos
   ) BMNOEXCEPT
{
    pos_ = (!sv_ || pos >= sv_->size()) ? bm::id_max : pos;
    pos_in_buf_ = ~size_type(0);
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void
str_sparse_vector<CharType, BV, MAX_STR_SIZE>::const_iterator::advance() BMNOEXCEPT
{
    if (pos_ == bm::id_max) // nothing to do, we are at the end
        return;
    ++pos_;
    
    if (pos_ >= sv_->size())
        this->invalidate();
    else
    {
        if (pos_in_buf_ != ~size_type(0))
        {
            ++pos_in_buf_;
            if (pos_in_buf_ >= buffer_matrix_type::n_rows)
                pos_in_buf_ = ~size_type(0);
        }
    }
}

//---------------------------------------------------------------------
//
//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
str_sparse_vector<CharType, BV, MAX_STR_SIZE>::back_insert_iterator::back_insert_iterator() BMNOEXCEPT
: sv_(0), bv_null_(0), pos_in_buf_(~size_type(0)), prev_nb_(0)
{}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
str_sparse_vector<CharType, BV, MAX_STR_SIZE>::back_insert_iterator::back_insert_iterator(
           str_sparse_vector<CharType, BV, MAX_STR_SIZE>* sv) BMNOEXCEPT
: sv_(sv), pos_in_buf_(~size_type(0))
{
    if (sv)
    {
        prev_nb_ = sv_->size() >> bm::set_block_shift;
        bv_null_ = sv_->get_null_bvect();
    }
    else
    {
        bv_null_ = 0; prev_nb_ = 0;
    }
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
str_sparse_vector<CharType, BV, MAX_STR_SIZE>::back_insert_iterator::back_insert_iterator(
const str_sparse_vector<CharType, BV, MAX_STR_SIZE>::back_insert_iterator& bi) BMNOEXCEPT
: sv_(bi.sv_), bv_null_(bi.bv_null_), pos_in_buf_(~size_type(0)), prev_nb_(bi.prev_nb_)
{
    BM_ASSERT(bi.empty());
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
str_sparse_vector<CharType, BV, MAX_STR_SIZE>::back_insert_iterator::~back_insert_iterator()
{
    this->flush();
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
bool
str_sparse_vector<CharType, BV, MAX_STR_SIZE>::back_insert_iterator::empty()
                                                                const BMNOEXCEPT
{
    return (pos_in_buf_ == ~size_type(0) || !sv_);
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void str_sparse_vector<CharType, BV, MAX_STR_SIZE>::back_insert_iterator::flush()
{
    if (this->empty())
        return;

    sv_->import_no_check(buf_matrix_, sv_->size(), pos_in_buf_+1, false);
    pos_in_buf_ = ~size_type(0);
    block_idx_type nb = sv_->size() >> bm::set_block_shift;
    if (nb != prev_nb_)
    {
        // optimize all previous blocks in all planes
        sv_->optimize_block(prev_nb_);
        prev_nb_ = nb;
    }
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void str_sparse_vector<CharType, BV, MAX_STR_SIZE>::back_insert_iterator::add(
const typename str_sparse_vector<CharType, BV, MAX_STR_SIZE>::back_insert_iterator::value_type* v)
{
    if (!v)
    {
        this->add_null();
        return;
    }
    size_type buf_idx = this->add_value(v);
    if (bv_null_)
    {
        size_type sz = sv_->size();
        bv_null_->set_bit_no_check(sz + buf_idx);
    }
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void str_sparse_vector<CharType, BV, MAX_STR_SIZE>::back_insert_iterator::add_null()
{
    /*size_type buf_idx = */this->add_value("");
}

//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
void str_sparse_vector<CharType, BV, MAX_STR_SIZE>::back_insert_iterator::add_null(
typename str_sparse_vector<CharType, BV, MAX_STR_SIZE>::back_insert_iterator::size_type count)
{
    for (size_type i = 0; i < count; ++i) // TODO: optimization
        this->add_value("");
}


//---------------------------------------------------------------------

template<class CharType, class BV, unsigned MAX_STR_SIZE>
typename str_sparse_vector<CharType, BV, MAX_STR_SIZE>::back_insert_iterator::size_type
str_sparse_vector<CharType, BV, MAX_STR_SIZE>::back_insert_iterator::add_value(
const str_sparse_vector<CharType, BV, MAX_STR_SIZE>::back_insert_iterator::value_type* v)
{
    BM_ASSERT(sv_);
    BM_ASSERT(v);
    if (pos_in_buf_ == ~size_type(0))
    {
        if (!buf_matrix_.is_init())
            buf_matrix_.init();
        pos_in_buf_ = 0;
        buf_matrix_.set_zero();
    }
    else
    if (pos_in_buf_ >= buffer_matrix_type::n_rows-1)
    {
        this->flush();
        pos_in_buf_ = 0;
        buf_matrix_.set_zero();
    }
    else
    {
        ++pos_in_buf_;
    }
    value_type* r = buf_matrix_.row(pos_in_buf_);
    for (unsigned i = 0; i < buffer_matrix_type::n_columns; ++i)
    {
        r[i] = v[i];
        if (!r[i])
            break;
    } // for i
    return pos_in_buf_;
}

//---------------------------------------------------------------------


} // namespace

#endif
