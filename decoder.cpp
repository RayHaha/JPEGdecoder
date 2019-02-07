#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdarg.h> 
#include <vector>
#include <queue>
#include <stdlib.h>
#pragma once


using namespace std;


#define W1 5681 /* 4096*sqrt(2)*cos(1*pi/16) */
#define W2 5352 /* 4096*sqrt(2)*cos(2*pi/16) */
#define W3 4816 /* 4096*sqrt(2)*cos(3*pi/16) */
#define W5 3218 /* 4096*sqrt(2)*cos(5*pi/16) */
#define W6 2217 /* 4096*sqrt(2)*cos(6*pi/16) */
#define W7 1130 /* 4096*sqrt(2)*cos(7*pi/16) */

static long IDCTTemp[8][8];
static int iclip[1024];
static int *iclp;

static int ZigZagArray[64] = 
{
	0,   1,   5,  6,   14,  15,  27,  28,
	2,   4,   7,  13,  16,  26,  29,  42,
	3,   8,  12,  17,  25,  30,  41,  43,
	9,   11, 18,  24,  31,  40,  44,  53,
	10,  19, 23,  32,  39,  45,  52,  54,
	20,  22, 33,  38,  46,  51,  55,  60,
	21,  34, 37,  47,  50,  56,  59,  61,
	35,  36, 48,  49,  57,  58,  62,  63,
};

queue<char> TheBit;
vector<int> CodeWeightDC0;
vector<int> CodeWeightDC1;
vector<int> CodeWeightAC0;
vector<int> CodeWeightAC1;
vector<vector<char> > CodeWordDC0;
vector<vector<char> > CodeWordDC1;
vector<vector<char> > CodeWordAC0;
vector<vector<char> > CodeWordAC1;
int data[64];
int LastDCY = 0;
int LastDCCb = 0;
int LastDCCr = 0;
vector<vector<int> > QuantizationVector;
// Start of frame
vector<int> FrameVector;

int MCU_Y[64*4];
int MCU_Cb[64];
int MCU_Cr[64];

unsigned char * m_colourspace;
unsigned char * m_rgb;

vector<vector<char> > BuildHuffmanTableCodeWord(vector<int> v){
	int CodeLengthNumber[16];
	for(int i=0; i<16; i++){
		CodeLengthNumber[i] = v[i+3];
	}
	vector<int> CodeLength;
	// get the code length
	for(int i=0; i<16; i++){
		int count = 0;
		while(count!=CodeLengthNumber[i]){
			CodeLength.push_back(i+1);
			count++;
		}
	}
	vector<vector<char> > CodeWord;
	CodeWord.resize(CodeLength.size());
	// get table
	vector<char> BeforeReverse;
	int NowCode = 0;
	for(int i=0; i<CodeLength.size(); i++){
		if(NowCode==0){
			for(int j=0; j<CodeLength[i]; j++){
				CodeWord[i].push_back('0');
			}
		}else{
			int CodeToCal = NowCode;
			while(CodeToCal!=0){
				int Remain = CodeToCal % 2;
				if(Remain==0){
					BeforeReverse.push_back('0');
				}else{
					BeforeReverse.push_back('1');
				}
				CodeToCal = CodeToCal / 2;
			}
			while(BeforeReverse.size()!=CodeLength[i]){
				BeforeReverse.push_back('0');
			}

			while(!BeforeReverse.empty()){
				char c = BeforeReverse[BeforeReverse.size()-1];
				BeforeReverse.pop_back();
				CodeWord[i].push_back(c);
			}
		}

		// calculate NowCode
		if(i!=CodeLength.size()-1){
			int l = CodeLength[i+1] - CodeLength[i];
			NowCode = NowCode + 1;
			if(l!=0){
				NowCode = NowCode * (pow(2,l));
			}
		}
	}
	return CodeWord;
}

vector<int> BuildHuffmanTableCodeWeight(vector<int> v){
	int CodeLengthNumber[16];
	for(int i=0; i<16; i++){
		CodeLengthNumber[i] = v[i+3];
	}
	vector<int> CodeLength;
	// get the code length
	for(int i=0; i<16; i++){
		int count = 0;
		while(count!=CodeLengthNumber[i]){
			CodeLength.push_back(i+1);
			count++;
		}
	}
	vector<int> CodeWeight;
	for(int i=0; i<CodeLength.size(); i++){
		CodeWeight.push_back(v[i+19]);
	}
	return CodeWeight;
}

/*void BuildQuantizationTable(vector<int> v){
	if(v.empty()){	// v is empty

	}else{

	}
}*/


