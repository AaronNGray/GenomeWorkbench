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

#include <ncbi_pch.hpp>

#include <bitset>
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <sstream>
#include <cassert>

//#define BMSSE2OPT
//#define BMSSE42OPT
//#define BMAVX2OPT

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4996)
#endif

#include <vector>
#include <random>
#include <memory>

#include <util/bitset/ncbi_bitset_alloc.hpp>
#include <util/bitset/ncbi_bitset_util.hpp>
#include <util/bitset/bmintervals.h>
#include <util/bitset/bmaggregator.h>
#include <util/bitset/bmsparsevec.h>
#include <util/bitset/bmsparsevec_algo.h>
#include <util/bitset/bmsparsevec_serial.h>
#include <util/bitset/bmstrsparsevec.h>
#include <util/bitset/bmsparsevec_compr.h>
#include <util/bitset/bmrandom.h>

//#include <util/bitset/bmdbg.h>

#include <math.h>

#include <common/test_assert.h>

using namespace std;

const unsigned int BSIZE = 150000000;
const unsigned int REPEATS = 300;

typedef  bitset<BSIZE>  test_bitset;

unsigned platform_test = 1;

std::random_device rand_dev;
std::mt19937 gen(rand_dev()); // mersenne_twister_engine 
std::uniform_int_distribution<> rand_dis(0, BSIZE); // generate uniform numebrs for [1, vector_max]


class TimeTaker
{
public:

    TimeTaker(const char* test_name, unsigned repeats) 
        : test_name_(test_name), repeats_(repeats) 
    {
        start_ = clock();
    }

    ~TimeTaker()
    {
        finish_ = clock();
        clock_t elapsed_clocks = finish_ - start_;
        double duration = (double)(finish_ - start_) / CLOCKS_PER_SEC;

        cout << test_name_ << " ; ";
        if (platform_test) 
        {
            cout << duration << endl;
            return;
        }

        cout <<  elapsed_clocks << ";" << duration << ";";
        if (repeats_)
        {
            double ops_per_sec = (double)repeats_ / duration;
            cout << ops_per_sec;
        }
        cout << endl;
    }

private:
    const char*  test_name_;
    clock_t      start_;
    clock_t      finish_;
    unsigned     repeats_;
};

typedef bm::bvector<> bvect;


// generate pseudo-random bit-vector, mix of compressed/non-compressed blocks
//
static
void generate_bvector(bvect& bv, unsigned vector_max = 40000000, bool optimize = true)
{
    unsigned i, j;
    for (i = 0; i < vector_max;)
    {
        // generate bit-blocks
        {
            bvect::bulk_insert_iterator iit(bv);
            for (j = 0; j < 65535 * 10; i += 10, j++)
            {
                iit = i;
            }
        }
        if (i > vector_max)
            break;
        // generate GAP (compressed) blocks
        for (j = 0; j < 65535; i += 120, j++)
        {
            unsigned len = rand() % 64;
            bv.set_range(i, i + len);
            i += len;
            if (i > vector_max)
                break;
        }
    }
    if (optimize)
        bv.optimize();
}


static
void SimpleFillSets(test_bitset& bset, 
                       bvect& bv,
                       unsigned min, 
                       unsigned max,
                       unsigned fill_factor,
                       bool set_flag=true)
{
    for (unsigned i = min; i < max; i+=fill_factor)
    {
        bset[i] = set_flag;
        bv[i] = set_flag;
    } // for i
}


//
// Interval filling.
// 111........111111........111111..........11111111.......1111111...
//
static
void FillSetsIntervals(test_bitset* bset,
                       bvect& bv,
                       unsigned min = 0, 
                       unsigned max = BSIZE,
                       unsigned fill_factor = 10,
                       bool set_flag=true)
{
    while(fill_factor==0)
    {
        fill_factor=rand()%10;
    }

    unsigned i, j;
    unsigned factor = 10 * fill_factor;
    for (i = min; i < max; ++i)
    {
        unsigned len, end; 

        do
        {
            len = unsigned(rand()) % factor;
            end = i+len;
            
        } while (end >= max);
        for (j = i; j < end; ++j)
        {
            if (set_flag)
            {
                if (bset)
                    bset->set(j, true);
                bv[j]= true;
            }
            else
            {
                if (bset)
                    bset->set(j, false);
                bv[j] = false;
            }
                           
        } // j
        i = end;
        len = rand() % 10;

        i+=len;
        {
            for(unsigned k=0; k < 1000 && i < max; k+=3,i+=3)
            {
                if (set_flag)
                {
                    if (bset)
                        bset->set(i, true);
                    bv[i] = true;
                }
                else
                {
                    if (bset)
                        bset->set(j, false);
                    bv[j] = false;
                }
            }
        }

    } // for i

}

static
void generate_sparse_bvector(bvect& bv,
                             unsigned min = 0,
                             unsigned max = BSIZE,
                             unsigned fill_factor = 65536)
{
    bvect::bulk_insert_iterator iit(bv);
    unsigned ff = fill_factor / 10;
    for (unsigned i = min; i < max; i+= ff)
    {
        //bv.set(i);
        iit = i;
        ff += ff / 2;
        if (ff > fill_factor)
            ff = fill_factor / 10;
    }
    iit.flush();
}


static
void GenerateTestCollection(std::vector<bvect>* target,
                            unsigned count = 30,
                            unsigned vector_max = 40000000,
                            bool optimize = true)
{
    assert(target);
    bvect bv_common; // sub-vector common for all collection
    generate_sparse_bvector(bv_common, vector_max/10, vector_max, 250000);
    
    unsigned cnt1 = (count / 2);
    
    unsigned i = 0;
    
    for (i = 0; i < cnt1; ++i)
    {
        std::unique_ptr<bvect> bv (new bvect);
        generate_bvector(*bv, vector_max, optimize);
        *bv |= bv_common;
        if (optimize)
            bv->optimize();
        target->push_back(std::move(*bv));
    } // for
    
    unsigned fill_factor = 10;
    for (; i < count; ++i)
    {
        std::unique_ptr<bvect> bv (new bvect);
        
        FillSetsIntervals(0, *bv, vector_max/ 10, vector_max, fill_factor);
        *bv |= bv_common;

        target->push_back(std::move(*bv));
    } // for
}

static
void MemCpyTest()
{
    unsigned* m1 = new unsigned[BSIZE/32];
    unsigned* m2 = new unsigned[BSIZE/32];
    
    unsigned int i,j;

    if (!platform_test)
    {
    TimeTaker tt("Memory ADD transfer test", REPEATS * 4);
    for (i = 0; i < REPEATS*4; ++i)
    {
        for (j = 0; j < BSIZE/32; j+=4)
        {
            m1[j+0] += m2[j+0];
            m1[j+1] += m2[j+1];
            m1[j+2] += m2[j+2];
            m1[j+3] += m2[j+3];
        }
    }
    }
    
    if (!platform_test)
    {
    TimeTaker tt("memcpy transfer test", REPEATS * 4);
    for (i = 0; i < REPEATS*4; ++i)
    {
        memcpy(m1, m2, BSIZE/32 * sizeof(unsigned));
    }
    }
    
    delete [] m1;
    delete [] m2;
}

static
void BitCountTest()
{
    {
    bvect*  bv = new bvect();
    test_bitset*  bset = new test_bitset();
    unsigned value = 0;

    FillSetsIntervals(bset, *bv, 0, BSIZE, 10);

    //if (!platform_test)
    {
    TimeTaker tt("BitCount. Random bitvector", REPEATS*20);
    for (unsigned i = 0; i < REPEATS*20; ++i)
    {    
        value+=bv->count();
    }
    }

    volatile unsigned* p = &value;
    unsigned c1;
    c1 = value = 0;

    if (!platform_test)
    {
    TimeTaker tt("BitCount. Random bitvector (STL)", REPEATS*2);
    for (unsigned i = 0; i < REPEATS*2; ++i)
    {    
        value += (unsigned)bset->count();
    }
    }

    c1 = *p;
    c1 = value = 0;
    stringstream s;
    s << value << c1; // to fool the optimization

    delete bset;
    delete bv;

    }
}

static
void BitForEachTest()
{
    // setup the test data
    //
    size_t value = 0;
    unsigned* test_arr = new unsigned[65536];
    for (unsigned j = 0; j < 65536; ++j)
    {
        test_arr[j] = j * j;
    }

    if (platform_test)
    {
    unsigned bit_list[32];
    TimeTaker tt("BitList algorithm. Conventional (AND based check)", REPEATS*10);
    

    for (unsigned i = 0; i < REPEATS*10; ++i)
    {    
        for (unsigned j = 0; j < 65536; ++j)
        {
            bm::bit_list(i*test_arr[j], bit_list);
        }
    }
    }

    // disabled (very slow)
    #if 0
    if (platform_test)
    {
        unsigned bit_list[32];
        TimeTaker tt("BitList4 algorithm(sub-octet+switch)", REPEATS*20);

        for (unsigned i = 0; i < REPEATS*100; ++i)
        {
            for (unsigned j = 0; j < 65536; ++j)
            {
                bm::bit_list_4(i*test_arr[j], bit_list);
            }
        }
    }
    #endif

    {
        unsigned bit_list[32];
        TimeTaker tt("BitScan on bitcount algorithm", REPEATS * 20);

        for (unsigned i = 0; i < REPEATS * 100; ++i)
        {
            for (unsigned j = 0; j < 65536; ++j)
            {
                bm::bitscan_popcnt(i*test_arr[j], bit_list);
            }
        }
    }


    {
        unsigned bit_list[64];
        TimeTaker tt("BitScan on bitcount (block)", REPEATS * 20);
        for (unsigned i = 0; i < REPEATS * 20; ++i)
        {
            for (unsigned j = 0; j < 65536; ++j)
            {
                unsigned cnt = bm::bitscan_popcnt(test_arr[j], bit_list);
                for (unsigned k =  0; j < cnt; j++)
                {
                    value += bit_list[k];
                }
            }
        }
    }

    {
        unsigned char bit_list[64];
        TimeTaker tt("BitScan on bitcount64 (block)", REPEATS * 20);
        for (unsigned i = 0; i < REPEATS * 20; ++i)
        {
            for (unsigned j = 0; j < 65536/2; j+=2)
            {
                unsigned cnt = bm::bitscan_wave(test_arr + j, bit_list);

                for (unsigned k =  0; j < cnt; j++)
                {
                    value += bit_list[k];
                }
            }
        }
    }


    char buf[256];
    sprintf(buf, "%i", (int)value); // to fool some smart compilers like ICC


    delete [] test_arr;
}

