#ifndef BMSPARSEVEC_SERIAL__H__INCLUDED__
#define BMSPARSEVEC_SERIAL__H__INCLUDED__
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

/*! \file bmsparsevec_serial.h
    \brief Serialization for sparse_vector<>
*/


#ifndef BM__H__INCLUDED__
// BitMagic utility headers do not include main "bm.h" declaration 
// #include "bm.h" or "bm64.h" explicitly 
# error missing include (bm.h or bm64.h)
#endif

#include "bmsparsevec.h"
#include "bmserial.h"
#include "bmbuffer.h"
#include "bmdef.h"

namespace bm
{

/** \defgroup svserial Sparse vector serialization
    Sparse vector serialization
    \ingroup svector
 */


/*!
    \brief layout class for serialization buffer structure
    
    Class keeps a memory block sized for the target sparse vector BLOB.
    This class also provides acess to bit-plane memory, so it becomes possible
    to use parallel storage methods to save bit-plains into
    different storage shards.
    
    \ingroup svserial
*/
template<class SV>
struct sparse_vector_serial_layout
{
    typedef typename SV::value_type   value_type;
    typedef typename SV::bvector_type bvector_type;
    typedef typename serializer<bvector_type>::buffer buffer_type;

    sparse_vector_serial_layout() {}
    
    ~sparse_vector_serial_layout() {}
    
    /*!
        \brief resize capacity
        \param capacity - new capacity
        \return new buffer or 0 if failed
    */
    unsigned char* reserve(size_t capacity)
    {
        if (capacity == 0)
        {
            freemem();
            return 0;
        }
        buf_.reinit(capacity);
        return buf_.data();
    }
    
    /// return current serialized size
    size_t  size() const { return buf_.size();  }
    
    /// Set new serialized size
    void resize(size_t ssize) { buf_.resize(ssize);  }
    
    /// return serialization buffer capacity
    size_t  capacity() const { return buf_.capacity(); }
    
    /// free memory
    void freemem() { buf_.release(); }
    
    /// Set plain output pointer and size
    void set_plain(unsigned i, unsigned char* ptr, size_t buf_size)
    {
        plain_ptrs_[i] = ptr;
        plane_size_[i] = buf_size;
    }
    
    /// Get plain pointer
    const unsigned char* get_plain(unsigned i) const { return plain_ptrs_[i]; }
    
    /// Return serialization buffer pointer
    const unsigned char* buf() const { return buf_.buf();  }
    /// Return serialization buffer pointer
    const unsigned char* data() const { return buf_.buf();  }

private:
    sparse_vector_serial_layout(const sparse_vector_serial_layout&);
    void operator=(const sparse_vector_serial_layout&);
protected:
    buffer_type    buf_;                       ///< serialization buffer
    unsigned char* plain_ptrs_[SV::sv_plains]; ///< pointers on serialized bit-plains
    size_t         plane_size_[SV::sv_plains]; ///< serialized plain size
};

// -------------------------------------------------------------------------

/*!
    \brief Serialize sparse vector into a memory buffer(s) structure
 
 Serialization format:

 | HEADER | BIT-VECTORS ... | REMAP_MATRIX

 Header structure:
 -----------------
   BYTE+BYTE: Magic-signature 'BM' or 'BC' (c-compressed)
   BYTE : Byte order ( 0 - Big Endian, 1 - Little Endian)
   {
        BYTE : Number of Bit-vector plains (total) (non-zero when < 255 plains)
        |
        BYTE: zero - flag of large plain matrix
        INT64: Nnmber of bit-vector plains
   }
   INT64: Vector size
   INT64: Offset of plain 0 from the header start (value 0 means plain is empty)
   INT64: Offset of plain 1 from
   ...
   INT32: reserved
 
Bit-vectors:
------------
   Based on current bit-vector serialization

Remap Matrix:
   SubHeader | Matrix BLOB
 
   sub-header:
     BYTE:  'R' (remapping) or 'N' (no remapping)
             N - means no other info is saved on the stream
     INT64:  remap matrix size
 
    \ingroup svector
    \ingroup svserial
*/
template<typename SV>
class sparse_vector_serializer
{
public:
    typedef typename SV::bvector_type       bvector_type;
    typedef const bvector_type*             bvector_type_const_ptr;
    typedef bvector_type*                   bvector_type_ptr;
    typedef typename SV::value_type         value_type;
    typedef typename SV::size_type          size_type;
    typedef typename bvector_type::allocator_type::allocator_pool_type
                                                   allocator_pool_type;

public:
    sparse_vector_serializer();