void ProcessHuffmanData0(){
	for(int i=0; i<64; i++){
		data[i] = 0;
	}

	// DC
	vector<char> Code;
	vector<char> Test;
	int FoundIndex = -1;
	for(int k=0; k<16; k++){
		if(!TheBit.empty()){
			char c = TheBit.front();
			TheBit.pop();
			Code.push_back(c);
			Test.push_back(c);
			for(int i=0; i<CodeWordDC0.size(); i++){
				int RightCount = 0;
				if(Code.size()==CodeWordDC0[i].size()){
					for(int j=0; j<CodeWordDC0[i].size(); j++){
						if(Code[j]==CodeWordDC0[i][j]){
							RightCount++;
							//cout<<RightCount<<" "<<Code.size()<<endl;
						}
					}
					if(RightCount==Code.size()){
						FoundIndex = i;
						k = 16;
						i = CodeWordDC0.size();
					}
				}
			}	
		}else{
			FoundIndex = -2;	// TheBit is empty
		}
		
	}
	Code.clear();
	int Weight = 0;
	if(FoundIndex==-2){

	}else if(FoundIndex==-1){
		cout<<"FailDC0"<<endl;
		for(int i=0; i<Test.size(); i++){
			cout<<Test[i];
		}
		cout<<""<<endl;
		Test.clear();
	}else{
		Test.clear();
		Weight = CodeWeightDC0[FoundIndex];		// get the weight value
		//cout<<Weight<<endl;
	}
	if(FoundIndex==-2){

	}else{
		// load the next weight bits
		for(int k=0; k<Weight; k++){
			char c = TheBit.front();
			TheBit.pop();
			Code.push_back(c);
		}
		// calculate the value
		int DCValue = 0;
		if(Code[0]=='0'){
			for(int i=0; i<Code.size(); i++){
				int exponent = Code.size() - i - 1;
				if(Code[i]=='0'){
					DCValue = DCValue + pow(2,exponent);
				}
			}
			DCValue = DCValue * (-1);
		}else{
			for(int i=0; i<Code.size(); i++){
				int exponent = Code.size() - i - 1;
				if(Code[i]=='1'){
					DCValue = DCValue + pow(2,exponent);
				}
			}
		}
		//cout<<DCValue<<endl;
		LastDCY = LastDCY + DCValue;
		data[0] = LastDCY;
	}
	

	// AC
	int ACCount = 1;
	bool EOB_Found = false;
	int DataIndex = 1;
	while((ACCount<=63) && (!EOB_Found)){
		Code.clear();
		Test.clear();
		FoundIndex = -1;
		for(int k=0; k<16; k++){
			if(!TheBit.empty()){
				char c = TheBit.front();
				TheBit.pop();
				Code.push_back(c);
				Test.push_back(c);
				for(int i=0; i<CodeWordAC0.size(); i++){
					int RightCount = 0;
					if(Code.size()==CodeWordAC0[i].size()){
						for(int j=0; j<CodeWordAC0[i].size(); j++){
							if(Code[j]==CodeWordAC0[i][j]){
								RightCount++;
								//cout<<RightCount<<" "<<Code.size()<<endl;
							}
						}
						if(RightCount==Code.size()){
							FoundIndex = i;
							k = 16;
							i = CodeWordAC0.size();
						}
					}
				}
			}else{
				FoundIndex = -2;
			}
			
		}
		Code.clear();
		Weight = 0;
		if(FoundIndex==-2){

		}else if(FoundIndex==-1){
			cout<<"FailAC0"<<endl;
			for(int i=0; i<Test.size(); i++){
				cout<<Test[i];
			}
			cout<<""<<endl;
			Test.clear();
		}else{
			Test.clear();
			Weight = CodeWeightAC0[FoundIndex];		// get the weight value
			//cout<<Weight<<endl;
		}
		if(FoundIndex==-2){
			EOB_Found = true;
		}else{
			// end or not
			if(Weight==0x00){
				EOB_Found = true;
			}else{
				// AC weight has two part
				int NumOfZero = Weight / 16;
				int BitToLoad = Weight % 16;
				// load the next weight bits
				for(int k=0; k<BitToLoad; k++){
					char c = TheBit.front();
					TheBit.pop();
					Code.push_back(c);
				}
				int ACValue = 0;
				if(Code[0]=='0'){
					for(int i=0; i<Code.size(); i++){
						int exponent = Code.size() - i - 1;
						if(Code[i]=='0'){
							ACValue = ACValue + pow(2,exponent);
						}
					}
					ACValue = ACValue * (-1);
				}else{
					for(int i=0; i<Code.size(); i++){
						int exponent = Code.size() - i - 1;
						if(Code[i]=='1'){
							ACValue = ACValue + pow(2,exponent);
						}
					}
				}
				//cout<<NumOfZero<<endl;
				//cout<<ACValue<<endl;

				for(int i=DataIndex; i<DataIndex+NumOfZero; i++){
					data[DataIndex] = 0;
				}
				DataIndex = DataIndex + NumOfZero;
				ACCount = ACCount + NumOfZero;

				data[DataIndex] = ACValue;
				DataIndex++;
			}
		}

		ACCount++;
	}

	/*for(int i=0; i<64; i++){
		cout<<data[i]<<" ";
	}*/

}