static
void WordSelectTest()
{
    const unsigned test_size = 50000000;

    std::vector<bm::id64_t> vect_v(test_size);
    std::vector<unsigned> vect_cnt(test_size);
    std::vector<unsigned> vect_r1(test_size);
    std::vector<unsigned> vect_r2(test_size);

    for (unsigned i = 0; i < test_size; ++i)
    {
        unsigned w;
        bm::id64_t w64;
        do
        {
            w = unsigned(rand());
            if (i % 3 == 0)
                w64 = w << rand()%2048;
            else
            if (i % 4 == 0)
                w64 = bm::id64_t(w) | (bm::id64_t(w) << 32);
            else
                w64 = bm::id64_t(w) | (0xFFAull << 33ull);
        } while (!w64);
        
        if (i % 3 == 0)
            w64 = w << rand()%32;
        else
        if (i % 4 == 0)
            w64 = w | (bm::id64_t(w) << 32);
        else
            w64 = w | (1ull << 60);
        
        unsigned bc = bm::word_bitcount64(w64);
        
        vect_v[i] = w64;
        vect_cnt[i] = bc;
    }
    
    {
        TimeTaker tt("select64 linear", 1);
        for (unsigned i = 0; i < vect_v.size(); ++i)
        {
            bm::id64_t w64 = vect_v[i];
            unsigned bc = vect_cnt[i];
            if (bc)
            {
                for (unsigned j = 1; j <= bc; ++j)
                {
                    unsigned idx = bm::word_select64_linear(w64, j);
                    vect_r1[i] = idx;
                }
            }
            else
            {
                vect_r1[i] = 0;
            }
        }
    }

    {
        TimeTaker tt("select64 bitscan", 1);
        for (unsigned i = 0; i < vect_v.size(); ++i)
        {
            bm::id64_t w64 = vect_v[i];
            unsigned bc = vect_cnt[i];
            if (bc)
            {
                for (unsigned j = 1; j <= bc; ++j)
                {
                    unsigned idx = bm::word_select64_bitscan(w64, j);
                    vect_r2[i] = idx;
                }
            }
            else
            {
                vect_r2[i] = 0;
            }
        }
    }

#ifdef BMBMI1OPT
    std::vector<unsigned> vect_r3(test_size);
    std::vector<unsigned> vect_r4(test_size);

    {
        TimeTaker tt("select64 BMI1 lead-zero", 1);
        for (unsigned i = 0; i < vect_v.size(); ++i)
        {
            bm::id64_t w64 = vect_v[i];
            unsigned bc = vect_cnt[i];
            if (bc)
            {
                for (unsigned j = 1; j <= bc; ++j)
                {
                    unsigned idx = bm::bmi1_select64_lz(w64, j);
                    vect_r3[i] = idx;
                }
            }
            else
            {
                vect_r3[i] = 0;
            }
        }
    }
    
    {
        TimeTaker tt("select64 BMI1 bitscan", 1);
        for (unsigned i = 0; i < vect_v.size(); ++i)
        {
            bm::id64_t w64 = vect_v[i];
            unsigned bc = vect_cnt[i];
            if (bc)
            {
                for (unsigned j = 1; j <= bc; ++j)
                {
                    unsigned idx = bm::bmi1_select64_tz(w64, j);
                    vect_r4[i] = idx;
                }
            }
            else
            {
                vect_r4[i] = 0;
            }
        }
    }
#endif

#ifdef BMBMI2OPT
    std::vector<unsigned> vect_r5(test_size);

    {
        TimeTaker tt("select64 BMI2 pdep", 1);
        for (unsigned i = 0; i < vect_v.size(); ++i)
        {
            bm::id64_t w64 = vect_v[i];
            unsigned bc = vect_cnt[i];
            if (bc)
            {
                for (unsigned j = 1; j <= bc; ++j)
                {
                    unsigned idx = bm::bmi2_select64_pdep(w64, j);
                    vect_r5[i] = idx;
                }
            }
            else
            {
                vect_r5[i] = 0;
            }
        }
    }
    
#endif


    // validation
    //
    for (unsigned i = 0; i < vect_v.size(); ++i)
    {
        auto r1 = vect_r1[i];
        auto r2 = vect_r2[i];
        
        if (r1 != r2)
        {
            std::cerr << "WordSelect64 error(1) at: " << i << std::endl;
            exit(1);
        }
#ifdef BMBMI1OPT
        auto r3 = vect_r3[i];
        auto r4 = vect_r4[i];

        if (r1 != r3)
        {
            std::cerr << "WordSelect64 BMI1 error(3) at: " << i << std::endl;
            exit(1);
        }
        if (r1 != r4)
        {
            std::cerr << "WordSelect64 BMI1 error(4) at: " << i << std::endl;
            exit(1);
        }
#endif

#ifdef BMBMI2OPT
        auto r5 = vect_r5[i];

        if (r1 != r5)
        {
            std::cerr << "WordSelect64 BMI2 error(5) at: " << i << std::endl;
            exit(1);
        }
#endif

    }

}

static
void BitCountSparseTest()
{
    bvect*  bv = new bvect();
    test_bitset*  bset = new test_bitset();
    size_t value = 0, c1;
    volatile size_t* p = &value;

    SimpleFillSets(*bset, *bv, 0, BSIZE, 130);
    
    {
        TimeTaker tt("BitCount: Sparse bitset ", REPEATS*10);
        for (unsigned i = 0; i < REPEATS*10; ++i)
        {    
            value += bv->count();
        }
    }

    if (!platform_test)
    {
        TimeTaker tt("BitCount: Sparse bitset (STL)", REPEATS*10);
        for (unsigned int i = 0; i < REPEATS*10; ++i)
        {    
            value += bset->count();
        }
    }

    c1 = *p;
    value = c1 = 0;
    
    BM_DECLARE_TEMP_BLOCK(tb)
    bv->optimize(tb);

    {
        TimeTaker tt("BitCount: GAP Sparse bitset", REPEATS*100);
        for (unsigned i = 0; i < REPEATS*100; ++i)
        {    
            value += bv->count();
        }
    }

    std::unique_ptr<bvect::rs_index_type> bc_arr(new bvect::rs_index_type());
    bv->build_rs_index(bc_arr.get());

    {
        unsigned right = 65535;
        TimeTaker tt("count_to: GAP Sparse bitset", REPEATS * 100);
        for (unsigned i = 0; i < REPEATS * 100000; ++i)
        {
            right = 65525 + (i * 10);
            if (right > BSIZE)
                right = 0;
            value += bv->count_to(right, *bc_arr);
        }
    }
    delete bv;
    delete bset;
}

static
void BitTestSparseTest()
{
    unique_ptr<bvect>  bv0(new bvect());
    unique_ptr<bvect>  bv1(new bvect());
    unique_ptr<bvect>  bv2(new bvect());
    unique_ptr<test_bitset>  bset0(new test_bitset());
    unique_ptr<test_bitset>  bset1(new test_bitset());
    unique_ptr<test_bitset>  bset2(new test_bitset());

    const unsigned repeats = REPEATS * 300000;

    size_t value = 0, c1;
    volatile size_t* p = &value;

    SimpleFillSets(*bset0, *bv0, 0, BSIZE, 9530);
    SimpleFillSets(*bset1, *bv1, 0, BSIZE, 1000);
    SimpleFillSets(*bset2, *bv2, 0, BSIZE, 120);


    {
        TimeTaker tt("BitTest: bitset ", repeats);
        for (unsigned i = 0; i < repeats; ++i)
        {
            unsigned idx = unsigned(rand_dis(gen));
            value += bv0->test(idx);
            value += bv1->test(idx);
            value += bv2->test(idx);
        }
    }


    c1 = *p;
    value = c1 = 0;

    BM_DECLARE_TEMP_BLOCK(tb)
    bv0->optimize(tb);
    bv1->optimize(tb);
    bv2->optimize(tb);

    {
        TimeTaker tt("BitTest: Sparse bitset (GAP) ", repeats);
        for (unsigned i = 0; i < repeats; ++i)
        {
            unsigned idx = unsigned(rand_dis(gen));
            value += bv0->test(idx);
            value += bv1->test(idx);
            value += bv2->test(idx);
        }
    }

}

static
void EnumeratorGoToTest()
{
    unique_ptr<bvect>  bv0(new bvect());
    unique_ptr<bvect>  bv1(new bvect());
    unique_ptr<bvect>  bv2(new bvect());
    unique_ptr<test_bitset>  bset0(new test_bitset());
    unique_ptr<test_bitset>  bset1(new test_bitset());
    unique_ptr<test_bitset>  bset2(new test_bitset());

    const unsigned repeats = REPEATS * 300000;

    size_t value = 0, c1;
    volatile size_t* p = &value;

    SimpleFillSets(*bset0, *bv0, 0, BSIZE, 512);
    SimpleFillSets(*bset1, *bv1, 0, BSIZE, 256);
    SimpleFillSets(*bset2, *bv2, 0, BSIZE, 120);


    {
        TimeTaker tt("Enumerator at bit pos:  ", repeats);
        for (unsigned i = 0; i < repeats; ++i)
        {
            unsigned idx = unsigned(rand_dis(gen));
            bvect::enumerator en0 = bv0->get_enumerator(idx);
            bvect::enumerator en1 = bv1->get_enumerator(idx);
            bvect::enumerator en2 = bv2->get_enumerator(idx);

            value += *en0;
            value += *en1;
            value += *en2;
        }
    }

    c1 = *p;
    value = c1 = 0;

    BM_DECLARE_TEMP_BLOCK(tb)
    bv0->optimize(tb);
    bv1->optimize(tb);
    bv2->optimize(tb);

    {
        TimeTaker tt("Enumerator at gap pos: ", repeats);
        for (unsigned i = 0; i < repeats; ++i)
        {
            unsigned idx = unsigned(rand_dis(gen));
            bvect::enumerator en0 = bv0->get_enumerator(idx);
            bvect::enumerator en1 = bv1->get_enumerator(idx);
            bvect::enumerator en2 = bv2->get_enumerator(idx);

            value += *en0;
            value += *en1;
            value += *en2;
        }
    }

}


static
void BitCompareTest()
{
    {
    bvect*  bv1 = new bvect();
    bvect*  bv2 = new bvect();
    test_bitset*  bset = new test_bitset();
    int value = 0;

    SimpleFillSets(*bset, *bv1, 0, BSIZE, 10);
    SimpleFillSets(*bset, *bv2, 0, BSIZE, 10);

    {
    TimeTaker tt("BitCompare: Random bitvector", REPEATS*10);
    for (unsigned int i = 0; i < REPEATS*10; ++i)
    {    
        value+=bv1->compare(*bv2);
    }
    }

    delete bset;
    delete bv1;
    delete bv2;

    }

    if (platform_test) return;

    unsigned cnt = REPEATS * 100000;
    unsigned* arr1 = new unsigned[cnt];
    unsigned* arr2 = new unsigned[cnt];

    unsigned i;
    for (i = 0; i < cnt; ++i)
    {
        if ((rand() % 10) == 0)
        {
            arr1[i] = 0;
        }
        else 
        {
            arr1[i] = unsigned(rand());
            arr2[i] = unsigned(rand());
        }
    }

    {
    TimeTaker tt("wordcmp complex: Random words comparison", cnt);

    for (i = 0; i < cnt; ++i)
    {    
        int res2 = bm::wordcmp(arr1[i], arr2[i]);
        int res = bm::wordcmp0(arr1[i], arr2[i]);

        if (res != res2)
        {
            cerr << "Incorrect result ! " << arr1[i] 
                 << "<=>" << arr2[i] << " res=" << res <<
                 endl;
            exit(1);
        }
    }
    }

    int c = 0;
    volatile void* p = &c;

    {
    TimeTaker tt("wordcmp0. Random words comparison", cnt);
    for (i = 0; i < cnt; ++i)
    {    
        c += bm::wordcmp0(arr1[i], arr2[i]);
    }
    }


    {
    TimeTaker tt("wordcmp. Random words comparison", cnt);
    for (i = 0; i < cnt; ++i)
    {    
        c += bm::wordcmp(arr1[i], arr2[i]);
    }
    }

    c = 0;;

    delete [] arr1;
    delete [] arr2;

    char buf[256];
    sprintf(buf, "%p", p);
}

extern "C" {
    static
    int bit_visitor_func(void* handle_ptr, bm::id_t bit_idx)
    {
        std::vector<bm::id_t>* vp = (std::vector<bm::id_t>*)handle_ptr;
        vp->push_back(bit_idx);
        return 0;
    }
} // extern C


static
void FindTest()
{
    bvect                 bv1, bv2, bv3, bv4;
    bvect                 bv_empty;
    test_bitset*  bset = new test_bitset();
    
    bv_empty[1] = true;
    bv_empty[1] = false;
    bv_empty[100000000] = true;
    bv_empty[100000000] = false;
    
    SimpleFillSets(*bset, bv1, 0, BSIZE/2, 3);
    SimpleFillSets(*bset, bv1, 0, BSIZE/2, 4);
    SimpleFillSets(*bset, bv1, 0, BSIZE/2, 5);
    
    FillSetsIntervals(bset, bv2, 0, BSIZE/2, 8);
    FillSetsIntervals(bset, bv3, 0, BSIZE/2, 12);
    FillSetsIntervals(bset, bv4, 0, BSIZE/2, 120);
    
    unsigned i;
    unsigned pos_sum = 0;
    {
        TimeTaker tt("bvector<>::find_reverse()", REPEATS*100);
        for (i = 0; i < REPEATS*100; ++i)
        {
            bm::id_t pos;
            bool found;
            found = bv1.find_reverse(pos);
            if (found)
                pos_sum += pos;
            found = bv2.find_reverse(pos);
            if (found)
                pos_sum += pos;
            found = bv3.find_reverse(pos);
            if (found)
                pos_sum += pos;
            found = bv4.find_reverse(pos);
            if (found)
                pos_sum += pos;
            found = bv_empty.find_reverse(pos);
            if (!found)
                pos_sum += pos;
            else
            {
                cerr << "incorrect find result!" << endl;
                exit(1);
            }
        }
    }
    char cbuf[256];
    sprintf(cbuf, "%i ", pos_sum); // attempt to avoid agressive optmizations

    {
        TimeTaker tt("bvector<>::find()", REPEATS*100);
        for (i = 0; i < REPEATS*100; ++i)
        {
            bm::id_t pos;
            bool found;
            found = bv1.find(0, pos);
            if (found)
                pos_sum += pos;
            found = bv2.find(0, pos);
            if (found)
                pos_sum += pos;
            found = bv3.find(0, pos);
            if (found)
                pos_sum += pos;
            found = bv4.find(0, pos);
            if (found)
                pos_sum += pos;
            found = bv_empty.find(0, pos);
            if (!found)
                pos_sum += pos;
            else
            {
                cerr << "incorrect find result!" << endl;
                exit(1);
            }
        }
    }

    delete bset;
}

