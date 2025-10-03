#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <emmintrin.h>   // SSE2
#include <tmmintrin.h>   // SSSE3
#include <immintrin.h>   // AVX2

/*

A couple hours spent with ChatGPT to try and get the SSE2 code working.
Gave up on sse2, but the SSSE3 code is working well.
Going to stick with SSE3.

Update: SSE2 code is now working, thanks to ChatGPT and CoPilot.

gcc -g -mavx2 -msse2 -mssse3 -O3 -O2 -o simdtest simdtest.c
 
*/

#define TESTDATALEN 66  // 32 bytes hex = 64 characters
//#define TESTDATALEN 4096  // 24 bytes hex = 48 characters
unsigned char testdata[TESTDATALEN + 1] = "0123456789ABCDEFabcdef0123456789ABCDEFabcdef0123456789ABCDEFabcdef";
//unsigned char testdata[TESTDATALEN + 1] = "B207501BE86A43D868140F4009547A511915C90D0E86519CFEBCDB4282E933C8321E0966F76384446EDB35F88B67880E713CFFAB04A2AEE4A235871EDC6579C82BA9AC0FCAA1CD75447199C82F9774D58437BEC117E81B1E4B4B076AD98B7F1CA0D511A9FA134191FBBB9C8B04DCE48603DDA69B7FE1CD904BFDAACBEABB986A2FD1DDEBC574C806839988DB791988C442CE362DC5BAF3A51D6AB55F6864A4BB343CA906DBF8BEE7CD5BCF0373645BE30654A2ED6A1A81FA9E17E2845AEF254B03B2EA3708C3DE2BCBB1A6D486B8D62A0D8D4A2FABBF2FAD0BC6447D78A1E0A8FBCDD49771678E3097F7F592960EB24E72B19980EF460990713E1AE19DF42C171C5AD5747124644047C257C2842B32A2F79A6B5BA663FF47F15105BF8B86DC391E757BB997CAEBD96014E23A95090CCECDAFDA2FFEB985A497902AA92A5B5789A97F7A3C32EC9A2AA9D717EB4D34663B275077D63CCDBF006BADFFEABF6B2B5A15E1CEAE02064D8CC1E7E0C1BC9FA78A4ADC7D29266DBF5E2A29B996EF6A20C69EE462C775973B4B8B764AB669D6FF932B5848B92770C5BAEAB7C292813145F297B9FD4E0B4A3E4E04B8B8083C60E02D04184C1B9C9BDB4601E5AA172A840C2797C8892644AC111D5A5F801782969D0B9A6E59356389BCDB8C27FB7B41FA6135E0EBFDBBF14B8A4F3E0509F6236E37F665438E092FDAE8A3DC565FDE2CCA35710AD62B35433DCDC5782217700DDB9B57D2974B8EFC7AF851FD5207A11CCD44A9DCB0610BDB87E87BDC4F67882530D3C61C2C8A07ED2AC3B5C155B701A993F941B37AFBA2CF45D8CD7F9732C2853C4855BF97CFF1BB2EAE9E6C0468D2833FDD2CA8B7FADDADD716FE0CAC5257E7DCD1C0FCC4737AD778F2D2D0BB1A0AC884BFD452118388157B609D24E765B52020DE07BEBBBB22AE394CABD189CE8179FC8F44D5CCB02E06F56CF11C5CA390F3A4BB0B64B5512BD8AC37BEFE62E9F7CA46EBCC84189B45696125288B795470D89B0FBFAC82D068517611743807D9B84D9638532FC403248484F38FE2E145D5EA3B6CDD2D8D8EC455EFFC6ED3E5093EB318171F11CC54F29947C3F181FE488117CC0F51DB0CF2E77FD2002E4E7B9D26D2633A202E3ECA34B13146A73AC735EF477433CE4EB4D4397FEDB6327AE7ED00C6C2A29AD5A63C284F3A56722C6BD22753ACD15FBDC00274608A0ABA81310952B427528DAEE41AF68F9EA3B01FA05585DD0468896CBBD14F5BA9189278B410199B2157E99F420AB8871851322B0D31CC26188A77B57811A3DC9F1E9E224CC74C5B9BEB3EBE8FA5F96E0D7A063DBDDEBC906EAB5F1F2CC308D9E2335D5A8FDA557AB18066A395F171CD5BEE40C47653326C7CBF08939B1DC909C7962A97D9697293DB719FB991AC991FA08A5DC3B876D03467CD88C2C8BD8564689B337A300B0D9F19527F0A9366FDC3568698D8C02D3E45405F8FB4DF91770CACB88970CFAB20AE8F9F0289DC9AE3DCF79A4B515EB1A6DCB8F0CEFB2A65C20386757B2E419F4A2DB940A5E0A78BD7BB5A10DE58CB6E72B9EA83B3F1F7FFC9637245B92F9062E14BDB0394DFBDF3EFC1CB7AC8BE8FC1301980475FC21B50521BD2DB1CBEB1DBFC043442C3A6EFCB84CE73C45D65ED0D1BFC81D4D30C5D726312D0D74447767BEB614E6874D68131FC0B964FE145C7B621FF2DFC137EE40F7F005FA534FD311A34A1B29E90712072F644C3F25F401ABD6337B2F0D68CF6A91DAEDD835C5381FEDE2BB955F3F8C1E2D1D1C9D334E0CA69A65AA45514D78D2E25E568C4E527778AAF11D668B3589D2042542A1B115F28933CB601F1E62E541726D1CF58E33978372DC44C7FA8EA88BACC0ABD513B73AA16865CE1B0C6691F77751FED4AC97A2FD9AFFA22EE48F8BA2C199886114A9BB5494070AB74A07AA73F28BBBE185BCADFD3ECE563AB615310E2670234E316C067E36B37D5F474DB2526DD23ED7A91EB46C4E5C77BDB0F544C3EB05FD17ACEBAACBCFCDF97F5BAFE48DFFD07B1F8AA1765F267AAD3C2C51B43EFF8E9FD9E243370684C0D6CA074A816300140487F602BED36C5DAB57C2C2416A51B7D0D1815292BC7372947246687F22DFEA4EB45A60376B6FAE3008016A601F0A55AEEBF0E73C8764E8D1A1C6D9AC1A9F820D93DD0FA5C0681846F073DE381CA2E060A4E850FC9405D5312747C2F16C611655E54BB97A59F012E23E1AB4D067019B45206880D8CFE0CC08D5F228E3B0517FF47A2B3E96BCED1C0D4AD45AEFA9AEF0A818B68B4B2000BE8B6E9CC56983D485D83EF87DA9CA142BA46D7FA15E1C072BEF52EA6F5779DAB587ED622F1BA45A9CED567C8C2EAC486402AFEFC3B2F6A2525D7838A2285369447BAB45C6604DE78B3B46AA376371C05B23D0A7BCA00D3C1A9C32C9696B2DEFC03BF58E05DFD3B205D80B529E96DBA21BA905A24693F966E57AC17967B60F83D7D9846430523090C93FB4B91143181195934451DF75767ADD91CB8E0F45AF367038CAA1F9A1E38CA8359777CF37C16A820E0D1B544C86C4E054DA01FE57EFBD597F2827DED7EF2124A9FE282BAE377C2E9449C29E31C34B7C9EAB8DFEE67FD96A9537D60420F0548889266935C39A187AB59E312AFD96A30B888CC2251ADA7A5E92E89DBC2582D9CA9038353F313C1B407ADB4BE0961170651C8570F758488B14BC1B158CE7E6EC4898F54212F0B89A85B75022A63C6F584CAB4E7674F1CB8F5F69002F7172EC1A55EF9D68BD1E9168D28774B85D62A63D7AD7CB67E8B1E8C714742891FD10A0814D592854E11F712B20B4A21BA11DB0AD9D4BD4446888445C56B43CC823874D72759BD1FEFF2D86393AB71D2489AE24D040D1962CFF886C4A44DCB9709D260737F3CDB8F76180B02A4D8C3392D";

