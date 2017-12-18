#include <stdio.h>
#include <string.h>

typedef unsigned long long word;
typedef unsigned char byte;
// Initialization vector
word IV[8] = {
	0x6a09e667f3bcc908, 0xbb67ae8584caa73b, 0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,
	0x510e527fade682d1, 0x9b05688c2b3e6c1f, 0x1f83d9abfb41bd6b, 0x5be0cd19137e2179
};
// Constant words
word K[80] = {
	0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc, 0x3956c25bf348b538,
	0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118, 0xd807aa98a3030242, 0x12835b0145706fbe,
	0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2, 0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235,
	0xc19bf174cf692694, 0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,
	0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5, 0x983e5152ee66dfab,
	0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4, 0xc6e00bf33da88fc2, 0xd5a79147930aa725,
	0x06ca6351e003826f, 0x142929670a0e6e70, 0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed,
	0x53380d139d95b3df, 0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,
	0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30, 0xd192e819d6ef5218,
	0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8, 0x19a4c116b8d2d0c8, 0x1e376c085141ab53,
	0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8, 0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373,
	0x682e6ff3d6b2b8a3, 0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
	0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b, 0xca273eceea26619c,
	0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178, 0x06f067aa72176fba, 0x0a637dc5a2c898a6,
	0x113f9804bef90dae, 0x1b710b35131c471b, 0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc,
	0x431d67c49c100d4c, 0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817
};
// Read data from text file
void readFromFile(char* filePath, char* data){
	FILE *file = fopen(filePath, "rt");
	strcpy(data, "");
	char tmp[1000];
	while(!feof(file)){
		fgets(tmp, 1000, file);
		strcat(data, tmp);
	}
	fclose(file);
}
// Add padding and length to get multiple of 1024 bits
void setBlock(byte* mess, int messLen, word* block, int* numBlock) {	
	*numBlock = (messLen + 16) / 128 + 1;
	int len = *numBlock * 16;
	int i, j;
	int isEnd = 0;
	for(i = 0; i < len - 2; i++) {
		block[i] = 0;
		if(isEnd == 0)
			for(j = 0; j < 8; j++) {
				block[i] <<= 8;
				if((i * 8 + j) < messLen) {
					block[i] += mess[i * 8 + j];
				} else {
					if(isEnd == 0) {
						block[i] += 0x80;
						isEnd = 1;
					}
				}
			}
	}
	block[len - 2] = 0;
	block[len - 1] = (word) messLen * 8;
}
// Copy data from array 'in' to array 'out'
void copyData(word* in, int len, word* out) {			
	int i;
	for(i = 0; i < len; i++)
		out[i] = in[i];
}
// Right rotation by 'n' bits
word rotR(word in, int step) {							
	int i;
	for(i = 0; i < step; i++)
		in = (in >> 1) + ((in & 1) << 63);
	return in;
}
// Generate words W[i]: i = 0:7
void wordExpansion(word* data, int startPos, word* W) {		
	int i;
	for(i = 0; i < 16; i++) W[i] = data[i + startPos];
	for(i = 16; i < 80; i++) {
		W[i] = (rotR(W[i - 2], 19) ^ rotR(W[i - 2], 61) ^ (W[i - 2] >> 6)) + W[i - 7] +
		       (rotR(W[i - 15], 1) ^ rotR(W[i - 15], 8) ^ (W[i - 15] >> 7)) + W[i - 16];
	}
}
// 1 round
void round(int id, word* in, word W, word* out) {	
	int i;
	for(i = 1; i < 8; i++) {
		if(i != 4) {
			out[i] = in[i - 1];
		}
	}
	word t1 = ((in[0] & in[1]) ^ (in[1] & in[2]) ^ (in[2] & in[0])) + (rotR(in[0], 28) ^ rotR(in[0], 34) ^ rotR(in[0], 39));
	word t2 = ((in[4] & in[5]) ^ ((~in[4]) & in[6])) + (rotR(in[4], 14) ^ rotR(in[4], 18) ^ rotR(in[4], 41)) + W + K[id] + in[7];
	out[4] = in[3] + t2;
	out[0] = t1 + t2;
}
// Compression function
void compressionFunc(word* init, word* W, word* res){	
	int i;
	round(0, init, W[0], res);
	for(i = 1; i < 80; i++){
		word tmp[8];
		round(i, res, W[i], tmp);
		copyData(tmp, 8, res);
	}
	for(i = 0; i < 8; i++){
		res[i] = res[i] + init[i];
	}
}
// Implement Hash Algorithm
void implement(word* data, int numBlock, word* out) {	
	int block;
	word res[8];
	for(block = 0; block < numBlock; block++) {
		word W[80];
		wordExpansion(data, 16 * block , W);
		word init[8];
		if(block == 0)
			copyData(IV, 8, init);
		else
			copyData(res, 8, init);
		compressionFunc(init, W, res);
	}
	copyData(res, 8, out);
}
// Main program
int main() {
	byte mess[100000];
	readFromFile("D:/testLTMM/JuliusCaesar.txt", mess);
	word data[2000];
	int numBlock;
	setBlock(mess, strlen(mess), data, &numBlock);
	word res[8];
	implement(data, numBlock, res);
	int i;
	printf("Hash value:\n");
	for(i = 0; i < 8; i++){
		printf("%016llX",res[i]);	
	}
	return 0;
}