static
void EnumeratorTest()
{
    bvect                 bv1, bv2, bv3, bv4;
    std::vector<bm::id_t> v1,  v2,  v3,  v4;
    
    test_bitset*  bset = new test_bitset();

    SimpleFillSets(*bset, bv1, 0, BSIZE/2, 3);
    SimpleFillSets(*bset, bv1, 0, BSIZE/2, 4);
    SimpleFillSets(*bset, bv1, 0, BSIZE/2, 5);
    
    FillSetsIntervals(bset, bv2, 0, BSIZE/2, 8);
    FillSetsIntervals(bset, bv3, 0, BSIZE/2, 12);
    FillSetsIntervals(bset, bv4, 0, BSIZE/2, 120);
    
    v1.reserve(bv1.count());
    v2.reserve(bv2.count());
    v3.reserve(bv3.count());
    v4.reserve(bv4.count());

    unsigned i;

    {
        TimeTaker tt("bvector<>::enumerator", REPEATS/10);
        for (i = 0; i < REPEATS/10; ++i)
        {
            {
                bvect::enumerator en = bv1.first();
                for (;en.valid();++en)
                {
                    v1.push_back(*en);
                }
            }
            {
                bvect::enumerator en = bv2.first();
                for (;en.valid();++en)
                {
                    v2.push_back(*en);
                }
            }
            {
                bvect::enumerator en = bv3.first();
                for (;en.valid();++en)
                {
                    v3.push_back(*en);
                }
            }
            {
                bvect::enumerator en = bv4.first();
                for (;en.valid();++en)
                {
                    v4.push_back(*en);
                }
            }
            v1.resize(0);
            v2.resize(0);
            v3.resize(0);
            v4.resize(0);

        } // for REPEATS
    }
    


    // -----------------------------------------------
    {
        TimeTaker tt("bvector<>::get_next()", REPEATS/10);
        for (i = 0; i < REPEATS/10; ++i)
        {
            if (bv1.any())
            {
                unsigned v = bv1.get_first();
                do
                {
                    v = bv1.get_next(v);
                    v1.push_back(v);
                } while(v);
            }
            if (bv2.any())
            {
                unsigned v = bv2.get_first();
                do
                {
                    v = bv2.get_next(v);
                    v2.push_back(v);
                } while(v);
            }
            if (bv3.any())
            {
                unsigned v = bv3.get_first();
                do
                {
                    v = bv3.get_next(v);
                    v3.push_back(v);
                } while(v);
            }
            if (bv4.any())
            {
                unsigned v = bv4.get_first();
                do
                {
                    v = bv4.get_next(v);
                    v4.push_back(v);
                } while(v);
            }
            v1.resize(0);
            v2.resize(0);
            v3.resize(0);
            v4.resize(0);

        } // for REPEATS
    }

    // -----------------------------------------------
    
    
    {
        TimeTaker tt("bm::visit_each_bit()", REPEATS/10);
        for (i = 0; i < REPEATS/10; ++i)
        {
            bm::visit_each_bit(bv1, (void*)&v1, bit_visitor_func);
            bm::visit_each_bit(bv2, (void*)&v2, bit_visitor_func);
            bm::visit_each_bit(bv3, (void*)&v3, bit_visitor_func);
            bm::visit_each_bit(bv4, (void*)&v4, bit_visitor_func);

            v1.resize(0);
            v2.resize(0);
            v3.resize(0);
            v4.resize(0);
        } // for REPEATS

    }
    
    // -----------------------------------------------


    delete bset;


}

static
void EnumeratorTestGAP()
{
    bvect*  bv = new bvect();
    test_bitset*  bset = new test_bitset();
    unsigned i;

    SimpleFillSets(*bset, *bv, 0, BSIZE, 2500);
    bv->count();

    for (unsigned k = 0; k < 2; ++k)
    {

    {
    unsigned v = 0;
    TimeTaker tt("Sparse bvector (enumerator)", REPEATS*10*(k+1));
    for (i = 0; i < REPEATS*10*(k+1); ++i)
    {    
        bvect::enumerator en = bv->first();
        bvect::enumerator bend = bv->end();

        while (en < bend)
        {
            v += *en;
            ++en;
        }
    }

    stringstream s;
    s << v << endl; // attempt to fool optimization

    }

    // -----------------------------------------------

    unsigned cnt = 0;
    {
    TimeTaker tt("Sparse bvector (get_next())", REPEATS*10*(k+1));

    for (i = 0; i < REPEATS*10*(k+1); ++i)
    {
        if (bv->any())
        {
            unsigned v = bv->get_first();
            do
            {
                v = bv->get_next(v);
                cnt += v;
            } while (v);
        }
    }
    }
    char buf[256];
    sprintf(buf, "%i", cnt); // to fool some smart compilers like ICC

    {

    BM_DECLARE_TEMP_BLOCK(tb)
    bv->optimize(tb);
    }

    if (!platform_test) 
    {
        cout << "Testing optimized vectors." << endl;
    }
    }

    delete bv;
    delete bset;
    // -----------------------------------------------

}

static
void SerializationTest()
{
    bvect bv_sparse;
    // stack declaration of temp block instead of re-allocation makes things faster
    BM_DECLARE_TEMP_BLOCK(tb)


    // prepare a test bitset with a small number of bits set somewhere
    // far from the beginning
    for (unsigned i = 0; i < 5; ++i)
    {
        bv_sparse[BSIZE/2 + i * 3] = true;
    }
    bv_sparse[100] = true;
    bv_sparse[70000] = true;
    bv_sparse[200000] = true;

    bv_sparse.optimize(tb);

    unsigned cnt = bv_sparse.count();
    bvect::statistics st;
    bv_sparse.calc_stat(&st);
    unsigned char*  buf = new unsigned char[st.max_serialize_mem];

    size_t len, id_size;
    len = id_size = 0;
    {
    TimeTaker tt("Small bvector serialization", REPEATS*70000);
    for (unsigned i = 0; i < REPEATS*70000; ++i)
    {
        len += bm::serialize(bv_sparse, buf, tb, bm::BM_NO_BYTE_ORDER|bm::BM_NO_GAP_LENGTH);
        id_size += cnt * (unsigned)sizeof(unsigned);
    }
    }
    
    delete [] buf; buf = 0;
        
    bvect*  bv = new bvect();
    test_bitset*  bset = new test_bitset();
    unsigned value = 0;

    SimpleFillSets(*bset, *bv, 0, BSIZE, 4);
    
    cnt = bv->count();
    bv->calc_stat(&st);
    buf = new unsigned char[st.max_serialize_mem];
    
    {
    TimeTaker tt("Large bvector serialization", REPEATS*4);
    for (unsigned i = 0; i < REPEATS*4; ++i)
    {
        len += bm::serialize(*bv, buf, tb, bm::BM_NO_BYTE_ORDER|bm::BM_NO_GAP_LENGTH);
        id_size += cnt * (unsigned)sizeof(unsigned);
    }
    }


    char cbuf[256] = {0, };
    sprintf(cbuf, "%u", value);
    /*
    cout << cbuf << " " << id_size << " " << len << " " << value << endl;
    */
    
    
    delete bv;
    delete bset;
    delete [] buf;
}

static
void InvertTest()
{
    bvect*  bv = new bvect();
    test_bitset*  bset = new test_bitset();
    unsigned i;
    //unsigned value = 0;

    SimpleFillSets(*bset, *bv, 0, BSIZE, 2500);
    {
    TimeTaker tt("Invert bvector", REPEATS*4);
    for (i = 0; i < REPEATS*4; ++i)
    {
        bv->flip();    
    }
    }

    if (!platform_test)
    {
    TimeTaker tt("Invert bvector (STL)", REPEATS*4);
    for (i = 0; i < REPEATS*4; ++i)
    {
        bset->flip();    
    }
    }

    delete bv;
    delete bset;
}

static
void OrTest()
{
    bvect bv1, bv2;
    bvect bvt1, bvt2;
    generate_bvector(bv1, 40000000, false);
    generate_bvector(bv2, 40000000, false);
    
    {
    TimeTaker tt("OR-optimize (2 operand) bvector test", REPEATS*4);
    for (unsigned i = 0; i < REPEATS*4; ++i)
    {
        bvt1 = bv1;
        bvt1 |= bv2;
        bvt1.optimize();
    }
    }
    
    {
    TimeTaker tt("OR-optimize (3 operand) bvector test", REPEATS*4);
    for (unsigned i = 0; i < REPEATS*4; ++i)
    {
        bvt2.bit_or(bv1, bv2, bvect::opt_compress);
    }
    }

    int cmp = bvt1.compare(bvt2);
    if (cmp)
    {
        cerr << "Error: OR mismatch!" << endl;
        exit(1);
    }
}

static
void AndTest()
{
    bvect*  bv1 = new bvect();
    test_bitset*  bset1 = new test_bitset();
    test_bitset*  bset2 = new test_bitset();
    bvect*  bv2 = new bvect();
    unsigned i;
    //unsigned value = 0;

    SimpleFillSets(*bset1, *bv1, 0, BSIZE, 100);
    SimpleFillSets(*bset1, *bv2, 0, BSIZE, 100);
    {
    TimeTaker tt("AND bvector test", REPEATS*4);
    for (i = 0; i < REPEATS*4; ++i)
    {
        *bv1 &= *bv2;
    }
    }

    if (!platform_test)
    {
    TimeTaker tt("AND bvector test(STL)", REPEATS*4);
    for (i = 0; i < REPEATS*4; ++i)
    {
        *bset1 &= *bset2;
    }
    }

    delete bv1;
    delete bv2;

    delete bset1;
    delete bset2;
}

static
void XorTest()
{
    bvect*  bv1 = new bvect();
    test_bitset*  bset1 = new test_bitset();
    test_bitset*  bset2 = new test_bitset();
    bvect*  bv2 = new bvect();
    unsigned i;
    //unsigned value = 0;

    SimpleFillSets(*bset1, *bv1, 0, BSIZE, 100);
    SimpleFillSets(*bset1, *bv2, 0, BSIZE, 100);
    {
        TimeTaker tt("XOR bvector test", REPEATS * 10);
        for (i = 0; i < REPEATS * 4; ++i)
        {
            *bv1 ^= *bv2;
        }
    }

    if (!platform_test)
    {
        TimeTaker tt("XOR bvector test(STL)", REPEATS * 10);
        for (i = 0; i < REPEATS * 4; ++i)
        {
            *bset1 ^= *bset2;
        }
    }

    delete bv1;
    delete bv2;

    delete bset1;
    delete bset2;
}

static
void SubTest()
{
    bvect bv1, bv2;
    bvect bvt0, bvt1, bvt2;
    generate_bvector(bv1, 40000000, false);
    generate_bvector(bv2, 40000000, false);
    
    {
    TimeTaker tt("AND-NOT bvector test", REPEATS*4);
    for (unsigned i = 0; i < REPEATS*4; ++i)
    {
        bvect bv_tmp(bv2);
        bv_tmp.invert();
        
        bvt0 = bv1;
        bvt0 &= bv_tmp;
        bvt0.optimize();
    }

    }
    
    {
    TimeTaker tt("SUB-optimize (2 operand) bvector test", REPEATS*4);
    for (unsigned i = 0; i < REPEATS*4; ++i)
    {
        bvt1 = bv1;
        bvt1 -= bv2;
        bvt1.optimize();
    }
    }
    
    {
    TimeTaker tt("SUB-optimize (3 operand) bvector test", REPEATS*4);
    for (unsigned i = 0; i < REPEATS*4; ++i)
    {
        bvt2.bit_sub(bv1, bv2, bvect::opt_compress);
    }
    }

    int cmp = bvt1.compare(bvt2);
    if (cmp)
    {
        cerr << "Error: (1)SUB mismatch!" << endl;
        exit(1);
    }
    cmp = bvt0.compare(bvt2);
    if (cmp)
    {
        cerr << "Error: (2)SUB mismatch!" << endl;
        exit(1);
    }

}

