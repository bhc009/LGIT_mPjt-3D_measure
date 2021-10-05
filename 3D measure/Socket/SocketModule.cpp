// SocketModule.cpp: implementation of the CSocketModule class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SocketModule.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CSocketModule, CSocketModule)
	//{{AFX_MSG_MAP(CSocketDlg)
	//ON_MESSAGE( WM_ACCEPT_SOCKET, OnAccept )
	//ON_MESSAGE( WM_RECEIVE_DATA, OnReceive )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CSocketModule::CSocketModule()
{
	m_iStatus		= _SERVER_;
	m_pServerSocket = NULL;
	m_pClientSocket = NULL;
	m_MasterIp		= "192.168.0.179";
	m_MasterPort	= 1980;

	m_szSendPacket		= _SOCKET_BUFFER_SIZE_;
	m_szReceivePacket	= _SOCKET_BUFFER_SIZE_;

	m_receiveData	= NULL;
	m_sendData		= NULL;
	m_receiveData	= new BYTE[_SOCKET_BUFFER_SIZE_];
	m_sendData		= new BYTE[_SOCKET_BUFFER_SIZE_];

	m_site = _SOCKET_MODULE_LGIT;

	m_bConnection = FALSE;
}


CSocketModule::~CSocketModule()
{
	DeleteSocket();
	
	if( m_receiveData )
	{
		delete[] m_receiveData;
	}
	m_receiveData = NULL;

	if( m_sendData )
	{
		delete[] m_sendData;
	}
	m_sendData = NULL;
}

BOOL CSocketModule::Create(int iType)
{
	DeleteSocket();

	m_iStatus = iType;

	switch( iType )
	{
		case _SERVER_:
			m_pServerSocket = new CServerSocket;
			if( !m_pServerSocket->Create(m_MasterPort, m_hWndParent) )
			{
				return FALSE;
			}
			m_pServerSocket->SetHWnd(m_hWndParent);

			// Set packet size
			m_szSendPacket = _MASTER_PACKET_SIZE;
			m_szReceivePacket = _CLIENT_PACKET_SIZE;

			if( m_receiveData )
			{
				delete[] m_receiveData;
			}
			m_receiveData = new BYTE[_CLIENT_PACKET_SIZE];
			if( m_sendData )
			{
				delete[] m_sendData;
			}
			m_sendData = new BYTE[_MASTER_PACKET_SIZE];

			break;
			
		case _CLIENT_:
			m_pClientSocket = new CClientSocket;
			m_pClientSocket->Create();
			m_pClientSocket->SetHWnd(m_hWndParent);

			// Set packet size
			m_szSendPacket = _CLIENT_PACKET_SIZE;
			m_szReceivePacket = _MASTER_PACKET_SIZE;

			if( m_receiveData )
			{
				delete[] m_receiveData;
			}
			m_receiveData = new BYTE[_MASTER_PACKET_SIZE];
			if( m_sendData )
			{
				delete[] m_sendData;
			}
			m_sendData = new BYTE[_CLIENT_PACKET_SIZE];

			break;
	}

	return TRUE;
}

void CSocketModule::SetParameter(HWND hWnd, DWORD iPortNum, CString strMasterIp)
{
	m_hWndParent	= hWnd;
	m_MasterPort	= iPortNum;
	m_MasterIp		= strMasterIp;
}

BOOL CSocketModule::Start()
{
	switch( m_iStatus )
	{
		case _SERVER_:
			if( m_pServerSocket==NULL )
			{
				return FALSE;
			}
			if( !m_pServerSocket->Listen() )
			{
				return FALSE;
			}
			break;

		case _CLIENT_:
			if( m_pClientSocket==NULL )
			{
				return FALSE;
			}
			if( !m_pClientSocket->Connect(m_MasterIp, m_MasterPort) )
			{
				return FALSE;
			}
			break;
	}

	m_bConnection = TRUE;

	return TRUE;
}

void CSocketModule::Stop()
{
	DeleteSocket();
}

void CSocketModule::Send(BYTE *strSend, int nSize)
{
	switch( m_iStatus )
	{
		case _SERVER_:
			if( m_pServerSocket )
				m_pServerSocket->m_pClientSocket.Send((LPCTSTR)strSend, nSize);
			break;

		case _CLIENT_:
			if( m_pClientSocket )
			{
				m_pClientSocket->Send((LPCTSTR)strSend, nSize);
			}
			break;
	}
}