void ProcessHuffmanData1(int mode){
	for(int i=0; i<64; i++){
		data[i] = 0;
	}

	// DC
	vector<char> Code;
	vector<char> Test;
	int FoundIndex = -1;
	for(int k=0; k<16; k++){
		if(!TheBit.empty()){
			char c = TheBit.front();
			TheBit.pop();
			Code.push_back(c);
			Test.push_back(c);
			for(int i=0; i<CodeWordDC1.size(); i++){
				int RightCount = 0;
				if(Code.size()==CodeWordDC1[i].size()){
					for(int j=0; j<CodeWordDC1[i].size(); j++){
						if(Code[j]==CodeWordDC1[i][j]){
							RightCount++;
							//cout<<RightCount<<" "<<Code.size()<<endl;
						}
					}
					if(RightCount==Code.size()){
						FoundIndex = i;
						k = 16;
						i = CodeWordDC1.size();
					}
				}
			}	
		}else{
			FoundIndex = -2;	// TheBit is empty
		}
		
	}
	Code.clear();
	int Weight = 0;
	if(FoundIndex==-2){

	}else if(FoundIndex==-1){
		cout<<"FailDC1"<<endl;
		for(int i=0; i<Test.size(); i++){
			cout<<Test[i];
		}
		cout<<""<<endl;
		Test.clear();
	}else{
		Test.clear();
		Weight = CodeWeightDC1[FoundIndex];		// get the weight value
		//cout<<Weight<<endl;
	}
	if(FoundIndex==-2){

	}else{
		// load the next weight bits
		for(int k=0; k<Weight; k++){
			char c = TheBit.front();
			TheBit.pop();
			Code.push_back(c);
		}
		// calculate the value
		int DCValue = 0;
		if(Code[0]=='0'){
			for(int i=0; i<Code.size(); i++){
				int exponent = Code.size() - i - 1;
				if(Code[i]=='0'){
					DCValue = DCValue + pow(2,exponent);
				}
			}
			DCValue = DCValue * (-1);
		}else{
			for(int i=0; i<Code.size(); i++){
				int exponent = Code.size() - i - 1;
				if(Code[i]=='1'){
					DCValue = DCValue + pow(2,exponent);
				}
			}
		}
		//cout<<DCValue<<endl;
		if(mode==1){
			//Cb
			LastDCCb = LastDCCb + DCValue;
			data[0] = LastDCCb;
		}else{
			//Cr
			LastDCCr = LastDCCr + DCValue;
			data[0] = LastDCCr;
		}
	}
	

	// AC
	int ACCount = 1;
	bool EOB_Found = false;
	int DataIndex = 1;
	while((ACCount<=63) && (!EOB_Found)){
		Code.clear();
		Test.clear();
		FoundIndex = -1;
		for(int k=0; k<16; k++){
			if(!TheBit.empty()){
				char c = TheBit.front();
				TheBit.pop();
				Code.push_back(c);
				Test.push_back(c);
				for(int i=0; i<CodeWordAC1.size(); i++){
					int RightCount = 0;
					if(Code.size()==CodeWordAC1[i].size()){
						for(int j=0; j<CodeWordAC1[i].size(); j++){
							if(Code[j]==CodeWordAC1[i][j]){
								RightCount++;
								//cout<<RightCount<<" "<<Code.size()<<endl;
							}
						}
						if(RightCount==Code.size()){
							FoundIndex = i;
							k = 16;
							i = CodeWordAC1.size();
						}
					}
				}
			}else{
				FoundIndex = -2;
			}
			
		}
		Code.clear();
		Weight = 0;
		if(FoundIndex==-2){

		}else if(FoundIndex==-1){
			cout<<"FailAC1"<<endl;
			for(int i=0; i<Test.size(); i++){
				cout<<Test[i];
			}
			cout<<""<<endl;
			Test.clear();
		}else{
			Test.clear();
			Weight = CodeWeightAC1[FoundIndex];		// get the weight value
			//cout<<Weight<<endl;
		}
		if(FoundIndex==-2){
			EOB_Found = true;
		}else{
			// end or not
			if(Weight==0x00){
				EOB_Found = true;
			}else{
				// AC weight has two part
				int NumOfZero = Weight / 16;
				int BitToLoad = Weight % 16;
				// load the next weight bits
				for(int k=0; k<BitToLoad; k++){
					char c = TheBit.front();
					TheBit.pop();
					Code.push_back(c);
				}
				int ACValue = 0;
				if(Code[0]=='0'){
					for(int i=0; i<Code.size(); i++){
						int exponent = Code.size() - i - 1;
						if(Code[i]=='0'){
							ACValue = ACValue + pow(2,exponent);
						}
					}
					ACValue = ACValue * (-1);
				}else{
					for(int i=0; i<Code.size(); i++){
						int exponent = Code.size() - i - 1;
						if(Code[i]=='1'){
							ACValue = ACValue + pow(2,exponent);
						}
					}
				}
				//cout<<NumOfZero<<endl;
				//cout<<ACValue<<endl;

				for(int i=DataIndex; i<DataIndex+NumOfZero; i++){
					data[DataIndex] = 0;
				}
				DataIndex = DataIndex + NumOfZero;
				ACCount = ACCount + NumOfZero;

				data[DataIndex] = ACValue;
				DataIndex++;
			}
		}

		ACCount++;
	}

	/*for(int i=0; i<64; i++){
		cout<<data[i]<<" ";
	}*/

}