static
void XorCountTest()
{
    bvect*  bv1 = new bvect();
    bvect*  bv2 = new bvect();
    test_bitset*  bset1 = new test_bitset();
    test_bitset*  bset2 = new test_bitset();
    unsigned i;

    SimpleFillSets(*bset1, *bv1, 0, BSIZE, 400);
    SimpleFillSets(*bset2, *bv2, 0, BSIZE, 500);

    unsigned count1 = 0;
    unsigned count2 = 0;
    unsigned test_count = 0;

    if (!platform_test)
    {
    bvect bv_tmp;
    TimeTaker tt("XOR COUNT bvector test with TEMP vector", REPEATS*10);
    for (i = 0; i < REPEATS*4; ++i)
    {
        bv_tmp.clear(false);
        bv_tmp |= *bv1;
        bv_tmp ^= *bv2;
        count1 += bv_tmp.count();
    }
    }

    if (!platform_test)
    {
    test_bitset*  bset_tmp = new test_bitset();
    TimeTaker tt("XOR COUNT bvector test with TEMP vector (STL)", REPEATS*10);
    for (i = 0; i < REPEATS*4; ++i)
    {
        bset_tmp->reset();
        *bset_tmp |= *bset1;
        *bset_tmp ^= *bset2;
        test_count += (unsigned)bset_tmp->count();
    }
    }


    {
    TimeTaker tt("XOR COUNT bvector test", REPEATS*10);
    for (i = 0; i < REPEATS*4; ++i)
    {
        count2 += bm::count_xor(*bv1, *bv2);
    }
    }

    
    if (!platform_test)    
        if (count1 != count2)
        {
            cout << "Check failed !" << endl;
            cout << count1 << " " << count2 << " " << test_count << endl;
            exit(1);
        }
    count1 = count2 = 0;
    
    // -----------------------------------------
    if (!platform_test)
    {
        cout << "One optimized vector" << endl;
    }
    BM_DECLARE_TEMP_BLOCK(tb)
    bv2->optimize(tb);

    if (!platform_test)
    {
    bvect bv_tmp;
    TimeTaker tt("XOR COUNT bvector test with TEMP vector", REPEATS*10);
    for (i = 0; i < REPEATS*4; ++i)
    {
        bv_tmp.clear(false);
        bv_tmp |= *bv1;
        bv_tmp ^= *bv2;
        count1 += (unsigned)bv_tmp.count();
    }
    }

    {
    TimeTaker tt("XOR COUNT bvector test", REPEATS*10);
    for (i = 0; i < REPEATS*4; ++i)
    {
        count2 += (unsigned)bm::count_xor(*bv1, *bv2);
    }
    }

    if (!platform_test)
        if (count1 != count2)
        {
            cout << "Check failed !" << endl;
            exit(1);
        }
    count1 = count2 = 0;

    // -----------------------------------------
    if (!platform_test)
    {
        cout << "Both vectors optimized" << endl;
    }
    bv1->optimize(tb);
    //bv1->stat();
    if (!platform_test)
    {
    bvect bv_tmp;
    TimeTaker tt("XOR COUNT bvector test with TEMP vector", REPEATS*10);
    for (i = 0; i < REPEATS*4; ++i)
    {
        bv_tmp.clear(false);
        bv_tmp |= *bv1;
        bv_tmp ^= *bv2;
        count1 += (unsigned)bv_tmp.count();
    }
    }

    {
    TimeTaker tt("XOR COUNT bvector(opt) test", REPEATS*10);
    for (i = 0; i < REPEATS*4; ++i)
    {
        count2 += (unsigned)bm::count_xor(*bv1, *bv2);
    }
    }
    if (!platform_test)
        if (count1 != count2)
        {
            cout << "Check failed !" << endl;
            exit(1);
        }
    count1 = count2 = 0;


    delete bv1;
    delete bv2;
    
    delete bset1;
    delete bset2;    
}

static
void AndCountTest()
{
    bvect*  bv1 = new bvect();
    bvect*  bv2 = new bvect();
    test_bitset*  bset1 = new test_bitset();
    test_bitset*  bset2 = new test_bitset();
    unsigned i;

    SimpleFillSets(*bset1, *bv1, 0, BSIZE, 400);
    SimpleFillSets(*bset2, *bv2, 0, BSIZE, 500);

    unsigned count1 = 0;
    unsigned count2 = 0;
    unsigned test_count = 0;

    if (!platform_test)
    {
        bvect bv_tmp;
        TimeTaker tt("AND COUNT bvector test with TEMP vector", REPEATS * 10);
        for (i = 0; i < REPEATS * 4; ++i)
        {
            bv_tmp.clear(false);
            bv_tmp |= *bv1;
            bv_tmp &= *bv2;
            count1 += bv_tmp.count();
        }
    }

    if (!platform_test)
    {
        test_bitset*  bset_tmp = new test_bitset();
        TimeTaker tt("AND COUNT bvector test with TEMP vector (STL)", REPEATS * 10);
        for (i = 0; i < REPEATS * 4; ++i)
        {
            bset_tmp->reset();
            *bset_tmp |= *bset1;
            *bset_tmp &= *bset2;
            test_count += (unsigned)bset_tmp->count();
        }
    }


    {
        TimeTaker tt("AND COUNT bvector test", REPEATS * 10);
        for (i = 0; i < REPEATS * 4; ++i)
        {
            count2 += bm::count_and(*bv1, *bv2);
        }
    }


    if (!platform_test)
        if (count1 != count2)
        {
            cout << "Check failed !" << endl;
            cout << count1 << " " << count2 << " " << test_count << endl;
            exit(1);
        }
    count1 = count2 = 0;

    // -----------------------------------------
    if (!platform_test)
    {
        cout << "One optimized vector" << endl;
    }
    BM_DECLARE_TEMP_BLOCK(tb)
        bv2->optimize(tb);

    if (!platform_test)
    {
        bvect bv_tmp;
        TimeTaker tt("AND COUNT bvector test with TEMP vector", REPEATS * 10);
        for (i = 0; i < REPEATS * 4; ++i)
        {
            bv_tmp.clear(false);
            bv_tmp |= *bv1;
            bv_tmp &= *bv2;
            count1 += (unsigned)bv_tmp.count();
        }
    }

    {
        TimeTaker tt("AND COUNT bvector test", REPEATS * 10);
        for (i = 0; i < REPEATS * 4; ++i)
        {
            count2 += (unsigned)bm::count_and(*bv1, *bv2);
        }
    }

    if (!platform_test)
        if (count1 != count2)
        {
            cout << "Check failed !" << endl;
            exit(1);
        }
    count1 = count2 = 0;

    // -----------------------------------------
    if (!platform_test)
    {
        cout << "Both vectors optimized" << endl;
    }
    bv1->optimize(tb);
    //bv1->stat();
    if (!platform_test)
    {
        bvect bv_tmp;
        TimeTaker tt("AND COUNT bvector test with TEMP vector", REPEATS * 10);
        for (i = 0; i < REPEATS * 4; ++i)
        {
            bv_tmp.clear(false);
            bv_tmp |= *bv1;
            bv_tmp &= *bv2;
            count1 += (unsigned)bv_tmp.count();
        }
    }

    {
        TimeTaker tt("AND COUNT bvector(opt) test", REPEATS * 10);
        for (i = 0; i < REPEATS * 4; ++i)
        {
            count2 += (unsigned)bm::count_and(*bv1, *bv2);
        }
    }
    if (!platform_test)
        if (count1 != count2)
        {
            cout << "Check failed !" << endl;
            exit(1);
        }
    count1 = count2 = 0;


    delete bv1;
    delete bv2;

    delete bset1;
    delete bset2;
}


static
void TI_MetricTest()
{
    bvect*  bv1 = new bvect();
    bvect*  bv2 = new bvect();
    test_bitset*  bset1 = new test_bitset();
    test_bitset*  bset2 = new test_bitset();
    unsigned i;

    SimpleFillSets(*bset1, *bv1, 0, BSIZE, 500);
    SimpleFillSets(*bset2, *bv2, 0, BSIZE, 250);

    unsigned count1 = 0;
    unsigned count2 = 0;
    unsigned countA=0, countB=0, test_countA=0, test_countB=0;
    unsigned test_count = 0;
    double ti1=0, ti2=0;
    {
    TimeTaker tt("Tversky Index bvector test vector", REPEATS);
    for (i = 0; i < REPEATS; ++i)
    {
        count1 = bm::count_and(*bv1, *bv2);
        
        countA = bm::count_sub(*bv1, *bv2);
        countB = bm::count_sub(*bv2, *bv1);
        
        ti1 = double(count1) / double(0.4*countA + 0.5*countB + count1);
    }
    }


    if (!platform_test)
    {
    test_bitset*  bset_tmp = new test_bitset();
    double test_dice = 0;
    TimeTaker tt("Dice bvector test with TEMP vector(STL)", REPEATS);
    for (i = 0; i < REPEATS; ++i)
    {
        bset_tmp->reset();
        *bset_tmp |= *bset1;
        *bset_tmp &= *bset2;
        test_count += (unsigned)bset_tmp->count();
        
        test_countA += (unsigned)bset1->count();
        test_countB += (unsigned)bset2->count();
        
        test_countA += (unsigned)bset1->count();
        test_countB += (unsigned)bset2->count();
        
        test_dice += double(2*test_count) / double(test_countA + test_countB);
    }
    }


    {
    bm::distance_metric_descriptor dmd[3];
    dmd[0].metric = bm::COUNT_AND;
    dmd[1].metric = bm::COUNT_SUB_AB;
    dmd[2].metric = bm::COUNT_SUB_BA;    
    
    TimeTaker tt("Tversky Index bvector test (pipeline)", REPEATS);
    for (i = 0; i < REPEATS; ++i)
    {
        bm::distance_operation(*bv1, *bv2, &dmd[0], (&dmd[0])+3);
                
        ti2 = double(dmd[0].result) / double(0.4*dmd[1].result + 0.5*dmd[2].result + dmd[0].result);
        
        dmd[0].result = dmd[1].result = dmd[2].result = 0;
    }
    }

    
    if (fabs(ti2 - ti1) > 0.1)
    {
        cout << "Check failed ! error=" << fabs(ti2 - ti1) << endl;
        cout << ti1 << " " << ti2 << endl;
        exit(1);
    }
    count1 = count2 = 0;

    // -----------------------------------------
    if (!platform_test)
    {
        cout << "One optimized vector" << endl;
    }
    BM_DECLARE_TEMP_BLOCK(tb)
    bv2->optimize(tb);
    bv1->count(); // trying to fool the CPU cache

    
    {
    TimeTaker tt("Dice metric bvector test", REPEATS);
    for (i = 0; i < REPEATS; ++i)
    {
        count1 = bm::count_and(*bv1, *bv2);
        
        countA = bm::count_sub(*bv1, *bv2);
        countB = bm::count_sub(*bv2, *bv1);
        
        ti1 = double(count1) / double(0.4*countA + 0.5*countB + count1);
    }
    }



    {
    bm::distance_metric_descriptor dmd[3];
    dmd[0].metric = bm::COUNT_AND;
    dmd[1].metric = bm::COUNT_SUB_AB;
    dmd[2].metric = bm::COUNT_SUB_BA;    
    
    TimeTaker tt("Tversky Index bvector test(pipeline)", REPEATS);
    for (i = 0; i < REPEATS; ++i)
    {
        bm::distance_operation(*bv1, *bv2, &dmd[0], (&dmd[0])+3);
                
        ti2 = double(dmd[0].result) / double(0.4*dmd[1].result + 0.5*dmd[2].result + dmd[0].result);
        
        dmd[0].result = dmd[1].result = dmd[2].result = 0;
    }
    }


    if (fabs(ti2 - ti1) > 0.1)
    {
        cout << "Check failed !" << endl;
        cout << ti1 << " " << ti2 << endl;
        exit(1);
    }
    count1 = count2 = 0;
    count1 = count2 = 0;

    // -----------------------------------------
    if (!platform_test)
    {
        cout << "Both vectors optimized" << endl;
    }
    bv1->optimize(tb);

    {
    TimeTaker tt("Tversky index bvector test", REPEATS);
    for (i = 0; i < REPEATS; ++i)
    {
        count1 = bm::count_and(*bv1, *bv2);
        
        countA = bm::count_sub(*bv1, *bv2);
        countB = bm::count_sub(*bv2, *bv1);
        
        ti1 = double(count1) / double(0.4*countA + 0.5*countB + count1);
    }
    }

    {
    bm::distance_metric_descriptor dmd[3];
    dmd[0].metric = bm::COUNT_AND;
    dmd[1].metric = bm::COUNT_SUB_AB;
    dmd[2].metric = bm::COUNT_SUB_BA;    
    
    TimeTaker tt("Tversky Index bvector test (pipeline)", REPEATS);
    for (i = 0; i < REPEATS; ++i)
    {
        bm::distance_operation(*bv1, *bv2, &dmd[0], (&dmd[0])+3);
                
        ti2 = double(dmd[0].result) / double(0.4*dmd[1].result + 0.5*dmd[2].result + dmd[0].result);
        
        dmd[0].result = dmd[1].result = dmd[2].result = 0;
    }
    }

    if (fabs(ti2 - ti1) > 0.1)
    {
        cout << "Check failed !" << endl;
        cout << ti1 << " " << ti2 << endl;
        exit(1);
    }


    delete bv1;
    delete bv2;
    
    delete bset1;
    delete bset2;    
}

