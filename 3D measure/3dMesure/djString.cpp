/*
[20161217-최동진]
1. djIsTrue(), djIsFalse() 등록

[20150429-최동진]
1. djVctCvtStrToFlt() 함수 오타 수

[20150225-최동진]
아래 추가
VOID djVctCvtStrToInt(vector<CString> * pStrList, vector<UINT> * pIntList);
BOOL djIsNumber(LPCTSTR pstrValue);
BOOL djIsRealNumber(LPCTSTR pstrValue);
BOOL djIsHexNumber(LPCTSTR pstrValue);

[20150120-최동진]
CString djMergeIntByComma(vector<UINT> * pIntList);

[20141226-최동진]
1.gkatn cnrk
VOID djVctCvtStrToInt(vector<CString> * pStrList, vector<INT> * pIntList);
VOID djVctCvtStrToFlt(vector<CString> * pStrList, vector<FLOAT> * pIntList);
CString djMergeFltByComma(vector<FLOAT> * pFltList);
CString djMergeIntByComma(vector<INT> * pIntList);


[20141021-최동진]
1. djVctCvtStrToFlt() 함수 추가

[20141020-최동진]
1. djMergeTextBy() 추가
2. djVctCvtIntToStr(), djVctCvtStrToInt() 함수 추가

[20140122-dongjin.choi]
*/



//
#include "stdafx.h"
#include "djString.h"

//20161112-최동진-구분자가 끝에 있는 경우 빈문자열이 추가되는 문제 조치
/*
BOOL djSplitTextBy(CString strText, CHAR ch, vector<CString> * pvTextList)
{
	//[파라미터 점검]
	if(pvTextList == NULL)
	{
		ASSERT(FALSE);

		return FALSE;
	}

	if((UINT)(pvTextList->size()) > 0)
		pvTextList->clear();

	//[토큰 분할]
	INT iStartOffset = 0;
	INT iSeperatorPos = 0;
	CString strToken;
	INT nTxtLen = strText.GetLength();

	while (iStartOffset < nTxtLen)
	{
		iSeperatorPos = strText.Find(ch, iStartOffset);

		if (iSeperatorPos < 0)
		{
			pvTextList->push_back(strText.Mid(iStartOffset));

			break;
		}
		else if (iStartOffset == iSeperatorPos)
		{
			pvTextList->push_back(_T(""));
			iStartOffset = iSeperatorPos + 1;
		}
		else
		{
			pvTextList->push_back(strText.Mid(iStartOffset, iSeperatorPos - iStartOffset));
			iStartOffset = iSeperatorPos + 1;
		}
	}

	//
	return TRUE;
}
*/
BOOL djSplitTextByComma(CString strText, vector<CString> * pvTextList)
{
	strText = djRemoveCharFromString(strText, _T(' '));

	//[파라미터 점검]
	if(pvTextList == NULL)
	{
		ASSERT(FALSE);

		return FALSE;
	}

	if((UINT)(pvTextList->size()) > 0)
		pvTextList->clear();

	//[토큰 분할]
	INT iStartOffset = 0;
	INT iSeperatorPos = 0;
	CString strToken;
	INT nTxtLen = strText.GetLength();

	while (iStartOffset < nTxtLen)
	{
		iSeperatorPos = strText.Find(_T(','), iStartOffset);

		if (iSeperatorPos < 0)
		{
			pvTextList->push_back(strText.Mid(iStartOffset));

			break;
		}
		else if (iStartOffset == iSeperatorPos)
		{
			pvTextList->push_back(_T(""));
			iStartOffset = iSeperatorPos + 1;
		}
		else
		{
			pvTextList->push_back(strText.Mid(iStartOffset, iSeperatorPos - iStartOffset));
			iStartOffset = iSeperatorPos + 1;
		}
	}

	//return djSplitTextBy(strText, _T(','), pvTextList);
	return TRUE;
}