BYTE *CSocketModule::OnReceive( unsigned int *iLength )
{
	BYTE *temp = m_receiveData;
	unsigned short length;

	if( m_iStatus==_SERVER_ )
	{
		*iLength = m_pServerSocket->m_pClientSocket.Receive(m_receiveData, _SOCKET_BUFFER_SIZE_);

		m_receiveData[*iLength] = _T('\0'); //terminate the string

	} else {
		*iLength = m_pClientSocket->Receive(m_receiveData, _SOCKET_BUFFER_SIZE_);

		m_receiveData[*iLength] = _T('\0'); //terminate the string
	}

// 	switch( m_iStatus )
// 	{
// 		case _SERVER_:
// 			m_pServerSocket->m_pClientSocket.Receive((LPSTR)temp,2);
// 			length = *((unsigned short*)temp);
// 			m_pServerSocket->m_pClientSocket.Receive((LPSTR)(temp+2),length-2);
// 			*iLength = length;
// 			break;
// 
// 		case _CLIENT_:
// 			//m_pClientSocket->Receive((LPSTR)temp,_MASTER_PACKET_SIZE);
// 			m_pClientSocket->Receive((LPSTR)temp,2);
// 			length = *((unsigned short*)temp);
// 			m_pClientSocket->Receive((LPSTR)(temp+2),length-2);
// 			*iLength = length;			
// 			break;
// 	}

	return temp;
}

void CSocketModule::DeleteSocket()
{
	if( m_iStatus==_CLIENT_ )
	{
		//m_pClientSocket->Send(_TERMINATE_CONNECTION_, _CLIENT_PACKET_SIZE);
// 		int szPackData;
// 		BYTE* pPacketData = PackClientData( _TERMINATE_CONNECTION_, &szPackData, NULL, 0 );
// 		Send( pPacketData, szPackData );
	}
	
	if( m_pServerSocket )
	{
		delete m_pServerSocket;
	}
	m_pServerSocket = NULL;

	if( m_pClientSocket )
	{
		delete m_pClientSocket;
	}
	m_pClientSocket = NULL;


	m_bConnection = FALSE;
}


//////////////////////////////////////////////////////////////////////////
// XML 송부건
// <message>
//		<body>
//			<RESULT>
//			<SLOPEANGLE>
//			<STEPHEIGHT>
//		<header>
//			<messageName>
//			<transactionId>
//			<timeStamp>
//////////////////////////////////////////////////////////////////////////
void CSocketModule::SendMeasureResult( bool bSuccess, double dStepHeight, double dSlopeAngle )
{
	OutputDebugString("SendMeasureResult : 시작");


	//
	// XML 형식으로 encoding
	//
	tinyxml2::XMLDocument xmlDocument;
	tinyxml2::XMLNode *pNode;

	pNode = xmlDocument.NewElement("message");
	xmlDocument.InsertFirstChild( pNode );

	tinyxml2::XMLElement *pElement = xmlDocument.NewElement("body");
	pNode->InsertEndChild(pElement);

	tinyxml2::XMLElement *pElement2 = xmlDocument.NewElement("RESULT");
	if( bSuccess )
	{
		pElement2->SetText("OK");
	} else {
		pElement2->SetText("FAIL");
	}
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("SLOPEANGLE");
	pElement2->SetText(dSlopeAngle);	// 일단 첫번째 결과만 보낸다.
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("STEPHEIGHT");
	pElement2->SetText(dStepHeight);	// 일단 첫번째 결과만 보낸다.
	pElement->InsertEndChild(pElement2);


	pElement = xmlDocument.NewElement("header");
	pNode->InsertEndChild(pElement);

	pElement2 = xmlDocument.NewElement("messageName");
	pElement2->SetText("REPLY_MEASUREMENT");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("transactionId");
	pElement2->SetText("ex4682e8-00ed-4ac");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("timeStamp");
	pElement2->SetText("20150728080712");
	pElement->InsertEndChild(pElement2);


	// File로 저장하기
	// 	tinyxml2::XMLError err = xmlDocument.SaveFile(XML_FILE_PATH);


	//
	// Encoding 후 송신하기
	//
	tinyxml2::XMLPrinter xmlPrinter;
	tinyxml2::XMLDocument xmlDocument2;
	xmlDocument.Print(&xmlPrinter);
	char *pTmp = (char*)xmlPrinter.CStr();

	if( m_site==_SOCKET_MODULE_LGIT )
	{
		CString str;
		str.Format("%c%s%c", 2, xmlPrinter.CStr(), 3);
		Send( (BYTE*)str.GetBuffer(), str.GetLength() );
	}

	if( m_site==_SOCKET_MODULE_LGD )
	{
		CString str;
		str.Format("%s", xmlPrinter.CStr() );
		Send( (BYTE*)str.GetBuffer(), str.GetLength() );
	}

	OutputDebugString("SendMeasureResult : 완료");
}