#if 0
static
void BitBlockTransposeTest()
{
/*
    bm::word_t BM_VECT_ALIGN block1[bm::set_block_size] BM_VECT_ALIGN_ATTR = { 0, };

    for (unsigned i = 0; i < bm::set_block_size; ++i)
    {
        block1[i] = 1 | (1 << 5) | (7 << 15) | (3 << 22);
    }
*/
    unsigned   BM_VECT_ALIGN tmatrix1[32][bm::set_block_plain_size] BM_VECT_ALIGN_ATTR;

    const unsigned blocks_count = 70000;
    bm::word_t* blocks[blocks_count];
    for (unsigned k = 0; k < blocks_count; ++k)
    {
        blocks[k] = bm::block_allocator::allocate(bm::set_block_size, 0);
        for (unsigned i = 0; i < bm::set_block_size; ++i)
        {
            blocks[k][i] = 1 | (1 << 5) | (7 << 15) | (3 << 22);
        }
    }

    unsigned cnt=0;
/*
    {
    TimeTaker tt("Bit-block transpose.", REPEATS*1000);
    for (unsigned i = 0; i < REPEATS*1000; ++i)
    {
        bm::bit_block_transpose(block1, tmatrix1);
    }
    }

    {
    TimeTaker tt("Bit-block trestore.", REPEATS*1000);
    for (unsigned i = 0; i < REPEATS*1000; ++i)
    {
        bm::bit_block_trestore(tmatrix1, block2);
        cnt += block2[10];

    }
    }

    {
    TimeTaker tt("Bit-block transpose distance.", REPEATS*1000);
    unsigned distance[bm::set_block_plain_cnt][bm::set_block_plain_cnt];
    for (unsigned i = 0; i < REPEATS*1000; ++i)
    {
        bm::bit_block_tmatrix_distance(tmatrix1, distance);
        cnt += distance[1][1];
    }
    }
    printf("", cnt);
*/

    unsigned d2[bm::set_block_plain_cnt][bm::set_block_plain_cnt];
    {
    TimeTaker tt("Bit-block transpose+distance", 100000);
    unsigned distance[bm::set_block_plain_cnt][bm::set_block_plain_cnt];
    unsigned idx = 0;
    for (unsigned i = 0; i < 100000; ++i)
    {
        bm::vect_bit_transpose<unsigned, 
                               bm::set_block_plain_cnt, 
                               bm::set_block_plain_size>
                               (blocks[idx], bm::set_block_size, tmatrix1);
        bm::tmatrix_distance<unsigned, 
                             bm::set_block_plain_cnt, 
                             bm::set_block_plain_size>
                             (tmatrix1, distance);
    
        cnt += distance[1][1];
        ++idx;
        if (idx >= blocks_count) idx = 0;
        memcpy(d2, distance, sizeof(distance));
    }

    }
    
    char cbuf[256];
    sprintf(cbuf, "%i %i", cnt, d2[10][10]);

    for (unsigned i = 0; i < blocks_count; ++i)
    {
        bm::block_allocator::deallocate(blocks[i], 0);
    }

}
#endif

static
void BitBlockRotateTest()
{
    bm::word_t blk0[bm::set_block_size] = { 0 };
    bm::word_t blk1[bm::set_block_size] = { 0 };
    unsigned i;
    unsigned repeats = 20000000;

    for (i = 0; i < bm::set_block_size; ++i)
    {
        blk0[i] = blk1[i] = unsigned(rand());
    }

    {
        TimeTaker tt("Bit-block left rotate 1", repeats);
        for (i = 0; i < repeats; ++i)
        {
            bm::bit_block_rotate_left_1(blk0);
        }
    }
    {
        TimeTaker tt("Bit-block left rotate 1 unrolled", repeats);
        for (i = 0; i < repeats; ++i)
        {
            bm::bit_block_rotate_left_1_unr(blk1);
        }
    }

    for (i = 0; i < bm::set_block_size; ++i)
    {
        if (blk0[i] != blk1[i])
        {
            cerr << "Steress Cyclic rotate check failed" << endl;
            exit(1);
        }
    }

}

static
void BitBlockShiftTest()
{
    bm::word_t BM_VECT_ALIGN blk0[bm::set_block_size] BM_VECT_ALIGN_ATTR = { 0 };
    bm::word_t BM_VECT_ALIGN blk1[bm::set_block_size] BM_VECT_ALIGN_ATTR = { 0 };
    unsigned i;
    unsigned repeats = 20000000;
    unsigned acc0, acc1;

    for (i = 0; i < bm::set_block_size; ++i)
    {
        blk0[i] = blk1[i] = unsigned(rand());
    }

    {
        TimeTaker tt("Bit-block shift-r(1)", repeats);
        {
            for (i = 0; i < repeats; ++i)
            {
                bm::bit_block_shift_r1(blk0, &acc0, 0);
            }
        }
    }

    {
        TimeTaker tt("Bit-block shift-r(1) unrolled", repeats);
        for (i = 0; i < repeats; ++i)
        {
            bm::bit_block_shift_r1_unr(blk1, &acc1, 0);
        }
    }

    for (i = 0; i < bm::set_block_size; ++i)
    {
        if (blk0[i] != blk1[i])
        {
            cerr << "Stress SHIFT-r(1) check failed" << endl;
            exit(1);
        }
    }
    
    
    for (i = 0; i < bm::set_block_size; ++i)
    {
        blk0[i] = blk1[i] = unsigned(rand());
    }

    {
        TimeTaker tt("Bit-block shift-l(1)", repeats);
        for (i = 0; i < repeats; ++i)
        {
            bm::bit_block_shift_l1(blk0, &acc0, 0);
        }
    }

    {
        TimeTaker tt("Bit-block shift-l(1) unrolled", repeats);
        for (i = 0; i < repeats; ++i)
        {
            bm::bit_block_shift_l1_unr(blk1, &acc1, 0);
        }
    }

    for (i = 0; i < bm::set_block_size; ++i)
    {
        if (blk0[i] != blk1[i])
        {
            cerr << "Stress SHIFT-l(1) check failed" << endl;
            exit(1);
        }
    }


}


inline
void ptest()
{
    bvect*  bv_small = new bvect(bm::BM_GAP);
    bvect*  bv_large = new bvect(bm::BM_GAP);

    test_bitset*  bset = new test_bitset();

    FillSetsIntervals(bset, *bv_large, 0, 2000000000, 10);

    for (unsigned i = 0; i < 2000; ++i)
    {
        bv_small->set(i*10000);
    }

    {
    TimeTaker tt("Operation &= test", REPEATS * 10);
    unsigned count = 0;
    for (unsigned i = 0; i < REPEATS*10; ++i)
    {
        bvect t1(bm::BM_GAP);
        t1 = *bv_small;
        t1 &= *bv_large;
        count += t1.count();
    }
    }



    {
    TimeTaker tt("Operation &= with enumerator test", REPEATS * 10);
    unsigned count = 0;
    for (unsigned i = 0; i < REPEATS*10; ++i)
    {
        bvect t1(bm::BM_GAP);
        bvect t2(bm::BM_GAP);
        t1 = *bv_small;
        
        for (bvect::enumerator it = t1.first(); it != t1.end(); ++it) {
            if ((*bv_large)[*it]) {
                t2.set_bit(*it);
            }
        }
        count += t2.count();
    }
    }


}


typedef bm::sparse_vector<unsigned, bvect> svect;
typedef bm::sparse_vector<unsigned, bvect> sparse_vector_u32;

typedef bm::sparse_vector<unsigned, bvect > sparse_vector_u32;
typedef bm::sparse_vector<unsigned long long, bvect > sparse_vector_u64;
typedef bm::rsc_sparse_vector<unsigned, sparse_vector_u32> rsc_sparse_vector_u32;


// create a benchmark svector with a few dufferent distribution patterns
//
static
void FillSparseIntervals(svect& sv)
{
    sv.resize(250000000);
    unsigned i;
    for (i = 256000; i < 712000 * 2; ++i)
    {
        sv.set(i, 0xFFE);
    }
    for (i = 712000 * 3; i < 712000 * 5; ++i)
    {
        sv.set(i, i);
    }
    for (i = 180000000; i < 190000000; ++i)
    {
        sv.set(i, rand() % 128000);
    }
    for (i = 200000000; i < 210000000; ++i)
    {
        sv.set(i, rand() % 128000);
    }
}

template<class SV>
void FillSparseNullVector(SV& sv, typename SV::size_type size,
                          unsigned data_size, unsigned null_factor)
{
    typename SV::size_type i = 0;
    for (; i < size; i+= null_factor)
    {
        typename SV::size_type k = 0;
        for (;k < data_size; ++k, ++i)
        {
            sv[i] = 0x0DFA;
        } // for k
    } // for i
}

static
void SparseVectorAccessTest()
{
    std::vector<unsigned> target, target1, target2;
    svect   sv1;
    svect   sv2;
    svect   sv3;

    FillSparseIntervals(sv1);
    BM_DECLARE_TEMP_BLOCK(tb)
    sv1.optimize(tb);
    target.resize(150000000);
    target1.resize(150000000);
    target2.resize(150000000);

    {
        TimeTaker tt("sparse_vector random element assignment test", REPEATS/10 );
        for (unsigned i = 0; i < REPEATS/10; ++i)
        {
            for (unsigned j = 256000; j < 19000000/2; ++j)
            {
                sv2.set(j, 0xFFF);
            }
        }
    }

    {
        TimeTaker tt("sparse_vector back_inserter test", REPEATS/10 );
        for (unsigned i = 0; i < REPEATS/10; ++i)
        {
            {
                sv3.resize(256000);
                svect::back_insert_iterator bi(sv3.get_back_inserter());
                for (unsigned j = 256000; j < 19000000/2; ++j)
                {
                    *bi = 0xFFF;
                }
            }
        }
    }

    // check just in case
    //
    if (!sv2.equal(sv3))
    {
        std::cerr << "Error! sparse_vector back_insert mismatch."
                  << std::endl;
        std::cerr << "sv2.size()=" << sv2.size() << std::endl;
        std::cerr << "sv3.size()=" << sv3.size() << std::endl;
        
        for (unsigned i = 0; i < sv2.size(); ++i)
        {
            unsigned v2 = sv2[i];
            unsigned v3 = sv3[i];
            if (v2 != v3)
            {
                std::cerr << "mismatch at: " << i
                << " v2=" << v2 << " v3=" << v3
                << std::endl;
                exit(1);
            }
        }
        exit(1);
    }


    unsigned long long cnt = 0;
    
    unsigned gather_from = 256000;
    unsigned gather_to = 19000000/2;
    std::vector<unsigned> idx;
    for (unsigned j = gather_from; j < gather_to; ++j)
    {
        idx.push_back(j);
    }
    std::vector<unsigned> target_v;
    target_v.resize(idx.size());
    std::vector<unsigned> target_v2;
    target_v2.resize(idx.size());

    {
        TimeTaker tt("sparse_vector random element access test", REPEATS/10 );
        for (unsigned i = 0; i < REPEATS/10; ++i)
        {
            unsigned k = 0;
            for (unsigned j = gather_from; j < gather_to; ++j)
            {
                target_v[k++] = sv1[j];
            }
        }
    }

    {
        TimeTaker tt("sparse_vectot<>::gather() ", REPEATS/5 );
        for (unsigned i = 0; i < REPEATS/10; ++i)
        {
            sv1.gather(target_v.data(), idx.data(), unsigned(idx.size()), bm::BM_UNSORTED);
        }
    }

    {
        TimeTaker tt("sparse_vector<>::decode()", REPEATS / 5);
        auto from = gather_from;
        for (unsigned i = 0; i < REPEATS / 10; ++i)
        {
            auto dsize = sv1.decode(target_v2.data(), gather_from, (unsigned)idx.size(), (i == 0));
            from += (dsize % 123);
        }
    }

    {
        if (target_v != target_v2)
        {
            std::cerr << "gather-decode check failed" << endl;
            exit(1);
        }
    }


    {
        TimeTaker tt("sparse_vector const_iterator test", REPEATS );
        for (unsigned i = 0; i < REPEATS/10; ++i)
        {
            auto it = sv1.begin();
            auto it_end = sv1.end();
            auto sz = target2.size();
            for (unsigned k = 0; it != it_end && k < sz; ++it, ++k)
            {
                auto v = *it;
                target2[k] = v;
            }
        }
    }

/*
    // check just in case
    //
    size_t sz = min(target1.size(), target2.size());
    for (unsigned j = 0; j < sz; ++j)
    {
        if (target1[j] != target2[j])
        {
            std::cerr << "Error! sparse_vector mismatch at: " << j
            << " t1 = " << target1[j] << " t2 = " << target2[j]
            << " sv[] = " << sv1[j]
            << std::endl;
            exit(1);
        }
    }
*/
    
    char buf[256];
    sprintf(buf, "%i", (int)cnt); // to fool some smart compilers like ICC

}

void RSC_SparseVectorFillTest()
{
    bvect bv;// { 10, 20, 100, 200 };

    generate_bvector(bv, 4000000);
    bvect::size_type first, last, mid;
    bv.find_range(first, last);
    mid = first + ((last - first) / 4);


    rsc_sparse_vector_u32 csv1;
    rsc_sparse_vector_u32 csv2(bv);

    {
        TimeTaker tt("rsc_sparse_vector() set values", REPEATS*1);

        bvect::enumerator en = bv.get_enumerator(mid);
        for (;en.valid(); ++en)
        {
            auto idx = *en;
            csv1.set(idx, 40);
        }
        en.go_to(0);
        for (;en.valid(); ++en)
        {
            auto idx = *en;
            if (idx >= mid)
                break;
            csv1.set(idx, 40);
        }
    }

    {
        TimeTaker tt("rsc_sparse_vector() set values (rs-index)", REPEATS*1);
        csv2.sync();

        bvect::enumerator en = bv.get_enumerator(mid);
        for (;en.valid(); ++en)
        {
            auto idx = *en;
            csv2.set(idx, 40);
        }

        en.go_to(0);
        for (;en.valid(); ++en)
        {
            auto idx = *en;
            if (idx >= mid)
                break;
            csv2.set(idx, 40);
        }

    }

    bool eq = csv1.equal(csv2);
    if (!eq)
    {
        cerr << "Error: rsc_sparse_vector() set values check failed" << endl;
        assert(0); exit(1);
    }

}