    /**
        Add skip-markers for faster range deserialization

        @param enable - TRUE searilization will add bookmark codes
        @param bm_interval - bookmark interval in (number of blocks)
                            (suggested between 4 and 512)
        smaller interval means more bookmarks added to the skip list thus
        more increasing the BLOB size
    */
    void set_bookmarks(bool enable, unsigned bm_interval = 256)
        { bvs_.set_bookmarks(enable, bm_interval); }

    /// Turn ON and OFF XOR compression of sparse vectors
    void set_xor_ref(bool is_enabled) { is_xor_ref_ = is_enabled; }

    /// Get XOR reference compression status (enabled/disabled)
    bool is_xor_ref() const { return is_xor_ref_; }
    
    /*!
        \brief Serialize sparse vector into a memory buffer(s) structure
     
        \param sv         - sparse vector to serialize
        \param sv_layout  - buffer structure to keep the result
        as defined in bm::serialization_flags
    */
    void serialize(const SV&                        sv,
                   sparse_vector_serial_layout<SV>& sv_layout);

protected:
    typedef typename
    bm::serializer<bvector_type>::bv_ref_vector_type bv_ref_vector_type;

    void build_xor_ref_vector(const SV& sv);

private:
    sparse_vector_serializer(const sparse_vector_serializer&) = delete;
    sparse_vector_serializer& operator=(const sparse_vector_serializer&) = delete;
protected:
    bm::serializer<bvector_type>     bvs_;

    bool                             is_xor_ref_;
    bv_ref_vector_type               bv_ref_;
};

/**
    sparse vector de-serializer

*/
template<typename SV>
class sparse_vector_deserializer
{
public:
    typedef typename SV::bvector_type       bvector_type;
    typedef const bvector_type*             bvector_type_const_ptr;
    typedef bvector_type*                   bvector_type_ptr;
    typedef typename SV::value_type         value_type;
    typedef typename SV::size_type          size_type;
    typedef typename bvector_type::allocator_type::allocator_pool_type allocator_pool_type;

public:
    sparse_vector_deserializer();
    ~sparse_vector_deserializer();

    /*!
        Deserialize sparse vector

        @param sv - [out] target sparse vector to populate
        @param buf - source memory pointer
    */
    void deserialize(SV& sv, const unsigned char* buf)
        { idx_range_set_ = false; deserialize_sv(sv, buf, 0); }

    /*!
        Deserialize sparse vector for the range [from, to]

        @param sv - [out] target sparse vector to populate
        @param buf - source memory pointer
        @param from - start vector index for deserialization range
        @param to - end vector index for deserialization range
    */
    void deserialize_range(SV& sv, const unsigned char* buf,
                           size_type from, size_type to);

    void deserialize(SV& sv, const unsigned char* buf,
                     size_type from, size_type to)
    {
        deserialize_range(sv, buf, from, to);
    }


    /*!
        Deserialize sparse vector using address mask vector
        Address mask defines (by set bits) which vector elements to be extracted
        from the compressed BLOB

        @param sv - [out] target sparse vector to populate
        @param buf - source memory pointer
        @param mask_bv - AND mask bit-vector (address gather vector)
    */
    void deserialize(SV& sv,
                     const unsigned char* buf,
                     const bvector_type& mask_bv)
        { idx_range_set_ = false; deserialize_sv(sv, buf, &mask_bv); }


protected:
    typedef typename bvector_type::allocator_type      alloc_type;
    typedef typename bm::serializer<bvector_type>::bv_ref_vector_type bv_ref_vector_type;


    /// Deserialize header/version and other common info
    ///
    /// @return number of bit-plains
    ///
    unsigned load_header(bm::decoder& dec, SV& sv, unsigned char& matr_s_ser);

    void deserialize_sv(SV& sv, const unsigned char* buf,
                        const bvector_type* mask_bv);


    /// deserialize bit-vector plains
    void deserialize_plains(SV& sv, unsigned plains,
                            const unsigned char* buf,
                            const bvector_type* mask_bv = 0);

    /// load offset table
    void load_plains_off_table(bm::decoder& dec, unsigned plains);

    /// load NULL bit-plain (returns new plains count)
    int load_null_plain(SV& sv,
                        int plains,
                        const unsigned char* buf,
                        const bvector_type* mask_bv);

    /// load string remap dict
    static void load_remap(SV& sv, const unsigned char* remap_buf_ptr);