//////////////////////////////////////////////////////////////////////////
// XML 송부건
// <message>
//		<body>
//			<RESULT>
//			<SLOPEANGLE>
//			<STEPHEIGHT>
//			<SLOPEANGLE2>
//			<STEPHEIGHT2>
//			<SLOPEANGLE3>
//			<STEPHEIGHT3>
//			<SLOPEANGLE4>
//			<STEPHEIGHT4>
//		<header>
//			<messageName>
//			<transactionId>
//			<timeStamp>
//////////////////////////////////////////////////////////////////////////
void CSocketModule::SendMeasureResult( bool bSuccess, double dStepHeight1, double dStepHeight2, double dStepHeight3, double dStepHeight4, double dSlopeAngle1, double dSlopeAngle2, double dSlopeAngle3, double dSlopeAngle4 )
{
	OutputDebugString("SendMeasureResult : 시작");


	//
	// XML 형식으로 encoding
	//
	tinyxml2::XMLDocument xmlDocument;
	tinyxml2::XMLNode *pNode;

	pNode = xmlDocument.NewElement("message");
	xmlDocument.InsertFirstChild( pNode );

	tinyxml2::XMLElement *pElement = xmlDocument.NewElement("body");
	pNode->InsertEndChild(pElement);

	tinyxml2::XMLElement *pElement2 = xmlDocument.NewElement("RESULT");
	if( bSuccess )
	{
		pElement2->SetText("OK");
	} else {
		pElement2->SetText("FAIL");
	}
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("SLOPEANGLE");
	pElement2->SetText(dSlopeAngle1);	// 일단 첫번째 결과만 보낸다.
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("STEPHEIGHT");
	pElement2->SetText(dStepHeight1);	// 일단 첫번째 결과만 보낸다.
	pElement->InsertEndChild(pElement2);


	pElement2 = xmlDocument.NewElement("SLOPEANGLE2");
	pElement2->SetText(dSlopeAngle2);	// 일단 첫번째 결과만 보낸다.
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("STEPHEIGHT2");
	pElement2->SetText(dStepHeight2);	// 일단 첫번째 결과만 보낸다.
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("SLOPEANGLE3");
	pElement2->SetText(dSlopeAngle3);	// 일단 첫번째 결과만 보낸다.
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("STEPHEIGHT3");
	pElement2->SetText(dStepHeight3);	// 일단 첫번째 결과만 보낸다.
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("SLOPEANGLE4");
	pElement2->SetText(dSlopeAngle4);	// 일단 첫번째 결과만 보낸다.
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("STEPHEIGHT4");
	pElement2->SetText(dStepHeight4);	// 일단 첫번째 결과만 보낸다.
	pElement->InsertEndChild(pElement2);



	pElement = xmlDocument.NewElement("header");
	pNode->InsertEndChild(pElement);

	pElement2 = xmlDocument.NewElement("messageName");
	pElement2->SetText("REPLY_MEASUREMENT");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("transactionId");
	pElement2->SetText("ex4682e8-00ed-4ac");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("timeStamp");
	pElement2->SetText("20150728080712");
	pElement->InsertEndChild(pElement2);


	// File로 저장하기
	// 	tinyxml2::XMLError err = xmlDocument.SaveFile(XML_FILE_PATH);


	//
	// Encoding 후 송신하기
	//
	tinyxml2::XMLPrinter xmlPrinter;
	tinyxml2::XMLDocument xmlDocument2;
	xmlDocument.Print(&xmlPrinter);
	char *pTmp = (char*)xmlPrinter.CStr();

	if( m_site==_SOCKET_MODULE_LGIT )
	{
		CString str;
		str.Format("%c%s%c", 2, xmlPrinter.CStr(), 3);
		Send( (BYTE*)str.GetBuffer(), str.GetLength() );
	}

	if( m_site==_SOCKET_MODULE_LGD )
	{
		CString str;
		str.Format("%s", xmlPrinter.CStr() );
		Send( (BYTE*)str.GetBuffer(), str.GetLength() );
	}

	OutputDebugString("SendMeasureResult : 완료");
}



