
#pragma once
#include <string>

namespace Helpers
{
	//�������ݣ������µ���ɢ����,�����ݳ���len
	std::string &GetBreakAway(std::string &strInfo,int len);
	//��������ַ���
	std::string CreateRandomString(size_t length);//��������ִ�

	//һ��������ʾת��
	std::string BasicToHex(const char *pSrc,int len);
	std::string HexToBasic(const char *pSrc,int len);
};