unsigned char result_ssse3[TESTDATALEN / 2];
unsigned char result_sse2[TESTDATALEN / 2];
unsigned char result_avx2[TESTDATALEN / 2];

void superScalarSSSE3(void);
void superScalarSSE2(void);
void superScalarAVX2_opt(void);

static inline unsigned char toNib(unsigned char c) {
    unsigned char lower = (unsigned char)(c | 0x20);                    // make letters lowercase
    unsigned char isLet = (unsigned)((unsigned)lower - 'a') <= ('f' - 'a'); // 'a'..'f'?
    return (unsigned char)((c & 0x0Fu) + (isLet ? 9u : 0u));            // digit: 0..9, letter: +9 => 10..15
}

void superScalarAVX2_opt(void)
{
    const __m256i lower_mask = _mm256_set1_epi8(0x20);
    const __m256i ch_a_m1    = _mm256_set1_epi8('a' - 1);
    const __m256i ch_f_p1    = _mm256_set1_epi8('f' + 1);
    const __m256i add9       = _mm256_set1_epi8(9);
    const __m256i low_nib    = _mm256_set1_epi8(0x0F);

    const __m256i idxEven = _mm256_setr_epi8(
         0,  2,  4,  6,  8, 10, 12, 14, (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,
         0,  2,  4,  6,  8, 10, 12, 14, (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80
    );
    const __m256i idxOdd  = _mm256_setr_epi8(
         1,  3,  5,  7,  9, 11, 13, 15, (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,
         1,  3,  5,  7,  9, 11, 13, 15, (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80
    );

    size_t i = 0;
    for (; i + 64 <= TESTDATALEN; i += 64) {
        __m256i v0 = _mm256_loadu_si256((const __m256i*)(testdata + i));
        __m256i v1 = _mm256_loadu_si256((const __m256i*)(testdata + i + 32));

        // Shuffle evens/odds per 128-bit lane (each has bytes in positions 0..7 of each lane)
        __m256i e0 = _mm256_shuffle_epi8(v0, idxEven);
        __m256i o0 = _mm256_shuffle_epi8(v0, idxOdd);
        __m256i e1 = _mm256_shuffle_epi8(v1, idxEven);
        __m256i o1 = _mm256_shuffle_epi8(v1, idxOdd);

        // --- Correctly assemble within each 256b load ---
        // From e0: low lane (0..7) then high lane (0..7) shifted into 8..15
        __m128i e0_lo = _mm256_castsi256_si128(e0);
        __m128i e0_hi = _mm256_extracti128_si256(e0, 1);
        __m128i e0_128 = _mm_or_si128(e0_lo, _mm_slli_si128(e0_hi, 8));

        __m128i o0_lo = _mm256_castsi256_si128(o0);
        __m128i o0_hi = _mm256_extracti128_si256(o0, 1);
        __m128i o0_128 = _mm_or_si128(o0_lo, _mm_slli_si128(o0_hi, 8));

        // From e1/o1: same pattern
        __m128i e1_lo = _mm256_castsi256_si128(e1);
        __m128i e1_hi = _mm256_extracti128_si256(e1, 1);
        __m128i e1_128 = _mm_or_si128(e1_lo, _mm_slli_si128(e1_hi, 8));

        __m128i o1_lo = _mm256_castsi256_si128(o1);
        __m128i o1_hi = _mm256_extracti128_si256(o1, 1);
        __m128i o1_128 = _mm_or_si128(o1_lo, _mm_slli_si128(o1_hi, 8));

        // Reassemble two 128b halves into one 256b: [v0-combined | v1-combined]
        __m256i evens = _mm256_inserti128_si256(_mm256_castsi128_si256(e0_128), e1_128, 1);
        __m256i odds  = _mm256_inserti128_si256(_mm256_castsi128_si256(o0_128), o1_128, 1);

        // --- branchless ASCII→nibble ---
        __m256i e_low = _mm256_or_si256(evens, lower_mask);
        __m256i o_low = _mm256_or_si256(odds,  lower_mask);

        __m256i e_is_letter = _mm256_and_si256(
            _mm256_cmpgt_epi8(e_low, ch_a_m1), _mm256_cmpgt_epi8(ch_f_p1, e_low));
        __m256i o_is_letter = _mm256_and_si256(
            _mm256_cmpgt_epi8(o_low, ch_a_m1), _mm256_cmpgt_epi8(ch_f_p1, o_low));

        evens = _mm256_add_epi8(_mm256_and_si256(evens, low_nib),
                                _mm256_and_si256(e_is_letter, add9));
        odds  = _mm256_add_epi8(_mm256_and_si256(odds,  low_nib),
                                _mm256_and_si256(o_is_letter, add9));

        __m256i high = _mm256_slli_epi16(evens, 4);
        __m256i out  = _mm256_or_si256(high, odds);

        _mm256_storeu_si256((__m256i*)(result_avx2 + i/2), out);
    }

    // Scalar tail (remaining < 64 chars)
    for (; i + 1 < TESTDATALEN; i += 2) {
        result_avx2[i/2] = (unsigned char)((toNib(testdata[i]) << 4) | toNib(testdata[i+1]));
    }
}


// this SIMD code is much faster than the  lookup table code (test4)
// this is 10x faster than the hex lookup table

void superScalarSSE2(void)
{
    const __m128i ascii0 = _mm_set1_epi8('0');
    const __m128i adj_uc = _mm_set1_epi8(7);   // 'A'..'F'  => -7 after '0' subtract
    const __m128i adj_lc = _mm_set1_epi8(39);  // 'a'..'f'  => -39 after '0' subtract

    const __m128i A_m1 = _mm_set1_epi8('A' - 1);
    const __m128i F_p1 = _mm_set1_epi8('F' + 1);
    const __m128i a_m1 = _mm_set1_epi8('a' - 1);
    const __m128i f_p1 = _mm_set1_epi8('f' + 1);

    size_t i;
    for (i = 0; i + 32 <= TESTDATALEN; i += 32) {
        const unsigned char* src = testdata + i;

        __m128i chunk1 = _mm_loadu_si128((const __m128i*)(src));
        __m128i chunk2 = _mm_loadu_si128((const __m128i*)(src + 16));

        __m128i lo1 = _mm_unpacklo_epi8(chunk1, _mm_setzero_si128());
        __m128i hi1 = _mm_unpackhi_epi8(chunk1, _mm_setzero_si128());
        __m128i lo2 = _mm_unpacklo_epi8(chunk2, _mm_setzero_si128());
        __m128i hi2 = _mm_unpackhi_epi8(chunk2, _mm_setzero_si128());

        // build evens (b0,b2,...,b30)
        __m128i even_bytes = _mm_setzero_si128();
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(lo1, 0), 0);
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(lo1, 2), 1);
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(lo1, 4), 2);
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(lo1, 6), 3);
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(hi1, 0), 4);
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(hi1, 2), 5);
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(hi1, 4), 6);
        even_bytes = _mm_insert_epi16(even_bytes, _mm_extract_epi16(hi1, 6), 7);

        __m128i even_bytes2 = _mm_setzero_si128();
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(lo2, 0), 0);
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(lo2, 2), 1);
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(lo2, 4), 2);
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(lo2, 6), 3);
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(hi2, 0), 4);
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(hi2, 2), 5);
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(hi2, 4), 6);
        even_bytes2 = _mm_insert_epi16(even_bytes2, _mm_extract_epi16(hi2, 6), 7);

		  // Combine evens from both halves
		  // this line by ChatGPT
		  __m128i evens = _mm_packus_epi16(even_bytes, even_bytes2);

        // build odds (b1,b3,...,b31)
        __m128i odd_bytes = _mm_setzero_si128();
        odd_bytes = _mm_insert_epi16(odd_bytes, _mm_extract_epi16(lo1, 1), 0);
        odd_bytes = _mm_insert_epi16(odd_bytes, _mm_extract_epi16(lo1, 3), 1);
        odd_bytes = _mm_insert_epi16(odd_bytes, _mm_extract_epi16(lo1, 5), 2);
        odd_bytes = _mm_insert_epi16(odd_bytes, _mm_extract_epi16(lo1, 7), 3);
        odd_bytes = _mm_insert_epi16(odd_bytes, _mm_extract_epi16(hi1, 1), 4);
        odd_bytes = _mm_insert_epi16(odd_bytes, _mm_extract_epi16(hi1, 3), 5);
        odd_bytes = _mm_insert_epi16(odd_bytes, _mm_extract_epi16(hi1, 5), 6);
        odd_bytes = _mm_insert_epi16(odd_bytes, _mm_extract_epi16(hi1, 7), 7);

        __m128i odd_bytes2 = _mm_setzero_si128();
        odd_bytes2 = _mm_insert_epi16(odd_bytes2, _mm_extract_epi16(lo2, 1), 0);
        odd_bytes2 = _mm_insert_epi16(odd_bytes2, _mm_extract_epi16(lo2, 3), 1);
        odd_bytes2 = _mm_insert_epi16(odd_bytes2, _mm_extract_epi16(lo2, 5), 2);
        odd_bytes2 = _mm_insert_epi16(odd_bytes2, _mm_extract_epi16(lo2, 7), 3);
        odd_bytes2 = _mm_insert_epi16(odd_bytes2, _mm_extract_epi16(hi2, 1), 4);
        odd_bytes2 = _mm_insert_epi16(odd_bytes2, _mm_extract_epi16(hi2, 3), 5);
        odd_bytes2 = _mm_insert_epi16(odd_bytes2, _mm_extract_epi16(hi2, 5), 6);
        odd_bytes2 = _mm_insert_epi16(odd_bytes2, _mm_extract_epi16(hi2, 7), 7);


		  // Combine odds from both halves
		  __m128i odds = _mm_packus_epi16(odd_bytes, odd_bytes2);

        // --- FIXED DECODE STAGE ---
        // Keep ASCII copies for case detection
        __m128i chars_e = evens;
        __m128i chars_o = odds;

        // Convert ASCII → [0..] by subtracting '0'
        evens = _mm_sub_epi8(evens, ascii0);
        odds  = _mm_sub_epi8(odds,  ascii0);

        // Uppercase mask: 'A' <= char <= 'F'
        __m128i u_e = _mm_and_si128(_mm_cmpgt_epi8(chars_e, A_m1), _mm_cmplt_epi8(chars_e, F_p1));
        __m128i u_o = _mm_and_si128(_mm_cmpgt_epi8(chars_o, A_m1), _mm_cmplt_epi8(chars_o, F_p1));

        // Lowercase mask: 'a' <= char <= 'f'
        __m128i l_e = _mm_and_si128(_mm_cmpgt_epi8(chars_e, a_m1), _mm_cmplt_epi8(chars_e, f_p1));
        __m128i l_o = _mm_and_si128(_mm_cmpgt_epi8(chars_o, a_m1), _mm_cmplt_epi8(chars_o, f_p1));

        // Apply case-specific adjustments **by subtraction**
        evens = _mm_sub_epi8(evens, _mm_and_si128(u_e, adj_uc));
        odds  = _mm_sub_epi8(odds,  _mm_and_si128(u_o, adj_uc));

        evens = _mm_sub_epi8(evens, _mm_and_si128(l_e, adj_lc));
        odds  = _mm_sub_epi8(odds,  _mm_and_si128(l_o, adj_lc));
        // --- END FIX ---

        // Pack nibbles: (high<<4) | low
        __m128i high_nibbles = _mm_slli_epi16(evens, 4);
        __m128i bytes = _mm_or_si128(high_nibbles, odds);

        _mm_storeu_si128((__m128i *)(result_sse2 + i / 2), bytes);
    }

    // Scalar tail
    for (; i + 1 < TESTDATALEN; i += 2) {
        unsigned char high = testdata[i];
        unsigned char low  = testdata[i + 1];

        high = (high >= '0' && high <= '9') ? high - '0' :
               (high >= 'A' && high <= 'F') ? high - 'A' + 10 :
               (high >= 'a' && high <= 'f') ? high - 'a' + 10 : 0;

        low = (low >= '0' && low <= '9') ? low - '0' :
              (low >= 'A' && low <= 'F') ? low - 'A' + 10 :
              (low >= 'a' && low <= 'f') ? low - 'a' + 10 : 0;

        result_sse2[i / 2] = (high << 4) | low;
    }
}