//////////////////////////////////////////////////////////////////////////
// XML 송부건
// <message>
//		<body>
//			<RESULT>
//			<MOVEX>
//			<MOVEY>
//		<header>
//			<messageName> REPLY_AUTOFOCUS
//			<transactionId>
//			<timeStamp>
//////////////////////////////////////////////////////////////////////////
void CSocketModule::SendAutoFocusResult( bool bSuccess, double dDx, double dDy )
{
	OutputDebugString("SendAutoFocusResult : 시작");


	tinyxml2::XMLDocument xmlDocument;
	tinyxml2::XMLNode *pNode;

	pNode = xmlDocument.NewElement("message");
	xmlDocument.InsertFirstChild( pNode );

	tinyxml2::XMLElement *pElement = xmlDocument.NewElement("body");
	pNode->InsertEndChild(pElement);

	tinyxml2::XMLElement *pElement2 = xmlDocument.NewElement("RESULT");
	if( bSuccess )
	{
		pElement2->SetText("OK");
	} else {
		pElement2->SetText("FAIL");
	}
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("MOVEX");
	pElement2->SetText(dDx);	// 일단 첫번째 결과만 보낸다.
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("MOVEY");
	pElement2->SetText(dDy);	// 일단 첫번째 결과만 보낸다.
	pElement->InsertEndChild(pElement2);


	pElement = xmlDocument.NewElement("header");
	pNode->InsertEndChild(pElement);

	pElement2 = xmlDocument.NewElement("messageName");
	pElement2->SetText("REPLY_AUTOFOCUS");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("transactionId");
	pElement2->SetText("ex4682e8-00ed-4ac");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("timeStamp");
	pElement2->SetText("20150728080712");
	pElement->InsertEndChild(pElement2);


	// File로 저장하기
	// 	tinyxml2::XMLError err = xmlDocument.SaveFile(XML_FILE_PATH);

	tinyxml2::XMLPrinter xmlPrinter;
	tinyxml2::XMLDocument xmlDocument2;
	xmlDocument.Print(&xmlPrinter);
	char *pTmp = (char*)xmlPrinter.CStr();
	CString str;
	str.Format("%s", pTmp);
	CString str1;
	str1.Format("%c", 2 );
	CString str2;
	str2.Format("%c", 3 );

	CString str3 = str1+str+str2;

	Send( (BYTE*)str3.GetBuffer(), str3.GetLength() );


	OutputDebugString("SendAutoFocusResult : 완료");
}


//////////////////////////////////////////////////////////////////////////
// XML 송부건
// <message>
//		<body>
//			<RESULT>
//		<header>
//			<messageName> REPLY_CAL
//			<transactionId>
//			<timeStamp>
//////////////////////////////////////////////////////////////////////////
void CSocketModule::SendCalibrationResult( bool bSuccess )
{
	OutputDebugString("SendCalibrationResult : 시작");


	tinyxml2::XMLDocument xmlDocument;
	tinyxml2::XMLNode *pNode;

	pNode = xmlDocument.NewElement("message");
	xmlDocument.InsertFirstChild( pNode );

	tinyxml2::XMLElement *pElement = xmlDocument.NewElement("body");
	pNode->InsertEndChild(pElement);

	tinyxml2::XMLElement *pElement2 = xmlDocument.NewElement("RESULT");
	if( bSuccess )
	{
		pElement2->SetText("OK");
	} else {
		pElement2->SetText("FAIL");
	}
	pElement->InsertEndChild(pElement2);


	pElement = xmlDocument.NewElement("header");
	pNode->InsertEndChild(pElement);

	pElement2 = xmlDocument.NewElement("messageName");
	pElement2->SetText("REPLY_CAL");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("transactionId");
	pElement2->SetText("ex4682e8-00ed-4ac");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("timeStamp");
	pElement2->SetText("20150728080712");
	pElement->InsertEndChild(pElement2);


	// File로 저장하기
	// 	tinyxml2::XMLError err = xmlDocument.SaveFile(XML_FILE_PATH);

	tinyxml2::XMLPrinter xmlPrinter;
	tinyxml2::XMLDocument xmlDocument2;
	xmlDocument.Print(&xmlPrinter);
// 	char *pTmp = (char*)xmlPrinter.CStr();
// 	CString str;
// 	str.Format("%s", pTmp);
// 	CString str1;
// 	str1.Format("%c", 2 );
// 	CString str2;
// 	str2.Format("%c", 3 );
// 
// 	CString str3 = str1+str+str2;

	if( m_site==_SOCKET_MODULE_LGIT )
	{
		CString str;
		str.Format("%c%s%c", 2, xmlPrinter.CStr(), 3);
		Send( (BYTE*)str.GetBuffer(), str.GetLength() );
	}


	if( m_site==_SOCKET_MODULE_LGD )
	{
		CString str;
		str.Format("%s", xmlPrinter.CStr() );
		Send( (BYTE*)str.GetBuffer(), str.GetLength() );
	}

// 	Send( (BYTE*)str3.GetBuffer(), str3.GetLength() );


	OutputDebugString("SendAutoFocusResult : 완료");
}