BOOL djSplitTextByCommaGroupingByBracket(CString strText, vector<vector<CString>> * pvTextList)
{
	//[파라미터 점검]
	strText = djRemoveCharFromString(strText, _T(' '));

	if(strText == _T(""))
	{
		ASSERT(FALSE);

		return FALSE;
	}

	if(pvTextList == NULL)
	{
		ASSERT(FALSE);

		return FALSE;
	}

	if(pvTextList->size() != 0)
		pvTextList->clear();

	
	//[텍스트 점검 : 괄호쌍 - 이중 괄호 안됨 + 연속 ',']
	UINT nTextLength = 0;
	BOOL bBracketOpened = FALSE;

	nTextLength = strText.GetLength();

	for(UINT i = 0 ; i < nTextLength ; i++)
	{
		if(strText[i] == _T('('))
		{
			if(bBracketOpened == TRUE)
				return FALSE;
			else 
				bBracketOpened = TRUE;
		}
		else if(strText[i] == _T(')'))
		{
			if(bBracketOpened == FALSE)
				return FALSE;
			else
				bBracketOpened = FALSE;
		}
		else if(strText[i] == _T(',')) 
		{
			if(i == 0)
				return FALSE;
			else if(strText[i - 1] == _T(','))
				return FALSE;
		}
		
	}

	if(strText[nTextLength - 1] != _T(','))
		strText = strText + _T(",");

	//[텍스트 분할]
	vector<CString> vCurGroup;
	
	UINT iStartOffset = 0;
	INT  iCommaPos = -1;

	vCurGroup.reserve(10);
	bBracketOpened = FALSE;

	//
	do
	{
		if(bBracketOpened == FALSE && strText[iStartOffset] == _T('('))
		{
			iStartOffset += 1;
			bBracketOpened = TRUE;
		}

		iCommaPos = strText.Find(_T(','), iStartOffset);

		if(bBracketOpened == FALSE)
		{
			vCurGroup.push_back(strText.Mid(iStartOffset, iCommaPos - iStartOffset));

			pvTextList->push_back(vCurGroup);

			vCurGroup.clear();

			iStartOffset = iCommaPos + 1;
		}
		else
		{
			if(strText[iCommaPos - 1] == _T(')'))
			{
				vCurGroup.push_back(strText.Mid(iStartOffset, iCommaPos - iStartOffset - 1));

				pvTextList->push_back(vCurGroup);

				vCurGroup.clear();

				bBracketOpened = FALSE;

				iStartOffset = iCommaPos + 1;
			}
			else
			{
				vCurGroup.push_back(strText.Mid(iStartOffset, iCommaPos - iStartOffset));

				iStartOffset = iCommaPos + 1;
			}
		}
	}
	while(iStartOffset < nTextLength);

	//
	return TRUE;
}



BOOL djSplitTextByCommaGroupingByColon(CString strText, vector<vector<CString>> * pvTextList)
{
	//
	UINT nTextLength = 0;

	//[파라미터 점검]
	strText = djRemoveCharFromString(strText, _T(' '));
	nTextLength = strText.GetLength();

	if(strText == _T(""))
	{
		ASSERT(FALSE);

		return FALSE;
	}

	if(strText.Right(2) != _T(",:"))
	{
		strText = strText + _T(",:");
	}


	if(pvTextList == NULL)
	{
		ASSERT(FALSE);

		return FALSE;
	}

	if(pvTextList->size() != 0)
		pvTextList->clear();

	
	//[텍스트 분할]
	vector<CString> vEmpty;
	vector<CString> * pvCurGroup = NULL;
	CString strValue;
	
	UINT iStartOffset = 0;
	INT  iCommaPos = -1;
	INT  iColonPos = -1;
	UINT iGroup = 0;

	pvTextList->push_back(vEmpty);
	pvCurGroup = &(*pvTextList)[iGroup];

	//
	do
	{
		iCommaPos = strText.Find(_T(','), iStartOffset);
		iColonPos = strText.Find(_T(':'), iStartOffset);

		if(iCommaPos < iColonPos)
		{
			pvCurGroup->push_back(strText.Mid(iStartOffset, iCommaPos - iStartOffset));

			iStartOffset = iCommaPos + 1;

			if(iStartOffset < nTextLength)
			{
				pvTextList->push_back(vEmpty);

				iGroup++;
				
				pvCurGroup = &(*pvTextList)[iGroup];
			}
		}
		else
		{
			do
			{
				pvCurGroup->push_back(strText.Mid(iStartOffset, iColonPos - iStartOffset));

				iStartOffset = iColonPos + 1;

				iColonPos = strText.Find(_T(':'), iStartOffset);
			}
			while(iColonPos < iCommaPos);

			pvCurGroup->push_back(strText.Mid(iStartOffset, iCommaPos - iStartOffset));

			iStartOffset = iCommaPos + 1;

			if(iStartOffset < nTextLength)
			{
				pvTextList->push_back(vEmpty);

				iGroup++;
				
				pvCurGroup = &(*pvTextList)[iGroup];
			}
		}
	}
	while(iStartOffset < nTextLength);

	//
	return TRUE;
}