void init_idct(){
	int i;
	iclp = iclip+512;
	for (i= -512; i<512; i++)
		iclp[i] = (i<-256) ? -256 : ((i>255) ? 255 : i);
	return;
}

void idct_row(int src[8], long dst[8]){
	long x0, x1, x2, x3, x4, x5, x6, x7, x8;
	/* shortcut */
	if (!src[0] && !src[1] && !src[2] && !src[3] &&
		!src[4] && !src[5] && !src[6] && !src[7]){
		dst[0]=dst[1]=dst[2]=dst[3]=dst[4]=dst[5]=dst[6]=dst[7]=0;
		return;
	}
	/* first stage */
	x0 = src[0];
	x1 = src[4];
	x2 = src[6];
	x3 = src[2];
	x4 = src[1];
	x5 = src[7];
	x6 = src[5];
	x7 = src[3];
	x8 = W7*(x4+x5);
	x4 = x8 + (W1-W7)*x4;
	x5 = x8 - (W1+W7)*x5;
	x8 = W3*(x6+x7);
	x6 = x8 - (W3-W5)*x6;
	x7 = x8 - (W3+W5)*x7;
	/* second stage */
	x8 = ((x0+x1)<<12) + 16; /* +16 for proper rounding in the fourth stage */
	x0 = ((x0-x1)<<12) + 16;
	x1 = W6*(x3+x2);
	x2 = x1 - (W2+W6)*x2;
	x3 = x1 + (W2-W6)*x3;
	x1 = x4 + x6;
	x4 -= x6;
	x6 = x5 + x7;
	x5 -= x7;
	/* third stage */
	x7 = x8 + x3;
	x8 -= x3;
	x3 = x0 + x2;
	x0 -= x2;
	x2 = (181*(x4+x5)+128)>>8;
	x4 = (181*(x4-x5)+128)>>8;
	/* fourth stage */
	dst[0] = (x7+x1)>>5;
	dst[1] = (x3+x2)>>5;
	dst[2] = (x0+x4)>>5;
	dst[3] = (x8+x6)>>5;
	dst[4] = (x8-x6)>>5;
	dst[5] = (x0-x4)>>5;
	dst[6] = (x3-x2)>>5;
	dst[7] = (x7-x1)>>5;
}

void idct_col(int dst[8][8], long src[8][8], int colIndex){
	long x0, x1, x2, x3, x4, x5, x6, x7, x8;
	/* first stage */
	x0 = src[0][colIndex];
	x1 = src[4][colIndex];
	x2 = src[6][colIndex];
	x3 = src[2][colIndex];
	x4 = src[1][colIndex];
	x5 = src[7][colIndex];
	x6 = src[5][colIndex];
	x7 = src[3][colIndex];
	x8 = W7*(x4+x5) + 2048;
	x4 = (x8+(W1-W7)*x4)>>12;
	x5 = (x8-(W1+W7)*x5)>>12;
	x8 = W3*(x6+x7) + 2048;
	x6 = (x8-(W3-W5)*x6)>>12;
	x7 = (x8-(W3+W5)*x7)>>12;
	/* second stage */
	x8 = x0 + x1;
	x0 -= x1;
	x1 = W6*(x3+x2) + 2048;
	x2 = (x1-(W2+W6)*x2)>>12;
	x3 = (x1+(W2-W6)*x3)>>12;
	x1 = x4 + x6;
	x4 -= x6;
	x6 = x5 + x7;
	x5 -= x7;
	/* third stage */
	x7 = x8 + x3 + 512;
	x8 += -x3 + 512;
	x3 = x0 + x2 + 512;
	x0 += -x2 + 512;
	x2 = (181*(x4+x5)+128)>>8;
	x4 = (181*(x4-x5)+128)>>8;
	/* fourth stage */
	dst[0][colIndex] = iclp[(x7+x1)>>10];
	dst[1][colIndex] = iclp[(x3+x2)>>10];
	dst[2][colIndex] = iclp[(x0+x4)>>10];
	dst[3][colIndex] = iclp[(x8+x6)>>10];
	dst[4][colIndex] = iclp[(x8-x6)>>10];
	dst[5][colIndex] = iclp[(x0-x4)>>10];
	dst[6][colIndex] = iclp[(x3-x2)>>10];
	dst[7][colIndex] = iclp[(x7-x1)>>10];
}