//////////////////////////////////////////////////////////////////////////
// XML 송부건
// <message>
//		<body>
//			<RESULT>
//		<header>
//			<messageName> REPLY_LENS10X
//			<transactionId>
//			<timeStamp>
//////////////////////////////////////////////////////////////////////////
void CSocketModule::SendLensChangeResult( bool bSuccess )
{
	OutputDebugString("SendLensChangeResult : 시작");


	tinyxml2::XMLDocument xmlDocument;
	tinyxml2::XMLNode *pNode;

	pNode = xmlDocument.NewElement("message");
	xmlDocument.InsertFirstChild( pNode );

	tinyxml2::XMLElement *pElement = xmlDocument.NewElement("body");
	pNode->InsertEndChild(pElement);

	tinyxml2::XMLElement *pElement2 = xmlDocument.NewElement("RESULT");
	if( bSuccess )
	{
		pElement2->SetText("OK");
	} else {
		pElement2->SetText("FAIL");
	}
	pElement->InsertEndChild(pElement2);


	pElement = xmlDocument.NewElement("header");
	pNode->InsertEndChild(pElement);

	pElement2 = xmlDocument.NewElement("messageName");
	pElement2->SetText("REPLY_LENS10X");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("transactionId");
	pElement2->SetText("ex4682e8-00ed-4ac");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("timeStamp");
	pElement2->SetText("20150728080712");
	pElement->InsertEndChild(pElement2);


	// File로 저장하기
	// 	tinyxml2::XMLError err = xmlDocument.SaveFile(XML_FILE_PATH);

	tinyxml2::XMLPrinter xmlPrinter;
	tinyxml2::XMLDocument xmlDocument2;
	xmlDocument.Print(&xmlPrinter);
	char *pTmp = (char*)xmlPrinter.CStr();


	if (m_site==_SOCKET_MODULE_LGIT)
	{
		CString str;
		str.Format("%c%s%c", 2, xmlPrinter.CStr(), 3);
		Send((BYTE*)str.GetBuffer(), str.GetLength());
	}

	if (m_site==_SOCKET_MODULE_LGD )
	{
		CString str;
		str.Format("%s", xmlPrinter.CStr());
		Send((BYTE*)str.GetBuffer(), str.GetLength());
	}



	OutputDebugString("SendLensChangeResult : 완료");
}