CString djRemoveCharFromString(CString strText, TCHAR ch)
{
	//
	strText.Trim();

	if(strText == _T(""))
		return strText;

	//
	CString strResult;
	UINT nTextLength = strText.GetLength();

	for(UINT i = 0 ; i < nTextLength ; i++)
	{
		if(strText[i] != ch)
		{
			strResult += strText[i];
		}
	}

	//
	return strResult;
}

CString djMergeTextBy(const vector<CString> * pvTextList, TCHAR ch)
{
	//
	ASSERT(pvTextList != NULL);

	//
	CString strText;
	INT nTextCount = (INT)pvTextList->size();

	if(nTextCount <= 4)
	{
		if(nTextCount == 4)
		{
			strText.Format(_T("%s%c%s%c%s%c%s"), (*pvTextList)[0], ch, (*pvTextList)[1], ch, (*pvTextList)[2], ch, (*pvTextList)[3]);
		}
		else if(nTextCount == 3)
		{
			strText.Format(_T("%s%c%s%c%s"), (*pvTextList)[0], ch, (*pvTextList)[1], ch, (*pvTextList)[2]);
		}
		else if(nTextCount == 2)
		{
			strText.Format(_T("%s%c%s"), (*pvTextList)[0], ch, (*pvTextList)[1]);
		}
		else if(nTextCount == 1)
		{
			strText = (*pvTextList)[0];
		}
		else if(nTextCount == 0)
		{
			strText = _T("");
		}
	}
	else
	{
		CString strTemp;

		strText.Format(_T("%s%c%s%c%s%c%s"), (*pvTextList)[0], ch, (*pvTextList)[1], ch, (*pvTextList)[2], ch, (*pvTextList)[3]);

		for(INT i = 4 ; i < nTextCount ; i++)
		{
			strTemp.Format(_T("%c%s"), ch, (*pvTextList)[i]);

			strText += strTemp;
		}
	}

	//
	return strText;
}

CString djMergeTextByComma(const vector<CString> * pvTextList)
{
	return djMergeTextBy(pvTextList, _T(','));
}

CString djMergeFltByComma(const vector<FLOAT> * pFltList)
{
	//
	ASSERT(pFltList != NULL);

	//
	CString strText;
	INT nTextCount = (INT)pFltList->size();

	if(nTextCount <= 4)
	{
		if(nTextCount == 4)
		{
			strText.Format(_T("%.3f,%.3f,%.3f,%.3f"), (*pFltList)[0], (*pFltList)[1], (*pFltList)[2], (*pFltList)[3]);
		}
		else if(nTextCount == 3)
		{
			strText.Format(_T("%.3f,%.3f,%.3f"), (*pFltList)[0], (*pFltList)[1], (*pFltList)[2]);
		}
		else if(nTextCount == 2)
		{
			strText.Format(_T("%.3f,%.3f"), (*pFltList)[0], (*pFltList)[1]);
		}
		else if(nTextCount == 1)
		{
			strText.Format(_T("%.3f"), (*pFltList)[0]);
		}
		else if(nTextCount == 0)
		{
			strText = _T("");
		}
	}
	else
	{
		CString strTemp;

		strText.Format(_T("%.3f,%.3f,%.3f,%.3f"), (*pFltList)[0], (*pFltList)[1], (*pFltList)[2], (*pFltList)[3]);

		for(INT i = 4 ; i < nTextCount ; i++)
		{
			strTemp.Format(_T(",%.3f"), (*pFltList)[i]);

			strText += strTemp;
		}
	}

	//
	return strText;
}