void idct(int block[8][8]){
	int i, j;
	for(i=0; i<8; i++){
		idct_row(block[i], IDCTTemp[i]);
	}
	for(i=0; i<8; i++){
		idct_col(block, IDCTTemp, i);
	}
	return;
}

inline void WriteBMP24(const char* szBmpFileName, int Width, int Height, unsigned char* RGB)
{
	#pragma pack(1)
	struct stBMFH // BitmapFileHeader & BitmapInfoHeader
	{
		// BitmapFileHeader
		char         bmtype[2];     // 2 bytes - 'B' 'M'
		unsigned int iFileSize;     // 4 bytes
		short int    reserved1;     // 2 bytes
		short int    reserved2;     // 2 bytes
		unsigned int iOffsetBits;   // 4 bytes
		// End of stBMFH structure - size of 14 bytes
		// BitmapInfoHeader
		unsigned int iSizeHeader;    // 4 bytes - 40
		unsigned int iWidth;         // 4 bytes
		unsigned int iHeight;        // 4 bytes
		short int    iPlanes;        // 2 bytes
		short int    iBitCount;      // 2 bytes
		unsigned int Compression;    // 4 bytes
		unsigned int iSizeImage;     // 4 bytes
		unsigned int iXPelsPerMeter; // 4 bytes
		unsigned int iYPelsPerMeter; // 4 bytes
		unsigned int iClrUsed;       // 4 bytes
		unsigned int iClrImportant;  // 4 bytes
		// End of stBMIF structure - size 40 bytes
		// Total size - 54 bytes
	};
	#pragma pack()

	// Round up the width to the nearest DWORD boundary
	int iNumPaddedBytes = 4 - (Width * 3) % 4;
	iNumPaddedBytes = iNumPaddedBytes % 4;

	stBMFH bh;
	memset(&bh, 0, sizeof(bh));
	bh.bmtype[0]='B';
	bh.bmtype[1]='M';
	bh.iFileSize = (Width*Height*3) + (Height*iNumPaddedBytes) + sizeof(bh);
	bh.iOffsetBits = sizeof(stBMFH);
	bh.iSizeHeader = 40;
	bh.iPlanes = 1;
	bh.iWidth = Width;
	bh.iHeight = Height;
	bh.iBitCount = 24;


	char temp[1024]={0};
	sprintf(temp, "%s", szBmpFileName);
	FILE* fp = fopen(temp, "wb");
	fwrite(&bh, sizeof(bh), 1, fp);
	for (int y=Height-1; y>=0; y--)
	{
		for (int x=0; x<Width; x++)
		{
			int i = (x + (Width)*y) * 3;
			unsigned int rgbpix = (RGB[i]<<16)|(RGB[i+1]<<8)|(RGB[i+2]<<0);
			fwrite(&rgbpix, 3, 1, fp);
		}
		if (iNumPaddedBytes>0)
		{
			unsigned char pad = 0;
			fwrite(&pad, iNumPaddedBytes, 1, fp);
		}
	}
	fclose(fp);
}

void DecodeSingleBlock(int stride, int offset, int mode, int TableNumber){

	/*for(int i=0; i<64; i++){
		if(i%8==0){
			cout<<""<<endl;
		}
		cout<<data[i]<<" ";
		
	}
	cout<<""<<endl;*/

	// Dequantization
	for(int k=0; k<64; k++){
		data[k] = (int)(data[k] * QuantizationVector[TableNumber][k+3]);
	}

	// DeZigZag
	int DeZZdata[64];
	for(int i=0; i<64; i++){
		DeZZdata[i] = 0;
	}
	for(int k=0; k<64; k++){
		DeZZdata[k] = data[ZigZagArray[k]];
	}

	


	// Transform Array
	int BlockArray[8][8];
	int count = 0;
	for( int y=0; y<8; y++){
		for( int x=0; x<8; x++){
			BlockArray[x][y]  =  DeZZdata[count];
			count++;
		}
	}

	// IDCT
	idct(BlockArray);

	/*for(int i=0; i<8; i++){
		for(int j=0; j<8; j++){
			cout<<BlockArray[j][i]<<" ";
		}
		cout<<""<<endl;
	}
	cout<<""<<endl;*/

	// shift
	int MCUIndex = offset;
	for(int y=0; y<8; y++){
		for(int x=0; x<8; x++){
			MCUIndex = MCUIndex + x;
			BlockArray[x][y] = BlockArray[x][y] + 128;
			if(BlockArray[x][y] > 255){
				BlockArray[x][y] = 255;
			}
			if(BlockArray[x][y] < 0){
				BlockArray[x][y] = 0;
			}
			if(mode==0){
				MCU_Y[MCUIndex] = BlockArray[x][y];
			}else if(mode==1){
				MCU_Cb[MCUIndex] = BlockArray[x][y];
			}else{
				MCU_Cr[MCUIndex] = BlockArray[x][y];
			}
			MCUIndex = MCUIndex - x;
		}
		MCUIndex = MCUIndex + stride;
	}


	/*for(int i=0; i<8; i++){
		for(int j=0; j<8; j++){
			cout<<BlockArray[i][j]<<" ";
		}
		cout<<""<<endl;
	}*/

}