//////////////////////////////////////////////////////////////////////////
// XML 송부건
// <message>
//		<body>
//			<RESULT>
//		<header>
//			<messageName> REPLY_RECIPE
//			<transactionId>
//			<timeStamp>
//////////////////////////////////////////////////////////////////////////
void CSocketModule::SendSetReceipeResult( bool bSuccess )
{
	OutputDebugString("SendSetReceipeResult : 시작");


	tinyxml2::XMLDocument xmlDocument;
	tinyxml2::XMLNode *pNode;

	pNode = xmlDocument.NewElement("message");
	xmlDocument.InsertFirstChild( pNode );

	tinyxml2::XMLElement *pElement = xmlDocument.NewElement("body");
	pNode->InsertEndChild(pElement);

	tinyxml2::XMLElement *pElement2 = xmlDocument.NewElement("RESULT");
	if( bSuccess )
	{
		pElement2->SetText("OK");
	} else {
		pElement2->SetText("FAIL");
	}
	pElement->InsertEndChild(pElement2);


	pElement = xmlDocument.NewElement("header");
	pNode->InsertEndChild(pElement);

	pElement2 = xmlDocument.NewElement("messageName");
	pElement2->SetText("REPLY_RECIPE");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("transactionId");
	pElement2->SetText("ex4682e8-00ed-4ac");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("timeStamp");
	pElement2->SetText("20150728080712");
	pElement->InsertEndChild(pElement2);


	// File로 저장하기
	// 	tinyxml2::XMLError err = xmlDocument.SaveFile(XML_FILE_PATH);

	tinyxml2::XMLPrinter xmlPrinter;
	tinyxml2::XMLDocument xmlDocument2;
	xmlDocument.Print(&xmlPrinter);
	char *pTmp = (char*)xmlPrinter.CStr();

	if (m_site==_SOCKET_MODULE_LGIT)
	{
		CString str;
		str.Format("%c%s%c", 2, xmlPrinter.CStr(), 3);
		Send((BYTE*)str.GetBuffer(), str.GetLength());
	}

	if (m_site==_SOCKET_MODULE_LGD)
	{
		CString str;
		str.Format("%s", xmlPrinter.CStr());
		Send((BYTE*)str.GetBuffer(), str.GetLength());
	}


	OutputDebugString("SendLensChangeResult : 완료");
}


//////////////////////////////////////////////////////////////////////////
// XML 송부건
// <message>
//		<body>
//			<RUN>
//			<LENS>
//		<header>
//			<messageName> REPLY_STATE
//			<transactionId>
//			<timeStamp>
//////////////////////////////////////////////////////////////////////////
void CSocketModule::SendState( bool bRun, int lens )
{
	OutputDebugString("SendState : 시작");


	tinyxml2::XMLDocument xmlDocument;
	tinyxml2::XMLNode *pNode;

	pNode = xmlDocument.NewElement("message");
	xmlDocument.InsertFirstChild( pNode );

	tinyxml2::XMLElement *pElement = xmlDocument.NewElement("body");
	pNode->InsertEndChild(pElement);

	tinyxml2::XMLElement *pElement2 = xmlDocument.NewElement("RUN");
	if( bRun )
	{
		pElement2->SetText("YES");
	} else {
		pElement2->SetText("NO");
	}
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("LENS");
	pElement2->SetText(lens);
	pElement->InsertEndChild(pElement2);


	pElement = xmlDocument.NewElement("header");
	pNode->InsertEndChild(pElement);

	pElement2 = xmlDocument.NewElement("messageName");
	pElement2->SetText("REPLY_STATE");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("transactionId");
	pElement2->SetText("ex4682e8-00ed-4ac");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("timeStamp");
	pElement2->SetText("20150728080712");
	pElement->InsertEndChild(pElement2);


	// File로 저장하기
	// 	tinyxml2::XMLError err = xmlDocument.SaveFile(XML_FILE_PATH);

	tinyxml2::XMLPrinter xmlPrinter;
	tinyxml2::XMLDocument xmlDocument2;
	xmlDocument.Print(&xmlPrinter);
	char *pTmp = (char*)xmlPrinter.CStr();

	if( m_site==_SOCKET_MODULE_LGIT )
	{
		CString str;
		str.Format("%c%s%c", 2, xmlPrinter.CStr(), 3);
		Send( (BYTE*)str.GetBuffer(), str.GetLength() );
	}

	if( m_site==_SOCKET_MODULE_LGD )
	{
		CString str;
		str.Format("%s", xmlPrinter.CStr() );
		Send( (BYTE*)str.GetBuffer(), str.GetLength() );
	}


	OutputDebugString("SendState : 완료");
}