    /// throw error on incorrect deserialization
    static void raise_invalid_header();
    /// throw error on incorrect deserialization
    static void raise_invalid_64bit();
    /// throw error on incorrect deserialization
    static void raise_invalid_bitdepth();

private:
    sparse_vector_deserializer(const sparse_vector_deserializer&) = delete;
    sparse_vector_deserializer& operator=(const sparse_vector_deserializer&) = delete;
protected:
    const unsigned char*                        remap_buf_ptr_;
    alloc_type                                  alloc_;
    bm::word_t*                                 temp_block_;
    allocator_pool_type                         pool_;
    bm::deserializer<bvector_type, bm::decoder> deserial_;
    bm::operation_deserializer<bvector_type>    op_deserial_;
    bm::rank_compressor<bvector_type>           rsc_compressor_;
    bvector_type                                not_null_mask_bv_;
    bvector_type                                rsc_mask_bv_;
    bm::heap_vector<size_t, alloc_type, true>   off_vect_;

    bv_ref_vector_type                          bv_ref_;

    bool                                        idx_range_set_;
    size_type                                   idx_range_from_;
    size_type                                   idx_range_to_;
};



/*!
    \brief Serialize sparse vector into a memory buffer(s) structure
 
    \param sv         - sparse vector to serialize
    \param sv_layout  - buffer structure to keep the result
    \param temp_block - temporary buffer 
                        (allocate with BM_DECLARE_TEMP_BLOCK(x) for speed)
    
    \ingroup svserial
    
    @sa serialization_flags
    @sa sparse_vector_deserializer
*/
template<class SV>
void sparse_vector_serialize(
                const SV&                        sv,
                sparse_vector_serial_layout<SV>& sv_layout,
                bm::word_t*                      temp_block = 0)
{
    (void)temp_block;
    bm::sparse_vector_serializer<SV> sv_serializer;
    sv_serializer.serialize(sv, sv_layout);
}

// -------------------------------------------------------------------------

/*!
    \brief Deserialize sparse vector
    \param sv         - target sparse vector
    \param buf        - source memory buffer
    \param temp_block - temporary block buffer to avoid re-allocations
 
    \return 0  (error processing via std::logic_error)
 
    \ingroup svserial
    @sa sparse_vector_deserializer
*/
template<class SV>
int sparse_vector_deserialize(SV& sv,
                              const unsigned char* buf,
                              bm::word_t* temp_block=0)
{
    (void)temp_block;
    bm::sparse_vector_deserializer<SV> sv_deserializer;
    sv_deserializer.deserialize(sv, buf);
    return 0;
}

// -------------------------------------------------------------------------

/**
    Seriaizer for compressed collections
*/
template<class CBC>
class compressed_collection_serializer
{
public:
    typedef CBC                                  compressed_collection_type;
    typedef typename CBC::bvector_type           bvector_type;
    typedef typename CBC::buffer_type            buffer_type;
    typedef typename CBC::statistics             statistics_type;
    typedef typename CBC::address_resolver_type  address_resolver_type;
    
public:
    void serialize(const CBC&    buffer_coll,
                   buffer_type&  buf,
                   bm::word_t*   temp_block = 0);
};

/**
    Deseriaizer for compressed collections
*/
template<class CBC>
class compressed_collection_deserializer
{
public:
    typedef CBC                                  compressed_collection_type;
    typedef typename CBC::bvector_type           bvector_type;
    typedef typename CBC::buffer_type            buffer_type;
    typedef typename CBC::statistics             statistics_type;
    typedef typename CBC::address_resolver_type  address_resolver_type;
    typedef typename CBC::container_type         container_type;

public:
    int deserialize(CBC&                 buffer_coll,
                    const unsigned char* buf,
                    bm::word_t*          temp_block=0);
};


// -------------------------------------------------------------------------

/**
    \brief Serialize compressed collection into memory buffer

Serialization format:


<pre>
 | MAGIC_HEADER | ADDRESS_BITVECTROR | LIST_OF_BUFFER_SIZES | BUFFER(s)
 
   MAGIC_HEADER:
   BYTE+BYTE: Magic-signature 'BM' or 'BC'
   BYTE : Byte order ( 0 - Big Endian, 1 - Little Endian)
 
   ADDRESS_BITVECTROR:
   INT64: address bit-vector size
   [memblock]: serialized address bit-vector
 
   LIST_OF_BUFFER_SIZES:
   INT64 - buffer sizes count
   INT32 - buffer size 0
   INT32 - buffer size 1
   ...
 
   BUFFERS:
   [memblock]: block0
   [memblock]: block1
   ...
 
</pre>
*/

template<class CBC>
void compressed_collection_serializer<CBC>::serialize(const CBC&    buffer_coll,
                                                      buffer_type&  buf,
                                                      bm::word_t*   temp_block)
{
    statistics_type st;
    buffer_coll.calc_stat(&st);
    
    buf.resize(st.max_serialize_mem);
    
    // ptr where bit-plains start
    unsigned char* buf_ptr = buf.data();
    
    bm::encoder enc(buf.data(), buf.capacity());
    ByteOrder bo = globals<true>::byte_order();
    enc.put_8('B');
    enc.put_8('C');
    enc.put_8((unsigned char)bo);
    
    unsigned char* mbuf1 = enc.get_pos(); // bookmark position
    enc.put_64(0);  // address vector size (reservation)

    buf_ptr = enc.get_pos();

    const address_resolver_type& addr_res = buffer_coll.resolver();
    const bvector_type& bv = addr_res.get_bvector();
    {
        bm::serializer<bvector_type > bvs(temp_block);
        bvs.gap_length_serialization(false);

        size_t addr_bv_size = bvs.serialize(bv, buf_ptr, buf.size());
        buf_ptr += addr_bv_size;

        enc.set_pos(mbuf1); // rewind to bookmark
        enc.put_64(addr_bv_size); // save the address vector size
    }
    enc.set_pos(buf_ptr); // restore stream position
    size_t coll_size = buffer_coll.size();
    
    enc.put_64(coll_size);
    
    // pass 1 (save buffer sizes)
    {
        for (unsigned i = 0; i < buffer_coll.size(); ++i)
        {
            const buffer_type& cbuf = buffer_coll.get(i);
            size_t sz = cbuf.size();
            enc.put_64(sz);
        } // for i
    }
    // pass 2 (save buffers)
    {
        for (unsigned i = 0; i < buffer_coll.size(); ++i)
        {
            const buffer_type& cbuf = buffer_coll.get(i);
            size_t sz = cbuf.size();
            enc.memcpy(cbuf.buf(), sz);
        } // for i
    }
    buf.resize(enc.size());
}

// -------------------------------------------------------------------------
template<class CBC>
int compressed_collection_deserializer<CBC>::deserialize(
                                CBC&                 buffer_coll,
                                const unsigned char* buf,
                                bm::word_t*          temp_block)
{
    // TODO: implement correct processing of byte-order corect deserialization
    //    ByteOrder bo_current = globals<true>::byte_order();
    
    bm::decoder dec(buf);
    unsigned char h1 = dec.get_8();
    unsigned char h2 = dec.get_8();

    BM_ASSERT(h1 == 'B' && h2 == 'C');
    if (h1 != 'B' && h2 != 'C')  // no magic header? issue...
    {
        return -1;
    }
    //unsigned char bv_bo =
        dec.get_8();

    // -----------------------------------------
    // restore address resolver
    //
    bm::id64_t addr_bv_size = dec.get_64();
    
    const unsigned char* bv_buf_ptr = dec.get_pos();
    
    address_resolver_type& addr_res = buffer_coll.resolver();
    bvector_type& bv = addr_res.get_bvector();
    bv.clear();
    
    bm::deserialize(bv, bv_buf_ptr, temp_block);
    addr_res.sync();
    
    typename bvector_type::size_type addr_cnt = bv.count();
    dec.seek((int)addr_bv_size);
    
    // -----------------------------------------
    // read buffer sizes
    //
    bm::id64_t coll_size = dec.get_64();
    if (coll_size != addr_cnt)
    {
        return -2; // buffer size collection does not match address vector
    }
    
	typedef std::vector<unsigned>::size_type vect_size_type;
	std::vector<bm::id64_t> buf_size_vec;
	buf_size_vec.resize(vect_size_type(coll_size));
    {
        for (unsigned i = 0; i < coll_size; ++i)
        {
            bm::id64_t sz = dec.get_64();
            buf_size_vec[i] = sz;
        } // for i
    }

    {
        container_type& buf_vect = buffer_coll.container();
        buf_vect.resize(vect_size_type(coll_size));
        for (unsigned i = 0; i < coll_size; ++i)
        {
            bm::id64_t sz = buf_size_vec[i];
            buffer_type& b = buf_vect.at(i);
            b.resize(sz);
            dec.memcpy(b.data(), sz);
        } // for i
    }
    buffer_coll.sync();
    return 0;
}

// -------------------------------------------------------------------------
//
// -------------------------------------------------------------------------

template<typename SV>
sparse_vector_serializer<SV>::sparse_vector_serializer()
{
    bvs_.gap_length_serialization(false);
    #ifdef BMXORCOMP
        is_xor_ref_ = true;
    #else
        is_xor_ref_ = false;
    #endif
}

// -------------------------------------------------------------------------

template<typename SV>
void sparse_vector_serializer<SV>::build_xor_ref_vector(const SV& sv)
{
    bv_ref_.reset();
    bv_ref_.build(sv.get_bmatrix());
}

// -------------------------------------------------------------------------

template<typename SV>
void sparse_vector_serializer<SV>::serialize(const SV&  sv,
                      sparse_vector_serial_layout<SV>&  sv_layout)
{
    typename SV::statistics sv_stat;
    sv.calc_stat(&sv_stat);
    unsigned char* buf = sv_layout.reserve(sv_stat.max_serialize_mem);
    
    bm::encoder enc(buf, (unsigned)sv_layout.capacity());
    unsigned plains = sv.stored_plains();

    // header size in bytes
    unsigned h_size = 1 + 1 +        // "BM" or "BC" (magic header)
                      1 +            // byte-order
                      1 +            // number of bit-plains (for vector)
                      8 +            // size (internal 64-bit)
                      (8 * plains) + // offsets of all plains
                      4;             //  reserve
    // for large plain matrixes
    {
        h_size += 1 + // version number
                  8;  // number of plains (64-bit)
    }

    // ---------------------------------
    // Setup XOR reference compression
    //
    if (is_xor_ref())
    {
        build_xor_ref_vector(sv);
        bvs_.set_ref_vectors(&bv_ref_);
    }

    // -----------------------------------------------------
    // Serialize all bvector plains
    //
    
    unsigned char* buf_ptr = buf + h_size; // ptr where plains start (start+hdr)

    unsigned i;
    for (i = 0; i < plains; ++i)
    {
        typename SV::bvector_type_const_ptr bv = sv.get_plain(i);
        if (!bv)  // empty plain
        {
            sv_layout.set_plain(i, 0, 0);
            continue;
        }
        if (is_xor_ref())
        {
            unsigned idx = (unsigned)bv_ref_.find(i);
            BM_ASSERT(idx != bv_ref_.not_found());
            bvs_.set_curr_ref_idx(idx);
        }

        size_t buf_size =
            bvs_.serialize(*bv, buf_ptr, sv_stat.max_serialize_mem);
        
        sv_layout.set_plain(i, buf_ptr, buf_size);
        buf_ptr += buf_size;
        if (sv_stat.max_serialize_mem > buf_size)
        {
            sv_stat.max_serialize_mem -= buf_size;
            continue;
        }
        BM_ASSERT(0); // TODO: throw an exception here
    } // for i

    bvs_.set_ref_vectors(0); // disangage XOR ref vector

    // -----------------------------------------------------
    // serialize the re-map matrix
    //
    if (bm::conditional<SV::is_remap_support::value>::test()) // test remapping trait
    {
        bm::encoder enc_m(buf_ptr, sv_stat.max_serialize_mem);
        if (sv.is_remap())
        {
            bm::id64_t remap_size = sv.remap_size();
            const unsigned char* matrix_buf = sv.get_remap_buffer();
            BM_ASSERT(matrix_buf);
            BM_ASSERT(remap_size);

            enc_m.put_8('R');
            enc_m.put_64(remap_size);
            enc_m.memcpy(matrix_buf, remap_size);
            enc_m.put_8('E'); // end of matrix (integrity check token)
        }
        else
        {
            enc_m.put_8('N');
        }
        buf_ptr += enc_m.size(); // add mattrix encoded data size
    }
    
    sv_layout.resize(size_t(buf_ptr - buf)); // set the true occupied size

    // -----------------------------------------------------
    // save the header
    //
    ByteOrder bo = bm::globals<true>::byte_order();
    
    enc.put_8('B');  // magic header 'BM' - bit matrix 'BC' - bit compressed
    if (sv.is_compressed())
        enc.put_8('C');
    else
        enc.put_8('M');
    
    enc.put_8((unsigned char)bo);  // byte order
    
    unsigned char matr_s_ser = 1;
#ifdef BM64ADDR
    matr_s_ser = 2;
#endif
    
    enc.put_8(0);              // number of plains == 0 (legacy magic number)
    enc.put_8(matr_s_ser);     // matrix serialization version
    enc.put_64(plains);        // number of rows in the bit-matrix
    enc.put_64(sv.size_internal());
    
    // save the offset table (part of the header)
    //
    for (i = 0; i < plains; ++i)
    {
        const unsigned char* p = sv_layout.get_plain(i);
        if (!p)
        {
            enc.put_64(0);
            continue;
        }
        size_t offset = size_t(p - buf);
        enc.put_64(offset);
    } // for
}

// -------------------------------------------------------------------------
//
// -------------------------------------------------------------------------

template<typename SV>
sparse_vector_deserializer<SV>::sparse_vector_deserializer()
    : remap_buf_ptr_(0)
{
    temp_block_ = alloc_.alloc_bit_block();
    not_null_mask_bv_.set_allocator_pool(&pool_);
    rsc_mask_bv_.set_allocator_pool(&pool_);
    idx_range_set_ = false;
}

// -------------------------------------------------------------------------

template<typename SV>
sparse_vector_deserializer<SV>::~sparse_vector_deserializer()
{
    if (temp_block_)
        alloc_.free_bit_block(temp_block_);
}

// -------------------------------------------------------------------------

template<typename SV>
void sparse_vector_deserializer<SV>::deserialize_range(SV& sv,
                                                 const unsigned char* buf,
                                                 size_type from, size_type to)
{
    idx_range_set_ = true; idx_range_from_ = from; idx_range_to_ = to;

    remap_buf_ptr_ = 0;
    bm::decoder dec(buf); // TODO: implement correct processing of byte-order

    unsigned char matr_s_ser = 0;
    unsigned plains = load_header(dec, sv, matr_s_ser);

    sv.clear();

    bm::id64_t sv_size = dec.get_64();
    if (sv_size == 0)
        return;  // empty vector

    sv.resize_internal(size_type(sv_size));
    bv_ref_.reset();

    load_plains_off_table(dec, plains); // read the offset vector of bit-plains

    // TODO: add range for not NULL plane
    plains = (unsigned)load_null_plain(sv, int(plains), buf, 0);

    // check if mask needs to be relaculated using the NULL (index) vector
    if (bm::conditional<SV::is_rsc_support::value>::test())
    {
        // recalculate plains range
        size_type sv_left, sv_right;
        bool range_valid = sv.resolve_range(from, to, &sv_left, &sv_right);
        if (!range_valid)
        {
            sv.clear();
            idx_range_set_ = false;
            return;
        }
        else
        {
            idx_range_set_ = true; idx_range_from_ = sv_left; idx_range_to_ = sv_right;
        }
    }

    deserialize_plains(sv, plains, buf, 0);

    op_deserial_.set_ref_vectors(0);
    deserial_.set_ref_vectors(0);
    bv_ref_.reset();


    // load the remap matrix
    //
    if (bm::conditional<SV::is_remap_support::value>::test()) // test remap trait
    {
        if (matr_s_ser)
            load_remap(sv, remap_buf_ptr_);
    } // if remap traits

    sv.sync(true); // force sync, recalculate RS index, remap tables, etc
    remap_buf_ptr_ = 0;

    idx_range_set_ = false;
}

// -------------------------------------------------------------------------

template<typename SV>
void sparse_vector_deserializer<SV>::deserialize_sv(SV& sv,
                                                 const unsigned char* buf,
                                                 const bvector_type* mask_bv)
{
    remap_buf_ptr_ = 0;
    bm::decoder dec(buf); // TODO: implement correct processing of byte-order

    unsigned char matr_s_ser = 0;
    unsigned plains = load_header(dec, sv, matr_s_ser);

    sv.clear();
    
    bm::id64_t sv_size = dec.get_64();
    if (sv_size == 0)
        return;  // empty vector
        
    sv.resize_internal(size_type(sv_size));
    bv_ref_.reset();

    load_plains_off_table(dec, plains); // read the offset vector of bit-plains

    plains = (unsigned)load_null_plain(sv, int(plains), buf, mask_bv);

    // check if mask needs to be relaculated using the NULL (index) vector
    if (bm::conditional<SV::is_rsc_support::value>::test())
    {
        if (mask_bv)
        {
            const bvector_type* bv_null = sv.get_null_bvector();
            BM_ASSERT(bv_null);
            rsc_mask_bv_.clear(true);
            not_null_mask_bv_.bit_and(*bv_null, *mask_bv, bvector_type::opt_compress);
            rsc_compressor_.compress(rsc_mask_bv_, *bv_null, not_null_mask_bv_);
            mask_bv = &rsc_mask_bv_;

            // if it needs range recalculation
            if (idx_range_set_) // range setting is in effect
            {
                //bool rf =
                rsc_mask_bv_.find_range(idx_range_from_, idx_range_to_);
            }
        }
    }

    deserialize_plains(sv, plains, buf, mask_bv);

    op_deserial_.set_ref_vectors(0);
    deserial_.set_ref_vectors(0);
    bv_ref_.reset();


    // load the remap matrix
    //
    if (bm::conditional<SV::is_remap_support::value>::test()) // test remap trait
    {
        if (matr_s_ser)
            load_remap(sv, remap_buf_ptr_);
    } // if remap traits
    
    sv.sync(true); // force sync, recalculate RS index, remap tables, etc
    remap_buf_ptr_ = 0;
}

// -------------------------------------------------------------------------

template<typename SV>
unsigned sparse_vector_deserializer<SV>::load_header(
        bm::decoder& dec, SV& sv, unsigned char& matr_s_ser)
{
    unsigned char h1 = dec.get_8();
    unsigned char h2 = dec.get_8();

    BM_ASSERT(h1 == 'B' && (h2 == 'M' || h2 == 'C'));

    bool sig2_ok = (h2 == 'M' || h2 == 'C');
    if (h1 != 'B' || !sig2_ok) //&& (h2 != 'M' || h2 != 'C'))  // no magic header?
        raise_invalid_header();

    unsigned char bv_bo = dec.get_8(); (void) bv_bo;
    unsigned plains = dec.get_8();
    if (plains == 0)  // bit-matrix
    {
        matr_s_ser = dec.get_8(); // matrix serialization version
        plains = (unsigned) dec.get_64(); // number of rows in the bit-matrix
    }
    #ifdef BM64ADDR
    #else
        if (matr_s_ser == 2) // 64-bit matrix
        {
            raise_invalid_64bit();
        }
    #endif

    unsigned sv_plains = sv.stored_plains();
    if (!plains || plains > sv_plains)
        raise_invalid_bitdepth();
    return plains;
}

// -------------------------------------------------------------------------

template<typename SV>
void sparse_vector_deserializer<SV>::deserialize_plains(
                                                SV& sv,
                                                unsigned plains,
                                                const unsigned char* buf,
                                                const bvector_type* mask_bv)
{
    if (mask_bv && !idx_range_set_)
        idx_range_set_ = mask_bv->find_range(idx_range_from_, idx_range_to_);

    op_deserial_.set_ref_vectors(&bv_ref_);
    deserial_.set_ref_vectors(&bv_ref_);

    // read-deserialize the plains based on offsets
    //       backward order to bring the NULL vector first
    //
    for (int i = int(plains-1); i >= 0; --i)
    {
        size_t offset = off_vect_[unsigned(i)];
        if (!offset) // empty vector
            continue;
        const unsigned char* bv_buf_ptr = buf + offset; // seek to position
        bvector_type*  bv = sv.get_plain(unsigned(i));
        BM_ASSERT(bv);

        bv_ref_.add(bv, unsigned(i));

        if (mask_bv) // gather mask set, use AND operation deserializer
        {
            typename bvector_type::mem_pool_guard mp_g_z(pool_, *bv);

            if (bm::conditional<SV::is_remap_support::value>::test()
                && !remap_buf_ptr_) // last plain vector (special case)
            {
                size_t read_bytes =
                    deserial_.deserialize(*bv, bv_buf_ptr, temp_block_);
                remap_buf_ptr_ = bv_buf_ptr + read_bytes;
                bv->bit_and(*mask_bv, bvector_type::opt_compress);
                continue;
            }
            if (idx_range_set_)
                deserial_.set_range(idx_range_from_, idx_range_to_);
            deserial_.deserialize(*bv, bv_buf_ptr);
            bv->bit_and(*mask_bv, bvector_type::opt_compress);
        }
        else
        {
            if (bm::conditional<SV::is_remap_support::value>::test() &&
                !remap_buf_ptr_)
            {
                size_t read_bytes =
                    deserial_.deserialize(*bv, bv_buf_ptr, temp_block_);
                remap_buf_ptr_ = bv_buf_ptr + read_bytes;
                if (idx_range_set_)
                    bv->keep_range(idx_range_from_, idx_range_to_);
                continue;
            }
            if (idx_range_set_)
            {
                deserial_.set_range(idx_range_from_, idx_range_to_);
                deserial_.deserialize(*bv, bv_buf_ptr);
                bv->keep_range(idx_range_from_, idx_range_to_);
            }
            else
            {
                //size_t read_bytes =
                deserial_.deserialize(*bv, bv_buf_ptr, temp_block_);
            }
        }

    } // for i
    deserial_.unset_range();

}

// -------------------------------------------------------------------------

template<typename SV>
int sparse_vector_deserializer<SV>::load_null_plain(SV& sv,
                                                    int plains,
                                                    const unsigned char* buf,
                                                    const bvector_type* mask_bv)
{
    BM_ASSERT(plains > 0);
    if (!sv.is_nullable())
        return plains;
    int i = plains - 1;
    size_t offset = off_vect_[unsigned(i)];
    if (offset)
    {
        // TODO: improve serialization format to avoid non-range decode of
        // the NULL vector just to get to the offset of remap table

        const unsigned char* bv_buf_ptr = buf + offset; // seek to position
        bvector_type*  bv = sv.get_plain(unsigned(i));
        bv_ref_.add(bv, unsigned(i));
        if (bm::conditional<SV::is_rsc_support::value>::test())
        {
            // load the whole not-NULL vector regardless of range
            // TODO: load [0, idx_range_to_]
            size_t read_bytes = deserial_.deserialize(*bv, bv_buf_ptr, temp_block_);
            remap_buf_ptr_ = bv_buf_ptr + read_bytes;
        }
        else // non-compressed SV
        {
            // NULL plain in string vector with substitute re-map
            //
            if (bm::conditional<SV::is_remap_support::value>::test())
            {
                BM_ASSERT(!remap_buf_ptr_);
                size_t read_bytes = deserial_.deserialize(*bv, bv_buf_ptr, temp_block_);
                remap_buf_ptr_ = bv_buf_ptr + read_bytes;
                if (idx_range_set_)
                    bv->keep_range(idx_range_from_, idx_range_to_);
            }
            else
            if (idx_range_set_)
            {
                deserial_.set_range(idx_range_from_, idx_range_to_);
                deserial_.deserialize(*bv, bv_buf_ptr, temp_block_);
                bv->keep_range(idx_range_from_, idx_range_to_);
                deserial_.unset_range();
            }
            else
            {
                deserial_.deserialize(*bv, bv_buf_ptr, temp_block_);
            }
            if (mask_bv)
                bv->bit_and(*mask_bv, bvector_type::opt_compress);
        }
    }
    return plains-1;
}

// -------------------------------------------------------------------------

template<typename SV>
void sparse_vector_deserializer<SV>::load_plains_off_table(
                                            bm::decoder& dec, unsigned plains)
{
    off_vect_.resize(plains);
    for (unsigned i = 0; i < plains; ++i)
    {
        size_t offset = (size_t) dec.get_64();
        off_vect_[i] = offset;
    } // for i
}

// -------------------------------------------------------------------------

template<typename SV>
void sparse_vector_deserializer<SV>::load_remap(SV& sv,
                                         const unsigned char* remap_buf_ptr)
{
    if (!remap_buf_ptr)
        return;

    bm::decoder dec_m(remap_buf_ptr);
    unsigned char rh = dec_m.get_8();
    switch (rh)
    {
    case 'N':
        break;
    case 'R':
        {
            size_t remap_size = (size_t) dec_m.get_64();
            unsigned char* remap_buf = sv.init_remap_buffer();
            BM_ASSERT(remap_buf);
            size_t target_remap_size = sv.remap_size();
            if (!remap_size || !remap_buf || remap_size != target_remap_size)
            {
                #ifndef BM_NO_STL
                    throw std::logic_error("Invalid serialization format (remap size)");
                #else
                    BM_THROW(BM_ERR_SERIALFORMAT);
                #endif
            }
            dec_m.memcpy(remap_buf, remap_size);
            unsigned char end_tok = dec_m.get_8();
            if (end_tok != 'E')
            {
                #ifndef BM_NO_STL
                    throw std::logic_error("Invalid serialization format");
                #else
                    BM_THROW(BM_ERR_SERIALFORMAT);
                #endif
            }
            sv.set_remap();
        }
        break;
    default:
    #ifndef BM_NO_STL
        throw std::logic_error("Invalid serialization format (remap error)");
    #else
        BM_THROW(BM_ERR_SERIALFORMAT);
    #endif
    } // switch
}

// -------------------------------------------------------------------------

template<typename SV>
void sparse_vector_deserializer<SV>::raise_invalid_header()
{
#ifndef BM_NO_STL
    throw std::logic_error("Invalid serialization signature header");
#else
    BM_THROW(BM_ERR_SERIALFORMAT);
#endif
}

// -------------------------------------------------------------------------

template<typename SV>
void sparse_vector_deserializer<SV>::raise_invalid_64bit()
{
#ifndef BM_NO_STL
    throw std::logic_error("Invalid serialization target (64-bit BLOB)");
#else
    BM_THROW(BM_ERR_SERIALFORMAT);
#endif
}

// -------------------------------------------------------------------------

template<typename SV>
void sparse_vector_deserializer<SV>::raise_invalid_bitdepth()
{
#ifndef BM_NO_STL
    throw std::logic_error("Invalid serialization target (bit depth)");
#else
    BM_THROW(BM_ERR_SERIALFORMAT);
#endif
}

// -------------------------------------------------------------------------

} // namespace bm

#include "bmundef.h"

#endif
