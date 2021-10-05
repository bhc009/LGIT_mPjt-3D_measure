#include "StdAfx.h"
#include "Protocol.h"
#include "XML/tinyxml2.h"


CProtocol::CProtocol(void)
{
// 	m_iSite = BHPROTOCOL_LGD;
 	m_iSite = BHPROTOCOL_LGIT;

	m_strRecipe.Format("Default recipe");
	m_strQRCode.Format("Default QR code");

	m_dPitchX = 0;
	m_dPitchY = 0;
	m_dSizeX = 0;
	m_dSizeY = 0;
	m_iShape = 0;
	m_iArrage = 0;
	m_dThickness = 0;
}


CProtocol::~CProtocol(void)
{
}


void CProtocol::setSite( int i )
{
	m_iSite = i;
}


//////////////////////////////////////////////////////////////////////////
//
// ���� ��� �ؼ�
//
//////////////////////////////////////////////////////////////////////////
int CProtocol::Decode(unsigned char *pChar, int nLength)
{
	if( nLength<40)
	{
		return BHPROTOCOL_FAIL;
	}


	// ���� ���� ����
	tinyxml2::XMLDocument xmlDocument;

	if( m_iSite==BHPROTOCOL_LGIT)
	{
		BYTE *temp = new BYTE[nLength-40+1];
		memset( temp, 0, nLength-40+1);
		memcpy( temp, pChar+39, nLength-40 );

		// decoding
		xmlDocument.Parse((char*)temp);

		delete[] temp;
	}
	if( m_iSite==BHPROTOCOL_LGD )
	{
		// decoding
		xmlDocument.Parse((char*)pChar);
	}



	// ��� �κ� ���� : 
	tinyxml2::XMLElement *pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("header")->FirstChildElement("messageName");

	char *pCharText = (char*)pElement->GetText();

	CString strRecive;
	strRecive.Format("%s", pCharText);



	// ��� �κ� ���� : 
	//////////////////////////////////////////////////////////////////////////
	if( strRecive=="REQUEST_RECIPE" )
	{
		// �ؼ�
		// pitch ����
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("PITCHX");
		pElement->QueryDoubleText(&m_dPitchX);

		// pitch ����
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("PITCHY");
		pElement->QueryDoubleText(&m_dPitchY);

		// size ����
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("SIZEX");
		pElement->QueryDoubleText(&m_dSizeX);

		// size ����
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("SIZEY");
		pElement->QueryDoubleText(&m_dSizeY);

		// shape ����
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("SHAPE");
		pElement->QueryIntText(&m_iShape);

		// �迭 ����
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("ARRANGE");
		pElement->QueryIntText(&m_iArrage);

		// size ����
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("THICKNESS");
		pElement->QueryDoubleText(&m_dThickness);

		return BHPROTOCOL_RECIPE;
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// ���� ���� ����� ���
	if( strRecive=="REQUEST_LENS10X" )
	{
		return BHPROTOCOL_LENS10X;
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// ���� ���� ����� ���
	if( strRecive=="REQUEST_STOP" )
	{
		return BHPROTOCOL_STOP;
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// Auto focus ����� ���
	if( strRecive=="REQUEST_AUTOFOCUS" )
	{
		return BHPROTOCOL_AUTOFOCUS;
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// Calibration ����� ���
	if( strRecive=="REQUEST_CAL" )
	{
		return BHPROTOCOL_CALIBRATION;
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// ���� ����� ���
	if( strRecive=="REQUEST_MEASUREMENT" )
	{
		// �� ��� Ȯ��
		int iMethode;
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("METHOD");
		pElement->QueryIntText(&iMethode);

		// ������ ��
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("RECIPE");
		m_strRecipe = pElement->GetText();

		// QR code
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("QRCODE");
		m_strQRCode = pElement->GetText();

		// * �̳����� ��� ��� ���� ������ ���� ������ QR �ڵ� ���� ����
		if( m_iSite==BHPROTOCOL_LGIT )
		{
// 			// ������ ��
// 			pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("RECIPE");
// 			m_strRecipe = pElement->GetText();
// 
// 			// QR code
// 			pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("QRCODE");
// 			m_strQRCode = pElement->GetText();

			// �β����� ��� ����
			pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("USE_THICK");
			int iUse = 0;
			pElement->QueryIntText(&iUse);
			if( iUse )
			{
				m_bUseThickness = TRUE;
			} else {
				m_bUseThickness = FALSE;
			}

			// ���� ��ĵ ����
			pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("SCAN_UP");
			pElement->QueryDoubleText(&m_d3dRangeUp);

			// ���� ��ĵ ����
			pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("SCAN_DOWN");
			pElement->QueryDoubleText(&m_d3dRangDown);
		}


		if( m_iSite==BHPROTOCOL_LGD )
		{
			m_d3dRangeUp = 30.0;
			m_d3dRangDown = 30.0;
		}


		//
		switch( iMethode )
		{
		case 1 : // 1��˻�
			return BHPROTOCOL_MESURE_LARGE;

		case 2 : // 2��˻�
			return BHPROTOCOL_MESURE_SMALL;

		default:
			return BHPROTOCOL_FAIL;
		}
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// Calibration ����� ���
	if( strRecive=="REQUEST_THICK" )
	{
		// �ؼ�
		// pitch ����
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("TARGET");
		CString str = pElement->GetText();
		if( str=="CHUCK")
		{
			m_bCAL = TRUE;
		} else {
			m_bCAL = FALSE;
		}


		// pitch ����
		int iIndex;
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("INDEX");
		pElement->QueryIntText(&iIndex);
		m_thicknessIndex = iIndex - 1;


		return BHPROTOCOL_THICKNESS;
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// ���� ���� ����� ���
	if( strRecive=="REQUEST_STATE" )
	{
		return BHPROTOCOL_STATE;
	}
	//
	//////////////////////////////////////////////////////////////////////////


	return BHPROTOCOL_FAIL;
}


//////////////////////////////////////////////////////////////////////////
//
// 3D ���а��� ���� ���� encoding
//
//////////////////////////////////////////////////////////////////////////
int CProtocol::EncodeState()
{

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// ������ ���� ��ɿ� ���� ���� ����
//
//
//	<head>
//		<messageName></messageName>
//		<transactionId></transactionId>
//		<timeStamp></timeStamp>
//	</head>
//	<message>
//		<result></result>
//	</message>
//
//////////////////////////////////////////////////////////////////////////
int CProtocol::EncodeRecipe(bool bSuccess)
{
	tinyxml2::XMLDocument xmlDocument;
	tinyxml2::XMLNode *pNode;


	// message ����
	pNode = xmlDocument.NewElement("message");
	xmlDocument.InsertFirstChild( pNode );


	// message > body ����
	tinyxml2::XMLElement *pElement = xmlDocument.NewElement("body");
	pNode->InsertEndChild(pElement);

	// message > body > result����
	tinyxml2::XMLElement *pElement2 = xmlDocument.NewElement("RESULT");
	if( bSuccess )
	{
		pElement2->SetText("OK");
	} else {
		pElement2->SetText("FAIL");
	}
	pElement->InsertEndChild(pElement2);


	// message > head ����
	pElement = xmlDocument.NewElement("header");
	pNode->InsertEndChild(pElement);

	// message > head > messageName ����
	pElement2 = xmlDocument.NewElement("messageName");
	pElement2->SetText("REPLY_RECIPE");
	pElement->InsertEndChild(pElement2);

	// message > head > transactionId ����
	pElement2 = xmlDocument.NewElement("transactionId");
	pElement2->SetText("ex4682e8-00ed-4ac");
	pElement->InsertEndChild(pElement2);

	// message > head > timeStamp ����
	pElement2 = xmlDocument.NewElement("timeStamp");
	pElement2->SetText("20150728080712");
	pElement->InsertEndChild(pElement2);


	// File�� �����ϱ�
	// 	tinyxml2::XMLError err = xmlDocument.SaveFile(XML_FILE_PATH);


	// ���ڿ� ����
	tinyxml2::XMLPrinter xmlPrinter;
	tinyxml2::XMLDocument xmlDocument2;
	xmlDocument.Print(&xmlPrinter);
	char *pTmp = (char*)xmlPrinter.CStr();


	m_strMessage.Format("%c%s%c", 2, pTmp, 3 );

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//
// <message>
//		<body>
//			<RESULT>
//		<header>
//			<messageName> REPLY_CAL
//			<transactionId>
//			<timeStamp>
//
//////////////////////////////////////////////////////////////////////////
int CProtocol::EncodeCalibration(bool bSuccess)
{
	tinyxml2::XMLDocument xmlDocument;
	tinyxml2::XMLNode *pNode;


	//////////////////////////////////////////////////////////////////////////
	// message ����
	pNode = xmlDocument.NewElement("message");
	xmlDocument.InsertFirstChild( pNode );
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// message > body ����
	tinyxml2::XMLElement *pElement = xmlDocument.NewElement("body");
	pNode->InsertEndChild(pElement);


	// message > body > result����
	tinyxml2::XMLElement *pElement2 = xmlDocument.NewElement("RESULT");
	if( bSuccess )
	{
		pElement2->SetText("OK");
	} else {
		pElement2->SetText("FAIL");
	}
	pElement->InsertEndChild(pElement2);
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// message > head ����
	pElement = xmlDocument.NewElement("header");
	pNode->InsertEndChild(pElement);

	// message > head > messageName ����
	pElement2 = xmlDocument.NewElement("messageName");
	pElement2->SetText("REPLY_CAL");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("transactionId");
	pElement2->SetText("ex4682e8-00ed-4ac");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("timeStamp");
	pElement2->SetText("20150728080712");
	pElement->InsertEndChild(pElement2);
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// ���ڿ� ����
	tinyxml2::XMLPrinter xmlPrinter;
	tinyxml2::XMLDocument xmlDocument2;
	xmlDocument.Print(&xmlPrinter);

	m_strMessage.Format("%c%s%c", 2, (char*)xmlPrinter.CStr(), 3 );
	//////////////////////////////////////////////////////////////////////////


	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
// <message>
//		<body>
//			<RESULT>
//			<SLOPEANGLE>
//			<STEPHEIGHT>
//		<header>
//			<messageName>
//			<transactionId>
//			<timeStamp>
//
//////////////////////////////////////////////////////////////////////////
int CProtocol::EncodeMeasureLarge( bool bSuccess, double dStepHeight, double dSlopeAngle )
{
	tinyxml2::XMLDocument xmlDocument;
	tinyxml2::XMLNode *pNode;


	//////////////////////////////////////////////////////////////////////////
	// message ����
	pNode = xmlDocument.NewElement("message");
	xmlDocument.InsertFirstChild( pNode );
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// message > body ����
	tinyxml2::XMLElement *pElement = xmlDocument.NewElement("body");
	pNode->InsertEndChild(pElement);


	// message > body > result����
	tinyxml2::XMLElement *pElement2 = xmlDocument.NewElement("RESULT");
	if( bSuccess )
	{
		pElement2->SetText("OK");
	} else {
		pElement2->SetText("FAIL");
	}
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("SLOPEANGLE");
	pElement2->SetText(dSlopeAngle);	// �ϴ� ù��° ����� ������.
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("STEPHEIGHT");
	pElement2->SetText(dStepHeight);	// �ϴ� ù��° ����� ������.
	pElement->InsertEndChild(pElement2);
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// message > head ����
	pElement = xmlDocument.NewElement("header");
	pNode->InsertEndChild(pElement);

	// message > head > messageName ����
	pElement2 = xmlDocument.NewElement("messageName");
	pElement2->SetText("REPLY_MEASUREMENT");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("transactionId");
	pElement2->SetText("ex4682e8-00ed-4ac");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("timeStamp");
	pElement2->SetText("20150728080712");
	pElement->InsertEndChild(pElement2);
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// ���ڿ� ����
	tinyxml2::XMLPrinter xmlPrinter;
	tinyxml2::XMLDocument xmlDocument2;
	xmlDocument.Print(&xmlPrinter);

	m_strMessage.Format("%c%s%c", 2, (char*)xmlPrinter.CStr(), 3 );
	//////////////////////////////////////////////////////////////////////////


	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
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
int CProtocol::EncodeMeasureSmall( bool bSuccess, double dStepHeight, double dSlopeAngle )
{
	tinyxml2::XMLDocument xmlDocument;
	tinyxml2::XMLNode *pNode;


	//////////////////////////////////////////////////////////////////////////
	// message ����
	pNode = xmlDocument.NewElement("message");
	xmlDocument.InsertFirstChild( pNode );
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// message > body ����
	tinyxml2::XMLElement *pElement = xmlDocument.NewElement("body");
	pNode->InsertEndChild(pElement);


	// message > body > result����
	tinyxml2::XMLElement *pElement2 = xmlDocument.NewElement("RESULT");
	if( bSuccess )
	{
		pElement2->SetText("OK");
	} else {
		pElement2->SetText("FAIL");
	}
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("SLOPEANGLE");
	pElement2->SetText(dSlopeAngle);	// �ϴ� ù��° ����� ������.
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("STEPHEIGHT");
	pElement2->SetText(dStepHeight);	// �ϴ� ù��° ����� ������.
	pElement->InsertEndChild(pElement2);
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// message > head ����
	pElement = xmlDocument.NewElement("header");
	pNode->InsertEndChild(pElement);

	// message > head > messageName ����
	pElement2 = xmlDocument.NewElement("messageName");
	pElement2->SetText("REPLY_MEASUREMENT");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("transactionId");
	pElement2->SetText("ex4682e8-00ed-4ac");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("timeStamp");
	pElement2->SetText("20150728080712");
	pElement->InsertEndChild(pElement2);
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// ���ڿ� ����
	tinyxml2::XMLPrinter xmlPrinter;
	tinyxml2::XMLDocument xmlDocument2;
	xmlDocument.Print(&xmlPrinter);

	m_strMessage.Format("%c%s%c", 2, (char*)xmlPrinter.CStr(), 3 );
	//////////////////////////////////////////////////////////////////////////


	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//
// <message>
//		<body>
//			<RESULT>
//		<header>
//			<messageName> REPLY_LENS10X
//			<transactionId>
//			<timeStamp>
//
//////////////////////////////////////////////////////////////////////////
int CProtocol::EncodeLens10x(bool bSuccess)
{
	tinyxml2::XMLDocument xmlDocument;
	tinyxml2::XMLNode *pNode;


	//////////////////////////////////////////////////////////////////////////
	// message ����
	pNode = xmlDocument.NewElement("message");
	xmlDocument.InsertFirstChild( pNode );
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// message > body ����
	tinyxml2::XMLElement *pElement = xmlDocument.NewElement("body");
	pNode->InsertEndChild(pElement);

	// message > body > result����
	tinyxml2::XMLElement *pElement2 = xmlDocument.NewElement("RESULT");
	if( bSuccess )
	{
		pElement2->SetText("OK");
	} else {
		pElement2->SetText("FAIL");
	}
	pElement->InsertEndChild(pElement2);
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// message > head ����
	pElement = xmlDocument.NewElement("header");
	pNode->InsertEndChild(pElement);

	// message > head > messageName ����
	pElement2 = xmlDocument.NewElement("messageName");
	pElement2->SetText("REPLY_LENS10X");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("transactionId");
	pElement2->SetText("ex4682e8-00ed-4ac");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("timeStamp");
	pElement2->SetText("20150728080712");
	pElement->InsertEndChild(pElement2);
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// ���ڿ� ����
	tinyxml2::XMLPrinter xmlPrinter;
	tinyxml2::XMLDocument xmlDocument2;
	xmlDocument.Print(&xmlPrinter);

	m_strMessage.Format("%c%s%c", 2, (char*)xmlPrinter.CStr(), 3 );
	//////////////////////////////////////////////////////////////////////////


	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//
// <message>
//		<body>
//			<RESULT>
//			<MOVEX>
//			<MOVEY>
//		<header>
//			<messageName> REPLY_AUTOFOCUS
//			<transactionId>
//			<timeStamp>
//
//////////////////////////////////////////////////////////////////////////
int CProtocol::EncodeAutofocus( bool bSuccess, double dDx, double dDy )
{
	tinyxml2::XMLDocument xmlDocument;
	tinyxml2::XMLNode *pNode;


	//////////////////////////////////////////////////////////////////////////
	// message ����
	pNode = xmlDocument.NewElement("message");
	xmlDocument.InsertFirstChild( pNode );
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// message > body ����
	tinyxml2::XMLElement *pElement = xmlDocument.NewElement("body");
	pNode->InsertEndChild(pElement);


	// message > body > result����
	tinyxml2::XMLElement *pElement2 = xmlDocument.NewElement("RESULT");
	if( bSuccess )
	{
		pElement2->SetText("OK");
	} else {
		pElement2->SetText("FAIL");
	}
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("MOVEX");
	pElement2->SetText(dDx);	// �ϴ� ù��° ����� ������.
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("MOVEY");
	pElement2->SetText(dDy);	// �ϴ� ù��° ����� ������.
	pElement->InsertEndChild(pElement2);
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// message > head ����
	pElement = xmlDocument.NewElement("header");
	pNode->InsertEndChild(pElement);

	// message > head > messageName ����
	pElement2 = xmlDocument.NewElement("messageName");
	pElement2->SetText("REPLY_AUTOFOCUS");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("transactionId");
	pElement2->SetText("ex4682e8-00ed-4ac");
	pElement->InsertEndChild(pElement2);

	pElement2 = xmlDocument.NewElement("timeStamp");
	pElement2->SetText("20150728080712");
	pElement->InsertEndChild(pElement2);
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// ���ڿ� ����
	tinyxml2::XMLPrinter xmlPrinter;
	tinyxml2::XMLDocument xmlDocument2;
	xmlDocument.Print(&xmlPrinter);

	m_strMessage.Format("%c%s%c", 2, (char*)xmlPrinter.CStr(), 3 );
	//////////////////////////////////////////////////////////////////////////


	return 0;
}

