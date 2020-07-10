#ifndef BMSPARSEVEC_H__INCLUDED__
#define BMSPARSEVEC_H__INCLUDED__
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

/*! \file bmsparsevec.h
    \brief Sparse constainer sparse_vector<> for integer types using
           bit-transposition transform
*/

#include <memory.h>

#ifndef BM_NO_STL
#include <stdexcept>
#endif

#ifndef BM__H__INCLUDED__
// BitMagic utility headers do not include main "bm.h" declaration 
// #include "bm.h" or "bm64.h" explicitly 
# error missing include (bm.h or bm64.h)
#endif


#include "bmtrans.h"
#include "bmalgo_impl.h"
#include "bmbuffer.h"
#include "bmbmatrix.h"
#include "bmdef.h"

namespace bm
{

/** \defgroup svector Sparse and compressed vectors
    Sparse vector for integer types using bit transposition transform
 
    @ingroup bmagic
 */


/** \defgroup sv sparse_vector<>
    Sparse vector for integer types using bit transposition transform
 
    @ingroup bmagic
 */


/*!
   \brief sparse vector with runtime compression using bit transposition method
 
   Sparse vector implements variable bit-depth storage model.
   Initial data is bit-transposed into bit-planes so each element
   may use less memory than the original native data type prescribes.
   For example, 32-bit integer may only use 20 bits.
 
   Another level of compression is provided by bit-vector (BV template parameter)
   used for storing bit planes. bvector<> implements varians of on the fly block
   compression, so if a significant area of a sparse vector uses less bits - it
   will save memory.
 
   Overall it provides variable bit-depth compression, sparse compression in
   bit-plains.
 
   @ingroup sv
*/
template<class Val, class BV>
class sparse_vector : public base_sparse_vector<Val, BV, 1>
{
public:
    typedef Val                                      value_type;
    typedef BV                                       bvector_type;
    typedef bvector_type*                            bvector_type_ptr;
    typedef typename bvector_type::size_type         size_type;
    typedef typename bvector_type::block_idx_type    block_idx_type;
    typedef const bvector_type*                      bvector_type_const_ptr;
	typedef const value_type&                        const_reference;
    typedef typename BV::allocator_type              allocator_type;
    typedef typename bvector_type::allocation_policy allocation_policy_type;
    typedef typename bvector_type::enumerator        bvector_enumerator_type;
    typedef typename allocator_type::allocator_pool_type allocator_pool_type;
    typedef bm::basic_bmatrix<BV>                    bmatrix_type;
    typedef base_sparse_vector<Val, BV, 1>           parent_type;
    

    /*! Statistical information about  memory allocation details. */
    struct statistics : public bv_statistics
    {};

    /*! Traits and features used in algorithms to correctly run 
        on a particular type of sparse vector
    */
    struct is_remap_support { enum trait { value = false }; };
    struct is_rsc_support { enum trait { value = false }; };
 
    /**
         Reference class to access elements via common [] operator
         @ingroup sv
    */
    class reference
    {
    public:
        reference(sparse_vector<Val, BV>& sv, size_type idx) BMNOEXCEPT
        : sv_(sv), idx_(idx)
        {}
        operator value_type() const BMNOEXCEPT { return sv_.get(idx_); }
        reference& operator=(const reference& ref)
        {
            sv_.set(idx_, (value_type)ref);
            return *this;
        }
        reference& operator=(value_type val)
        {
            sv_.set(idx_, val);
            return *this;
        }
        bool operator==(const reference& ref) const BMNOEXCEPT
                                { return bool(*this) == bool(ref); }
        bool is_null() const BMNOEXCEPT { return sv_.is_null(idx_); }
    private:
        sparse_vector<Val, BV>& sv_;
        size_type               idx_;
    };
    
    
    /**
        Const iterator to traverse the sparse vector.
     
        Implementation uses buffer for decoding so, competing changes
        to the original vector may not match the iterator returned values.
     
        This iterator keeps an operational buffer for 8K elements,
        so memory footprint is not negligable (about 64K for unsigned int)
     
        @ingroup sv
    */
    class const_iterator
    {
    public:
    friend class sparse_vector;

#ifndef BM_NO_STL
        typedef std::input_iterator_tag  iterator_category;
#endif
        typedef sparse_vector<Val, BV>                     sparse_vector_type;
        typedef sparse_vector_type*                        sparse_vector_type_ptr;
        typedef typename sparse_vector_type::value_type    value_type;
        typedef typename sparse_vector_type::size_type     size_type;
        typedef typename sparse_vector_type::bvector_type  bvector_type;
        typedef typename bvector_type::allocator_type      allocator_type;
        typedef typename bvector_type::allocator_type::allocator_pool_type allocator_pool_type;
        typedef bm::byte_buffer<allocator_type>            buffer_type;

        typedef unsigned                    difference_type;
        typedef unsigned*                   pointer;
        typedef value_type&                 reference;

    public:
        const_iterator() BMNOEXCEPT;
        const_iterator(const sparse_vector_type* sv) BMNOEXCEPT;
        const_iterator(const sparse_vector_type* sv, size_type pos) BMNOEXCEPT;
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
        value_type operator*() const  { return this->value(); }
        
        
        /// \brief Advance to the next available value
        const_iterator& operator++() BMNOEXCEPT { this->advance(); return *this; }

        /// \brief Advance to the next available value
        const_iterator& operator++(int)
            { const_iterator tmp(*this);this->advance(); return tmp; }


        /// \brief Get current position (value)
        value_type value() const;
        
        /// \brief Get NULL status
        bool is_null() const BMNOEXCEPT;
        
        /// Returns true if iterator is at a valid position
        bool valid() const BMNOEXCEPT { return pos_ != bm::id_max; }
        
        /// Invalidate current iterator
        void invalidate() BMNOEXCEPT { pos_ = bm::id_max; }
        
        /// Current position (index) in the vector
        size_type pos() const BMNOEXCEPT{ return pos_; }
        
        /// re-position to a specified position
        void go_to(size_type pos) BMNOEXCEPT;
        
        /// advance iterator forward by one
        /// @return true if it is still valid
        bool advance() BMNOEXCEPT;
        
        void skip_zero_values() BMNOEXCEPT;
    private:
        enum buf_size_e
        {
            n_buf_size = 1024 * 8
        };
        
    private:
        const sparse_vector_type*         sv_;      ///!< ptr to parent
        size_type                         pos_;     ///!< Position
        mutable buffer_type               buffer_;  ///!< value buffer
        mutable value_type*               buf_ptr_; ///!< position in the buffer
    };
    
