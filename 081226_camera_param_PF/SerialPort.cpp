#include "stdafx.h"
#include "SerialPort.h"

CSerialPort::CSerialPort(void) { m_hPort=INVALID_HANDLE_VALUE; }
CSerialPort::~CSerialPort(void) { Close(); }

BOOL CSerialPort::Open(CString name, int baudRate, int byteSize, int parity, int stopBits) {
	CString str;
	DWORD dwError;
	DCB dcb;
	Close();
	m_Name = name;
	m_hPort = CreateFile(name, 
					GENERIC_READ | GENERIC_WRITE,
					0, // exclusive access
					NULL, // no security
					OPEN_EXISTING,
					0, // no overlapped I/O
					NULL); // null template
	if(m_hPort==INVALID_HANDLE_VALUE){ 
		dwError=GetLastError();
		str.Format(_T("Error %d in openning port  "),dwError);
		str+=name;
		AfxMessageBox(str);
		return FALSE;
	}
	if(GetCommState(m_hPort,&dcb)==FALSE){
		dwError=GetLastError();
		str.Format(_T("Error %d in getting state of port  "),dwError);
		str+=name;
		AfxMessageBox(str);
		Close();
		return FALSE;
	}
	dcb.BaudRate = baudRate;
	dcb.ByteSize = byteSize;
	dcb.Parity = parity;
	dcb.StopBits = stopBits;
	dcb.fAbortOnError = FALSE;
	if(SetCommState(m_hPort,&dcb)==FALSE){
		dwError=GetLastError();
		str.Format(_T("Error %d in setting state of port  "),dwError);
		str+=name;
		AfxMessageBox(str);
		Close();	
		return FALSE;
	}
	return TRUE;
}

void CSerialPort::Close()
{
	if(m_hPort!=INVALID_HANDLE_VALUE){
		CloseHandle(m_hPort);
		m_hPort=INVALID_HANDLE_VALUE;
	}
}


BYTE CSerialPort::ConvertToByte(char c1, char c2)
{
	BYTE result;
	BYTE b1, b2;
	if(c1 >='0' && c1 <='9')
		b1=c1-'0';
	else if(c1 >='A' && c1 <='F')
		b1=c1-'A' + 10;
	else
		b1=0;

	if(c2 >='0' && c2 <='9')
		b2=c2-'0';
	else if(c2 >='A' && c2 <='F')
		b2=c2-'A' + 10;
	else
		b2=0;
	result = (b1 << 4) | b2;
	return result;
}

void CSerialPort::ConvertStringToBytes(const char * pIn, BYTE * pOut, int & outLen)
{
	int i,j,k;
	char c, c1, c2;
	
	i=0;
	j=1;
	k=0;

	c=pIn[i];
	while(c!='\0'){
		if(c!=' '){
			if(j==1){
				c1=c;
				j=2;
			}
			else{
				j=1;
				c2=c;
				pOut[k]=ConvertToByte(c1,c2);
				k++;
			}
		}
		i++;
		c=pIn[i];
	}
	outLen=k;
}

void CSerialPort::ConvertToChar(BYTE b, char & c1, char & c2)
{
	BYTE b1, b2;
	b1 = (b>>4) & 0x0F;
	if(b1<10)
		c1 = '0' + b1;
	else
		c1 = 'A' + (b1-10);

	b2 = b & 0x0F;
	if(b2<10)
		c2 = '0' + b2;
	else
		c2 = 'A' + (b2-10);
}

void CSerialPort::ConvertBytesToString(const BYTE * pIn, int len, char * pOut)
{
	int i;
	int outLen;
	char c1, c2;
	outLen=0;
	for(i=0;i<len;i++){
		ConvertToChar(pIn[i],c1,c2);
		pOut[outLen++]=c1;
		pOut[outLen++]=c2;
		pOut[outLen++]=' ';		
	}
	pOut[outLen]='\0';
}

