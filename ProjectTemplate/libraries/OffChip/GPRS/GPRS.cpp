#include "GPRS.h"

GPRS::GPRS(USART& usart) :mUsart(usart), mIsConnected(false)
{
	mTimeOut = 1;
}

bool GPRS::Kick(void)
{
	mUsart << "AT\r\n";
	return RecvFind("OK");
}

bool GPRS::Restart(void)
{
	mUsart << "ATZ\r\n";
	return RecvFind("OK");
}

bool GPRS::GetVersion(char*)
{
	mUsart << "AT+GSV\r\n";
	return RecvFind("OK");
}

bool GPRS::GetConnectStatus(void)
{
	return mIsConnected;
}

bool GPRS::SetEcho(bool echo)
{
	mUsart << "ATE" << (echo ? 1 : 0) << "\r\n";
	return RecvFind("OK");
}
bool GPRS::Restore(void)
{
	mUsart << "ATZ0\r\n";
	return RecvFind("OK");
}
bool GPRS::SetUart(uint32_t baudrate, GPRS_pattern pattern)
{
	if (pattern > GPRS_PATTERN_DEF)
		return false;
	if (pattern == GPRS_PATTERN_CUR)
	{
		mUsart << "AT+UART_CUR=";
	}
	else if (pattern == GPRS_PATTERN_DEF)
	{
		mUsart << "AT+UART_DEF=";
	}
	else
	{
		mUsart << "AT+UART=";
	}
	mUsart << (int)baudrate << ",8,1,0,0\r\n";
	if (RecvFind("OK"))
		return true;
	return false;
}
void GPRS::ClearBuffer()
{
	unsigned int i = 0;
	for (i = 0; i < GPRS_RECEIVE_BUFFER_SIZE; i++)
		mReceiveBuffer[i] = 0;
}