    /**
        Back insert iterator implements buffered insert, faster than generic
        access assignment.
     
        Limitations for buffered inserter:
        1. Do not use more than one inserter per vector at a time
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
        typedef sparse_vector<Val, BV>                     sparse_vector_type;
        typedef sparse_vector_type*                        sparse_vector_type_ptr;
        typedef typename sparse_vector_type::value_type    value_type;
        typedef typename sparse_vector_type::size_type     size_type;
        typedef typename sparse_vector_type::bvector_type  bvector_type;
        typedef typename bvector_type::allocator_type      allocator_type;
        typedef typename bvector_type::allocator_type::allocator_pool_type allocator_pool_type;
        typedef bm::byte_buffer<allocator_type>            buffer_type;

        typedef void difference_type;
        typedef void pointer;
        typedef void reference;
        
    public:
        back_insert_iterator();
        back_insert_iterator(sparse_vector_type* sv);
        back_insert_iterator(const back_insert_iterator& bi);
        
        back_insert_iterator& operator=(const back_insert_iterator& bi)
        {
            BM_ASSERT(bi.empty());
            this->flush(); sv_ = bi.sv_; bv_null_ = bi. bv_null_;
            return *this;
        }

        ~back_insert_iterator();
        
        /** push value to the vector */
        back_insert_iterator& operator=(value_type v) { this->add(v); return *this; }
        /** noop */
        back_insert_iterator& operator*() { return *this; }
        /** noop */
        back_insert_iterator& operator++() { return *this; }
        /** noop */
        back_insert_iterator& operator++( int ) { return *this; }
        
        /** add value to the container*/
        void add(value_type v);
        
        /** add NULL (no-value) to the container */
        void add_null();
        
        /** add a series of consequitve NULLs (no-value) to the container */
        void add_null(size_type count);

        /** return true if insertion buffer is empty */
        bool empty() const;
        
        /** flush the accumulated buffer */
        void flush();
        
        // ---------------------------------------------------------------
        // open internals
        // (TODO: create proper friend declarations)
        //
        /**
            Get access to not-null vector
            @internal
        */
        bvector_type* get_null_bvect() const BMNOEXCEPT { return bv_null_; }
        
        /** add value to the buffer without changing the NULL vector
            @param v - value to push back
            @return index of added value in the internal buffer
            @internal
        */
        size_type add_value_no_null(value_type v);
        
        /**
            Reconfшпгку back inserter not to touch the NULL vector
        */
        void disable_set_null() BMNOEXCEPT { set_not_null_ = false; }
        // ---------------------------------------------------------------
        
    protected:
        typedef typename bvector_type::block_idx_type     block_idx_type;
    private:
        enum buf_size_e
        {
            n_buf_size = 1024 * 8
        };

    private:
        bm::sparse_vector<Val, BV>* sv_;      ///!< pointer on the parent vector
        bvector_type*               bv_null_; ///!< not NULL vector pointer
        buffer_type                 buffer_;  ///!< value buffer
        value_type*                 buf_ptr_; ///!< position in the buffer
        block_idx_type              prev_nb_; ///!< previous block added
        bool                        set_not_null_;
    };
    
    friend const_iterator;
    friend back_insert_iterator;

public:
    // ------------------------------------------------------------
    /*! @name Construction and assignment  */
    ///@{

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
    sparse_vector(bm::null_support null_able = bm::no_null,
                  allocation_policy_type ap = allocation_policy_type(),
                  size_type bv_max_size = bm::id_max,
                  const allocator_type&   alloc  = allocator_type());
    
    /*! copy-ctor */
    sparse_vector(const sparse_vector<Val, BV>& sv);

    /*! copy assignmment operator */
    sparse_vector<Val,BV>& operator = (const sparse_vector<Val, BV>& sv)
    {
        if (this != &sv)
            parent_type::copy_from(sv);
        return *this;
    }

#ifndef BM_NO_CXX11
    /*! move-ctor */
    sparse_vector(sparse_vector<Val, BV>&& sv) BMNOEXCEPT;


    /*! move assignmment operator */
    sparse_vector<Val,BV>& operator = (sparse_vector<Val, BV>&& sv) BMNOEXCEPT
    {
        if (this != &sv)
        {
            clear();
            swap(sv);
        }
        return *this;
    }
#endif

    ~sparse_vector() BMNOEXCEPT;
    ///@}

    
    // ------------------------------------------------------------
    /*! @name Element access */
    ///@{

    /** \brief Operator to get write access to an element  */
    reference operator[](size_type idx) BMNOEXCEPT
                            { return reference(*this, idx); }

    /*!
        \brief get specified element without bounds checking
        \param idx - element index
        \return value of the element
    */
    value_type operator[](size_type idx) const BMNOEXCEPT
                                    { return this->get(idx); }

    /*!
        \brief access specified element with bounds checking
        \param idx - element index
        \return value of the element
    */
    value_type at(size_type idx) const;
    /*!
        \brief get specified element without bounds checking
        \param idx - element index
        \return value of the element
    */
    value_type get(size_type idx) const BMNOEXCEPT;

    /*!
        \brief set specified element with bounds checking and automatic resize
        \param idx - element index
        \param v   - element value
    */
    void set(size_type idx, value_type v);

    /*!
        \brief increment specified element by one
        \param idx - element index
    */
    void inc(size_type idx);

    /*!
        \brief push value back into vector
        \param v   - element value
    */
    void push_back(value_type v);
    
    /*!
        \brief push back specified amount of NULL values
        \param count   - number of NULLs to push back
    */
    void push_back_null(size_type count);
    
    /*!
        \brief insert specified element into container
        \param idx - element index
        \param v   - element value
    */
    void insert(size_type idx, value_type v);

    /*!
        \brief erase specified element from container
        \param idx - element index
    */
    void erase(size_type idx);

    /*!
        \brief clear specified element with bounds checking and automatic resize
        \param idx - element index
        \param set_null - if true the value receives NULL (unassigned) value
    */
    void clear(size_type idx, bool set_null = false);

    ///@}

    // ------------------------------------------------------------
    /*! @name Iterator access */
    //@{

    /** Provide const iterator access to container content  */
    const_iterator begin() const BMNOEXCEPT;

    /** Provide const iterator access to the end    */
    const_iterator end() const BMNOEXCEPT
        { return const_iterator(this, bm::id_max); }

    /** Get const_itertor re-positioned to specific element
    @param idx - position in the sparse vector
    */
    const_iterator get_const_iterator(size_type idx) const BMNOEXCEPT
        { return const_iterator(this, idx); }
 
    /** Provide back insert iterator
        Back insert iterator implements buffered insertion,
        which is faster, than random access or push_back
    */
    back_insert_iterator get_back_inserter()
        { return back_insert_iterator(this); }
    ///@}


    // ------------------------------------------------------------
    /*! @name Various traits                                     */
    //@{
    
    /** \brief set specified element to unassigned value (NULL)
        \param idx - element index
    */
    void set_null(size_type idx);
    
    /** \brief trait if sparse vector is "compressed" (false)
    */
    static
    bool is_compressed() BMNOEXCEPT { return false; }
    
    ///@}


    // ------------------------------------------------------------
    /*! @name Loading of sparse vector from C-style array       */
    //@{
    
    /*!
        \brief Import list of elements from a C-style array
        \param arr  - source array
        \param arr_size - source size
        \param offset - target index in the sparse vector
        \param set_not_null - import should register in not null vector
    */
    void import(const value_type* arr,
                size_type arr_size,
                size_type offset = 0,
                bool      set_not_null = true);
    