BOOL CSerialPort::Write(const char * pBuff)
{
	if(m_hPort==INVALID_HANDLE_VALUE)
		return FALSE;
	int len;
	ConvertStringToBytes(pBuff,this->m_pBuffer,len);
    return Write(this->m_pBuffer,len);
}

BOOL CSerialPort::Write(const BYTE * pBuff, int len)
{
	if(m_hPort==INVALID_HANDLE_VALUE)
		return FALSE;

	BOOL result;
	CString str;
	DWORD dwError, bytesWritten=0;
	result = WriteFile(m_hPort, pBuff, len, &bytesWritten, NULL);
	if(result==FALSE || bytesWritten!=len){
		dwError=GetLastError();
		str.Format(_T("Error %d in sending data of port  "),dwError);
		str+=m_Name;
		AfxMessageBox(str);
		Close();		
		return FALSE;
	}
	return TRUE;
}

BOOL CSerialPort::Read(BYTE * pBuff, BYTE endByte, int & nBytesRead)
{
	if(m_hPort==INVALID_HANDLE_VALUE)
		return FALSE;

	BOOL result;
	CString str;
	DWORD dwError, bytesRead=0;
	int total=0;
	while(true){
		result = ReadFile(m_hPort, pBuff+total, 1, &bytesRead, NULL);
		if(result==FALSE){
			dwError=GetLastError();
			str.Format(_T("Error %d in reading data of port  "),dwError);
			str+=m_Name;
			AfxMessageBox(str);
			Close();		
			return FALSE;
		}
		if(bytesRead==1){
			if(pBuff[total]==endByte){                
				nBytesRead=total + bytesRead;
				return TRUE;
			}
			total+=bytesRead;
		}
		else
			Sleep(1);
	}
	return FALSE;
}

BOOL CSerialPort::ReadAll(BYTE * pBuff, int & nBytesRead)
{
	if(m_hPort==INVALID_HANDLE_VALUE)
		return FALSE;

	BOOL result;
	CString str;
	DWORD dwError, bytesRead=0;
	int total=0;
	result = ReadFile(m_hPort, pBuff+total, 20, &bytesRead, NULL);
	if(result==FALSE){
		dwError=GetLastError();
		str.Format(_T("Error %d in reading data of port  "),dwError);
		str+=m_Name;
		AfxMessageBox(str);
		Close();		
		return FALSE;
	}
	total+=bytesRead;
	while(bytesRead==20){
		result = ReadFile(m_hPort, pBuff+total, 20, &bytesRead, NULL);
		if(result==FALSE){
			dwError=GetLastError();
			str.Format(_T("Error %d in reading data of port  "),dwError);
			str+=m_Name;
			AfxMessageBox(str);
			Close();
			return FALSE;
		}
		total+=bytesRead;
	}
	nBytesRead=total;
	return TRUE;
}