void DecodeMCU(int HorizontalFactor, int VerticalFactor){
	// Y mode 0
	for(int y=0; y<VerticalFactor; y++){
		for(int x=0; x<HorizontalFactor; x++){
			//cout<<"Y"<<endl;
			int stride = HorizontalFactor*8;
			int offset = x*8 + y*64*HorizontalFactor;

			ProcessHuffmanData0();
			DecodeSingleBlock(stride,offset,0,FrameVector[10]);
		}
	}
	// Cb mode 1
	//cout<<"Cb"<<endl;
	ProcessHuffmanData1(1);
	DecodeSingleBlock(8,0,1,FrameVector[13]);

	// Cr mode 2
	//cout<<"Cr"<<endl;
	ProcessHuffmanData1(2);
	DecodeSingleBlock(8,0,2,FrameVector[16]);
}

void ConvertYCrCbtoRGB(int y, int cb, int cr, int* r, int* g, int* b){
	float red, green, blue;

	red   = y + 1.402f*(cb-128);
	green = y-0.34414f*(cr-128)-0.71414f*(cb-128);
	blue  = y+1.772f*(cr-128);

	if(red>255){
		red = 255;
	}
	if(red<0){
		red = 0;
	}
	if(green>255){
		green = 255;
	}
	if(green<0){
		green = 0;
	}
	if(blue>255){
		blue = 255;
	}
	if(blue<0){
		blue = 0;
	}

	*r = red;
	*g = green;
	*b = blue;
}

void YCbCr_to_RGB24_Block8x8(int w, int h, int imgx, int imgy, int imgw, int imgh){
	int *Y, *Cb, *Cr;
	unsigned char *pix;

	int r, g, b;

	Y  = MCU_Y;
	Cb = MCU_Cb;
	Cr = MCU_Cr;

	int olw = 0; // overlap
	if ( imgx > (imgw-8*w) )
	{
		olw = imgw-imgx;
	}

	int olh = 0; // overlap
	if ( imgy > (imgh-8*h) )
	{
		olh = imgh-imgy;
	}

	for (int y=0; y<(8*h - olh); y++)
	{
		for (int x=0; x<(8*w - olw); x++)
		{
			int poff = x*3 + imgw*3*y;
			pix = &(m_colourspace[poff]);
			
			int yoff = x + y*(w*8);
			int coff = (int)(x*(1.0f/w)) + (int)(y*(1.0f/h))*8;

			int yc =  Y[yoff];
			int cb = Cb[coff];
			int cr = Cr[coff];

			ConvertYCrCbtoRGB(yc,cr,cb,&r,&g,&b);

			pix[0] = r;
			pix[1] = g;
			pix[2] = b;

		}
	}
}

int main (int argc, char **argv){

	init_idct();
	char *in_file, *out_file;
	in_file = argv[1];
    	out_file = argv[2];
	fstream filestr;
	filebuf *pbuf;
	int cc;
	pbuf=filestr.rdbuf();
	pbuf->open (in_file, fstream::in | fstream::out|fstream::binary);
	// monalisa teatime gig-sn01 gig-sn08

	int SaveMode = 0;

	// Application
	vector<int> ApplicationVector;

	// Definition quantization table
	QuantizationVector.resize(4);
	int QuantizationNumber = -1;

	

	// Define Huffman table
	vector<vector<int> > HuffmanVector;
	HuffmanVector.resize(4);
	int HuffmanNumber = -1;

	// Start of Scan
	vector<int> ScanVector;

	// information of picture
	queue<int> InformationQueue;


	// 讀資料並存檔
     	while (filestr.good()){
		cc=filestr.get();
		//cout.width (3);
		//cout <<hex<< cc;
		if(cc==0xff){
			if(SaveMode==10){
				SaveMode = 11;
				//InformationQueue.push(cc);
			}else{
				SaveMode = 1;
			}
		}else if(SaveMode==1){
			if(cc==0xd8){
				// Start of image
				SaveMode = 0;
			}else if(cc==0xe0){
				// Application
				SaveMode = 2;
			}else if(cc==0xdb){
				// Definition quantization table
				SaveMode = 3;
				QuantizationNumber++;
			}else if(cc==0xc0){
				// Start of frame
				SaveMode = 4;
			}else if(cc==0xc4){
				// Define Huffman table
				SaveMode = 5;
				HuffmanNumber++;
			}else if(cc==0xdd){
				// Define Restart Interval
				SaveMode = 6;
			}else if(cc==0xda){
				// Start of Scan
				SaveMode = 7;
			}else if(cc==0xd9){
				// End of Image
				SaveMode = 0;
			}else if(cc==0xfe){
				SaveMode = 12;
			}else{
				SaveMode = 0;
			}
		}else  if(SaveMode==2){
			ApplicationVector.push_back(cc);
		}else if(SaveMode==3){
			QuantizationVector[QuantizationNumber].push_back(cc);
		}else if(SaveMode==4){
			FrameVector.push_back(cc);
		}else if(SaveMode==5){
			HuffmanVector[HuffmanNumber].push_back(cc);
		}else if(SaveMode==6){

		}else if(SaveMode==7){
			ScanVector.push_back(cc);
			if(cc==0x00){
				SaveMode = 8;
			}			
		}else if(SaveMode==8){
			ScanVector.push_back(cc);
			if(cc==0x3f){
				SaveMode = 9;
			}else{
				SaveMode = 7;
			}
		}else if(SaveMode==9){
			ScanVector.push_back(cc);
			if(cc==0x00){
				SaveMode = 10;
			}else{
				SaveMode = 7;
			}
		}else if(SaveMode==10){
			InformationQueue.push(cc);
		}else if(SaveMode==11){
			if(cc==0x00){
				// send 0xff
				SaveMode = 10;
				// insert 0xff
				InformationQueue.push(0xff);
			}else if(cc==0xd9){
				SaveMode = 0;
			}
		}
	}
	pbuf->close();

	//cout<<" "<<endl;
	//cout<<"test"<<endl;

	// teatime
	if(QuantizationNumber==0 && HuffmanNumber==0){
		// Quantization
		QuantizationVector[1].push_back(0);
		QuantizationVector[1].push_back(0);
		QuantizationVector[1].push_back(1);
		for(int i=0; i<64; i++){
			int temp = QuantizationVector[0][i+68];
			QuantizationVector[1].push_back(temp);
		}
		for(int i=0; i<64; i++){
			QuantizationVector[0].pop_back();
		}

		// Huffman
		for(int i=1; i<4; i++){
			HuffmanVector[i].push_back(0);
			HuffmanVector[i].push_back(0);
		}
		int vector1start = 0;
		int array[16];
		int tt = 0;
		for(int i=0; i<16; i++){
			array[i] = HuffmanVector[0][i+3];
			tt = tt + array[i];
		}
		tt = tt + 20;
		vector1start = tt;
		HuffmanVector[1].push_back(1);
		int temp = 0;
		for(int i=0; i<16; i++){
			array[i] = HuffmanVector[0][tt+i];
			temp = temp + array[i];
		}
		for(int i=0; i<temp+16; i++){
			int tempvalue = HuffmanVector[0][tt+i];
			HuffmanVector[1].push_back(tempvalue);
		}
		tt = tt + temp + 17;
		HuffmanVector[2].push_back(0x10);
		temp = 0;
		for(int i=0; i<16; i++){
			array[i] = HuffmanVector[0][tt+i];
			temp = temp + array[i];
		}
		for(int i=0; i<temp+16; i++){
			int tempvalue = HuffmanVector[0][tt+i];
			HuffmanVector[2].push_back(tempvalue);
		}
		tt = tt + temp + 17;
		HuffmanVector[3].push_back(0x11);
		temp = 0;
		for(int i=0; i<16; i++){
			array[i] = HuffmanVector[0][tt+i];
			temp = temp + array[i];
		}
		for(int i=0; i<temp+16; i++){
			int tempvalue = HuffmanVector[0][tt+i];
			HuffmanVector[3].push_back(tempvalue);
		}

		int thesize = HuffmanVector[0].size();
		for(int i=vector1start; i<thesize; i++){
			HuffmanVector[0].pop_back();
		}


	}
	/*for(int i=0; i<4; i++){
		cout<<hex<<HuffmanVector[i][2]<<endl;
		cout<<hex<<HuffmanVector[i][3]<<endl;
	}
	cout<<"-----------------------------"<<endl;*/


	// initial
	for(int i=0; i<64; i++){
		MCU_Cb[i] = 0;
		MCU_Cr[i] = 0;
	}
	for(int i=0; i<256; i++){
		MCU_Y[i] = 0;
	}

	// Build Huffman Table
	for(int i=0; i<HuffmanVector.size(); i++){
		if(HuffmanVector[i][2]==0x00){	// DC0
			CodeWordDC0 = BuildHuffmanTableCodeWord(HuffmanVector[i]);
			CodeWeightDC0 = BuildHuffmanTableCodeWeight(HuffmanVector[i]);
			//cout<<"111111111111"<<endl;
		}else if(HuffmanVector[i][2]==0x01){	// DC1
			CodeWordDC1 = BuildHuffmanTableCodeWord(HuffmanVector[i]);
			CodeWeightDC1 = BuildHuffmanTableCodeWeight(HuffmanVector[i]);
			//cout<<"222222222222"<<endl;
		}else if(HuffmanVector[i][2]==0x10){	// AC0
			CodeWordAC0 = BuildHuffmanTableCodeWord(HuffmanVector[i]);
			CodeWeightAC0 = BuildHuffmanTableCodeWeight(HuffmanVector[i]);
			//cout<<"333333333333"<<endl;
		}else{	// AC1
			CodeWordAC1 = BuildHuffmanTableCodeWord(HuffmanVector[i]);
			CodeWeightAC1 = BuildHuffmanTableCodeWeight(HuffmanVector[i]);
			//cout<<"444444444444"<<endl;
		}
	}
	

	/*for(int i=0; i<CodeWordAC1.size(); i++){
		for(int j=0; j<CodeWordAC1[i].size(); j++){
			cout<<CodeWordAC1[i][j];
		}
		cout<<" "<<CodeWeightAC1[i]<<endl;
	}*/

	// read the information of picture
	while(!InformationQueue.empty()){
		int bit16 = InformationQueue.front();
		InformationQueue.pop();
		vector<char> BitBeforeReverse;
		int CountBit = 0;
		while(bit16!=0){
			int Remain = bit16 % 2;
			bit16 = bit16 / 2;
			if(Remain==0){
				BitBeforeReverse.push_back('0');
			}else if(Remain==1){
				BitBeforeReverse.push_back('1');
			}else{

			}
			CountBit++;
		}
		// start with 0
		for(int i=CountBit; i<8; i++){
			BitBeforeReverse.push_back('0');
		}
		while(!BitBeforeReverse.empty()){
			char c = BitBeforeReverse[BitBeforeReverse.size()-1];
			BitBeforeReverse.pop_back();
			TheBit.push(c);
		}

			
	}
	/*while(!TheBit.empty()){
		cout<<TheBit.front();
		TheBit.pop();
	}*/
	int HorizontalFactor = FrameVector[9] / 16;
	int VerticalFactor = FrameVector[9] % 16;
	int ImageHeight = FrameVector[3] * 256 + FrameVector[4];
	int ImageWidth = FrameVector[5] * 256 + FrameVector[6];
	//DecodeMCU(HorizontalFactor,VerticalFactor);
	/*for(int i=0; i<CodeWordDC0.size(); i++){
		for(int j=0; j<CodeWordDC0[i].size(); j++){
			cout<<CodeWordDC0[i][j];
		}
		cout<<" "<<CodeWeightDC0[i]<<endl;
	}
	cout<<"65656565"<<endl;
	for(int i=0; i<CodeWordAC0.size(); i++){
		for(int j=0; j<CodeWordAC0[i].size(); j++){
			cout<<CodeWordAC0[i][j];
		}
		cout<<" "<<CodeWeightAC0[i]<<endl;
	}*/

	int xMCUSize = 8 * HorizontalFactor;
	int yMCUSize = 8 * VerticalFactor;
	//cout<<ImageHeight<<endl;
	//cout<<ImageWidth<<endl;
	//cout<<yMCUSize<<endl;
	//cout<<xMCUSize<<endl;

	if(m_rgb==NULL){
		int h = ImageHeight*3;
		int w = ImageWidth*3;
		int height = h + (8*HorizontalFactor) - (h%(8*HorizontalFactor));
		int width  = w + (8*VerticalFactor) - (w%(8*VerticalFactor));
		m_rgb = new unsigned char[width * height];

		memset(m_rgb, 0, width*height);
	}

	for(int y=0; y<ImageHeight; y+=yMCUSize){
		//cout<<"11111"<<y<<endl;
		for(int x=0; x<ImageWidth; x+=xMCUSize){
			//cout<<"00000"<<x<<endl;
			m_colourspace = m_rgb + x*3 + (y *ImageWidth*3);
			DecodeMCU(HorizontalFactor,VerticalFactor);
			YCbCr_to_RGB24_Block8x8(HorizontalFactor,VerticalFactor,x,y,ImageWidth,ImageHeight);
		}
	}
	//DecodeMCU(HorizontalFactor,VerticalFactor);

	unsigned char* rgbpix = NULL;

	rgbpix = m_rgb;

	char * szBmpFileOutName = out_file;

	WriteBMP24(szBmpFileOutName, ImageWidth, ImageHeight, rgbpix);

	/*for(int i=0; i<CodeWordAC1.size(); i++){
		for(int j=0; j<CodeWordAC1[i].size(); j++){
			cout<<CodeWordAC1[i][j]<<" ";
		}
		cout<<" "<<endl;
	}*/
	/*cout<<"00000"<<endl;
	cout<<QuantizationVector[2].size()<<endl;
	cout<<"00000"<<endl;*/
	cout<<""<<endl;
	return 0;
}