    /*!
        \brief Import list of elements from a C-style array (pushed back)
        \param arr  - source array
        \param arr_size - source array size
        \param set_not_null - import should register in not null vector
    */
    void import_back(const value_type* arr,
                     size_type         arr_size,
                     bool              set_not_null = true);
    ///@}

    // ------------------------------------------------------------
    /*! @name Export content to C-style array                    */
    ///@{

    /*!
        \brief Bulk export list of elements to a C-style array
     
        For efficiency, this is left as a low level function,
        it does not do any bounds checking on the target array, it will
        override memory and crash if you are not careful with allocation
        and request size.
     
        \param arr  - dest array
        \param idx_from - index in the sparse vector to export from
        \param dec_size - decoding size (array allocation should match)
        \param zero_mem - set to false if target array is pre-initialized
                          with 0s to avoid performance penalty
     
        \return number of actually exported elements (can be less than requested)
     
        \sa gather
    */
    size_type decode(value_type* arr,
                     size_type   idx_from,
                     size_type   dec_size,
                     bool        zero_mem = true) const;
    
    
    /*!
        \brief Gather elements to a C-style array
     
        Gather collects values from different locations, for best
        performance feed it with sorted list of indexes.
     
        Faster than one-by-one random access.
     
        For efficiency, this is left as a low level function,
        it does not do any bounds checking on the target array, it will
        override memory and crash if you are not careful with allocation
        and request size.
     
        \param arr  - dest array
        \param idx - index list to gather elements
        \param size - decoding index list size (array allocation should match)
        \param sorted_idx - sort order directive for the idx array
                            (BM_UNSORTED, BM_SORTED, BM_UNKNOWN)
        Sort order affects both performance and correctness(!), use BM_UNKNOWN
        if not sure.
     
        \return number of actually exported elements (can be less than requested)
     
        \sa decode
    */
    size_type gather(value_type* arr,
                     const size_type* idx,
                     size_type   size,
                     bm::sort_order sorted_idx) const;
    ///@}

    /*! \brief content exchange
    */
    void swap(sparse_vector<Val, BV>& sv) BMNOEXCEPT;

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
    sparse_vector<Val, BV>& clear_range(size_type left,
                                        size_type right,
                                        bool set_null = false);

    ///@}

    // ------------------------------------------------------------
    /*! @name Size, etc       */
    ///@{

    /*! \brief return size of the vector
        \return size of sparse vector
    */
    size_type size() const BMNOEXCEPT { return this->size_; }
    
    /*! \brief return true if vector is empty
        \return true if empty
    */
    bool empty() const BMNOEXCEPT { return (size() == 0); }
    
    /*! \brief resize vector
        \param sz - new size
    */
    void resize(size_type sz) { parent_type::resize(sz); }
    ///@}
        
    // ------------------------------------------------------------
    /*! @name Comparison       */
    ///@{

    /*!
        \brief check if another sparse vector has the same content and size
     
        \param sv        - sparse vector for comparison
        \param null_able - flag to consider NULL vector in comparison (default)
                           or compare only value content plains
     
        \return true, if it is the same
    */
    bool equal(const sparse_vector<Val, BV>& sv,
               bm::null_support null_able = bm::use_null) const BMNOEXCEPT;

    ///@}

    // ------------------------------------------------------------
    /*! @name Element comparison        */
    ///@{

    /**
        \brief Compare vector element with argument
     
        \param idx - vactor element index
        \param val - argument to compare with
     
        \return 0 - equal, < 0 - vect[i] < str, >0 otherwise
    */
    int compare(size_type idx, const value_type val) const BMNOEXCEPT;
    
    ///@}

    // ------------------------------------------------------------
    /*! @name Memory optimization                                */
    ///@{

    /*!
        \brief run memory optimization for all vector plains
        \param temp_block - pre-allocated memory block to avoid unnecessary re-allocs
        \param opt_mode - requested compression depth
        \param stat - memory allocation statistics after optimization
    */
    void optimize(bm::word_t* temp_block = 0,
          typename bvector_type::optmode opt_mode = bvector_type::opt_compress,
          typename sparse_vector<Val, BV>::statistics* stat = 0);

    /*!
       \brief Optimize sizes of GAP blocks

       This method runs an analysis to find optimal GAP levels for all bit plains
       of the vector.
    */
    void optimize_gap_size();

    /*!
        @brief Calculates memory statistics.

        Function fills statistics structure containing information about how
        this vector uses memory and estimation of max. amount of memory
        bvector needs to serialize itself.

        @param st - pointer on statistics structure to be filled in.

        @sa statistics
    */
    void calc_stat(
        struct sparse_vector<Val, BV>::statistics* st) const BMNOEXCEPT;
    ///@}

    // ------------------------------------------------------------
    /*! @name Merge, split, partition data                        */
    ///@{

    /*!
        \brief join all with another sparse vector using OR operation
        \param sv - argument vector to join with
        \return slf reference
        @sa merge
    */
    sparse_vector<Val, BV>& join(const sparse_vector<Val, BV>& sv);

    /*!
        \brief merge with another sparse vector using OR operation
        Merge is different from join(), because it borrows data from the source
        vector, so it gets modified.
     
        \param sv - [in, out]argument vector to join with (vector mutates)
     
        \return slf reference
        @sa join
    */
    sparse_vector<Val, BV>& merge(sparse_vector<Val, BV>& sv);


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
    void copy_range(const sparse_vector<Val, BV>& sv,
                    size_type left, size_type right,
                    bm::null_support splice_null = bm::use_null);
    
    /**
        @brief Apply value filter, defined by mask vector
     
        All bit-plains are ANDed against the filter mask.
    */
    void filter(const bvector_type& bv_mask);

    ///@}
    

    // ------------------------------------------------------------
    /*! @name Access to internals                                */
    ///@{

    
    /*! \brief syncronize internal structures */
    void sync(bool /*force*/) {}
    
    
    /*!
        \brief Bulk export list of elements to a C-style array
     
        Use of all extract() methods is restricted.
        Please consider decode() for the same purpose.
     
        \param arr  - dest array
        \param size - dest size
        \param offset - target index in the sparse vector to export from
        \param zero_mem - set to false if target array is pre-initialized
                          with 0s to avoid performance penalty   
        \return number of exported elements
     
        \sa decode
     
        @internal
    */
    size_type extract(value_type* arr,
                      size_type size,
                      size_type offset = 0,
                      bool      zero_mem = true) const BMNOEXCEPT2;

    /** \brief extract small window without use of masking vector
        \sa decode
        @internal
    */
    size_type extract_range(value_type* arr,
                            size_type size,
                            size_type offset,
                            bool      zero_mem = true) const;
    
    /** \brief extract medium window without use of masking vector
        \sa decode
        @internal
    */
    size_type extract_plains(value_type* arr,
                             size_type size,
                             size_type offset,
                             bool      zero_mem = true) const;
    
    /** \brief address translation for this type of container
        \internal
    */
    static
    size_type translate_address(size_type i) BMNOEXCEPT { return i; }
    
    /**
        \brief throw range error
        \internal
    */
    static
    void throw_range_error(const char* err_msg);

    /**
        \brief throw bad alloc
        \internal
    */
    static
    void throw_bad_alloc();


