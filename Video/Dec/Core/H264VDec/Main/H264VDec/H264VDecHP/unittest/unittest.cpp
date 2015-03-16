//
// unittest.cpp
//


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include "windows.h"

static CRITICAL_SECTION crit;
static volatile LONG interlocked_ = 0;

void *func1( void * ptr )
{
	for(int i=0; i<3; i++)
	{
	fprintf(stderr, "[%lu] func1 started(%d)\n", GetTickCount(), i);
	fprintf(stderr, "[%lu] func1 EnterCriticalSection\n", GetTickCount());
	EnterCriticalSection( &crit );
	fprintf(stderr, "[%lu] func1 CriticalSection\n", GetTickCount());
	sleep(3);
	LeaveCriticalSection( &crit );
	fprintf(stderr, "[%lu] func1 LeaveCriticalSection\n", GetTickCount());
	fprintf(stderr, "[%lu] func1 finished\n", GetTickCount());
	}

	return 0;
}

void *func2( void * ptr )
{
	for(int i=0; i<3; i++)
	{
	fprintf(stderr, "[%lu] func2 started(%d)\n", GetTickCount(), i);
	fprintf(stderr, "[%lu] func2 EnterCriticalSection\n", GetTickCount());
	EnterCriticalSection( &crit );
	fprintf(stderr, "[%lu] func2 CriticalSection\n", GetTickCount());
	sleep(2);
	LeaveCriticalSection( &crit );
	fprintf(stderr, "[%lu] func2 LeaveCriticalSection\n", GetTickCount());
	fprintf(stderr, "[%lu] func2 finished\n", GetTickCount());
	}

	return 0;
}

void critical_section_test()
{
	pthread_t thd1, thd2;
	DWORD dwStart, dwStop, dwElapsed;

	InitializeCriticalSection( &crit );

	fprintf(stderr, "[%lu] unittest started\n", GetTickCount());

	dwStart = GetTickCount();

	pthread_create( &thd1, NULL, func1, NULL );
	pthread_create( &thd2, NULL, func2, NULL );

	fprintf(stderr, "[%lu] threads started\n", GetTickCount());

	pthread_join( thd1, NULL );
	pthread_join( thd2, NULL );

	dwStop = GetTickCount();

	DeleteCriticalSection( &crit );

	dwElapsed = dwStop - dwStart;

	if( dwElapsed >= 15000)
		fprintf(stderr,"[PASS] CriticalSection Test\n");
	else
		fprintf(stderr,"[FAIL] CriticalSection Test\n");

	fprintf(stderr, "[%lu] unittest finished\n", GetTickCount());
}

void *func3( void * ptr )
{
	fprintf(stderr, "[%lu] func3 started\n", GetTickCount());
	for(int i=0; i<1000; i++)
	{
		InterlockedIncrement( &interlocked_ );
		//usleep(4);
		InterlockedDecrement( &interlocked_ );
		//usleep(5);
		InterlockedIncrement( &interlocked_ );
	}
	fprintf(stderr, "[%lu] func3 finished\n", GetTickCount());

	return 0;
}

void *func4( void * ptr )
{
	fprintf(stderr, "[%lu] func4 started\n", GetTickCount());
	for(int i=0; i<999; i++)
	{
		InterlockedDecrement( &interlocked_ );
		//usleep(5);
		InterlockedIncrement( &interlocked_ );
		//usleep(4);
		InterlockedDecrement( &interlocked_ );
	}
	fprintf(stderr, "[%lu] func4 finished\n", GetTickCount());

   return 0;
}

void interlocked_test()
{
	pthread_t thd1, thd2;
	DWORD dwStart, dwStop, dwElapsed;

	fprintf(stderr, "[%lu] Interlocked Test started\n", GetTickCount());

	dwStart = GetTickCount();

	pthread_create( &thd1, NULL, func3, NULL );
	pthread_create( &thd2, NULL, func4, NULL );

	fprintf(stderr, "[%lu] threads started\n", GetTickCount());

	pthread_join( thd1, NULL );
	pthread_join( thd2, NULL );

	dwStop = GetTickCount();

	dwElapsed = dwStop - dwStart;

	if( interlocked_ == 1)
		fprintf(stderr,"[PASS] Interlocked Test Test\n");
	else
		fprintf(stderr,"[FAIL] Interlocked Test Test: %d\n",interlocked_);

	fprintf(stderr, "[%lu] Interlocked Test finished\n", GetTickCount());
}