bool GPRS::IsReged(void)
{
	u8 i = 0, count = 0;
	while (i == 0)
	{
		count++;
		ClearBuffer();
		mUsart << "AT+CGREG?\r\n";
		if (RecvFind(",1", ",5"))
		{
			i = 1;
			break;
		}
		if (count > 50)
			return false;
	}
	return true;
}
/**********Wait Regis*******/
bool GPRS::init(void)
{
	u8 count = 0;
	ClearBuffer();
	while (!Kick() && (count++ < 50));
	count = 0;
	while (!SetEcho(false));
	count = 0;
	while (!IsPin() && (count++ < 50));
	count = 0;
	while (!GetCCSQ() && (count++ < 50));
	count = 0;
	while (!IsReged() && (count++ < 50));
	count = 0;
	Close(1);
	while (!Deactivate_GPRS_PDP_Context() && (count++ < 50));//"AT+CIPSHUT" 
	count = 0;
	while (!GPRS_Mobile_Station_Class((char*)"B")); 				 //"AT+CGCLASS=\"B\"
	count = 0;
	while (!Define_PDP_Context()); 											//"AT+CGDCONT=1,\"IP\",\"CMNET\"
	count = 0;
	while (!AttachGPRSService(true) && (count++ < 50)); //"AT+CGATT=1"
	count = 0;
	while (!Set_CSD_or_GPRS_for_Connection_Mode());  		//"AT+CIPCSGP=1,\"CMNET\""
	count = 0;
	while (!Add_an_IP_Head(true) && (count++ < 50));		//Add an IP Head at the Beginning of a Package Received
	count = 0;
//	while(!PDPContextActivation(false)&&(count++<2)); //"AT+CGACT="
//	count = 0;
//	while(!PDPContextActivation(true)&&(count++<50));
//	count = 0;
//	while(!SetAPN((char*)"CMNET")&&(count++<50));			//"AT+CSTT=\""
//	count = 0;
//	while(!DeactivateMobileContext()&&(count++<50));	//"AT+CIICR" 
//	count = 0;
	return true;
}
bool GPRS::AttachGPRSService(bool IsAttached)
{
	mUsart << "AT+CGATT=" << (IsAttached ? "1" : "0") << "\r\n";
	return RecvFind("OK");
}
bool GPRS::PDPContextActivation(bool IsActive)
{
	mUsart << "AT+CGACT=" << (IsActive ? "1" : "0") << ",1" << "\r\n";//
	return RecvFind("OK");
}
bool GPRS::CloseConnected(bool IsFast)
{
	mUsart << "AT+CIPCLOSE=" << (IsFast ? "1" : "0") << "\r\n";
	return RecvFind("OK");
}
bool GPRS::Add_an_IP_Head(bool IsHead)
{
	mUsart << "AT+CIPHEAD=" << (IsHead ? "1" : "0") << "\r\n";
	return RecvFind("OK");
}
bool GPRS::Define_PDP_Context(void)
{
	mUsart << "AT+CGDCONT=1,\"IP\",\"CMNET\"" << "\r\n";
	return RecvFind("OK");
}
bool GPRS::GPRS_Mobile_Station_Class(char* cla)
{
	mUsart << "AT+CGCLASS=\"" << cla << "\"" << "\r\n";
	return RecvFind("OK");
}
bool GPRS::Deactivate_GPRS_PDP_Context(void)
{
	mUsart << "AT+CIPSHUT" << "\r\n";
	return RecvFind("SHUT OK");
}
//�����ƶ����� ����������·
bool GPRS::DeactivateMobileContext(void)
{
	mUsart << "AT+CIICR" << "\r\n";
	return RecvFind("OK");
}
//����APN ����㡢�û��������롢 �������� 
bool GPRS::SetAPN(char* APNstr)
{
	mUsart << "AT+CSTT=\"" << APNstr << "\"\r\n";
	return RecvFind("OK");
}
bool GPRS::Set_CSD_or_GPRS_for_Connection_Mode(void)
{
	mUsart << "AT+CIPCSGP=1,\"CMNET\"" << "\r\n";
	return RecvFind("OK");
}
bool GPRS::IsPin(void)
{
	mUsart << "AT+CPIN?" << "\r\n";
	return RecvFind("READY");
}
bool GPRS::SetMUX(bool isEnableMUX)
{
	mIsEnableMUX = isEnableMUX;
	mUsart << "AT+CIPMUX=" << (isEnableMUX ? "1" : "0") << "\r\n";
	return RecvFind("OK");
}
bool GPRS::CreateConnectMutipleMode(char* ipAddr, short port, Socket_Type socketType, signed char muxID)
{
	char type[4] = "TCP";
	if (socketType == Socket_Type_Dgram)
	{
		type[0] = 'U';
		type[1] = 'D';
	}
	if (muxID != -1)
		mUsart << "AT+CIPSTART=" << muxID << ",\"" << type << "\",\"" << ipAddr << "\"," << port << "\r\n";
	else
		mUsart << "AT+CIPSTART=" << "\"" << type << "\",\"" << ipAddr << "\"," << port << "\r\n";
	return RecvFind("OK", "ALREAY CONNECT", 5);
}
bool GPRS::Connect(char* ipAddr, short port, Socket_Type socketType, Socket_Protocol socketProtocol)
{
	if (CreateConnectMutipleMode(ipAddr, port, socketType))
		mIsConnected = true;
	else
		mIsConnected = false;
	return mIsConnected;
}
bool GPRS::SendMultipleMode(char* data, unsigned int num, signed char muxID)
{
	if (muxID != -1)
		mUsart << "AT+CIPSEND=" << muxID << "," << (int)num << "\r\n";
	else
		mUsart << "AT+CIPSEND=" << (int)num << "\r\n";
	if (!RecvFind(">"))
		return false;
	mUsart.SendBytes((uint8_t*)data, num);
	return RecvFind("OK\r\n", "ERROR");
}
bool GPRS::Write(char* data, unsigned int num)
{
	return SendMultipleMode(data, num);
}
unsigned int GPRS::Read(char* data)
{
	float starttime = TaskManager::Time();
	if (!mIsConnected)
		return 0;
	unsigned char temp[13];
	unsigned short bufferSize = 0;
	unsigned short dataLength = 0;
	unsigned short count = 0;
	bool flag = false;
	while (TaskManager::Time() - starttime < mTimeOut){
		bufferSize = mUsart.RxSize();
		if (bufferSize > 6)
		{
			for (count = 0; (count < bufferSize) && (count < GPRS_RECEIVE_BUFFER_SIZE); ++count)
			{
				mUsart.GetBytes(temp, 1);
				if (temp[0] == '+')
				{
					if (!mIsEnableMUX)
						RecvFindAndFilter(":", ",", ":", (char*)temp, mTimeOut);
					else
					{
						if (RecvFind(":", mTimeOut))
						{
							char* index1 = strstr(mReceiveBuffer, ",");
							index1 = strstr(index1, ",");
							char* index2 = strstr(mReceiveBuffer, ":");

							if (index1&&index2)
							{
								index1 += strlen(",");
								*index2 = '\0';
								strcpy((char*)temp, index1);
								return true;
							}
						}
					}
					dataLength = atoi((char*)temp);
					flag = true;
					break;
				}
			}
		}
		if (flag)
			break;
	}
	count = 0;
	while (TaskManager::Time() - starttime < mTimeOut){
		if (mUsart.RxSize()>0)
		{
			mUsart.GetBytes((unsigned char*)(data + count), 1);
			++count;
			if (count == dataLength || count == GPRS_RECEIVE_BUFFER_SIZE)
				break;
		}
	}
	return count;
}
unsigned int GPRS::Read(char* data, unsigned int num)
{
	float starttime = TaskManager::Time();
	if (!mIsConnected)
		return 0;
	unsigned int count = 0;
	while (TaskManager::Time() - starttime < mTimeOut&&count < num){
		if (mUsart.RxSize() > 0)
		{
			mUsart.GetBytes((unsigned char*)(data + count), 1);
			++count;
		}
	}
	return count;
}
bool GPRS::IsAlive()
{
	if (!mIsConnected)
		return false;
	if (GetStatus() == 3)
		return true;
	return false;
}
bool GPRS::Close(signed char muxID)
{
	if (muxID != -1)
		mUsart << "AT+CIPCLOSE=" << muxID << "\r\n";
	else
		mUsart << "AT+CIPCLOSE\r\n";
	return RecvFind("OK", "ERROR");
}
void GPRS::SetTimeOut(float timetOut)
{
	mTimeOut = timetOut;
}
bool GPRS::Call(char* phone)
{
	mUsart << "ATD" << phone << ";\r\n";
	return RecvFind("OK", "ERROR");
}
bool GPRS::SetMsgMode(bool IsText)
{
	mUsart << "AT+CMGF=" << (IsText ? "1" : "0") << "\r\n"; //1:Text ֻ����Ӣ���ַ������� 0 PDU ���Ķ���
	return RecvFind("OK");
}
bool GPRS::SetTECharset(char* charset)
{
	mUsart << "AT+CSCS=\"" << charset << "\"\r\n";
	return RecvFind("OK");
}
bool GPRS::SendMsg(char* phone, char* msg, bool IsPDU, u8 length)
{
	mUsart.ClearRxBuf();
	if (IsPDU)
		mUsart << "AT+CMGS=" << length << "\r\n";
	else
		mUsart << "AT+CMGS=\"" << phone << "\"\r\n";
	if (RecvFind(">"))
	{
		mUsart << msg;
		mUsart.SendByte(0x1A);
		mUsart << "\r\n";
	}
	else
		return false;
	return RecvFind("OK");
}
char GPRS::GetStatus(char* muxID, char* type, char* ipAddr, short remotePort, short localPort)
{
	char status = 0;
	bool result = false;
	mUsart << "AT+CIPSTATUS\r\n";
	result = RecvFindAndFilter("+CIPSTATU", "STATUS:", "\r\n", &status);
	status -= '0';
	return result;
}
u8 GPRS::GetCCSQ(void)
{
	mUsart << "AT+CSQ\r\n";
	float starttime = TaskManager::Time();
	unsigned char temp[13];
	unsigned short bufferSize = 0;
	unsigned short CSQvalue = 0;
	unsigned short count = 0;
	bool flag = false;
	while (TaskManager::Time() - starttime < mTimeOut)
	{
		bufferSize = mUsart.RxSize();
		if (bufferSize > 13)//+CSQ: 16,0
		{
			for (count = 0; count < bufferSize&&count < GPRS_RECEIVE_BUFFER_SIZE; ++count)
			{
				mUsart.GetBytes(temp, 1);
				if (temp[0] == '+')
				{
					if (RecvFindAndFilter(",", ":", ",", (char*)temp, mTimeOut))
					{
						CSQvalue = atoi((char*)temp);
						flag = true;
						break;
					}
				}
			}
		}
		if (flag)
			break;
	}
	return CSQvalue;
}
bool GPRS::ReceiveAndWait(const char* targetString, unsigned char timeOut)
{
	u8 temp;
	mReceiveBufferIndex = 0;
	ClearBuffer();
	double tartTime = TaskManager::Time();
	while ((TaskManager::Time() - tartTime) < timeOut)
	{
		while (mUsart.RxSize() > 0)
		{
			mUsart.GetBytes(&temp, 1);
			if (temp == '\0')
				continue;
			mReceiveBuffer[mReceiveBufferIndex++] = temp;
		}

		if (strstr(mReceiveBuffer, targetString))
			return true;

	}
	if (mReceiveBufferIndex > 0)
	{
		mReceiveBuffer[mReceiveBufferIndex] = '\0';
	}
	return false;
}
bool GPRS::ReceiveAndWait(char const* targetString, const char* targetString2, unsigned char timeOut)
{
	u8 temp;
	mReceiveBufferIndex = 0;
	ClearBuffer();
	double tartTime = TaskManager::Time();
	while ((TaskManager::Time() - tartTime) < timeOut)
	{
		while (mUsart.RxSize() > 0)
		{
			mUsart.GetBytes(&temp, 1);
			if (temp == '\0')
				continue;
			mReceiveBuffer[mReceiveBufferIndex++] = temp;

		}

		if (strstr(mReceiveBuffer, targetString) ||
			strstr(mReceiveBuffer, targetString2))
			return true;

	}
	if (mReceiveBufferIndex > 0)
	{
		mReceiveBuffer[mReceiveBufferIndex] = '\0';
	}
	return false;
}
bool GPRS::ReceiveAndWait(char const* targetString, const char* targetString2, const char* targetString3, unsigned char timeOut)
{
	u8 temp;
	mReceiveBufferIndex = 0;
	ClearBuffer();
	double tartTime = TaskManager::Time();
	while ((TaskManager::Time() - tartTime) < timeOut)
	{
		while (mUsart.RxSize() > 0)
		{
			mUsart.GetBytes(&temp, 1);
			if (temp == '\0')
				continue;
			mReceiveBuffer[mReceiveBufferIndex++] = temp;
		}

		if (strstr(mReceiveBuffer, targetString) ||
			strstr(mReceiveBuffer, targetString2) ||
			strstr(mReceiveBuffer, targetString3))
			return true;
	}
	if (mReceiveBufferIndex > 0)
	{
		mReceiveBuffer[mReceiveBufferIndex] = '\0';
	}
	return false;
}
bool GPRS::RecvFind(char const *target, unsigned char timeout)
{
	if (!ReceiveAndWait((char*)target, timeout))
		return false;
	return true;
}
bool GPRS::RecvFind(char const *target, char const *target2, unsigned char timeout)
{
	if (!ReceiveAndWait((char*)target, target2, timeout))
		return false;
	return true;
}
bool GPRS::RecvFind(char const *target, char const *target2, char const *target3, unsigned char timeout)
{
	if (!ReceiveAndWait((char*)target, target2, target3, timeout))
		return false;
	return true;
}
bool GPRS::RecvFindAndFilter(char const *target, char const * begin, char const * end, char* Data, float timeout)
{
	if (!ReceiveAndWait((char*)target, timeout))
		return false;
	char* index1 = strstr(mReceiveBuffer, begin);
	char* index2 = strstr(mReceiveBuffer, end);

	if (index1&&index2)
	{
		index1 += strlen(begin);
		*index2 = '\0';
		strcpy(Data, index1);
		return true;
	}
	return false;
}
//****************** Ӧ������ŷ��
bool GPRS::SendTagData(char* data, u8 num)
{
	u8 count = 0, i = 0;
	char sendData[130] = {0};
	DataFrame txFrame;
	txFrame.fnCode = FC_SEND_TAGDATA;
	txFrame.dataLength = DATA_LENGTH[FC_SEND_TAGDATA][DIRECTION_SEND];
	datacpy((char *)txFrame.data, data, (u16)DATA_LENGTH[FC_SEND_TAGDATA][DIRECTION_SEND]);
	txFrame.checkSum = CreateCheckCode((char *)txFrame.data, (u16)txFrame.dataLength) + txFrame.header + txFrame.fnCode + txFrame.dataLength;
	sendData[0] = txFrame.header;
	sendData[1] = txFrame.fnCode;
	sendData[2] = txFrame.dataLength;
	datacpy(&sendData[3], (char *)txFrame.data, (u16)DATA_LENGTH[FC_SEND_TAGDATA][DIRECTION_SEND]);
	sendData[DATA_LENGTH[FC_SEND_TAGDATA][DIRECTION_SEND] + 3] = txFrame.checkSum;
	while(!Write(sendData, (u16)DATA_LENGTH[FC_SEND_TAGDATA][DIRECTION_SEND] + 4) && (count++ < 20));
	if(count > 19)
		return false;
	return true;
}
//****************** Ӧ������ŷ��
