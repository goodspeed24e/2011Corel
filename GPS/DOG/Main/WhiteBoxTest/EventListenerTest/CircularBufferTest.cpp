#include "CircularBuffer.h"
#include "AsyncCircularBuffer.h"
#include <gtest/gtest.h>

#pragma warning(disable: 4267) // warning C4267: 'argument' : conversion from 'size_t' to 'int', possible loss of data

namespace
{

class CircularBuffeTestFixture : public ::testing::Test
{
protected:
	CircularBuffeTestFixture()
	{}

	virtual ~CircularBuffeTestFixture()
	{}

	static void SetUpTestCase() 
	{}

	static void TearDownTestCase() 
	{}

	virtual void SetUp()
	{}

	virtual void TearDown()
	{}
};

// Check queue's initial state, and basic operations
// - check Push() operation
// - check Pop() operation
// - check Pop() for empty queue
TEST_F(CircularBuffeTestFixture, Basic)
{
	const size_t QSIZE = 12;
	TCircularBuffer<int> testQueue(QSIZE);

	// queue is initialized with empty
	EXPECT_EQ( testQueue.GetSize(), 0);

	// check Push() operation
	for(int i=0; i<QSIZE; ++i)
	{
		testQueue.Push(i);
		EXPECT_EQ( testQueue.GetSize(), i+1);
	}
	EXPECT_EQ( testQueue.GetSize(), QSIZE);
	EXPECT_EQ( testQueue.Front(), 0);

	// check the result of Push()
	for(int i=0; i<QSIZE; ++i)
	{
		EXPECT_EQ( testQueue[i], i);
	}

	// check Pop() operation
	for(int i=0; i<QSIZE; ++i)
	{
		EXPECT_EQ( testQueue.Front(), i);
		testQueue.Pop();
		EXPECT_EQ( testQueue.GetSize(), QSIZE-1-i);
	}
	EXPECT_EQ( testQueue.GetSize(), 0);

	// check Pop() for empty queue
	for(int i=0; i<5; ++i)
	{
		testQueue.Pop();
		EXPECT_EQ( testQueue.GetSize(), 0);
	}

	// push new data to a empty queue
	for(int i=0; i<5; ++i)
	{
		testQueue.Push(i);
		EXPECT_EQ( testQueue.GetSize(), i+1);
	}
	EXPECT_EQ( testQueue.GetSize(), 5);

	testQueue.Clear();
	EXPECT_EQ( testQueue.GetSize(), 0);
}


// Test Push() when queue is full
TEST_F(CircularBuffeTestFixture, PushWhenFullTest)
{
	const size_t QSIZE = 17;
	TCircularBuffer<int> testQueue(QSIZE);

	// fill queue to full
	for(int i=0; i<QSIZE; ++i)
	{
		testQueue.Push(i);
	}
	EXPECT_EQ( testQueue.GetSize(), QSIZE);


	// test Push() when queue is full
	const size_t QTEST_RANGE = (size_t)(QSIZE*2.5);
	for(size_t i=QSIZE; i<QTEST_RANGE; ++i)
	{
		testQueue.Push(i);
		EXPECT_EQ( testQueue.GetSize(), QSIZE);
		EXPECT_EQ( testQueue[testQueue.GetSize()-1], i);
	}

	// check operator[] returns with correct order
	for(size_t i=0; i<testQueue.GetSize()-1; ++i)
	{
		EXPECT_GT(testQueue[i+1], testQueue[i]);
	}

	testQueue.Clear();
	EXPECT_EQ( testQueue.GetSize(), 0);
}


// Test for out_of_range exception
// - check out_of_range exception for Fornt() when queue is empty
// - check out_of_range exception for operator[] when queue is full
// - check range when queue is not full
TEST_F(CircularBuffeTestFixture, OutOfRangeTest)
{
	const size_t QSIZE = 10;
	TCircularBuffer<int> testQueue(QSIZE);

	// check out_of_range exception for Fornt() when queue is empty
	EXPECT_THROW(testQueue.Front(), std::out_of_range);

	// fill queue to full
	for(int i=0; i<QSIZE; ++i)
	{
		testQueue.Push(i);
	}
	EXPECT_EQ( testQueue.GetSize(), QSIZE);

	// Front() will not cause exception when the queue is not empty
	testQueue.Front();

	// check out_of_range exception for operator[] when queue is full
	EXPECT_THROW(testQueue[-1], std::out_of_range);
	EXPECT_THROW(testQueue[QSIZE], std::out_of_range);


	testQueue.Clear();
	EXPECT_EQ( testQueue.GetSize(), 0);

	// check range when queue is not full
	const size_t Q_HALFSIZE = QSIZE/2;
	for(int i=0; i<Q_HALFSIZE; ++i)
	{
		testQueue.Push(i);
	}
	EXPECT_EQ( testQueue.GetSize(), Q_HALFSIZE);

	// check out_of_range exception for operator[] in a non-full queue
	EXPECT_THROW(testQueue[Q_HALFSIZE], std::out_of_range);
}


} // namespace