    /**
    \brief find position of compressed element by its rank
    */
    static
    bool find_rank(size_type rank, size_type& pos) BMNOEXCEPT;

    /**
        \brief size of sparse vector (may be different for RSC)
    */
    size_type effective_size() const BMNOEXCEPT { return size(); }

    /**
        \brief Always 1 (non-matrix type)
    */
    size_type effective_vector_max() const BMNOEXCEPT { return 1; }

    ///@}

    /// Set allocator pool for local (non-threaded)
    /// memory cyclic(lots of alloc-free ops) opertations
    ///
    void set_allocator_pool(allocator_pool_type* pool_ptr) BMNOEXCEPT;
    
protected:
    enum octet_plains
    {
        sv_octet_plains = sizeof(value_type)
    };


    /*! \brief set value without checking boundaries */
    void set_value(size_type idx, value_type v);
    
    /*! \brief set value without checking boundaries or support of NULL */
    void set_value_no_null(size_type idx, value_type v);

    /*! \brief push value back into vector without NULL semantics */
    void push_back_no_null(value_type v);
    
    /*! \brief insert value without checking boundaries */
    void insert_value(size_type idx, value_type v);
    /*! \brief insert value without checking boundaries or support of NULL */
    void insert_value_no_null(size_type idx, value_type v);

    void resize_internal(size_type sz) { resize(sz); }
    size_type size_internal() const BMNOEXCEPT { return size(); }

    bool is_remap() const BMNOEXCEPT { return false; }
    size_t remap_size() const BMNOEXCEPT { return 0; }
    const unsigned char* get_remap_buffer() const BMNOEXCEPT { return 0; }
    unsigned char* init_remap_buffer() BMNOEXCEPT { return 0; }
    void set_remap() BMNOEXCEPT { }

    bool resolve_range(size_type from, size_type to,
                       size_type* idx_from, size_type* idx_to) const BMNOEXCEPT
    {
        *idx_from = from; *idx_to = to; return true;
    }

    /// Increment element by 1 without chnaging NULL vector or size
    void inc_no_null(size_type idx);

    /// increment by v  without chnaging NULL vector or size
    void inc_no_null(size_type idx, value_type v);

protected:
    template<class V, class SV> friend class rsc_sparse_vector;
    template<class SVect> friend class sparse_vector_scanner;
    template<class SVect> friend class sparse_vector_serializer;
    template<class SVect> friend class sparse_vector_deserializer;

};


//---------------------------------------------------------------------
//---------------------------------------------------------------------


template<class Val, class BV>
sparse_vector<Val, BV>::sparse_vector(
        bm::null_support null_able,
        allocation_policy_type  ap,
        size_type               bv_max_size,
        const allocator_type&   alloc)
: parent_type(null_able, ap, bv_max_size, alloc)
{}

//---------------------------------------------------------------------

template<class Val, class BV>
sparse_vector<Val, BV>::sparse_vector(const sparse_vector<Val, BV>& sv)
: parent_type(sv)
{}

//---------------------------------------------------------------------
#ifndef BM_NO_CXX11

template<class Val, class BV>
sparse_vector<Val, BV>::sparse_vector(sparse_vector<Val, BV>&& sv) BMNOEXCEPT
{
    parent_type::swap(sv);
}

#endif


//---------------------------------------------------------------------

