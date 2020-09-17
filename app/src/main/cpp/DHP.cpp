#include "DHP.h"
#include "FileOpt.h"
#include "DataChange.h"
#include "helpers.h"

#define AUTH_OFFSET_BYTE	8

namespace DP {
	//��֤��Կ����
	static inline UINT GetAuthKeyLen() { //56
		return DataChange::HeadLen() - AUTH_OFFSET_BYTE;
	}
	//���ɵĸ�����Կ���ȣ���DataChange��HeadLenһ��
	static inline UINT GetLongKeyLen() { // 64
		return DataChange::HeadLen();
	}
	//��֤���泤��
	static inline UINT GetAuthKeySaveLen() { //112
		return GetAuthKeyLen() * 2;
	}
	//����key ���һ��������Կ
	static inline std::string GetLongKey(const char * lpPass) {
		std::string skey(lpPass);
		return std::move(Helpers::GetBreakAway(skey, GetLongKeyLen()));
	}
	//����key ���һ������key��������֤key (HEX)
	static inline std::string GetAuthKeySave(const char * lpPass) {
		std::string keyAuth(lpPass);
		Helpers::GetBreakAway(keyAuth, GetAuthKeyLen());
		return  std::move(Helpers::BasicToHex(keyAuth.c_str(), keyAuth.length()));
	}
	//������֤��Կ���浽�ļ�������ͷ������
	std::string CreateAndSave(LPCTSTR  lpFileOut, const char *lpPass)
	{
		//����һ����֤key ,����key��չ��HeadLen()/2�ֽ�(k1,��֤),��չ��HeadLen()�ֽ�(k2,���ܼ���key)
		std::string k2;
		if (lpFileOut != nullptr && lpPass != nullptr && strlen(lpPass) > 3) {

			FileOpt fileOut;
			if (fileOut.Open(lpFileOut, false)) {
				std::string strSave = GetAuthKeySave(lpPass);

				if (strSave.length() == GetAuthKeySaveLen() && fileOut.Write(strSave.c_str(), strSave.length())) {
					k2 = GetLongKey(lpPass);//����				
				}
				fileOut.Close();
			}
		}
		return k2;
	}
	//���ļ���ȡ��֤��Կ
	static std::string GetSaveKey(LPCTSTR  lpFileKey) {
		FileOpt fileOut;
		if (fileOut.Open(lpFileKey, true)) {
			constexpr int optCount = 2048;
			char keyBuff[optCount];
			auto readed = fileOut.Read(keyBuff, optCount);
			if (readed == GetAuthKeySaveLen()) {//hex *2
				keyBuff[readed] = 0;
				return std::string(keyBuff, readed);
			}
		}
		return std::string();
	}
	//��֤�ļ���Կ������ͷ������(��֤ʧ�ܷ��ؿ�)
	std::string AuthInFile(LPCTSTR lpFileKey, const char * lpPass)
	{
		auto keyInfo = GetSaveKey(lpFileKey);
		if (keyInfo.size() == GetAuthKeySaveLen()) {
			std::string k1 = GetAuthKeySave(lpPass);

			if (k1 == keyInfo) {
				return GetLongKey(lpPass);//
			}
		}
		return std::string();
	}

	std::string GetSKey(const char * lpPass)
	{
		return GetLongKey(lpPass);//
	}

	bool IsKeyFileValid(LPCTSTR  lpFileKey)
	{
		return GetSaveKey(lpFileKey).size() == GetAuthKeySaveLen();
	}

	bool ProcessFile(LPCTSTR  lpFileIn, LPCTSTR  lpFileOut, const std::string &key, bool bEncode)
	{
		bool bSuccessed = false;
		if (lpFileIn != lpFileOut && lpFileOut != nullptr)
		{
			FileOpt fileIn, fileOut;
			if (fileIn.Open(lpFileIn, true) && fileOut.Open(lpFileOut, false))
			{
				if (key.size() == GetLongKeyLen())
				{
					constexpr int nReadCount = 32 * 1024;
					UINT nRead = 0;
					char bufIn[nReadCount];

					DataChange dataChange;
					if (bEncode) {
						auto head = dataChange.CreateHead(key);//set key
						bSuccessed = fileOut.Write(head.c_str(), head.length());//д��ͷ��Ϣ
					}
					else {
						nRead = fileIn.Read(bufIn, GetLongKeyLen());
						bSuccessed = (nRead == GetLongKeyLen()) && (dataChange.SetHead(key, std::string(bufIn, nRead)));
					}

					while (bSuccessed)
					{
						nRead = fileIn.Read(bufIn, nReadCount);
						if (nRead > 0)
						{
							if (bEncode)
								dataChange.Encode(bufIn, nRead);
							else
								dataChange.Decode(bufIn, nRead);
							if (!fileOut.Write(bufIn, nRead)) {
								bSuccessed = false;
								break;
							}
						}

						if (fileIn.IsEof())
							break;
					}
				}
			}
		}

		return bSuccessed;
	}

