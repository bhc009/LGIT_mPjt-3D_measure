#include "StdAfx.h"
#include "DistanceMeasure.h"


UINT threadProcDistanceMeasure(LPVOID lParam )
{
	CDistanceMeasure *pMain = (CDistanceMeasure*)lParam;


	while( TRUE )
	{
		WaitForSingleObject( pMain->m_hEventMeasure, INFINITE );
		
		double i;
		while( pMain->m_bMeasure )
		{
			pMain->m_listData.push_back(i);
			i += 0.1;
			
			Sleep(pMain->m_interval);
		}

	}

	return 0;
}


CDistanceMeasure::CDistanceMeasure(void)
{
	m_hEventMeasure = CreateEvent( NULL, FALSE, FALSE, NULL );
	m_hThreadMeasure = AfxBeginThread( threadProcDistanceMeasure, (LPVOID)this );

	m_bMeasure = false;

	m_interval = 500;

	HANDLE m_hEventMeasure = CreateEvent( NULL, TRUE, FALSE, NULL );
	// 	m_hThreadMeasure = AfxBeginThread( threadProcMeasure, (LPVOID)this );

}


CDistanceMeasure::~CDistanceMeasure(void)
{
	if( m_hThreadMeasure )
	{
		delete m_hThreadMeasure;
	}
	m_hThreadMeasure = NULL;
}


bool CDistanceMeasure::startMeasure()
{
	m_bMeasure = true;

	m_listData.clear();

	SetEvent(m_hEventMeasure);

	return true;
}


bool CDistanceMeasure::endMeasure()
{
	m_bMeasure = false;

	return true;
}