template<class Val, class BV>
sparse_vector<Val, BV>::~sparse_vector() BMNOEXCEPT
{}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::swap(sparse_vector<Val, BV>& sv) BMNOEXCEPT
{
    parent_type::swap(sv);
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::throw_range_error(const char* err_msg)
{
#ifndef BM_NO_STL
    throw std::range_error(err_msg);
#else
    BM_ASSERT_THROW(false, BM_ERR_RANGE);
#endif
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::throw_bad_alloc()
{
    BV::throw_bad_alloc();
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::set_null(size_type idx)
{
    clear(idx, true);
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::import(const value_type* arr,
                                    size_type         arr_size,
                                    size_type         offset,
                                    bool              set_not_null)
{
    unsigned char b_list[sizeof(Val)*8];
    unsigned row_len[sizeof(Val)*8] = {0, };
    
    const unsigned transpose_window = 256;
    bm::tmatrix<size_type, sizeof(Val)*8, transpose_window> tm; // matrix accumulator
    
    if (arr_size == 0)
        throw_range_error("sparse_vector range error (import size 0)");
    
    if (offset < this->size_) // in case it touches existing elements
    {
        // clear all plains in the range to provide corrrect import of 0 values
        this->clear_range(offset, offset + arr_size - 1);
    }
    
    // transposition algorithm uses bitscan to find index bits and store it
    // in temporary matrix (list for each bit plain), matrix here works
    // when array gets to big - the list gets loaded into bit-vector using
    // bulk load algorithm, which is faster than single bit access
    //
    
    size_type i;
    for (i = 0; i < arr_size; ++i)
    {
        unsigned bcnt = bm::bitscan(arr[i], b_list);
        const size_type bit_idx = i + offset;
        
        for (unsigned j = 0; j < bcnt; ++j)
        {
            unsigned p = b_list[j];
            unsigned rl = row_len[p];
            tm.row(p)[rl] = bit_idx;
            row_len[p] = ++rl;
            
            if (rl == transpose_window)
            {
                bvector_type* bv = this->get_plain(p);
                const size_type* r = tm.row(p);
                bv->set(r, rl, BM_SORTED);
                row_len[p] = 0;
                tm.row(p)[0] = 0;
            }
        } // for j
    } // for i
    
    // process incomplete transposition lines
    //
    for (unsigned k = 0; k < tm.rows(); ++k)
    {
        unsigned rl = row_len[k];
        if (rl)
        {
            bvector_type* bv = this->get_plain(k);
            const size_type* r = tm.row(k);
            bv->set(r, rl, BM_SORTED);
        }
    } // for k
    
    
    if (i + offset > this->size_)
        this->size_ = i + offset;
    
    if (set_not_null)
    {
        bvector_type* bv_null = this->get_null_bvect();
        if (bv_null) // configured to support NULL assignments
            bv_null->set_range(offset, offset + arr_size - 1);
    }
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::import_back(const value_type* arr,
                                         size_type         arr_size,
                                         bool              set_not_null)
{
    this->import(arr, arr_size, this->size(), set_not_null);
}

//---------------------------------------------------------------------

template<class Val, class BV>
typename sparse_vector<Val, BV>::size_type
sparse_vector<Val, BV>::decode(value_type* arr,
                               size_type   idx_from,
                               size_type   dec_size,
                               bool        zero_mem) const
{
    return extract(arr, dec_size, idx_from, zero_mem);
}

//---------------------------------------------------------------------

template<class Val, class BV>
typename sparse_vector<Val, BV>::size_type
sparse_vector<Val, BV>::gather(value_type*       arr,
                               const size_type*  idx,
                               size_type         size,
                               bm::sort_order    sorted_idx) const
{
    BM_ASSERT(arr);
    BM_ASSERT(idx);
    BM_ASSERT(size);

    if (size == 1) // corner case: get 1 value
    {
        arr[0] = this->get(idx[0]);
        return size;
    }
    ::memset(arr, 0, sizeof(value_type)*size);
    
    for (size_type i = 0; i < size;)
    {
        bool sorted_block = true;
        
        // look ahead for the depth of the same block
        //          (speculate more than one index lookup per block)
        //
        block_idx_type nb = (idx[i] >> bm::set_block_shift);
        size_type r = i;
        
        switch (sorted_idx)
        {
        case BM_UNKNOWN:
            {
                size_type idx_prev = idx[r];
                for (; (r < size) && (nb == (idx[r] >> bm::set_block_shift)); ++r)
                {
                    sorted_block = !(idx[r] < idx_prev); // sorted check
                    idx_prev = idx[r];
                }
            }
            break;
        case BM_UNSORTED:
            sorted_block = false;
            for (; r < size; ++r)
            {
                block_idx_type nb_next = (idx[r] >> bm::set_block_shift);
                if (nb != nb_next)
                    break;
            } // for r
            break;            
            // no break(!) intentional fall through
        case BM_SORTED:
            #ifdef BM64ADDR
                r = bm::idx_arr_block_lookup_u64(idx, size, nb, r);
            #else
                r = bm::idx_arr_block_lookup_u32(idx, size, nb, r);
            #endif
            break;
        case BM_SORTED_UNIFORM:
            r = size;
            break;
        default:
            BM_ASSERT(0);
        } // switch
        
        // single element hit, use plain random access
        if (r == i+1)
        {
            arr[i] = this->get(idx[i]);
            ++i;
            continue;
        }

        // process block co-located elements at ones for best (CPU cache opt)
        //
        unsigned i0 = unsigned(nb >> bm::set_array_shift); // top block address
        unsigned j0 = unsigned(nb &  bm::set_array_mask);  // address in sub-block
        
        unsigned eff_plains = this->effective_plains(); // TODO: get real effective plains for [i,j]
        for (unsigned j = 0; j < eff_plains; ++j)
        {
            const bm::word_t* blk = this->bmatr_.get_block(j, i0, j0);
            if (!blk)
                continue;
            value_type vm;
            const value_type mask1 = 1;
            
            if (blk == FULL_BLOCK_FAKE_ADDR)
            {
                vm = (mask1 << j);
                for (size_type k = i; k < r; ++k)
                    arr[k] |= vm;
                continue;
            }
            if (BM_IS_GAP(blk))
            {
                const bm::gap_word_t* gap_blk = BMGAP_PTR(blk);
                unsigned is_set;
                
                if (sorted_block) // b-search hybrid with scan lookup
                {
                    for (size_type k = i; k < r; )
                    {
                        unsigned nbit = unsigned(idx[k] & bm::set_block_mask);
                        
                        unsigned gidx = bm::gap_bfind(gap_blk, nbit, &is_set);
                        unsigned gap_value = gap_blk[gidx];
                        if (is_set)
                        {
                            arr[k] |= vm = (mask1 << j);
                            for (++k; k < r; ++k) // speculative look-up
                            {
                                if (unsigned(idx[k] & bm::set_block_mask) <= gap_value)
                                    arr[k] |= vm;
                                else
                                    break;
                            }
                        }
                        else // 0 GAP - skip. not set
                        {
                            for (++k;
                                 (k < r) &&
                                 (unsigned(idx[k] & bm::set_block_mask) <= gap_value);
                                 ++k) {}
                        }
                    } // for k
                }
                else // unsorted block gather request: b-search lookup
                {
                    for (size_type k = i; k < r; ++k)
                    {
                        unsigned nbit = unsigned(idx[k] & bm::set_block_mask);
                        is_set = bm::gap_test_unr(gap_blk, nbit);
                        arr[k] |= (value_type(bool(is_set)) << j);
                    } // for k
                }
                continue;
            }
            bm::bit_block_gather_scatter(arr, blk, idx, r, i, j);
        } // for (each plain)
        
        i = r;

    } // for i

    return size;
}

//---------------------------------------------------------------------

template<class Val, class BV>
typename sparse_vector<Val, BV>::size_type
sparse_vector<Val, BV>::extract_range(value_type* arr,
                                      size_type size,
                                      size_type offset,
                                      bool      zero_mem) const
{
    if (size == 0)
        return 0;
    if (zero_mem)
        ::memset(arr, 0, sizeof(value_type)*size);

    size_type start = offset;
    size_type end = start + size;
    if (end > this->size_)
    {
        end = this->size_;
    }
    
    // calculate logical block coordinates and masks
    //
    block_idx_type nb = (start >>  bm::set_block_shift);
    unsigned i0 = unsigned(nb >> bm::set_array_shift); // top block address
    unsigned j0 = unsigned(nb &  bm::set_array_mask);  // address in sub-block
    unsigned nbit = unsigned(start & bm::set_block_mask);
    unsigned nword  = unsigned(nbit >> bm::set_word_shift);
    unsigned mask0 = 1u << (nbit & bm::set_word_mask);
    const bm::word_t* blk = 0;
    unsigned is_set;
    
    for (unsigned j = 0; j < sizeof(Val)*8; ++j)
    {
        blk = this->bmatr_.get_block(j, i0, j0);
        bool is_gap = BM_IS_GAP(blk);
        
        for (size_type k = start; k < end; ++k)
        {
            block_idx_type nb1 = (k >>  bm::set_block_shift);
            if (nb1 != nb) // block switch boundaries
            {
                nb = nb1;
                i0 = unsigned(nb >> bm::set_array_shift);
                j0 = unsigned(nb &  bm::set_array_mask);
                blk = this->bmatr_.get_block(j, i0, j0);
                is_gap = BM_IS_GAP(blk);
            }
            if (!blk)
                continue;
            nbit = unsigned(k & bm::set_block_mask);
            if (is_gap)
            {
                is_set = bm::gap_test_unr(BMGAP_PTR(blk), nbit);
            }
            else
            {
                if (blk == FULL_BLOCK_FAKE_ADDR)
                {
                    is_set = 1;
                }
                else
                {
                    BM_ASSERT(!IS_FULL_BLOCK(blk));
                    nword  = unsigned(nbit >> bm::set_word_shift);
                    mask0 = 1u << (nbit & bm::set_word_mask);
                    is_set = (blk[nword] & mask0);
                }
            }
            size_type idx = k - offset;
            value_type vm = (bool) is_set;
            vm <<= j;
            arr[idx] |= vm;
            
        } // for k

    } // for j
    return 0;
}

//---------------------------------------------------------------------

template<class Val, class BV>
typename sparse_vector<Val, BV>::size_type
sparse_vector<Val, BV>::extract_plains(value_type* arr,
                                       size_type   size,
                                       size_type   offset,
                                       bool        zero_mem) const
{
    if (size == 0)
        return 0;

    if (zero_mem)
        ::memset(arr, 0, sizeof(value_type)*size);
    
    size_type start = offset;
    size_type end = start + size;
    if (end > this->size_)
    {
        end = this->size_;
    }
    
    for (size_type i = 0; i < parent_type::value_bits(); ++i)
    {
        const bvector_type* bv = this->bmatr_.get_row(i);
        if (!bv)
            continue;
       
        value_type mask = 1;
        mask <<= i;
        typename BV::enumerator en(bv, offset);
        for (;en.valid(); ++en)
        {
            size_type idx = *en - offset;
            if (idx >= size)
                break;
            arr[idx] |= mask;
        } // for
        
    } // for i

    return 0;
}


//---------------------------------------------------------------------

template<class Val, class BV>
typename sparse_vector<Val, BV>::size_type
sparse_vector<Val, BV>::extract(value_type* BMRESTRICT arr,
                                size_type   size,
                                size_type   offset,
                                bool        zero_mem) const BMNOEXCEPT2
{
    /// Decoder functor
    /// @internal
    ///
    struct sv_decode_visitor_func
    {
        sv_decode_visitor_func(value_type* BMRESTRICT varr,
                               value_type  mask,
                               size_type   off) BMNOEXCEPT2
        : arr_(varr), mask_(mask), sv_off_(off)
        {}

        void add_bits(size_type bv_offset,
                      const unsigned char* bits, unsigned bits_size) BMNOEXCEPT
        {
            // can be negative (-1) when bv base offset = 0 and sv = 1,2..
            size_type base = bv_offset - sv_off_; 
            value_type m = mask_;
            for (unsigned i = 0; i < bits_size; ++i)
                arr_[bits[i] + base] |= m;
        }
        void add_range(size_type bv_offset, size_type sz) BMNOEXCEPT
        {
            auto base = bv_offset - sv_off_;
            value_type m = mask_;
            for (size_type i = 0; i < sz; ++i)
                arr_[i + base] |= m;
        }

        value_type* BMRESTRICT arr_;       ///< target array for reverse transpose
        value_type             mask_;      ///< bit-plane mask 
        size_type              sv_off_;    ///< SV read offset
    };

    if (!size)
        return 0;

    if (zero_mem)
        ::memset(arr, 0, sizeof(value_type)*size);
    
    size_type end = offset + size;
    if (end > this->size_)
        end = this->size_;

    sv_decode_visitor_func func(arr, 0, offset);

    for (size_type i = 0; i < parent_type::value_bits(); ++i)
    {
        const bvector_type* bv = this->bmatr_.get_row(i);
        if (!bv)
            continue;
        func.mask_ = (value_type(1) << i); // set target plane OR mask
        bm::for_each_bit_range_no_check(*bv, offset, end-1, func);
    } // for i
    return end - offset;
}

//---------------------------------------------------------------------

template<class Val, class BV>
typename sparse_vector<Val, BV>::value_type
sparse_vector<Val, BV>::at(typename sparse_vector<Val, BV>::size_type idx) const
{
    if (idx >= this->size_)
        throw_range_error("sparse vector range error");
    return this->get(idx);
}

//---------------------------------------------------------------------

template<class Val, class BV>
typename sparse_vector<Val, BV>::value_type
sparse_vector<Val, BV>::get(
        typename sparse_vector<Val, BV>::size_type i) const BMNOEXCEPT
{
    BM_ASSERT(i < bm::id_max);
    BM_ASSERT(i < size());
    
    value_type v = 0;
    unsigned eff_plains = this->effective_plains();
    for (unsigned j = 0; j < eff_plains; j+=4)
    {
        bool b = this->bmatr_.test_4rows(j);
        if (b)
        {
            value_type vm = (value_type)this->bmatr_.get_half_octet(i, j);
            v |= vm << j;
        }
    } // for j
    return v;
}


//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::set(size_type idx, value_type v)
{ 
    if (idx >= size())
    {
        this->size_ = idx+1;
    }
    set_value(idx, v);
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::clear(size_type idx, bool set_null)
{
    if (idx >= size())
        this->size_ = idx+1;

    set_value(idx, (value_type)0);
    if (set_null)
    {
        bvector_type* bv_null = this->get_null_bvect();
        if (bv_null)
            bv_null->set(idx, false);
    }
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::push_back(value_type v)
{
    set_value(this->size_, v);
    ++(this->size_);
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::push_back_null(size_type count)
{
    BM_ASSERT(count);
    BM_ASSERT(bm::id_max - count > this->size_);
    BM_ASSERT(this->is_nullable());
    
    this->size_ += count;
}


//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::insert(size_type idx, value_type v)
{
    if (idx >= size())
    {
        this->size_ = idx+1;
        set_value(idx, v);
        return;
    }
    insert_value(idx, v);
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::insert_value(size_type idx, value_type v)
{
    insert_value_no_null(idx, v);
    this->insert_null(idx, true);
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::insert_value_no_null(size_type idx, value_type v)
{
    unsigned bsr = v ? bm::bit_scan_reverse(v) : 0u;
    value_type mask = 1u;
    unsigned i = 0;
    for (; i <= bsr; ++i)
    {
        if (v & mask)
        {
            bvector_type* bv = this->get_plain(i);
            bv->insert(idx, true);
        }
        else
        {
            bvector_type_ptr bv = this->bmatr_.get_row(i);
            if (bv)
                bv->insert(idx, false);
        }
        mask <<= 1;
    } // for i
    // insert 0 into all other existing plains
    unsigned eff_plains = this->effective_plains();
    for (; i < eff_plains; ++i)
    {
        bvector_type* bv = this->bmatr_.get_row(i);
        if (bv)
            bv->insert(idx, false);
    } // for i
    
    this->size_++;
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::erase(size_type idx)
{
    BM_ASSERT(idx < this->size_);
    if (idx >= this->size_)
        return;
    this->erase_column(idx);
    this->size_--;
}


//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::push_back_no_null(value_type v)
{
    set_value_no_null(this->size_, v);
    ++(this->size_);
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::set_value(size_type idx, value_type v)
{
    set_value_no_null(idx, v);
    bvector_type* bv_null = this->get_null_bvect();
    if (bv_null)
        bv_null->set_bit_no_check(idx);
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::set_value_no_null(size_type idx, value_type v)
{
    // calculate logical block coordinates and masks
    //
    block_idx_type nb = (idx >>  bm::set_block_shift);
    unsigned i0 = unsigned(nb >> bm::set_array_shift); // top block address
    unsigned j0 = unsigned(nb &  bm::set_array_mask);  // address in sub-block

    // clear the plains where needed
    unsigned eff_plains = this->effective_plains();
    unsigned bsr = v ? bm::bit_scan_reverse(v) : 0u;
        
    for (unsigned i = bsr; i < eff_plains; ++i)
    {
        const bm::word_t* blk = this->bmatr_.get_block(i, i0, j0);
        if (blk)
        {
            bvector_type* bv = this->bmatr_.get_row(i);
            if (bv)
                bv->clear_bit_no_check(idx);
        }
    } // for i

    if (v)
    {
        value_type mask = 1u;
        for (unsigned j = 0; j <= bsr; ++j)
        {
            if (v & mask)
            {
                bvector_type* bv = this->get_plain(j);
                bv->set_bit_no_check(idx);
            }
            else
            {
                const bm::word_t* blk = this->bmatr_.get_block(j, i0, j0);
                if (blk)
                {
                    bvector_type* bv = this->bmatr_.get_row(j);
                    bv->clear_bit_no_check(idx);
                }
            }
            mask <<=  1;
        } // for j
    }
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::inc(size_type idx)
{
    if (idx >= this->size_)
        this->size_ = idx+1;
    inc_no_null(idx);
    bvector_type* bv_null = this->get_null_bvect();
    if (bv_null)
        bv_null->set_bit_no_check(idx);
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::inc_no_null(size_type idx)
{
    for (unsigned i = 0; i < parent_type::sv_value_plains; ++i)
    {
        bvector_type* bv = this->get_plain(i);
        bool carry_over = bv->inc(idx);
        if (!carry_over)
            break;
    }
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::inc_no_null(size_type idx, value_type v)
{
    value_type v_prev = get(idx);
    set_value_no_null(idx, v + v_prev);
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::clear() BMNOEXCEPT
{
    parent_type::clear();
}

//---------------------------------------------------------------------

template<class Val, class BV>
bool sparse_vector<Val, BV>::find_rank(size_type rank, size_type& pos) BMNOEXCEPT
{
    BM_ASSERT(rank);
    pos = rank - 1; 
    return true;
}

//---------------------------------------------------------------------

template<class Val, class BV>
sparse_vector<Val, BV>&
sparse_vector<Val, BV>::clear_range(
    typename sparse_vector<Val, BV>::size_type left,
    typename sparse_vector<Val, BV>::size_type right,
    bool set_null)
{
    parent_type::clear_range(left, right, set_null);
    return *this;
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::calc_stat(
     struct sparse_vector<Val, BV>::statistics* st) const BMNOEXCEPT
{
    BM_ASSERT(st);
    typename bvector_type::statistics stbv;
    parent_type::calc_stat(&stbv);
    if (st)
    {
        st->reset();
        st->add(stbv);
    }
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::optimize(
    bm::word_t*                                  temp_block, 
    typename bvector_type::optmode               opt_mode,
    typename sparse_vector<Val, BV>::statistics* st)
{
    typename bvector_type::statistics stbv;
    stbv.reset();
    parent_type::optimize(temp_block, opt_mode, st ? &stbv : 0);
    
    if (st)
    {
        st->reset();
        st->add(stbv);
    }
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::optimize_gap_size()
{
    unsigned stored_plains = this->stored_plains();
    for (unsigned j = 0; j < stored_plains; ++j)
    {
        bvector_type* bv = this->bmatr_.get_row(j);
        if (bv)
        {
            bv->optimize_gap_size();
        }
    }
}

//---------------------------------------------------------------------

template<class Val, class BV>
sparse_vector<Val, BV>&
sparse_vector<Val, BV>::join(const sparse_vector<Val, BV>& sv)
{
    size_type arg_size = sv.size();
    if (this->size_ < arg_size)
    {
        resize(arg_size);
    }
    bvector_type* bv_null = this->get_null_bvect();
    unsigned plains;
    if (bv_null)
        plains = this->stored_plains();
    else
        plains = this->plains();
    
    for (unsigned j = 0; j < plains; ++j)
    {
        const bvector_type* arg_bv = sv.bmatr_.row(j);
        if (arg_bv)
        {
            bvector_type* bv = this->bmatr_.get_row(j);
            if (!bv)
                bv = this->get_plain(j);
            *bv |= *arg_bv;
        }
    } // for j
    
    // our vector is NULL-able but argument is not (assumed all values are real)
    if (bv_null && !sv.is_nullable())
    {
        bv_null->set_range(0, arg_size-1);
    }
    
    return *this;
}

//---------------------------------------------------------------------

template<class Val, class BV>
sparse_vector<Val, BV>&
sparse_vector<Val, BV>::merge(sparse_vector<Val, BV>& sv)
{
    size_type arg_size = sv.size();
    if (this->size_ < arg_size)
    {
        resize(arg_size);
    }
    bvector_type* bv_null = this->get_null_bvect();
    unsigned plains;
    if (bv_null)
        plains = this->stored_plains();
    else
        plains = this->plains();
    
    for (unsigned j = 0; j < plains; ++j)
    {
        bvector_type* arg_bv = sv.bmatr_.get_row(j);//sv.plains_[j];
        if (arg_bv)
        {
            bvector_type* bv = this->bmatr_.get_row(j);//this->plains_[j];
            if (!bv)
                bv = this->get_plain(j);
            bv->merge(*arg_bv);
        }
    } // for j
    
    // our vector is NULL-able but argument is not (assumed all values are real)
    if (bv_null && !sv.is_nullable())
    {
        bv_null->set_range(0, arg_size-1);
    }
    
    return *this;
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::copy_range(const sparse_vector<Val, BV>& sv,
                                        typename sparse_vector<Val, BV>::size_type left,
                                        typename sparse_vector<Val, BV>::size_type right,
                                        bm::null_support splice_null)
{
    if (left > right)
        bm::xor_swap(left, right);
    //this->clear();
    this->copy_range_plains(sv, left, right, splice_null);
    this->resize(sv.size());
}
//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::filter(
                    const typename sparse_vector<Val, BV>::bvector_type& bv_mask)
{
    bvector_type* bv_null = this->get_null_bvect();
    unsigned plains;
    if (bv_null)
    {
        plains = this->stored_plains();
        bv_null->bit_and(bv_mask);
    }
    else
        plains = this->plains();
    
    for (unsigned j = 0; j < plains; ++j)
    {
        bvector_type* bv = this->bmatr_.get_row(j);
        if (bv)
            bv->bit_and(bv_mask);
    }
}

//---------------------------------------------------------------------

template<class Val, class BV>
int sparse_vector<Val, BV>::compare(size_type idx,
                                    const value_type val) const BMNOEXCEPT
{
    // TODO: consider bit-by-bit comparison to minimize CPU hit miss in plans get()
    value_type sv_value = get(idx);
    return (sv_value > val) - (sv_value < val);
}

//---------------------------------------------------------------------

template<class Val, class BV>
bool sparse_vector<Val, BV>::equal(const sparse_vector<Val, BV>& sv,
                                   bm::null_support null_able) const BMNOEXCEPT
{
    return parent_type::equal(sv, null_able);
}

//---------------------------------------------------------------------

template<class Val, class BV>
typename sparse_vector<Val, BV>::const_iterator
sparse_vector<Val, BV>::begin() const BMNOEXCEPT
{
    typedef typename sparse_vector<Val, BV>::const_iterator it_type;
    return it_type(this);
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::set_allocator_pool(
    typename sparse_vector<Val, BV>::allocator_pool_type* pool_ptr) BMNOEXCEPT
{
    this->bmatr_.set_allocator_pool(pool_ptr);
}


//---------------------------------------------------------------------
//
//---------------------------------------------------------------------


template<class Val, class BV>
sparse_vector<Val, BV>::const_iterator::const_iterator() BMNOEXCEPT
: sv_(0), pos_(bm::id_max), buf_ptr_(0)
{}

//---------------------------------------------------------------------

template<class Val, class BV>
sparse_vector<Val, BV>::const_iterator::const_iterator(
    const typename sparse_vector<Val, BV>::const_iterator& it) BMNOEXCEPT
: sv_(it.sv_), pos_(it.pos_), buf_ptr_(0)
{}

//---------------------------------------------------------------------

template<class Val, class BV>
sparse_vector<Val, BV>::const_iterator::const_iterator(
  const typename sparse_vector<Val, BV>::const_iterator::sparse_vector_type* sv
  ) BMNOEXCEPT
: sv_(sv), buf_ptr_(0)
{
    BM_ASSERT(sv_);
    pos_ = sv_->empty() ? bm::id_max : 0u;
}

//---------------------------------------------------------------------

template<class Val, class BV>
sparse_vector<Val, BV>::const_iterator::const_iterator(
 const typename sparse_vector<Val, BV>::const_iterator::sparse_vector_type* sv,
 typename sparse_vector<Val, BV>::size_type pos) BMNOEXCEPT
: sv_(sv), buf_ptr_(0)
{
    BM_ASSERT(sv_);
    this->go_to(pos);
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::const_iterator::go_to(size_type pos) BMNOEXCEPT
{
    pos_ = (!sv_ || pos >= sv_->size()) ? bm::id_max : pos;
    buf_ptr_ = 0;
}

//---------------------------------------------------------------------

template<class Val, class BV>
bool sparse_vector<Val, BV>::const_iterator::advance() BMNOEXCEPT
{
    if (pos_ == bm::id_max) // nothing to do, we are at the end
        return false;
    ++pos_;
    if (pos_ >= sv_->size())
    {
        this->invalidate();
        return false;
    }
    if (buf_ptr_)
    {
        ++buf_ptr_;
        if (buf_ptr_ - ((value_type*)buffer_.data()) >= n_buf_size)
            buf_ptr_ = 0;
    }
    return true;
}

//---------------------------------------------------------------------

template<class Val, class BV>
typename sparse_vector<Val, BV>::const_iterator::value_type
sparse_vector<Val, BV>::const_iterator::value() const
{
    BM_ASSERT(this->valid());
    value_type v;
    
    if (!buf_ptr_)
    {
        buffer_.reserve(n_buf_size * sizeof(value_type));
        buf_ptr_ = (value_type*)(buffer_.data());
        sv_->extract(buf_ptr_, n_buf_size, pos_, true);
    }
    v = *buf_ptr_;
    return v;
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::const_iterator::skip_zero_values() BMNOEXCEPT
{
    value_type v = value();
    if (buf_ptr_)
    {
        v = *buf_ptr_;
        value_type* buf_end = ((value_type*)buffer_.data()) + n_buf_size;
        while(!v)
        {
            ++pos_;
            if (++buf_ptr_ < buf_end)
                v = *buf_ptr_;
            else
                break;
        }
        if (pos_ >= sv_->size())
        {
            pos_ = bm::id_max;
            return;
        }
        if (buf_ptr_ >= buf_end)
            buf_ptr_ = 0;
    }
}

//---------------------------------------------------------------------

template<class Val, class BV>
bool sparse_vector<Val, BV>::const_iterator::is_null() const BMNOEXCEPT
{
    return sv_->is_null(pos_);
}


//---------------------------------------------------------------------
//
//---------------------------------------------------------------------


template<class Val, class BV>
sparse_vector<Val, BV>::back_insert_iterator::back_insert_iterator()
: sv_(0), bv_null_(0), buf_ptr_(0), prev_nb_(0), set_not_null_(true)
{}

//---------------------------------------------------------------------

template<class Val, class BV>
sparse_vector<Val, BV>::back_insert_iterator::back_insert_iterator(
   typename sparse_vector<Val, BV>::back_insert_iterator::sparse_vector_type* sv)
: sv_(sv), buf_ptr_(0), set_not_null_(true)
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

template<class Val, class BV>
sparse_vector<Val, BV>::back_insert_iterator::back_insert_iterator(
    const typename sparse_vector<Val, BV>::back_insert_iterator& bi)
: sv_(bi.sv_), bv_null_(bi.bv_null_), buf_ptr_(0), prev_nb_(bi.prev_nb_),
  set_not_null_(bi.set_not_null_)
{
    BM_ASSERT(bi.empty());
}

//---------------------------------------------------------------------

template<class Val, class BV>
sparse_vector<Val, BV>::back_insert_iterator::~back_insert_iterator()
{
    this->flush();
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::back_insert_iterator::add(
         typename sparse_vector<Val, BV>::back_insert_iterator::value_type v)
{
    size_type buf_idx = this->add_value_no_null(v);
    if (bv_null_)
    {
        typename sparse_vector<Val, BV>::size_type sz = sv_->size();
        bv_null_->set_bit_no_check(sz + buf_idx);
    }
}

//---------------------------------------------------------------------

template<class Val, class BV>
typename sparse_vector<Val, BV>::size_type
sparse_vector<Val, BV>::back_insert_iterator::add_value_no_null(
         typename sparse_vector<Val, BV>::back_insert_iterator::value_type v)
{
    BM_ASSERT(sv_);
    if (!buf_ptr_) // not allocated (yet)
    {
        buffer_.reserve(n_buf_size * sizeof(value_type));
        buf_ptr_ = (value_type*)(buffer_.data());
        *buf_ptr_ = v;
        ++buf_ptr_;
        return 0;
    }
    if (buf_ptr_ - ((value_type*)buffer_.data()) >= n_buf_size)
    {
        this->flush();
        buf_ptr_ = (value_type*)(buffer_.data());
    }
    
    *buf_ptr_ = v;
    size_type buf_idx = size_type(buf_ptr_ - ((value_type*)buffer_.data()));
    ++buf_ptr_;
    return buf_idx;
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::back_insert_iterator::add_null()
{
    BM_ASSERT(bv_null_);
    this->add_value_no_null(value_type(0));
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::back_insert_iterator::add_null(
    typename sparse_vector<Val, BV>::back_insert_iterator::size_type count)
{
    if (count < 32)
    {
        for (size_type i = 0; i < count; ++i)
            this->add_value_no_null(value_type(0));
    }
    else
    {
        this->flush();
        sv_->push_back_null(count);
    }
}

//---------------------------------------------------------------------

template<class Val, class BV>
bool sparse_vector<Val, BV>::back_insert_iterator::empty() const
{
    return (!buf_ptr_ || !sv_);
}

//---------------------------------------------------------------------

template<class Val, class BV>
void sparse_vector<Val, BV>::back_insert_iterator::flush()
{
    if (this->empty())
        return;
    value_type* d = (value_type*)buffer_.data();
    sv_->import_back(d, size_type(buf_ptr_ - d), false); //!set_not_null_);
    buf_ptr_ = 0;
    block_idx_type nb = sv_->size() >> bm::set_block_shift;
    if (nb != prev_nb_)
    {
        // optimize all previous blocks in all planes
        sv_->optimize_block(prev_nb_);
        prev_nb_ = nb;
    }
}

//---------------------------------------------------------------------


} // namespace bm

#include "bmundef.h"


#endif