void RSC_SparseVectorAccesTest()
{
    BM_DECLARE_TEMP_BLOCK(tb)

    unsigned test_size = 250000000;
    unsigned dec_size = 65536 * 3;

    std::vector<unsigned> vect, vect1, vect2;
    vect.resize(dec_size);
    vect1.resize(dec_size);
    vect2.resize(dec_size);


    sparse_vector_u32   sv1(bm::use_null);
    rsc_sparse_vector_u32::size_type sz, sz1;


    FillSparseNullVector(sv1, test_size, 2, 150);
    {
        rsc_sparse_vector_u32 csv1;
        csv1.load_from(sv1);
        csv1.optimize(tb);
        sv1.clear();

        {
            TimeTaker tt("rsc_sparse_vector()::decode() test (sparse)", REPEATS*10 );
            unsigned from = 0;
            for (unsigned i = 0; i < REPEATS*10; ++i)
            {
                sz = csv1.decode(&vect[0], from, dec_size);
                assert(sz); (void)sz;
                from += 3;//rand() % dec_size;
                if (from > csv1.size())
                    from = 0;
            } // for
        }
        {
            TimeTaker tt("rsc_sparse_vector()::decode_buf() test (sparse)", REPEATS*10 );
            unsigned from = 0;
            for (unsigned i = 0; i < REPEATS*10; ++i)
            {
                sz1 = csv1.decode_buf(&vect1[0], &vect2[0], from, dec_size);
                assert(sz); (void)sz;
                from += 3;//rand() % dec_size;
                if (from > csv1.size())
                    from = 0;
            } // for
        }
        assert (sz == sz1);
        for (unsigned i = 0; i < sz; ++i)
        {
            auto v = vect[i];
            auto v1 = vect1[i];
            assert(v == v1);
        }

    }

    FillSparseNullVector(sv1, test_size, 64, 5);
    {
        rsc_sparse_vector_u32 csv1;
        csv1.load_from(sv1);
        csv1.optimize(tb);
        sv1.clear();

        {
            TimeTaker tt("rsc_sparse_vector<>::decode() test (dense)", REPEATS*10 );
            unsigned from = 0;
            for (unsigned i = 0; i < REPEATS*10; ++i)
            {
                sz = csv1.decode(&vect[0], from, dec_size);

                assert(sz);(void)sz;
                from += 7; // rand() % dec_size;
                if (from > csv1.size())
                    from = 0;
            } // for
        }
        {
            TimeTaker tt("rsc_sparse_vector()::decode_buf() test (dense)", REPEATS*10 );
            unsigned from = 0;
            for (unsigned i = 0; i < REPEATS*10; ++i)
            {
                sz1 = csv1.decode_buf(&vect1[0], &vect2[0], from, dec_size);
                assert(sz); (void)sz;
                from += 7; // rand() % dec_size;
                if (from > csv1.size())
                    from = 0;
            } // for
        }
        assert (sz == sz1);
        for (unsigned i = 0; i < sz; ++i)
        {
            assert(vect[i] == vect1[i]);
        }

    }



}




static
void OptimizeTest()
{
    std::vector<bvect> bv_coll;
    std::vector<bvect> bv_coll_control;

    GenerateTestCollection(&bv_coll, 100, 80000000, false);
    bv_coll_control = bv_coll;
    
    BM_DECLARE_TEMP_BLOCK(tb)
    {
    TimeTaker tt("bvector<>::optimize() ", 1);
        for (unsigned k = 0; k < bv_coll.size(); ++k)
        {
            bv_coll[k].optimize(tb);
        }
    }
    for (unsigned k = 0; k < bv_coll.size(); ++k)
    {
        const bvect& bv1 = bv_coll[k];
        const bvect& bv2 = bv_coll_control[k];
        int cmp = bv1.compare(bv2);
        if (cmp != 0)
        {
            std::cerr << "Optimization error!" << endl;
            exit(1);
        }
    }

    
}

static
void AggregatorTest()
{
    int res;
    bvect* bv_arr[128] = { 0, };
    bvect* bv_arr2[128] = { 0, };
    bm::aggregator<bvect> agg;
    
    std::vector<bvect> bv_coll;
    GenerateTestCollection(&bv_coll, 25, 80000000);
    std::vector<bvect> bv_coll2;
    GenerateTestCollection(&bv_coll2, 7, 50000000);

    if (!bv_coll.size())
        return;
    
    std::vector<bvect>& bvc = bv_coll;
    for (unsigned k = 0; k < bv_coll.size(); ++k)
    {
        bv_arr[k] = &(bvc[k]);
    } // for
    std::vector<bvect>& bvc2 = bv_coll2;
    for (unsigned k = 0; k < bv_coll2.size(); ++k)
    {
        bv_arr2[k] = &(bvc2[k]);
    } // for

    std::unique_ptr<bvect> bv_target1(new bvect);
    std::unique_ptr<bvect> bv_target2(new bvect);

    {
    TimeTaker tt("Horizontal aggregator OR ", REPEATS);
    for (unsigned i = 0; i < REPEATS; ++i)
    {
        agg.combine_or_horizontal(*bv_target1, bv_arr, unsigned(bv_coll.size()));
    } // for
    }

    {
    TimeTaker tt("aggregator OR", REPEATS);
    for (unsigned i = 0; i < REPEATS; ++i)
    {
        agg.combine_or(*bv_target2, bv_arr, unsigned(bv_coll.size()));
    } // for
    }

    res = bv_target1->compare(*bv_target2);
    if (res != 0)
    {
        std::cerr << "Error: Aggregator OR integrity failed." << std::endl;
        exit(1);
    }


    // ------------------------------------------------------------------
    {
    TimeTaker tt("Horizontal aggregator AND", REPEATS);
    for (unsigned i = 0; i < REPEATS; ++i)
    {
        agg.combine_and_horizontal(*bv_target1, bv_arr, unsigned(bv_coll.size()));
    } // for
    }

    {
    TimeTaker tt("aggregator AND", REPEATS);
    for (unsigned i = 0; i < REPEATS; ++i)
    {
        agg.combine_and(*bv_target2, bv_arr, unsigned(bv_coll.size()));
    } // for
    }
    auto and_cnt = bv_target1->count();

    res = bv_target1->compare(*bv_target2);
    if (res != 0)
    {
        std::cerr << "Error: Aggregator AND integrity failed." << std::endl;
        exit(1);
    }

    {
    TimeTaker tt("Horizontal aggregator AND-SUB", REPEATS);
    for (unsigned i = 0; i < REPEATS; ++i)
    {
        agg.combine_and_sub_horizontal(*bv_target1,
                                        bv_arr, unsigned(bv_coll.size()),
                                        bv_arr2, unsigned(bv_coll2.size())
                                        );
    } // for
    }
    
    auto and_sub_cnt = bv_target1->count();
    if (and_sub_cnt > and_cnt)
    {
        std::cerr << "Error: Aggregator count AND-SUB integrity failed." << std::endl;
        exit(1);
    }

    {
    TimeTaker tt("aggregator AND-SUB", REPEATS);
    for (unsigned i = 0; i < REPEATS; ++i)
    {
        agg.combine_and_sub(*bv_target2,
                            bv_arr, unsigned(bv_coll.size()),
                            bv_arr2, unsigned(bv_coll2.size()),
                            false
                            );
    } // for
    }
    
    res = bv_target1->compare(*bv_target2);
    if (res != 0)
    {
        std::cerr << "Error: Aggregator AND-SUB integrity failed." << std::endl;
        exit(1);
    }


    //std::cout << bv_target1->count() << std::endl;
}


static
void BvectorShiftTest()
{

    {
        std::vector<bvect> bv_coll;
        GenerateTestCollection(&bv_coll, 25, 80000000);

        if (!bv_coll.size())
            return;

        {
            TimeTaker tt("bvector<>::shift_right() ", REPEATS);
            for (unsigned i = 0; i < REPEATS; ++i)
            {
                for (unsigned k = 0; k < bv_coll.size(); ++k)
                {
                    bv_coll[k].shift_right();
                } // for
            } // for
        }
    }

    {
        std::vector<bvect> bv_coll;
        GenerateTestCollection(&bv_coll, 25, 80000000);

        if (!bv_coll.size())
            return;

        {
            TimeTaker tt("bvector<>::shift_left() ", REPEATS);
            for (unsigned i = 0; i < REPEATS; ++i)
            {
                for (unsigned k = 0; k < bv_coll.size(); ++k)
                {
                    bv_coll[k].shift_left();
                } // for
            } // for
        }
    }


    bvect mask_bv; // mask vector
    mask_bv.init();
    generate_bvector(mask_bv, 75000000, false); // mask is shorter on both ends

    std::vector<bvect> bv_coll1;
    GenerateTestCollection(&bv_coll1, 25, 80000000);
    
    std::vector<bvect> bv_coll2;
    GenerateTestCollection(&bv_coll2, 25, 80000000);
    

    {
        {
            TimeTaker tt("bvector<>::shift_right()+AND ", REPEATS);
            for (unsigned i = 0; i < REPEATS; ++i)
            {
                bvect bv(mask_bv);
                for (unsigned k = 0; k < bv_coll1.size(); ++k)
                {
                    bv.shift_right();
                    bv &= bv_coll1[k];
                } // for
            } // for
        }
    }

    {
        bm::aggregator<bvect> agg;
        
        {
            TimeTaker tt("aggregator::shift_right_and() ", REPEATS);
            agg.add(&mask_bv);
            for (unsigned k = 0; k < bv_coll2.size(); ++k)
            {
                agg.add(&bv_coll2[k]);
            }
            for (unsigned i = 0; i < REPEATS; ++i)
            {
                bvect bv;
                agg.combine_shift_right_and(bv);
            }
        }
    }
/*
    // check correcness
    //
    for (unsigned k = 0; k < bv_coll1.size(); ++k)
    {
        const bvect& bv1 = bv_coll1[k];
        const bvect& bv2 = bv_coll2[k];
        auto cmp = bv1.compare(bv2);
        if (cmp != 0)
        {
            cerr << "Mismatch error!" << endl;
            exit(1);
        }
    } // for
*/

    //std::cout << bv_target1->count() << std::endl;
}



static
void Set2SetTransformTest()
{
    svect   sv;
    
    FillSparseIntervals(sv);
    BM_DECLARE_TEMP_BLOCK(tb)
    sv.optimize(tb);
    
    bvect bv_values;
    bvect bv_non_values;
    bvect bv_sample;
    bvect bv_non_sample;


    {
    bm::sparse_vector_scanner<svect> scanner;
    scanner.find_nonzero(sv, bv_values);
    scanner.find_zero(sv, bv_non_values);
    
    unsigned non_v_count = bv_non_values.count() / 20;
    if (non_v_count > 3000000)
        non_v_count = 3000000;

    if (non_v_count)
    {
        bm::random_subset<bvect> rand_sampler;
        rand_sampler.sample(bv_sample, bv_values, 6000000);
        rand_sampler.sample(bv_non_sample, bv_non_values, non_v_count);
        //cout << "zero mix = " << bv_non_sample.count() << endl;
        bv_sample |= bv_non_sample; // add some missing values
    }
    }
    
    //cout << bv_sample.count() << endl;
    //cout << sv.size() << endl;

    bm::set2set_11_transform<svect> set2set;

    int cnt = 0;

    {
    TimeTaker tt("set2set_11_transform::run()", REPEATS/10);

        for (unsigned i = 0; i < REPEATS/10; ++i)
        {
            bvect bv_out;
            set2set.run(bv_sample, sv, bv_out);

            cnt += bv_out.any();
        }
    }

    /*
    {
    TimeTaker tt("set2set_11_transform::one_pass_run", REPEATS/10);

        for (unsigned i = 0; i < REPEATS/10; ++i)
        {
            bvect bv_out;
            set2set.one_pass_run(bv_sample, sv, bv_out);

            cnt += bv_out.any();
        }
    }
    */

    char buf[256];
    sprintf(buf, "%i", (int)cnt); // to fool some smart compilers like ICC

}

