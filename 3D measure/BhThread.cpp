#include "StdAfx.h"
#include "BhThread.h"


CBhThread::CBhThread(void)
{
	m_hEventMeasure = CreateEvent( NULL, TRUE, FALSE, NULL );

	m_hEventAutofocus = CreateEvent( NULL, TRUE, FALSE, NULL );

	m_hEventCalibration = CreateEvent( NULL, TRUE, FALSE, NULL );

	m_hEventChangeLens = CreateEvent( NULL, TRUE, FALSE, NULL );
}


CBhThread::~CBhThread(void)
{
}