CString djMergeIntByComma(const vector<INT> * pIntList)
{
	//
	ASSERT(pIntList != NULL);

	//
	CString strText;
	INT nTextCount = (INT)pIntList->size();

	if(nTextCount <= 4)
	{
		if(nTextCount == 4)
		{
			strText.Format(_T("%d,%d,%d,%d"), (*pIntList)[0], (*pIntList)[1], (*pIntList)[2], (*pIntList)[3]);
		}
		else if(nTextCount == 3)
		{
			strText.Format(_T("%d,%d,%d"), (*pIntList)[0], (*pIntList)[1], (*pIntList)[2]);
		}
		else if(nTextCount == 2)
		{
			strText.Format(_T("%d,%d"), (*pIntList)[0], (*pIntList)[1]);
		}
		else if(nTextCount == 1)
		{
			strText.Format(_T("%d"), (*pIntList)[0]);
		}
		else if(nTextCount == 0)
		{
			strText = _T("");
		}
	}
	else
	{
		CString strTemp;

		strText.Format(_T("%d,%d,%d,%d"), (*pIntList)[0], (*pIntList)[1], (*pIntList)[2], (*pIntList)[3]);

		for(INT i = 4 ; i < nTextCount ; i++)
		{
			strTemp.Format(_T(",%d"), (*pIntList)[i]);

			strText += strTemp;
		}
	}

	//
	return strText;
}

CString djMergeIntByComma(const vector<UINT> * pIntList)
{
	//
	ASSERT(pIntList != NULL);

	//
	CString strText;
	INT nTextCount = (INT)pIntList->size();

	if(nTextCount <= 4)
	{
		if(nTextCount == 4)
		{
			strText.Format(_T("%d,%d,%d,%d"), (*pIntList)[0], (*pIntList)[1], (*pIntList)[2], (*pIntList)[3]);
		}
		else if(nTextCount == 3)
		{
			strText.Format(_T("%d,%d,%d"), (*pIntList)[0], (*pIntList)[1], (*pIntList)[2]);
		}
		else if(nTextCount == 2)
		{
			strText.Format(_T("%d,%d"), (*pIntList)[0], (*pIntList)[1]);
		}
		else if(nTextCount == 1)
		{
			strText.Format(_T("%d"), (*pIntList)[0]);
		}
		else if(nTextCount == 0)
		{
			strText = _T("");
		}
	}
	else
	{
		CString strTemp;

		strText.Format(_T("%d,%d,%d,%d"), (*pIntList)[0], (*pIntList)[1], (*pIntList)[2], (*pIntList)[3]);

		for(INT i = 4 ; i < nTextCount ; i++)
		{
			strTemp.Format(_T(",%d"), (*pIntList)[i]);

			strText += strTemp;
		}
	}

	//
	return strText;
}

BOOL djSplitTextByTabW(const wchar_t * pSrcText, vector<wstring> * pTxtList)
{
	//[파라미터 점검]
	if(pTxtList == NULL)
	{
		ASSERT(FALSE);

		return FALSE;
	}

	if((UINT)(pTxtList->size()) > 0)
		pTxtList->clear();

	//토크닝을 위한 임시 문자열 생성
	INT nStrLen = (INT)wcslen(pSrcText);
	wchar_t * pNewSrcTxt = new wchar_t[nStrLen + 1];
	
	wcscpy_s(pNewSrcTxt, nStrLen + 1, pSrcText);

	//토큰 분할
	wchar_t * pTokenPos = NULL;
	wchar_t * pNextTokenPos = NULL;
	wchar_t tokens[] = L"\t";

	pTokenPos = wcstok_s(pNewSrcTxt, tokens, &pNextTokenPos);

	while(pTokenPos != NULL)
	{
		//
		wstring txt(pTokenPos);

		pTxtList->push_back(txt);

		//
		pTokenPos = wcstok_s(NULL, tokens, &pNextTokenPos);
	}

	//임시 문자열 제거
	delete [] pNewSrcTxt;
	pNewSrcTxt = NULL;

	//
	return TRUE;
}


VOID djVctCvtIntToStr(const vector<INT> * pIntList, vector<CString> * pStrList)
{
	//
	ASSERT(pIntList != NULL);
	ASSERT(pStrList != NULL);

	//
	pStrList->clear();

	//
	INT nItemNum = (INT)(*pIntList).size();

	for(INT i = 0 ; i < nItemNum ; i++)
	{
		CString strValue;

		strValue.Format(_T("%d"), (*pIntList)[i]);

		pStrList->push_back(strValue);
	}
}