static
void RangeCopyTest()
{
    const unsigned vect_max = BSIZE;
    bvect bv;
    generate_bvector(bv, vect_max);

    {
        TimeTaker tt("bvector<>::copy_range()", REPEATS * 25);
        for (unsigned i = 0; i < REPEATS * 25; ++i)
        {
            unsigned from = vect_max / 4;
            from = from * (rand() % 3);
            unsigned to = from + 1 + (65536 * rand() % 5);
            bvect bv_cp;
            bv_cp.copy_range(bv, from, to);
        } // for
    }
    {
        TimeTaker tt("bvector<>:: copy range constructor", REPEATS * 25);
        for (unsigned i = 0; i < REPEATS * 25; ++i)
        {
            unsigned from = vect_max / 4;
            from = from * (rand() % 3);
            unsigned to = from + 1 + (65536 * rand() % 5);
            bvect bv_cp(bv, from, to);
        } // for
    }

    {
        TimeTaker tt("copy range with AND", REPEATS * 25);
        for (unsigned i = 0; i < REPEATS * 25; ++i)
        {
            unsigned from = vect_max / 4;
            from = from * (rand() % 3);
            unsigned to = from + 1 + (65536 * rand() % 5);
            bvect bv_cp;
            bv_cp.set_range(from, to);
            bv_cp &= bv;
        } // for
    }

}

/// Reference (naive) interval detector based on population counting 
/// and boundaries tests
///
template<typename BV>
bool test_interval(const BV& bv,
        typename BV::size_type left, typename BV::size_type right) noexcept
{
    if (left > right)
        bm::xor_swap(left, right); // make sure left <= right
    bool is_left(0), is_right(0);
    if (left) // check left-1 bit (if exists)
        is_left = bv.test(left - 1);
    if ((is_left == false) && (right < bm::id_max - 1))
        is_right = bv.test(right + 1); // check [...right] range condition
    if (is_left == false && is_right == false)
    {
        typename BV::size_type cnt = bv.count_range(left, right);
        if (cnt == (1 + right - left))
            return true;
    }
    return false;
}

static
void IntervalsTest()
{
    const unsigned vect_max = BSIZE * 2;
    bvect bv, bv_inv;

    // generate the test vector
    {
        bool b;
        bvect::size_type istart(0), ilen(0);
        bvect::bulk_insert_iterator iit(bv);
        for (istart = 0; istart < vect_max; )
        {
            for (bvect::size_type i = istart; i <= (istart+ilen); ++i)
            {
                iit = i;
            } // for i
            iit.flush();

            ilen += 1;
            b = bv.test(istart + ilen);
            assert(!b);
            istart += (ilen + 2);
            if (ilen > 1024)
                ilen = 0;
            
        } // for istart
    }
    bv_inv = bv;
    bv_inv.invert();
    //auto cnt = bm::count_intervals(bv);
    //cout << cnt << endl;

    const char* msg = "bvector<>::is_all_one_range() (BITS)";
    const char* msg2 = "bvector<>::any_range() (BITS)";
    const char* msg3 = "bvector<>::count_range() (BITS)";
    const char* msg4 = "bvector<>::is_interval() (BITS)";
    const char* msg5 = "reference_is_interval() (BITS)";
    const char* msg6 = "bvector<>::find_interval_start() (BITS)";
    const char* msg7 = "bvector<>::find_interval_end() (BITS)";
    const char* msg8 = "interval_enumerator<> (BITS)";
    const char* msg9 = "bvector<>::enumerator (BITS)";

    for (unsigned pass = 0; pass < 2; ++pass)
    {
        {
            TimeTaker tt(msg3, REPEATS * 1);
            bvect::size_type istart(0), ilen(0);
            for (istart = 0; istart < vect_max; )
            {
                for (bvect::size_type i = istart; i <= (istart + ilen); ++i)
                {
                    auto cnt = bv.count_range(istart, i);
                    if (!cnt)
                    {
                        cerr << "Errro: count_range test failed! (1)" << endl;
                        assert(0); exit(1);
                    }
                    auto icnt = bv_inv.count_range(istart, i);
                    if (icnt)
                    {
                        cerr << "Errro: count_range test failed! (2)" << endl;
                        assert(0); exit(1);
                    }
                } // for i
                ilen += 1;
                istart += (ilen + 2);
                if (ilen > 1024)
                    ilen = 0;
            } // for istart
        }

        {
            TimeTaker tt(msg, REPEATS * 1);
            bvect::size_type istart(0), ilen(0);
            for (istart = 0; istart < vect_max; )
            {
                for (bvect::size_type i = istart; i <= (istart+ilen); ++i)
                {
                    bool all_one = bv.is_all_one_range(istart, i);
                    if (!all_one)
                    {
                        cerr << "Errro: is_all_one_range test failed! (1)" << endl;
                        assert(0); exit(1);
                    }
                    all_one = bv_inv.is_all_one_range(istart, i);
                    if (all_one)
                    {
                        cerr << "Errro: is_all_one_range test failed! (2)" << endl;
                        assert(0); exit(1);
                    }
                } // for i
                ilen += 1;
                istart += (ilen + 2);
                if (ilen > 1024)
                    ilen = 0;
            } // for istart
        }

        {
            TimeTaker tt(msg4, REPEATS * 1);
            bvect::size_type istart(0), ilen(0);
            for (istart = 0; istart < vect_max; )
            {
                for (bvect::size_type i = istart; i <= (istart + ilen); ++i)
                {
                    bool is_int = bm::is_interval(bv, istart, i);
                    if (!is_int)
                    {
                        if (i == istart + ilen)
                        {
                            cerr << "Error: is_interval test failed! (1)" << endl;
                            is_int = bm::is_interval(bv, istart, i);
                            assert(0); exit(1);
                        }
                    }
                    else
                    {
                        assert(i == istart + ilen);
                    }

                    is_int = bm::is_interval(bv_inv, istart, i);
                    if (is_int)
                    {
                        cerr << "Errro: is_interval test failed! (2)" << endl;
                        assert(0); exit(1);
                    }
                } // for i
                ilen += 1;
                istart += (ilen + 2);
                if (ilen > 1024)
                    ilen = 0;
            } // for istart
        }

        {
            TimeTaker tt(msg5, REPEATS * 1);
            bvect::size_type istart(0), ilen(0);
            for (istart = 0; istart < vect_max; )
            {
                for (bvect::size_type i = istart; i <= (istart + ilen); ++i)
                {
                    bool is_int = test_interval(bv, istart, i);
                    if (!is_int)
                    {
                        if (i == istart + ilen)
                        {
                            cerr << "Error: test_interval test failed! (1)" << endl;
                            assert(0); exit(1);
                        }
                    }
                    else
                    {
                        assert(i == istart + ilen);
                    }
                    is_int = test_interval(bv_inv, istart, i);
                    if (is_int)
                    {
                        cerr << "Errro: test_interval test failed! (2)" << endl;
                        assert(0); exit(1);
                    }
                } // for i
                ilen += 1;
                istart += (ilen + 2);
                if (ilen > 1024)
                    ilen = 0;
            } // for istart
        }

        {
            TimeTaker tt(msg2, REPEATS * 1);
            bvect::size_type istart(0), ilen(0);
            for (istart = 0; istart < vect_max; )
            {
                for (bvect::size_type i = istart; i <= (istart+ilen); ++i)
                {
                    bool any_one = bv.any_range(istart, i);
                    if (!any_one)
                    {
                        cerr << "Errro: any_range test failed! (1)" << endl;
                        assert(0); exit(1);
                    }
                    any_one = bv_inv.any_range(istart, i);
                    if (any_one)
                    {
                        cerr << "Errro: any_range test failed! (2)" << endl;
                        assert(0); exit(1);
                    }

                } // for i
                ilen += 1;
                istart += (ilen + 2);
                if (ilen > 1024)
                    ilen = 0;
            } // for istart
        }

        {
            TimeTaker tt(msg6, REPEATS * 1);
            bvect::size_type istart(0), ilen(0);
            for (istart = 0; istart < vect_max; )
            {
                for (bvect::size_type i = istart; i <= (istart + ilen); ++i)
                {
                    bvect::size_type pos;
                    auto diff = i - istart;
                    if (!diff)
                        continue;
                    bool b = bm::find_interval_start(bv, istart+diff, pos);
                    assert(b); (void)b;
                    assert(pos == istart);
                } // for i
                ilen += 1;
                istart += (ilen + 2);
                if (ilen > 1024)
                    ilen = 0;
            } // for istart
        }

        {
            TimeTaker tt(msg7, REPEATS * 1);
            bvect::size_type istart(0), ilen(0);
            for (istart = 0; istart < vect_max; )
            {
                for (bvect::size_type i = istart; i <= (istart + ilen); ++i)
                {
                    bvect::size_type pos;
                    auto diff = i - istart;
                    if (!diff)
                        continue;
                    bool b = bm::find_interval_end(bv, istart+diff, pos);
                    assert(b); (void)b;
                    assert(pos == istart+ilen);
                } // for i
                ilen += 1;
                istart += (ilen + 2);
                if (ilen > 1024)
                    ilen = 0;
            } // for istart
        }

        bvect::size_type cnt_c = bv.count();
        {
            TimeTaker tt(msg8, REPEATS * 1);
   
            for (unsigned i = 0; i < REPEATS; ++i)
            {
                bvect::size_type cnt = 0;
                bm::interval_enumerator<bvect> ien(bv);
                if (ien.valid())
                {
                    do {
                        auto s = ien.start();
                        auto e = ien.end();
                        cnt += e - s + 1;
                    } while (ien.advance());
                }
                assert(cnt == cnt_c);
                if (cnt != cnt_c)
                {
                    cerr << "Count mismatch!" << endl;
                    exit(1);
                }
            }
        }
        {
            bvect::size_type sum = 0;
            TimeTaker tt(msg9, REPEATS * 1);

            for (unsigned i = 0; i < REPEATS; ++i)
            {
                bvect::size_type cnt = 0;
                bvect::enumerator en = bv.get_enumerator(0);
                while (en.valid())
                {
                    auto v = *en; 
                    sum += v;
                    ++cnt;
                    ++en;
                }
                assert(cnt == cnt_c);
            }
            char buf[256];
            sprintf(buf, "%u", sum); // this is to prevent unwanted optimizations by some compilers

        }

        bv.optimize();
        bv_inv.optimize();

        msg = "bvector<>::is_all_one_range() (BITS+GAPS)";
        msg2 = "bvector<>::any_range() (BITS+GAPS)";
        msg3 = "bvector<>::count_range() (BITS+GAPS)";
        msg4 = "bvector<>::is_interval() (BITS+GAPS)";
        msg5 = "reference_is_interval() (BITS+GAPS)";
        msg6 = "bvector<>::find_interval_start() (BITS+GAPS)";
        msg7 = "bvector<>::find_interval_end() (BITS+GAPS)";
        msg8 = "interval_enumerator<> (BITS+GAPS)";
        msg9 = "bvector<>::enumerator (BITS+GAPS)";
    } // for pass

}

static
void RankCompressionTest()
{
    bvect bv_i1, bv_s1, bv11, bv12, bv21, bv22;
    bvect bv_i2, bv_s2;
    bvect bv11_s, bv12_s, bv21_s, bv22_s;

    generate_bvector(bv_i1);
    generate_bvector(bv_i2);
    bv_i2.optimize();

    bm::random_subset<bvect> rsub;
    rsub.sample(bv_s1, bv_i1, 1000);
    rsub.sample(bv_s2, bv_i2, 1000);
    bv_s2.optimize();

    bm::rank_compressor<bvect> rc;

    std::unique_ptr<bvect::rs_index_type> bc1(new bvect::rs_index_type());
    bv_i1.build_rs_index(bc1.get());
    std::unique_ptr<bvect::rs_index_type> bc2(new bvect::rs_index_type());
    bv_i2.build_rs_index(bc2.get());

    {
        TimeTaker tt("Rank compression test", REPEATS * 10);
        for (unsigned i = 0; i < REPEATS * 10; ++i)
        {
            rc.compress(bv11, bv_i1, bv_s1);
            rc.compress(bv12, bv_i2, bv_s2);
        } // for
    }
    {
        TimeTaker tt("Rank compression (by source) test", REPEATS * 10);
        for (unsigned i = 0; i < REPEATS * 10; ++i)
        {
            rc.compress_by_source(bv21, bv_i1, *bc1, bv_s1);
            rc.compress_by_source(bv22, bv_i2, *bc2, bv_s2);
        } // for
    }
    
    {
        TimeTaker tt("Rank decompression test", REPEATS * 10);
        for (unsigned i = 0; i < REPEATS * 10; ++i)
        {
            rc.decompress(bv11_s, bv_i1, bv11);
            rc.decompress(bv12_s, bv_i2, bv12);
            rc.decompress(bv21_s, bv_i1, bv21);
            rc.decompress(bv22_s, bv_i2, bv22);
        } // for
    }

    int cmp;
    cmp = bv11_s.compare(bv_s1);
    if (cmp != 0)
    {
        std::cerr << "1. Rank check failed!" << std::endl;
        exit(1);
    }
    cmp = bv12_s.compare(bv_s2);
    if (cmp != 0)
    {
        std::cerr << "2. Rank check failed!" << std::endl;
        exit(1);
    }
    cmp = bv21_s.compare(bv_s1);
    if (cmp != 0)
    {
        std::cerr << "3. Rank check failed!" << std::endl;
        exit(1);
    }
    cmp = bv22_s.compare(bv_s2);
    if (cmp != 0)
    {
        std::cerr << "4. Rank check failed!" << std::endl;
        exit(1);
    }
    
}