BOOL CSerialPort::WaitAndReadAll(BYTE * pBuff, int & nBytesRead, DWORD timeout)	//wait to read all data in buffer
{
	if(m_hPort==INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD t1, t2;
	BOOL result;
	CString str;
	DWORD dwError, bytesRead=0;
	int total=0;
	t1=GetTickCount();
	while(total==0){	
		//AfxMessageBox(_T("1"));
		result = ReadFile(m_hPort, pBuff+total, 0, &bytesRead, NULL);
		//AfxMessageBox(_T("2"));
		if(result==FALSE){
			dwError=GetLastError();
			str.Format(_T("Error %d in reading data of port  "),dwError);
			str+=m_Name;
			AfxMessageBox(str);
			Close();		
			return FALSE;
		}
		total+=bytesRead;
		if(total==0){
			Sleep(1);
			t2=GetTickCount();
			if(t2-t1>timeout){
				str.Format(_T("Timeout in reading data of port  "));
				str+=m_Name;
				AfxMessageBox(str);
				Close();		
				return FALSE;
			}
		}
	}

	while(bytesRead>0){
		result = ReadFile(m_hPort, pBuff+total, 0, &bytesRead, NULL);
		if(result==FALSE){
			dwError=GetLastError();
			str.Format(_T("Error %d in reading data of port  "),dwError);
			str+=m_Name;
			AfxMessageBox(str);
			Close();
			return FALSE;
		}
		total+=bytesRead;
	}
	nBytesRead=total;
	return TRUE;
}

BOOL CSerialPort::SetTimeouts(DWORD ReadIntervalTimeout, 
							  DWORD ReadTotalTimeoutMultiplier, 
							  DWORD ReadTotalTimeoutConstant,
							  DWORD WriteTotalTimeoutMultiplier, 
							  DWORD WriteTotalTimeoutConstant)
{
	if(m_hPort==INVALID_HANDLE_VALUE)
		return FALSE;

	COMMTIMEOUTS timeout;
	timeout.ReadIntervalTimeout=ReadIntervalTimeout;
	timeout.ReadTotalTimeoutConstant=ReadTotalTimeoutConstant;
	timeout.ReadTotalTimeoutMultiplier=ReadTotalTimeoutMultiplier;
	timeout.WriteTotalTimeoutMultiplier=WriteTotalTimeoutMultiplier;
	timeout.WriteTotalTimeoutConstant=WriteTotalTimeoutConstant;
	BOOL result;
	result=SetCommTimeouts(this->m_hPort,&timeout);
	if(result==FALSE){
		DWORD dwError=GetLastError();
		CString str;
		str.Format(_T("Error %d in setting timeouts of port  "),dwError);
		str+=m_Name;
		AfxMessageBox(str);
		Close();
		return FALSE;
	}
	return TRUE;
}

BOOL CSerialPort::GetTimeouts(LPCOMMTIMEOUTS lpCommTimeouts)
{
	if(m_hPort==INVALID_HANDLE_VALUE)
		return FALSE;

	BOOL result;
	result = GetCommTimeouts(m_hPort,lpCommTimeouts);
	if(result==FALSE){
		DWORD dwError=GetLastError();
		CString str;
		str.Format(_T("Error %d in setting timeouts of port  "),dwError);
		str+=m_Name;
		AfxMessageBox(str);
		Close();
		return FALSE;
	}
	return TRUE;
}

BOOL CSerialPort::ReadnByte(BYTE * pBuff, int nDemandRead)
{
	if(m_hPort==INVALID_HANDLE_VALUE)
		return FALSE;

	BOOL result;
	CString str;
	DWORD dwError, nRealRead=0;
	int total=0;
	while(true)
	{
		result = ReadFile(m_hPort, pBuff+total, 1, &nRealRead, NULL);
		if(result==FALSE){
			dwError=GetLastError();
			str.Format(_T("Error %d in reading data of port  "),dwError);
			str+=m_Name;
			AfxMessageBox(str);
			Close();		
			return FALSE;
		}
		if(nRealRead==1){
			total++;
			if(total==nDemandRead){                
				return TRUE;
			}
		}
		else
			Sleep(1);
	}
	return FALSE;
}

BOOL CSerialPort::VISCACompletionWrite(const char * pBuff)
{
	if(m_hPort==INVALID_HANDLE_VALUE)
		return FALSE;

	int len;
	ConvertStringToBytes(pBuff,this->m_pBuffer,len);
    return VISCACompletionWrite(this->m_pBuffer,len);	
}

BOOL CSerialPort::VISCACompletionWrite(const BYTE * pBuff, int len)
{
	if(m_hPort==INVALID_HANDLE_VALUE)
		return FALSE;

	int n;
	if(this->Write(pBuff,len)==FALSE)
		return FALSE;
	if(Read(m_pBuffer,0xFF,n)==FALSE)
		return FALSE;
	//check ACK
	if(m_pBuffer[1]!=0x40 && m_pBuffer[1]!=0x41 && m_pBuffer[1]!=0x42){
		TRACE0("Reply message is not ACK message");
		return FALSE;
	}
	if(Read(m_pBuffer,0xFF,n)==FALSE)
		return FALSE;
	//check Completion
	if(m_pBuffer[1]!=0x50 && m_pBuffer[1]!=0x51 && m_pBuffer[1]!=0x52){
		TRACE0("Reply message is not completion message");
		return FALSE;
	}
	return TRUE;	
}


BOOL CSerialPort::VISCAInitialzation()
{
	if(m_hPort==INVALID_HANDLE_VALUE)
		return FALSE;

	char * p;
	int len;

	p="81 01 04 00 02 FF";		//turn on
	if(this->VISCACompletionWrite(p)==FALSE){  //turn on
		TRACE0("Error in command : Turn on");
		return FALSE;
	}

	p="88 01 00 01 FF";		//IF Clear (broadcast), do not check completion
	if(this->Write(p)==FALSE){  //turn on
		TRACE0("Error in command :IF Clear");
		return FALSE;
	}
	//get result
	if(Read(m_pBuffer,0xFF,len)==FALSE){
		TRACE0("Error in command :IF Clear");
		return FALSE;
	}

	
	//p="81 01 04 58 02 FF";	//AF sensitivity hight
	p="81 01 04 58 03 FF";	//AF sensitivity low
	if(this->VISCACompletionWrite(p)==FALSE){
		TRACE0("Error in command : AF sensitivity hight");
		return FALSE;
	}
	
	p="81 01 04 39 00 FF";							//CAM_AE, Full auto
	if(this->VISCACompletionWrite(p)==FALSE){
		TRACE0("Error in command : CAM_AE, Full auto");
		return FALSE;
	}


	//p="81 01 04 5A 02 FF";							//Slow shutter Auto
	p="81 01 04 5A 03 FF";							//Slow shutter Manual
	if(this->VISCACompletionWrite(p)==FALSE){
		TRACE0("Error in command : Slow shutter Auto");
		return FALSE;
	}
		
	
	//p="81 01 04 33 03 FF";							//Back light compensation OFF
	p="81 01 04 33 02 FF";							//Back light compensation ON
	if(this->VISCACompletionWrite(p)==FALSE){
		TRACE0("Error in command : Back light compensation OFF");
		return FALSE;
	}
	
	p="81 01 04 3E 03 FF";							//exposure compensation OFF
	//p="81 01 04 3E 02 FF";							//exposure compensation ON
	if(this->VISCACompletionWrite(p)==FALSE){
		TRACE0("Error in command : Exposure compensation OFF");
		return FALSE;
	}
		
	p="88 30 01 FF";								//Address set broadcast
	if(this->Write(p)==FALSE){
		TRACE0("Error in command : Address set broadcast");
		return FALSE;
	}
	//get result
	if(Read(m_pBuffer,0xFF,len)==FALSE){
		TRACE0("Error in command : Address set broadcast");
		return FALSE;
	}
	
	p="81 01 06 04 FF";				//go to the home position
	if(this->VISCACompletionWrite(p)==FALSE){
		TRACE0("Error in command : Go to the home position");
		return FALSE;
	}

	p="81 01 04 06 03 FF";						//Digital zoom OFF
	if(this->VISCACompletionWrite(p)==FALSE){
		TRACE0("Error in command : Digital zoom OFF");
		return FALSE;
	}

	p="81 01 04 38 03 FF";							//Auto Focus OFF
	if(this->VISCACompletionWrite(p)==FALSE){
		TRACE0("Error in command : Auto Focus OFF");
		return FALSE;
	}

	p="81 01 04 28 08 04 00 00 FF";							//Near limit : 8400
	if(this->VISCACompletionWrite(p)==FALSE){
		TRACE0("Error in command : Near Limit 8400 (Near)");
		return FALSE;
	}

	p="81 01 04 47 00 00 00 00 FF";							//focal length (zoom) F=0000 (3.1mm)
	if(this->VISCACompletionWrite(p)==FALSE){
		TRACE0("Error in command : Zoom to 0000 (3.1mmm)");
		return FALSE;
	}

	p="81 01 04 18 02 FF";							//Forced Infinity
	if(this->VISCACompletionWrite(p)==FALSE){
		TRACE0("Error in command : Forced Infinity");
		return FALSE;
	}

	return TRUE;
}