//////////////////////////////////////////////////////////////////////////
// XML 송부건
// <message>
//		<body>
//			<RESULT>
//		<header>
//			<messageName> REPLY_STOP
//			<transactionId>
//			<timeStamp>
//////////////////////////////////////////////////////////////////////////
void CSocketModule::SendStop( bool bValid )
{
	OutputDebugString("SendStop : 시작");


	tinyxml2::XMLDocument xmlDocument;
	tinyxml2::XMLNode *pNode;

	pNode = xmlDocument.NewElement("message");
	xmlDocument.InsertFirstChild( pNode );

	tinyxml2::XMLElement *pElement = xmlDocument.NewElement("body");
	pNode->InsertEndChild(pElement);

	tinyxml2::XMLElement *pElement2 = xmlDocument.NewElement("RESULT");
	if( bValid )
	{
		pElement2->SetText("OK");
	} else {
		pElement2->SetText("FAIL");
	}
	pElement->InsertEndChild(pElement2);



	pElement = xmlDocument.NewElement("header");
	pNode->InsertEndChild(pElement);

	pElement2 = xmlDocument.NewElement("messageName");
	pElement2->SetText("REPLY_STOP");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("transactionId");
	pElement2->SetText("ex4682e8-00ed-4ac");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("timeStamp");
	pElement2->SetText("20150728080712");
	pElement->InsertEndChild(pElement2);


	// File로 저장하기
	// 	tinyxml2::XMLError err = xmlDocument.SaveFile(XML_FILE_PATH);

	tinyxml2::XMLPrinter xmlPrinter;
	tinyxml2::XMLDocument xmlDocument2;
	xmlDocument.Print(&xmlPrinter);
	char *pTmp = (char*)xmlPrinter.CStr();

	if( m_site==_SOCKET_MODULE_LGIT )
	{
		CString str;
		str.Format("%c%s%c", 2, xmlPrinter.CStr(), 3);
		Send( (BYTE*)str.GetBuffer(), str.GetLength() );
	}

	if( m_site==_SOCKET_MODULE_LGD )
	{
		CString str;
		str.Format("%s", xmlPrinter.CStr() );
		Send( (BYTE*)str.GetBuffer(), str.GetLength() );
	}

	OutputDebugString("SendStop : 완료");
}


//////////////////////////////////////////////////////////////////////////
// XML 송부건
// <message>
//		<body>
//			<RESULT>
//		<header>
//			<messageName> REPLY_THICKNESS
//			<transactionId>
//			<timeStamp>
//////////////////////////////////////////////////////////////////////////
void CSocketModule::SendThicknessResult( double dThickness )
{
	OutputDebugString("SendThicknessResult : 시작");


	tinyxml2::XMLDocument xmlDocument;
	tinyxml2::XMLNode *pNode;

	pNode = xmlDocument.NewElement("message");
	xmlDocument.InsertFirstChild( pNode );

	tinyxml2::XMLElement *pElement = xmlDocument.NewElement("body");
	pNode->InsertEndChild(pElement);

	tinyxml2::XMLElement *pElement2 = xmlDocument.NewElement("RESULT");
// 	if( bSuccess )
// 	{
		pElement2->SetText("OK");
// 	} else {
// 		pElement2->SetText("FAIL");
// 	}
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("VALUE");
	pElement2->SetText(dThickness);
	pElement->InsertEndChild(pElement2);


	pElement = xmlDocument.NewElement("header");
	pNode->InsertEndChild(pElement);

	pElement2 = xmlDocument.NewElement("messageName");
	pElement2->SetText("REPLY_THICK");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("transactionId");
	pElement2->SetText("ex4682e8-00ed-4ac");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("timeStamp");
	pElement2->SetText("20150728080712");
	pElement->InsertEndChild(pElement2);


	// File로 저장하기
	// 	tinyxml2::XMLError err = xmlDocument.SaveFile(XML_FILE_PATH);

	tinyxml2::XMLPrinter xmlPrinter;
	tinyxml2::XMLDocument xmlDocument2;
	xmlDocument.Print(&xmlPrinter);
	char *pTmp = (char*)xmlPrinter.CStr();

	if( m_site==_SOCKET_MODULE_LGIT )
	{
		CString str;
		str.Format("%c%s%c", 2, xmlPrinter.CStr(), 3);
		Send( (BYTE*)str.GetBuffer(), str.GetLength() );
	}

	if( m_site==_SOCKET_MODULE_LGD )
	{
		CString str;
		str.Format("%s", xmlPrinter.CStr() );
		Send( (BYTE*)str.GetBuffer(), str.GetLength() );
	}

	OutputDebugString("SendState : 완료");
}

