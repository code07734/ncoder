#include <memory>
#include <ctime>
#include <iostream>
#include <fstream>
#include <filesystem>

#include "ncode/nctypes.h"
//#include "ncode/ncfx.h"
//#include "ncode/filegen.h"

using namespace std;
namespace fs = filesystem;

struct states{
	u64 iter,iterout,seqAddr;
	u8 halt,mapF,numSeq,numSeqEnd,offset;
};

u8 isNum(u8 n){
	u64 checkNum = 0x7e0000007e03ff;
	n-=0x30;

	if(n>63){checkNum=0;};
	n=checkNum>>(n);
	n&=1;
	
	return n;
}

u8 letterToNum(u8 n){
	n-=0x30;
	if(n>9){n-=7;}
	if(n>0x20){n-=0x20;}
	return n;
}

void strToNum(u8 *vmem, u8 *out,u64 len,u64 length, states *s){
    u64 *out64, buff64=0, reverseIter=s->iter-1;
    u8 sh=0;
    //out64=(u64*)(out+*iterout);

    while(length>0){
    	out[s->iterout]^=(letterToNum(vmem[reverseIter]))&15;
    	out[s->iterout]^=((letterToNum(vmem[reverseIter-1]))<<4);

    	reverseIter-=2;
    	
    	s->iterout+=1;
    	length-=1;
    }
    if((1^(len&1))==0){
    	out[s->iterout-1]^=((letterToNum(vmem[reverseIter+1]))<<4);
    }
    
}

void ncoder(u8 *vmem, states *s, u64 fileSize){
	u8 *out;
	u64 dd, len=0, length=0;
	
	out = vmem + 0x1000;
	
	s->offset=1;
	while(s->halt^1){
		dd=vmem[s->iter];

		s->iter==fileSize ? s->halt|=1 : s->halt;
		dd==0x3a ? s->mapF^=1 : s->mapF;

		s->mapF ? s->numSeq|=isNum(dd) : s->numSeq;
		s->numSeq ? s->numSeqEnd=isNum(dd)^1 : s->numSeqEnd;

		if(s->mapF&s->numSeqEnd){
			length=((len+1)>>1);

			out[s->iterout]=length;
			s->iterout+=1;

			if(len>0){
				strToNum(vmem,out,len,length,s);
			}
			
			s->numSeq=0;
			s->numSeqEnd=0;
			len=0;
		}

		len+=s->numSeq;

		s->iter+=s->offset;
	}
}

void init(u8 *vmem){
	u64 fileSize=0;
	states *s;

	s = (states*)(vmem + 0x2000);

	ifstream inFile("f0", ios::binary);
	ofstream outFile("f1", ios::binary);
	ofstream dumpFile("dump", ios::binary);

	fileSize = fs::file_size("f0");

	inFile.read((i8*)vmem,fileSize);

	ncoder(vmem,s,fileSize);

	outFile.write((i8*)vmem+0x1000,s->iterout);
	dumpFile.write((i8*)vmem+0x2000,0x100);

	inFile.close();
	outFile.close();
	dumpFile.close();
}

int main(){	
	u8 *vmem;
	u64 *vmem64, i=0;

	//vmem = new u8[0x10000]();
	//vmem64 = (u64*)vmem;
	
	init(vmem);

	delete [] vmem;
	return 0;
}
