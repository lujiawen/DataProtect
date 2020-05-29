
#include "helpers.h"
#include <time.h>
#include "MD5.h"

namespace Helpers
{

void CreateRandomData(char *pData,size_t nlen)
{
	size_t i=0;
	int nThisRand;
	while(i<nlen)
	{
		nThisRand=rand();
		//����rand����ǰ(�ϴ�rand������rand��δ��ʼ�����ڴ�����)�Ľ��������������ÿ��rand����ȫ��������
		(pData)[i++]+=(nThisRand + pData[nThisRand%nlen])%0xFF;
	}
}

static bool CreateRandomString(size_t len,	std::string* str)
{
		str->clear();

		char *bytes=new char[len];

		if(bytes)
		{
			static int keyself=0;
			++keyself;
			srand((unsigned int)time(NULL));//time is first
			CreateRandomData(bytes,len);//δ��ʼ����

			//���ݶ�ջ�����ڴ��ַ�˻�
			srand((DWORD)(long long)(&len)*(DWORD)(long long)(bytes));
			CreateRandomData(bytes, len);
#ifdef _WIN32
			srand(GetTickCount()+keyself);//tickcout
			CreateRandomData(bytes,len);

			//���ݴ���ʣ��ռ�����������
			ULARGE_INTEGER i64FreeBytesToCaller,i64TotalBytes,i64FreeBytes;
			TCHAR ch[MAX_PATH]={0};
			GetSystemDirectory(ch,MAX_PATH);
			if(GetDiskFreeSpaceEx (ch,&i64FreeBytesToCaller,&i64TotalBytes,&i64FreeBytes))
			{
				srand(i64FreeBytes.LowPart);
				CreateRandomData(bytes,len);
			}
				
#else
			if ( len > 16) {//
				srand(bytes[3] * bytes[13]);//
				CreateRandomData(bytes, len);
				srand(bytes[5] * bytes[15]);//
				CreateRandomData(bytes, len);
			}
#endif

			str->assign(bytes,len);

			delete [] bytes;
		}
		return true;
}

//��չΪ��ЧժҪ
std::string GetEQBreakAway(const std::string &str) {
	std::string strOut;
	auto lensrc = str.length();
	if (lensrc > 0) {
		constexpr int aligned_set = 16;//ѡ��md5������16�ֽ�
		auto newLen = ((lensrc + aligned_set - 1) / aligned_set) * aligned_set;
		strOut.reserve(newLen);

		size_t index = 0;
		std::string strMD5;
		while (index < lensrc) {//����
			//��Ч���ɣ�n �ֽ����� n�ֽ�ժҪ
			strMD5 = CMD5Checksum::GetMD5((BYTE*)(str.c_str() + index), (index + aligned_set <= lensrc ? aligned_set : lensrc - index), false);
			strOut.append(strMD5.c_str(), aligned_set);
			index += aligned_set;
		}
		
	}
	return strOut;

}

std::string & GetBreakAway(std::string & strInfo, int len)
{
	if (strInfo.length() > 0) {
		while (strInfo.size() < len) {//���С��һ����Сֵ�����䵽��Сֵ
			strInfo.append(1, char(strInfo.c_str()[len % strInfo.size()] + strInfo.size() * 3) );
		}

		//����һ��ȫ��ģ�ֱ�ӵ�Ч��չ��������볤�ȴ���16�������ǰ��һ������Ҫ���Ǿ���ı�һ����Ҳ��һ��
		auto mTotal = CMD5Checksum::GetMD5((BYTE*)strInfo.c_str(), strInfo.length(), false);

		//������չ
		size_t index = 0;
		while(index < strInfo.size()){
			strInfo[index] = strInfo[index] ^ mTotal[index%mTotal.size()];//ÿ���ֽڶ����
			index++;
		}

		//�����ɢ
		strInfo = GetEQBreakAway(strInfo);
		if(strInfo.length()!=len)
			strInfo.assign(strInfo.c_str(), len);
	}
	return strInfo;
}

std::string CreateRandomString( size_t length )
{
	std::string str;
	CreateRandomString(length,&str);

	return str;
}

char dec2hex(unsigned char ch)
{
	if(ch>=0 && ch<16)
	{
		if(ch<10) return ch+'0';
		else return ch-10+'A';
	}
	return 0; 
}
std::string BasicToHex( const char *pSrc,int len )
{
	std::string result;
	if(pSrc && len>0)
	{
		result.reserve(len*2);
		int nIndex=0;
		unsigned char chThis;
		char ch[2];
		while(nIndex<len)
		{
			chThis=(unsigned char)pSrc[nIndex];
			ch[0]=dec2hex(chThis/16);
			ch[1]=dec2hex(chThis%16);
			++nIndex;
			result.append(ch,2);
		}
	}

	return result;
}

char hex2dec(unsigned char ch)
{
	if(ch >= '0' && ch <= '9')     
		return ch - '0';     
	else if(ch >= 'a' && ch <= 'f')     
		return (ch - 'a' + 10);     
	else if(ch >= 'A' && ch <= 'F')     
		return (ch - 'A' + 10);     
	assert(0);
	return 0; 
}

std::string HexToBasic( const char *pSrc,int len )
{
	std::string result;
	if(pSrc && len>0 && len%2==0 )
	{
		result.reserve(len/2);
		char ch;
		int nIndex=0;
		while(nIndex<len)
		{
			ch=hex2dec(pSrc[nIndex])*16+hex2dec(pSrc[nIndex+1]);
			result.push_back(ch);
			nIndex+=2;
		}
	}

	return result;
}


}