static
void generate_serialization_test_set(sparse_vector_u32&   sv,
                                     unsigned vector_max = BSIZE)
{
    sparse_vector_u32::back_insert_iterator bi(sv.get_back_inserter());

    unsigned v = 0;
    for (unsigned i = 0; i < vector_max; ++i)
    {
        unsigned plato = rand() % 16;
        for (unsigned j = 0; i < vector_max && j < plato; ++i, ++j)
        {
            *bi = v;
        } // for j
        if (++v > 100000)
            v = 0;
        unsigned nulls = rand() % 16;
        if (nulls)
            bi.add_null(nulls);
        i += nulls;
    } // for i

    sv.optimize();
}


static
void generate_scanner_test_set(std::vector<unsigned>& vect,
                               bvect&               bv_null,
                               sparse_vector_u32&   sv,
                               unsigned vector_max = BSIZE)
{
    sparse_vector_u32::back_insert_iterator bi(sv.get_back_inserter());

    vect.resize(vector_max);
    bv_null.reset();

    for (unsigned i = 0; i < vector_max; ++i)
    {
        unsigned v = unsigned(rand_dis(gen));
        vect[i] = v;
        bv_null[i] = true; // not NULL(assigned) element
        *bi = v; // push back an element to sparse vector
        if (i % 64 == 0)
        {
            bi.add_null(5);  // add 5 unassigned elements using back inserter
            i += 5;  // insert a small NULL plate (unassigned values)
        }
    } // for
    sv.optimize();
}

static
void vector_search(const std::vector<unsigned>& vect,
                   const bvect&                 bv_null,
                   unsigned                     value,
                   bvect&                       bv_res)
{
    bv_res.init(); // always use init() if set_bit_no_check()
    for (size_t i = 0; i < vect.size(); ++i)
    {
        if (vect[i] == value)
            bv_res.set_bit_no_check((bm::id_t)i);
    } // for
    bv_res &= bv_null; // correct results to only include non-NULL values
}



static
void SparseVectorScannerTest()
{
    std::vector<unsigned> vect;
    bvect bv_null;
    sparse_vector_u32 sv(bm::use_null);
    
    generate_scanner_test_set(vect, bv_null, sv, BSIZE);

    // generate a search vector for benchmarking
    std::vector<unsigned> search_vect;
    {
        bm::bvector<> bv_tmp;
        search_vect.reserve(REPEATS);
        for (unsigned i = 0; i < REPEATS;)
        {
            bm::id_t idx = bm::id_t(rand_dis(gen));
            if (!bv_tmp.test(idx)) // check if number is unique
            {
                search_vect.push_back(idx);
                bv_tmp[idx] = 1;
                ++i;
            }
        }
    }

    bm::sparse_vector_scanner<sparse_vector_u32> scanner;

    bvect bv_res1, bv_res2, bv_res3;

    unsigned search_repeats = REPEATS;
    {
        TimeTaker tt("std::vector<> scan ", search_repeats);
        for (unsigned i = 0; i < search_repeats; ++i)
        {
            unsigned vs = search_vect[i];
            vector_search(vect, bv_null, vs, bv_res1);
        } // for
    }

    {
    TimeTaker tt("horizontal sparse vector scanner find_eq()", search_repeats);
    for (unsigned i = 0; i < search_repeats; ++i)
    {
        {
        bvect bv1;
        scanner.find_eq_with_nulls_horizontal(sv, search_vect[i], bv1);
        bv_res2 |= bv1;
        }
    } // for
    scanner.correct_nulls(sv, bv_res2);
    }
    
    {
    TimeTaker tt("sparse vector scanner find_eq() ", search_repeats);
    {
        scanner.find_eq(sv, search_vect.begin(), search_vect.end(), bv_res3);
    } // for
    }
    
    int res = bv_res3.compare(bv_res1);
    if (res != 0)
    {
        std::cerr << "1. Sparse scanner integrity check failed!" << std::endl;
        exit(1);
    }

    res = bv_res2.compare(bv_res1);
    if (res != 0)
    {
        std::cerr << "2. Sparse scanner integrity check failed!" << std::endl;
        exit(1);
    }
}


static
void SparseVectorSerializationTest()
{
    const unsigned char* buf;
    bool eq;
    size_t sz1, sz2;

    sparse_vector_u32 sv1(bm::use_null);
    sparse_vector_u32 sv2(bm::use_null);
    sparse_vector_u32 sv3(bm::use_null);

    generate_serialization_test_set(sv1, BSIZE);

    bm::sparse_vector_serial_layout<sparse_vector_u32> sv_lay;

    bm::sparse_vector_serializer<sparse_vector_u32> sv_serializer;
    bm::sparse_vector_deserializer<sparse_vector_u32> sv_deserial;


    {
        {
            TimeTaker tt("bm::sparse_vector<> serialization XOR disabled ", 1);

            sv_serializer.set_xor_ref(false); // disable XOR compression
            sv_serializer.serialize(sv1, sv_lay);
        }

        buf = sv_lay.buf();
        sz1 = sv_lay.size();

        sv_deserial.deserialize(sv2, buf);

        eq = sv1.equal(sv2);
        if (!eq)
        {
            cerr << "Error: SparseVectorSerializationTest() integrity failure! (1)" << endl;
            sparse_vector_u32::size_type pos;

            bool f = bm::sparse_vector_find_first_mismatch(sv1, sv2, pos);
            assert(f);
            cerr << "Mismatch at: " << pos << " " << f << endl;

            sv_deserial.deserialize(sv2, buf);

            exit(1);
        }
        sv2.resize(0);
    }

    {
        TimeTaker tt("bm::sparse_vector<> serialization XOR enabled ", 1);
        sv_serializer.set_xor_ref(true); // enable XOR compression
        sv_serializer.serialize(sv1, sv_lay);
    }

    buf = sv_lay.buf();
    sz2 = sv_lay.size();
    sv_deserial.deserialize(sv3, buf);
    eq = sv1.equal(sv3);
    if (!eq)
    {
        cerr << "Error: SparseVectorSerializationTest() integrity failure! (2)" << endl;
        sparse_vector_u32::size_type pos;
        bool f = bm::sparse_vector_find_first_mismatch(sv1, sv3, pos);
        assert(f); (void)f;
        cerr << "Mismatch at: " << pos << endl;

        sv_deserial.deserialize(sv3, buf);

        exit(1);
    }

    if (sz2 >= sz1)
    {
        cerr << "XOR negative compression!" << endl;
        assert(0);
    }
    else
    {
        //cout << "sz1 = " << sz1 << " gain=" << (sz1 - sz2) << endl;
    }

}

static
void SparseVectorRangeDeserializationTest()
{
    std::vector<unsigned> vect;
    bvect bv_null;
    sparse_vector_u32 sv1(bm::use_null);
    sparse_vector_u32 sv2(bm::use_null);

    generate_scanner_test_set(vect, bv_null, sv1, BSIZE);

    bm::sparse_vector_deserializer<sparse_vector_u32> sv_deserial;
    bm::sparse_vector_serializer<sparse_vector_u32> sv_serializer;
    sv_serializer.set_bookmarks(false);

    bm::sparse_vector_serial_layout<sparse_vector_u32> sv_lay;
    const unsigned char* buf;

    sv_serializer.serialize(sv1, sv_lay);

    buf = sv_lay.buf();
    
    {
        TimeTaker tt("bm::sparse_vector<> Range Deserialization() - NO bookmarks ", 1);
        for (unsigned i = 0; i < 15; ++i)
        {
            sv_deserial.deserialize(sv2, buf, 0, 65536 * 2);
            sv_deserial.deserialize(sv2, buf, BSIZE / 4, BSIZE / 2);
            sv_deserial.deserialize(sv2, buf, BSIZE / 2, (65536 * 2) + (BSIZE / 2));
        }
    }

    assert(sv1.size() == sv2.size());
    // validation
    {
        sparse_vector_u32::size_type to = (65536 * 2) + (BSIZE / 2);
        for (sparse_vector_u32::size_type from = BSIZE / 2; from <= to; ++from)
        {
            auto v1 = sv1[from];
            auto v2 = sv2[from];
            if (v1 != v2)
            {
                cerr << "Range extraction failed!" << endl;
                exit(1);
            }
        }
    }


    // book-mark enabled serialization
    //
    sv_serializer.set_bookmarks(true, 64);
    sv_serializer.serialize(sv1, sv_lay);

    buf = sv_lay.buf();

    {
        TimeTaker tt("bm::sparse_vector<> Range Deserialization() - WITH bookmarks ", 1);
        for (unsigned i = 0; i < 15; ++i)
        {
            sv_deserial.deserialize_range(sv2, buf, 0, 65536 * 2);
            sv_deserial.deserialize_range(sv2, buf, BSIZE / 4, BSIZE / 2);
            sv_deserial.deserialize_range(sv2, buf, BSIZE / 2, (65536 * 2) + (BSIZE / 2));
        }
    }

    // validation
    //
    {
        sparse_vector_u32::size_type to = (65536 * 2) + (BSIZE / 2);
        for (sparse_vector_u32::size_type from = BSIZE / 2; from <= to; ++from)
        {
            auto v1 = sv1[from];
            auto v2 = sv2[from];
            if (v1 != v2)
            {
                cerr << "Bookmark Range extraction failed!" << endl;
                exit(1);
            }
        }
    }

}




typedef bm::str_sparse_vector<char, bvect, 32> str_svect_type;

static
void GenerateTestStrCollection(std::vector<string>& str_coll, unsigned max_coll)
{
    string prefix = "az";
    string str;
    for (unsigned i = 0; i < max_coll; ++i)
    {
        str = prefix;
        str.append(to_string(i));
        str_coll.emplace_back(str);
        
        {
            prefix.clear();
            unsigned prefix_len = rand() % 5;
            for (unsigned j = 0; j < prefix_len; ++j)
            {
                char cch = char('a' + rand() % 26);
                prefix.push_back(cch);
            } // for j
        }
    } // for i
    
}


static
void StrSparseVectorTest()
{
    const unsigned max_coll = 20000000;
    
   std::vector<string> str_coll;
   str_svect_type str_sv;

   GenerateTestStrCollection(str_coll, max_coll);
   
    {
       TimeTaker tt("bm::str_sparse_vector<>::push_back() ", 1);
       for (auto str : str_coll)
       {
           str_sv.push_back(str);
       }
    }
    str_sv.optimize();
    
    {
       str_svect_type str_sv0;
       TimeTaker tt("bm::str_sparse_vector<>::back_insert_iterator ", 1);
       str_svect_type::back_insert_iterator bi = str_sv0.get_back_inserter();
       for (auto str : str_coll)
       {
           bi = str;
       }

    }
    
    {
    string str;

        TimeTaker tt("bm::str_sparse_vector<> - random access ", 1);
        for (unsigned i = 0; i < str_sv.size(); ++i)
        {
            str_sv.get(i, str);
            const std::string& sc = str_coll[i];
            if (str != sc)
            {
                cerr << "String random access check failure!" << endl;
                exit(1);
            }
        }
    }

    {
        TimeTaker tt("bm::str_sparse_vector<>::const_iterator ", 1);
        str_svect_type::const_iterator it = str_sv.begin();
        str_svect_type::const_iterator it_end = str_sv.end();

        for (unsigned i=0; it < it_end; ++it, ++i)
        {
            const char* s = *it;
            const std::string& sc = str_coll[i];
            int cmp = ::strcmp(s, sc.c_str());
            if (cmp != 0)
            {
                cerr << "String random access check failure!" << endl;
                exit(1);
            }
        }
    }

}


int main(void)
{
//    ptest();

    TimeTaker tt("TOTAL", 1);

    MemCpyTest();

    BitCountTest();

    BitCountSparseTest();

    BitForEachTest();

    WordSelectTest();

    BitTestSparseTest();

    BitCompareTest();

    OptimizeTest();

    FindTest();

    BitBlockRotateTest();

    BitBlockShiftTest();

    EnumeratorTest();

    EnumeratorTestGAP();

    EnumeratorGoToTest();

    BvectorShiftTest();

    RangeCopyTest();

    IntervalsTest();

    AggregatorTest();

    OrTest();

    AndTest();
    XorTest();

    SubTest();

    InvertTest();  

    XorCountTest();
    AndCountTest();

    TI_MetricTest();

    SerializationTest();

    SparseVectorAccessTest();

    SparseVectorScannerTest();

    SparseVectorSerializationTest();

    SparseVectorRangeDeserializationTest();

    RSC_SparseVectorFillTest();

    RSC_SparseVectorAccesTest();

    RankCompressionTest();

    Set2SetTransformTest();

    StrSparseVectorTest();

    return 0;
}


#ifdef _MSC_VER
#pragma warning( pop )
#endif



