#include <stdio.h>

typedef unsigned int Word;

Word Hash[] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19}; //H0

Word POTR(int n, Word x){ return (x >> n) | (x << (32 - n)); } // Rotate n bit
Word SHR(int n, Word x){ return x >> n; }
Word Ch(Word x, Word y, Word z){ return (x & y) ^ (~x & z); }
Word Maj(Word x, Word y, Word z){ return (x & y) ^ (x & z) ^ (y & z); }
Word Usigma0(Word x){ return POTR(2,x) ^ POTR(13,x) ^ POTR(22,x);}
Word Usigma1(Word x){ return POTR(6,x) ^ POTR(11,x) ^ POTR(25,x);}
Word Lsigma0(Word x){ return POTR(7,x) ^ POTR(18,x) ^ SHR(3,x);}
Word Lsigma1(Word x){ return POTR(17,x) ^ POTR(19,x) ^ SHR(10,x);}

void schedule(Word block[16],Word result[64]){
    for(int t=0;t<16;t++){
        result[t] = block[t];
    }
    for(int t=16;t<64;t++){
        result[t] = Lsigma1(result[t-2]) + result[t-7] + Lsigma0(result[t-15]) + result[t-16];
    }
}

typedef struct {
    Word A;
    Word B;
    Word C;
    Word D;
    Word E;
    Word F;
    Word G;
    Word H;
} ABCDEFG;

ABCDEFG ABCDEFG_to_ABCDEFG_v2(ABCDEFG src, Word W[64], int count){
    Word K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

    ABCDEFG dst;
    if(count == 64) return src;
    Word T1 = W[count] + K[count] + src.H + Ch(src.E,src.F,src.G) + Usigma1(src.E);
    Word T2 = Maj(src.A,src.B,src.C) + Usigma0(src.A);
    dst.H = src.G;
    dst.G = src.F;
    dst.F = src.E;
    dst.E = src.D + T1;
    dst.D = src.C;
    dst.C = src.B;
    dst.B = src.A;
    dst.A = T1 + T2;
    return dst;
}

void sha256_v3(Word block[16]){
    Word W[64];
    schedule(block,W); // setup W

    ABCDEFG init;
    init.A = Hash[0];
    init.B = Hash[1];
    init.C = Hash[2];
    init.D = Hash[3];
    init.E = Hash[4];
    init.F = Hash[5];
    init.G = Hash[6];
    init.H = Hash[7];

    for(int i=0;i<64;i++){
        init = ABCDEFG_to_ABCDEFG_v2(init, W, i);
    }

    Hash[0] += init.A;
    Hash[1] += init.B;
    Hash[2] += init.C;
    Hash[3] += init.D;
    Hash[4] += init.E;
    Hash[5] += init.F;
    Hash[6] += init.G;
    Hash[7] += init.H;
}

void byte64_to_word16(unsigned char source[64], Word result[16]){
    for(int i = 0;i<16;i++){
        result[i] = source[i*4] << 24 | source[i*4+1] << 16 | source[i*4+2] << 8 | source[i*4+3];
    }
}

void padding(unsigned char source[64]){
    unsigned long long i;
    for(i=0;source[i] != '\0';i++);

    source[i] = 0x80;

    i *= 8;
    source[56] = (i >> 56) & 0xff;
    source[57] = (i >> 48) & 0xff;
    source[58] = (i >> 40) & 0xff;
    source[59] = (i >> 32) & 0xff;
    source[60] = (i >> 24) & 0xff;
    source[61] = (i >> 16) & 0xff;
    source[62] = (i >> 8)  & 0xff;
    source[63] = (i >> 0)  & 0xff;

}

int main(){
    unsigned char source[64] = "this is a 64byte example strings";
    //unsigned char source[64] = "";
    //unsigned char source[64] = "hello world";

    printf("source = ");
    for(int i = 0;i<64;i++){
        printf("0x%02x ",source[i]);
    }
    printf("\n");

    padding(source);

    printf("padding = ");
    for(int i = 0;i<64;i++){
        printf("0x%02x ",source[i]);
    }
    printf("\n");

    Word block[16];
    byte64_to_word16(source,block);

    sha256_v3(block);
    
    printf("sha256(\"%s\") = \n",source);
    for(int i = 0;i<8;i++){
        printf("%08x ",Hash[i]);
    }

    return 0;
}