static HANDLE evt_;

void *func5( void * ptr )
{
	fprintf(stderr, "[%lu] func5 started\n", GetTickCount());

	fprintf(stderr, "[%lu] func5 sleep for 2 seconds\n", GetTickCount());
	sleep( 2 );
	fprintf(stderr, "[%lu] func5 SetEvent fire\n", GetTickCount());
	SetEvent( evt_ );
	fprintf(stderr, "[%lu] func5 finished\n", GetTickCount());

	fprintf(stderr, "[%lu] func5 sleep for 2 seconds\n", GetTickCount());
	sleep( 2 );
	fprintf(stderr, "[%lu] func5 SetEvent fire\n", GetTickCount());
	ResetEvent( evt_ );
	SetEvent( evt_ );
	fprintf(stderr, "[%lu] func5 finished\n", GetTickCount());

	return 0;
}

static DWORD dwElapsed[4];

void *func6( void * ptr )
{
	DWORD dwStart, dwStop;
	fprintf(stderr, "[%lu] func6 started\n", GetTickCount());

	dwStart = GetTickCount();
	fprintf(stderr, "[%lu] 1:WaitForSingleObject\n", GetTickCount());
	WaitForSingleObject( evt_, INFINITE );
	fprintf(stderr, "[%lu] 2:WaitForSingleObject\n", GetTickCount());
	WaitForSingleObject( evt_, INFINITE );
	fprintf(stderr, "[%lu] 3:WaitForSingleObject\n", GetTickCount());
	dwStop = GetTickCount();
	dwElapsed[0] = dwStop - dwStart;
	ResetEvent( evt_ );

	dwStart = GetTickCount();
	fprintf(stderr, "[%lu] 4:WaitForSingleObject\n", GetTickCount());
	WaitForSingleObject( evt_, INFINITE );
	fprintf(stderr, "[%lu] 5:WaitForSingleObject\n", GetTickCount());
	WaitForSingleObject( evt_, INFINITE );
	fprintf(stderr, "[%lu] 6:WaitForSingleObject\n", GetTickCount());
	dwStop = GetTickCount();
	dwElapsed[1] = dwStop - dwStart;
	ResetEvent( evt_ );

	dwStart = GetTickCount();
	fprintf(stderr, "[%lu] 7:WaitForSingleObject\n", GetTickCount());
	WaitForSingleObject( evt_, INFINITE );
	fprintf(stderr, "[%lu] 8:WaitForSingleObject\n", GetTickCount());
	WaitForSingleObject( evt_, INFINITE );
	fprintf(stderr, "[%lu] 9:WaitForSingleObject\n", GetTickCount());
	dwStop = GetTickCount();
	dwElapsed[2] = dwStop - dwStart;


	fprintf(stderr, "[%lu] func6 finished\n", GetTickCount());

   return 0;
}

void event_test()
{
	pthread_t thd1, thd2;
	DWORD dwStart, dwStop;

	evt_ = CreateEvent( 0, TRUE, TRUE, 0 );

	fprintf(stderr, "[%lu] Event Test started\n", GetTickCount());

	dwStart = GetTickCount();

	pthread_create( &thd1, NULL, func5, NULL );
	pthread_create( &thd2, NULL, func6, NULL );

	fprintf(stderr, "[%lu] threads started\n", GetTickCount());

	pthread_join( thd1, NULL );
	pthread_join( thd2, NULL );

	dwStop = GetTickCount();

   for(int i=0; i < 3 ; i++ )
		fprintf(stderr,"dwElapsed[%lu]=%d\n",i,dwElapsed[i]);

	if( dwElapsed[0] > 2) {
		fprintf(stderr,"[FAIL] Event Test 1\n");
		return;
	}
	if( dwElapsed[1] < 2000) {
		fprintf(stderr,"[FAIL] Event Test 2\n");
		return;
	}
	if( dwElapsed[2] < 2000) {
		fprintf(stderr,"[FAIL] Event Test 3\n");
		return;
	}

	fprintf(stderr,"[PASS] Event Test\n");

	fprintf(stderr, "[%lu] Event Test finished\n", GetTickCount());
}

int main()
{
	critical_section_test();
	interlocked_test();
	event_test();
}
