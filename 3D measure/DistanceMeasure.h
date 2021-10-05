#pragma once

#include <list>
using namespace std;

class CDistanceMeasure
{
public:
	CDistanceMeasure(void);
	~CDistanceMeasure(void);


	HANDLE m_hEventMeasure;
	CWinThread *m_hThreadMeasure;

	bool m_bMeasure;
	int m_interval;

	list<double> m_listData;

	bool startMeasure();
	bool endMeasure();
};

