#pragma once
#include <string>
//����Ҫ���㷨�ǿ��Թ����ģ����ܸ������ģ�ԭ�ģ��㷨��ϣ��������Կ��(��Կ���ռ��ܴ�)

//��һ���㷨�������������ݳ���(������ͬ����Ϊ��ͬ���)[����Ҫ��Key2����Ϊ֪����Կ����Կmd5���𲻴�]
//����key�ĳ��ȣ��������Ч��md5����Ϊkey1 :����key Ϊ 15 �ֽڣ���ôkey1Ϊ16�ֽ� ��key1�Ǳ䳤������key 16�ֽڶ���
//����key1��md5���������Ч�ȳ�key2����һ����ʽ�ֽ�Ϊ key2/2
//���ս��Ϊ src^key1^key2^��ʽ�ֽ�
//����ȫ�� ��Ϊ key1^key2^��ʽ�ֽڣ��ǹ̶���Կ��������Ĺ̶����ݣ�����ͨ������������������⿪���������ļ�(��Ȼ�ò�����Կ)

//�ڶ����㷨���������ݳ���(ÿ�μ������ݣ���Կ����ͬ)
//���ɶ����������  �������^��Կ��ɢ  ��Ϊ��ʼ���������������ݿ�ʼ
// key = ��ɢ������ݣ���һ����ʽ�ֽ�Ϊ len/2
//���ս��Ϊ src^key^��ʽ�ֽ�
#include "pubheader.h"
class DataChange
{
public:
	//��ȡͷ���ֽ���
	static UINT HeadLen();
	//����ͷ��Ϣ
	std::string InitEncode(const std::string &key);
	//���� info , key
	bool InitDecode(const std::string &key, const std::string &head);
	void Encode(char *pData,UINT nlen);
	void Decode(char *pData,UINT nlen);//
	DataChange(void) = default;
	~DataChange(void);

protected:
	void Reset();

protected:
	std::string m_pk;//ԭʼ��Чkey
	unsigned int  m_nKeyCurrent;//��ǰ����N��key
	char m_chLianshi;//��ǰ��ʽ�ֽ�

};