	bool ProcessStr(const std::string & strIn, std::string & strOut, const std::string & key, bool bEncode)
	{
		bool bSuccessed = false;
		const auto lkeylen = GetLongKeyLen();
		if (!strIn.empty() && key.size() == lkeylen)
		{
			std::string decode;
			size_t dataSize = 0;
			const char *pData=nullptr;

			DataChange dataChange;
			if (bEncode) {//����
				auto head = dataChange.CreateHead(key);//set key
				strOut.reserve((strIn.size() + lkeylen) * 2 + 1);//reserver
				strOut = Helpers::BasicToHex(head.c_str(), head.length());//д��ͷ��Ϣ
				bSuccessed = true;

				dataSize = strIn.size();
				pData = strIn.c_str();
			}
			else if (strIn.size() > lkeylen * 2) {//����
				decode = Helpers::HexToBasic(strIn.c_str(), strIn.length());//������hexת������
				bSuccessed = (decode.size() > lkeylen) && dataChange.SetHead(key, decode.substr(0, lkeylen));

				dataSize = decode.size() - lkeylen;
				pData = decode.c_str() + lkeylen;
			}

			if (bSuccessed)
			{
				char *buf = new char[dataSize];
				if (buf) {
					memcpy(buf, pData, dataSize);

					if (bEncode) {
						dataChange.Encode(buf, dataSize);
						strOut.append(Helpers::BasicToHex(buf, dataSize));
					}
					else {
						dataChange.Decode(buf, dataSize);
						strOut.assign(buf, dataSize);
					}
				}

				bSuccessed = buf != nullptr;
			}
		}

		return bSuccessed;
	}

	bool StrEncode2File(const char * lpStrIn, size_t strLen, LPCTSTR  lpFileOut, const std::string &key)
	{
		DateSave ds;
		if (ds.Open(lpFileOut, key)) {
			return ds.Write(lpStrIn, strLen);
		}
		return false;
	}

	bool FileDecode2Str(LPCTSTR  lpFileIn, std::string& strOut, const std::string &key)
	{
		DataLoad dl;
		strOut.clear();
		if (dl.Open(lpFileIn, key)) {
			constexpr  size_t  load = 32 * 1024;
			char buf[load];
			size_t l = load;
			while (l == load) {
				 l = dl.Read(buf, load);
				 strOut.append(buf, l);
			}

		}
		return !strOut.empty();
	}

	bool DateSave::Open(LPCTSTR lpFile, const std::string &skey)
	{
		bool bSuccessed = false;
		if (skey.size() == GetLongKeyLen() && lpFile != nullptr)	{
			if (m_fileOut.Open(lpFile, false))
			{
				auto head = m_dataChange.CreateHead(skey);//set key
				bSuccessed = m_fileOut.Write(head.c_str(), head.length());//д��ͷ��Ϣ
			}
		}

		return bSuccessed;
	}

	bool DateSave::Write(const char * data, size_t size)
	{
		constexpr size_t once = 32 * 1024;
		size_t wtd=0,curSize=0;//��д����ǰ
		char dataSet[once];//д�뻺��
		if (!m_fileOut.IsOpend()) return false;

		while (wtd < size) {
			if (size - wtd > once) {//����һ�δ�����
				curSize = once;
			}
			else {//�ܴ�����
				curSize = size - wtd;				
			}

			memcpy(dataSet, data + wtd, curSize);//���ݸ���
			wtd += curSize;
			m_dataChange.Encode(dataSet, (UINT)curSize);//����
			if (!m_fileOut.Write(dataSet, curSize))//д��
				return false;
		}
		return true;
	}

	void DateSave::Close()
	{
		m_fileOut.Close();
	}

	bool DataLoad::Open(LPCTSTR  lpFileIn, const std::string & skey)
	{
		bool bSuccessed = false;
		if (lpFileIn != nullptr && skey.size() == GetLongKeyLen())
		{
			if (m_fileIn.Open(lpFileIn, true))
			{
				constexpr int nReadCount = 1024;
				UINT nRead = 0;
				char bufIn[nReadCount];

				nRead = m_fileIn.Read(bufIn, GetLongKeyLen());
				bSuccessed = (nRead == GetLongKeyLen()) && (m_dataChange.SetHead(skey, std::string(bufIn, nRead)));
			}
		}
		return bSuccessed;
	}

	size_t DataLoad::Read(char * data, size_t size)
	{
		size_t rdd = 0;
		if (m_fileIn.IsOpend()) {
			rdd = m_fileIn.Read(data, size);
			if (rdd > 0) {
				m_dataChange.Decode(data, rdd);
			}
		}
		return rdd;
	}

	void DataLoad::Close()
	{
		m_fileIn.Close();
	}

	void DateLoadLine::ReadLine(std::string & line)
	{
		size_t start = 0;//��ʼλ��
		constexpr size_t szRead = 4096;
		char buf[szRead];
		line.clear();
		do {
			GetLine(line, start);//����
			if (!line.empty()) break;//�ҵ�����
			if (m_fileIn.IsEof()) {//����Ѿ���������󽻻�
				if (m_readCache.size())
					line = std::move(m_readCache);//��������
				break;
			}
			else {//��ȡ
				start = m_readCache.size();//�²���λ��
				auto r = DataLoad::Read(buf, szRead);//��ȡ�����ݲ�׷��
				if (r > 0) {
					m_readCache.append(buf, r);//����׷��
				}
			}
		} while (1);
	}

	void DateLoadLine::GetLine(std::string & str, size_t start)
	{
		auto sz = m_readCache.size();
		if (sz > start) {
			auto i = m_readCache.find_first_of('\n', start);
			if (i != std::string::npos) {
				if(i>0)
					str = m_readCache.substr(0, i);
				if (i == sz - 1)
					m_readCache.clear();
				else
					m_readCache.assign(m_readCache.c_str() + i + 1, sz - i - 1);//���� \n
			}
		}
	}

}