// this SIMD code is much faster than the  lookup table code (lookup64k)
// however, this does not speed up the clob converion much, as most time is spent in the database
void superScalarSSSE3(void)
{
    strcpy((char *)result_ssse3, "\0");

    size_t i;
    for (i = 0; i + 32 <= TESTDATALEN; i += 32) {
        // Load 32 hex characters into two registers
        __m128i block1 = _mm_loadu_si128((const __m128i *)(testdata + i));
        __m128i block2 = _mm_loadu_si128((const __m128i *)(testdata + i + 16));

        // Create shuffle masks for extracting even and odd indices
        // 0x80 indicates that byte is not used (masked out)
        __m128i idxEven = _mm_setr_epi8(
            0,  2,  4,  6,  8, 10, 12, 14,
            (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80
        );
        __m128i idxOdd = _mm_setr_epi8(
            1,  3,  5,  7,  9, 11, 13, 15,
            (char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80,(char)0x80
        );

        // Extract even and odd bytes from block1 and block2
        __m128i evens_block1 = _mm_shuffle_epi8(block1, idxEven); // even indices from first 16 chars
        __m128i odds_block1  = _mm_shuffle_epi8(block1, idxOdd);  // odd indices from first 16 chars
        __m128i evens_block2 = _mm_shuffle_epi8(block2, idxEven); // even indices from next 16 chars
        __m128i odds_block2  = _mm_shuffle_epi8(block2, idxOdd);  // odd indices from next 16 chars

        // Combine evens from block1 and block2 into one 128-bit register
        // evens_block1 has 8 valid bytes at the start
        // evens_block2 has 8 valid bytes at the start, we shift it left by 8 bytes and OR
        __m128i evens = _mm_or_si128(evens_block1, _mm_slli_si128(evens_block2, 8));

        // Combine odds similarly
        __m128i odds = _mm_or_si128(odds_block1, _mm_slli_si128(odds_block2, 8));

        // Now we have:
        // evens = [b0, b2, b4, b6, b8, b10, b12, b14, b16, b18, b20, b22, b24, b26, b28, b30]
        // odds  = [b1, b3, b5, b7, b9, b11, b13, b15, b17, b19, b21, b23, b25, b27, b29, b31]

        // Convert ASCII chars to nibbles
        __m128i zero = _mm_set1_epi8('0');

        // Subtract '0'
        evens = _mm_sub_epi8(evens, zero);
        odds  = _mm_sub_epi8(odds, zero);

        // Masks for uppercase and lowercase
        __m128i upperA = _mm_set1_epi8('A'-1);
        __m128i upperF = _mm_set1_epi8('F'+1);
        __m128i lowerA = _mm_set1_epi8('a'-1);
        __m128i lowerF = _mm_set1_epi8('f'+1);

        __m128i chars_evens = _mm_add_epi8(evens, zero); // convert back to ASCII space
        __m128i chars_odds  = _mm_add_epi8(odds, zero);

        __m128i ucase_mask_e = _mm_and_si128(_mm_cmpgt_epi8(chars_evens, upperA), _mm_cmplt_epi8(chars_evens, upperF));
        __m128i lcase_mask_e = _mm_and_si128(_mm_cmpgt_epi8(chars_evens, lowerA), _mm_cmplt_epi8(chars_evens, lowerF));
        __m128i ucase_mask_o = _mm_and_si128(_mm_cmpgt_epi8(chars_odds,  upperA), _mm_cmplt_epi8(chars_odds,  upperF));
        __m128i lcase_mask_o = _mm_and_si128(_mm_cmpgt_epi8(chars_odds,  lowerA), _mm_cmplt_epi8(chars_odds,  lowerF));

        // For uppercase: subtract additional 7
        evens = _mm_sub_epi8(evens, _mm_and_si128(ucase_mask_e, _mm_set1_epi8(7)));
        odds  = _mm_sub_epi8(odds,  _mm_and_si128(ucase_mask_o, _mm_set1_epi8(7)));

        // For lowercase: subtract additional 39
        evens = _mm_sub_epi8(evens, _mm_and_si128(lcase_mask_e, _mm_set1_epi8(39)));
        odds  = _mm_sub_epi8(odds,  _mm_and_si128(lcase_mask_o, _mm_set1_epi8(39)));

        // Now evens are high nibbles, odds are low nibbles
        __m128i high_shifted = _mm_slli_epi16(evens, 4);
        __m128i bytes = _mm_or_si128(high_shifted, odds);

        // Store result
        _mm_storeu_si128((__m128i *)(result_ssse3 + i/2), bytes);
    }

    // Process any remaining characters (less than 32)
    for (; i < TESTDATALEN; i += 2) {
        unsigned char high = testdata[i];
        unsigned char low = testdata[i + 1];

        high = (high >= '0' && high <= '9') ? high - '0' :
               (high >= 'A' && high <= 'F') ? high - 'A' + 10 :
               (high >= 'a' && high <= 'f') ? high - 'a' + 10 : 0;

        low = (low >= '0' && low <= '9') ? low - '0' :
              (low >= 'A' && low <= 'F') ? low - 'A' + 10 :
              (low >= 'a' && low <= 'f') ? low - 'a' + 10 : 0;

        result_ssse3[i / 2] = (high << 4) | low;
    }
}

void print_result(const char *label, const unsigned char *data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; ++i) {
        printf("%02X", data[i]);
    }
    printf("\n");
}

int main(void) {
    // Initialize output buffers
    memset(result_ssse3, 0, sizeof(result_ssse3));
    memset(result_sse2, 0, sizeof(result_sse2));
    memset(result_avx2, 0, sizeof(result_avx2));

    // Convert
    superScalarSSSE3();
    superScalarSSE2();
    superScalarAVX2_opt();

    // Show output
    print_result("   SSSE3", result_ssse3, TESTDATALEN / 2);
    print_result("    SSE2", result_sse2, TESTDATALEN / 2);
    print_result(" SE2AVX2", result_avx2, TESTDATALEN / 2);

    // Compare
    int diff = 0;
    for (size_t i = 0; i < TESTDATALEN / 2; ++i) {

		 // this printf shows side by side comparison of all three results
		  //printf("Compare SSSE3/SSE2/AVX2 byte %zu: SSSE3=0x%02X SSE2=0x%02X AVX2=0x%02X\n",
					//i, result_ssse3[i], result_sse2[i], result_avx2[i]);
		  /*
        if (result_ssse3[i] != result_sse2[i]) {
            printf("Mismatch at byte %zu: SSSE3=0x%02X SSE2=0x%02X\n",
                   i, result_ssse3[i], result_sse2[i]);
            diff++;
        }
		  */

        if (result_ssse3[i] != result_avx2[i]) {
            printf("Mismatch at byte %zu: SSSE3=0x%02X AVX2=0x%02X\n",
                   i, result_ssse3[i], result_avx2[i]);
            diff++;
        }
    }

	 /*
    if (!diff) {
        printf("✅ Outputs match!\n");
    } else {
        printf("❌ %d mismatches found\n", diff);
    }
	*/

    return diff;
}
