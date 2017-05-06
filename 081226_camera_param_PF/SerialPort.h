#pragma once

#define BUFFER_SIZE 2048
class CSerialPort
{
private:
	CString m_Name;
	HANDLE m_hPort;
	BYTE m_pBuffer[BUFFER_SIZE];

	BYTE ConvertToByte(char c1, char c2);
	void ConvertToChar(BYTE b, char & c1, char & c2);
public:
	//BOOL Open(CString name, int baudRate=9600, int byteSize=8, int parity=NOPARITY, int stopBits=ONESTOPBIT); 
	BOOL Open(CString name, int baudRate, int byteSize, int parity, int stopBits); 
	void Close();
	void ConvertStringToBytes(const char * pIn, BYTE * pOut, int & outLen);
	void ConvertBytesToString(const BYTE * pIn, int len, char * pOut);
	BOOL Write(const BYTE * pBuff, int len);
	BOOL Write(const char * pBuff);
	BOOL Read(BYTE * pBuff, BYTE endByte, int & bytesRead);
	BOOL ReadAll(BYTE * pBuff, int & bytesRead);	//read all data in buffer
	BOOL WaitAndReadAll(BYTE * pBuff, int & bytesRead, DWORD timeout);	//wait to read all data in buffer
	BOOL SetTimeouts(DWORD ReadIntervalTimeout, DWORD ReadTotalTimeoutMultiplier, DWORD ReadTotalTimeoutConstant,
        DWORD WriteTotalTimeoutMultiplier, DWORD WriteTotalTimeoutConstant);
	BOOL GetTimeouts(LPCOMMTIMEOUTS lpCommTimeouts);
	BOOL CSerialPort::ReadnByte(BYTE * pBuff, int nDemandRead);
	CSerialPort(void);
	virtual ~CSerialPort(void);

	//VISCA comletion command write
	BOOL VISCACompletionWrite(const BYTE * pBuff, int len);
	BOOL VISCACompletionWrite(const char * pBuff);

	//VISCA inittialization
	BOOL VISCAInitialzation();
};