VOID djVctCvtFltToStr(const vector<FLOAT> * pFltList, vector<CString> * pStrList)
{
	//
	ASSERT(pFltList != NULL);
	ASSERT(pStrList != NULL);

	//
	pStrList->clear();

	//
	INT nItemNum = (INT)(*pFltList).size();

	for(INT i = 0 ; i < nItemNum ; i++)
	{
		CString strValue;

		strValue.Format(_T("%.3f"), (*pFltList)[i]);

		pStrList->push_back(strValue);
	}
}


VOID djVctCvtStrToInt(const vector<CString> * pStrList, vector<INT> * pIntList)
{
	//
	ASSERT(pIntList != NULL);
	ASSERT(pStrList != NULL);

	//
	pIntList->clear();

	//
	INT nItemNum = (INT)(*pStrList).size();

	for(INT i = 0 ; i < nItemNum ; i++)
	{
		pIntList->push_back(_tstoi((*pStrList)[i]));
	}
}

VOID djVctCvtStrToInt(const vector<CString> * pStrList, vector<UINT> * pIntList)
{
	//
	ASSERT(pIntList != NULL);
	ASSERT(pStrList != NULL);

	//
	pIntList->clear();

	//
	INT nItemNum = (INT)(*pStrList).size();

	for(INT i = 0 ; i < nItemNum ; i++)
	{
		pIntList->push_back(_tstoi((*pStrList)[i]));
	}
}

VOID djVctCvtStrToFlt(const vector<CString> * pStrList, vector<FLOAT> * pFltList)
{
	//
	ASSERT(pFltList != NULL);
	ASSERT(pStrList != NULL);

	//
	pFltList->clear();

	//
	INT nItemNum = (INT)(*pStrList).size();

	for(INT i = 0 ; i < nItemNum ; i++)
	{
		pFltList->push_back((FLOAT)_tstof((*pStrList)[i]));
	}
}

BOOL djIsNumber(LPCTSTR pstrValue)
{
	//
	ASSERT(pstrValue != NULL);

	//
	CString strNumber = pstrValue;
	CString strTemp;
	int nLen = strNumber.GetLength();

	for( int i=0 ; i < nLen ; ++i)
	{
		strTemp = strNumber.GetAt(i);
		if(strTemp == _T("-"))
			continue;

		if(isdigit(strNumber.GetAt(i)) == FALSE)
			return FALSE;
	}

	return TRUE;
}

BOOL djIsRealNumber(LPCTSTR pstrValue)
{
	//
	ASSERT(pstrValue != NULL);

	//
	CString strNumber = pstrValue;
	int nLen = strNumber.GetLength();
	CString strTemp;
	int nDotCount=0;

	for( int i=0 ; i < nLen ; ++i)
	{
		strTemp = strNumber.GetAt(i);
		if(strTemp == _T(".") || strTemp == _T("-"))
			continue;

		if( isdigit(strNumber.GetAt(i)) == FALSE)
		{
			return FALSE;
		}

		if(strTemp == _T("."))
		{
			nDotCount ++;
			if(nDotCount > 1)
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

BOOL djIsHexNumber(LPCTSTR pstrValue)
{
	//
	ASSERT(pstrValue != NULL);

	//
	CString strNumber = pstrValue;
	CString strTemp;
	int nLen = strNumber.GetLength();

	strNumber.MakeUpper();

	for( int i=0 ; i < nLen ; ++i)
	{
		strTemp = strNumber.GetAt(i);

		if(!(((_T("0") <= strTemp) && (strTemp <= _T("9"))) || ((_T("A") <= strTemp) && (strTemp <= _T("F")))))
			return FALSE;
	}

	return TRUE;
}

BOOL djIsTrue(LPCTSTR pstrValue)
{
	//
	ASSERT(pstrValue != NULL);

	//
	CString strVal = pstrValue;

	strVal.Trim();
	strVal.MakeUpper();

	if (strVal == _T("T") || strVal == _T("TRUE") || strVal == _T("1"))
		return TRUE;
	else
		return FALSE;
}

BOOL djIsFalse(LPCTSTR pstrValue)
{
	//
	ASSERT(pstrValue != NULL);

	//
	CString strVal = pstrValue;

	strVal.Trim();
	strVal.MakeUpper();

	if (strVal == _T("F") || strVal == _T("FALSE") || strVal == _T("0"))
		return TRUE;
	else
		return FALSE;
}