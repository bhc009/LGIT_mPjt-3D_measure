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
// 수신 명령 해석
//
//////////////////////////////////////////////////////////////////////////
int CProtocol::Decode(unsigned char *pChar, int nLength)
{
	if( nLength<40)
	{
		return BHPROTOCOL_FAIL;
	}


	// 수신 버퍼 생성
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



	// 명령 부분 추출 : 
	tinyxml2::XMLElement *pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("header")->FirstChildElement("messageName");

	char *pCharText = (char*)pElement->GetText();

	CString strRecive;
	strRecive.Format("%s", pCharText);



	// 명령 부분 추출 : 
	//////////////////////////////////////////////////////////////////////////
	if( strRecive=="REQUEST_RECIPE" )
	{
		// 해석
		// pitch 정보
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("PITCHX");
		pElement->QueryDoubleText(&m_dPitchX);

		// pitch 정보
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("PITCHY");
		pElement->QueryDoubleText(&m_dPitchY);

		// size 정보
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("SIZEX");
		pElement->QueryDoubleText(&m_dSizeX);

		// size 정보
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("SIZEY");
		pElement->QueryDoubleText(&m_dSizeY);

		// shape 정보
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("SHAPE");
		pElement->QueryIntText(&m_iShape);

		// 배열 정보
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("ARRANGE");
		pElement->QueryIntText(&m_iArrage);

		// size 정보
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("THICKNESS");
		pElement->QueryDoubleText(&m_dThickness);

		return BHPROTOCOL_RECIPE;
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// 렌즈 변경 명령일 경우
	if( strRecive=="REQUEST_LENS10X" )
	{
		return BHPROTOCOL_LENS10X;
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// 렌즈 변경 명령일 경우
	if( strRecive=="REQUEST_STOP" )
	{
		return BHPROTOCOL_STOP;
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// Auto focus 명령일 경우
	if( strRecive=="REQUEST_AUTOFOCUS" )
	{
		return BHPROTOCOL_AUTOFOCUS;
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// Calibration 명령일 경우
	if( strRecive=="REQUEST_CAL" )
	{
		return BHPROTOCOL_CALIBRATION;
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// 측정 명령일 경우
	if( strRecive=="REQUEST_MEASUREMENT" )
	{
		// 상세 모드 확인
		int iMethode;
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("METHOD");
		pElement->QueryIntText(&iMethode);

		// 레시피 명
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("RECIPE");
		m_strRecipe = pElement->GetText();

		// QR code
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("QRCODE");
		m_strQRCode = pElement->GetText();

		// * 이노텍의 경우 결과 파일 저장을 위한 레시피 QR 코드 정보 받음
		if( m_iSite==BHPROTOCOL_LGIT )
		{
// 			// 레시피 명
// 			pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("RECIPE");
// 			m_strRecipe = pElement->GetText();
// 
// 			// QR code
// 			pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("QRCODE");
// 			m_strQRCode = pElement->GetText();

			// 두께센서 사용 여부
			pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("USE_THICK");
			int iUse = 0;
			pElement->QueryIntText(&iUse);
			if( iUse )
			{
				m_bUseThickness = TRUE;
			} else {
				m_bUseThickness = FALSE;
			}

			// 상위 스캔 범위
			pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("SCAN_UP");
			pElement->QueryDoubleText(&m_d3dRangeUp);

			// 하위 스캔 범위
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
		case 1 : // 1면검사
			return BHPROTOCOL_MESURE_LARGE;

		case 2 : // 2면검사
			return BHPROTOCOL_MESURE_SMALL;

		default:
			return BHPROTOCOL_FAIL;
		}
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// Calibration 명령일 경우
	if( strRecive=="REQUEST_THICK" )
	{
		// 해석
		// pitch 정보
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("TARGET");
		CString str = pElement->GetText();
		if( str=="CHUCK")
		{
			m_bCAL = TRUE;
		} else {
			m_bCAL = FALSE;
		}


		// pitch 정보
		int iIndex;
		pElement = xmlDocument.FirstChildElement("message")->FirstChildElement("body")->FirstChildElement("INDEX");
		pElement->QueryIntText(&iIndex);
		m_thicknessIndex = iIndex - 1;


		return BHPROTOCOL_THICKNESS;
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// 렌즈 변경 명령일 경우
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
// 3D 광학계의 현재 상태 encoding
//
//////////////////////////////////////////////////////////////////////////
int CProtocol::EncodeState()
{

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// 레시피 설정 명령에 대한 응답 생성
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


	// message 생성
	pNode = xmlDocument.NewElement("message");
	xmlDocument.InsertFirstChild( pNode );


	// message > body 생성
	tinyxml2::XMLElement *pElement = xmlDocument.NewElement("body");
	pNode->InsertEndChild(pElement);

	// message > body > result생성
	tinyxml2::XMLElement *pElement2 = xmlDocument.NewElement("RESULT");
	if( bSuccess )
	{
		pElement2->SetText("OK");
	} else {
		pElement2->SetText("FAIL");
	}
	pElement->InsertEndChild(pElement2);


	// message > head 생성
	pElement = xmlDocument.NewElement("header");
	pNode->InsertEndChild(pElement);

	// message > head > messageName 생성
	pElement2 = xmlDocument.NewElement("messageName");
	pElement2->SetText("REPLY_RECIPE");
	pElement->InsertEndChild(pElement2);

	// message > head > transactionId 생성
	pElement2 = xmlDocument.NewElement("transactionId");
	pElement2->SetText("ex4682e8-00ed-4ac");
	pElement->InsertEndChild(pElement2);

	// message > head > timeStamp 생성
	pElement2 = xmlDocument.NewElement("timeStamp");
	pElement2->SetText("20150728080712");
	pElement->InsertEndChild(pElement2);


	// File로 저장하기
	// 	tinyxml2::XMLError err = xmlDocument.SaveFile(XML_FILE_PATH);


	// 문자열 생성
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
	// message 생성
	pNode = xmlDocument.NewElement("message");
	xmlDocument.InsertFirstChild( pNode );
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// message > body 생성
	tinyxml2::XMLElement *pElement = xmlDocument.NewElement("body");
	pNode->InsertEndChild(pElement);


	// message > body > result생성
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
	// message > head 생성
	pElement = xmlDocument.NewElement("header");
	pNode->InsertEndChild(pElement);

	// message > head > messageName 생성
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
	// 문자열 생성
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
	// message 생성
	pNode = xmlDocument.NewElement("message");
	xmlDocument.InsertFirstChild( pNode );
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// message > body 생성
	tinyxml2::XMLElement *pElement = xmlDocument.NewElement("body");
	pNode->InsertEndChild(pElement);


	// message > body > result생성
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
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// message > head 생성
	pElement = xmlDocument.NewElement("header");
	pNode->InsertEndChild(pElement);

	// message > head > messageName 생성
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
	// 문자열 생성
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
	// message 생성
	pNode = xmlDocument.NewElement("message");
	xmlDocument.InsertFirstChild( pNode );
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// message > body 생성
	tinyxml2::XMLElement *pElement = xmlDocument.NewElement("body");
	pNode->InsertEndChild(pElement);


	// message > body > result생성
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
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// message > head 생성
	pElement = xmlDocument.NewElement("header");
	pNode->InsertEndChild(pElement);

	// message > head > messageName 생성
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
	// 문자열 생성
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
	// message 생성
	pNode = xmlDocument.NewElement("message");
	xmlDocument.InsertFirstChild( pNode );
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// message > body 생성
	tinyxml2::XMLElement *pElement = xmlDocument.NewElement("body");
	pNode->InsertEndChild(pElement);

	// message > body > result생성
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
	// message > head 생성
	pElement = xmlDocument.NewElement("header");
	pNode->InsertEndChild(pElement);

	// message > head > messageName 생성
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
	// 문자열 생성
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
	// message 생성
	pNode = xmlDocument.NewElement("message");
	xmlDocument.InsertFirstChild( pNode );
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// message > body 생성
	tinyxml2::XMLElement *pElement = xmlDocument.NewElement("body");
	pNode->InsertEndChild(pElement);


	// message > body > result생성
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
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// message > head 생성
	pElement = xmlDocument.NewElement("header");
	pNode->InsertEndChild(pElement);

	// message > head > messageName 생성
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
	// 문자열 생성
	tinyxml2::XMLPrinter xmlPrinter;
	tinyxml2::XMLDocument xmlDocument2;
	xmlDocument.Print(&xmlPrinter);

	m_strMessage.Format("%c%s%c", 2, (char*)xmlPrinter.CStr(), 3 );
	//////////////////////////////////////////////////////////////////////////


	return 0;
}

