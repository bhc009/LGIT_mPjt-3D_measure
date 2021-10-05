/*
[20161217-�ֵ���]
1. djIsTrue(), djIsFalse() ���

[20150429-�ֵ���]
1. djVctCvtStrToFlt() �Լ� ��Ÿ ��

[20150225-�ֵ���]
�Ʒ� �߰�
VOID djVctCvtStrToInt(vector<CString> * pStrList, vector<UINT> * pIntList);
BOOL djIsNumber(LPCTSTR pstrValue);
BOOL djIsRealNumber(LPCTSTR pstrValue);
BOOL djIsHexNumber(LPCTSTR pstrValue);

[20150120-�ֵ���]
CString djMergeIntByComma(vector<UINT> * pIntList);

[20141226-�ֵ���]
1.gkatn cnrk
VOID djVctCvtStrToInt(vector<CString> * pStrList, vector<INT> * pIntList);
VOID djVctCvtStrToFlt(vector<CString> * pStrList, vector<FLOAT> * pIntList);
CString djMergeFltByComma(vector<FLOAT> * pFltList);
CString djMergeIntByComma(vector<INT> * pIntList);


[20141021-�ֵ���]
1. djVctCvtStrToFlt() �Լ� �߰�

[20141020-�ֵ���]
1. djMergeTextBy() �߰�
2. djVctCvtIntToStr(), djVctCvtStrToInt() �Լ� �߰�

[20140122-dongjin.choi]
*/

//
#pragma once

//
using namespace std;

//
#include <vector>

//[���ڿ�]
//BOOL djSplitTextBy(CString strText, TCHAR ch, vector<CString> * pvTextList);
BOOL djSplitTextByComma(CString strText, vector<CString> * pvTextList);
BOOL djSplitTextByCommaGroupingByBracket(CString strText, vector<vector<CString>> * pvTextList);
BOOL djSplitTextByCommaGroupingByColon(CString strText, vector<vector<CString>> * pvTextList);
CString djRemoveCharFromString(CString strText, TCHAR ch);
CString djMergeTextBy(const vector<CString> * pvTextList, TCHAR ch);
CString djMergeTextByComma(const vector<CString> * pvTextList);
CString djMergeFltByComma(const vector<FLOAT> * pFltList);
CString djMergeIntByComma(const vector<INT> * pIntList);
CString djMergeIntByComma(const vector<UINT> * pIntList);

//[����]
VOID djVctCvtIntToStr(const vector<INT> * pIntList, vector<CString> * pStrList);
VOID djVctCvtFltToStr(const vector<FLOAT> * pFltList, vector<CString> * pStrList);
VOID djVctCvtStrToInt(const vector<CString> * pStrList, vector<INT> * pIntList);
VOID djVctCvtStrToInt(const vector<CString> * pStrList, vector<UINT> * pIntList);
VOID djVctCvtStrToFlt(const vector<CString> * pStrList, vector<FLOAT> * pFltList);

//
BOOL djIsNumber(LPCTSTR pstrValue);
BOOL djIsRealNumber(LPCTSTR pstrValue);
BOOL djIsHexNumber(LPCTSTR pstrValue);

BOOL djIsTrue(LPCTSTR pstrValue); //T, TRUE, 1
BOOL djIsFalse(LPCTSTR pstrValue); //F, FALSE, 0

//unicode
BOOL djSplitTextByTabW(const wchar_t * pSrcText, vector<wstring> * pTxtList);
