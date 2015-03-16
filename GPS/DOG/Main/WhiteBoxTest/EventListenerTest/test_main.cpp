#include <gtest/gtest.h>

class MyEnvironment : public ::testing::Environment
{ 
public: 
	virtual ~MyEnvironment() 
	{} 
	
	// Override this to define how to set up the environment. 
	virtual void SetUp() 
	{
	} 

	// Override this to define how to tear down the environment. 
	virtual void TearDown() 
	{
	} 
};


int main(int argc, char **argv) 
{
	::testing::InitGoogleTest(&argc, argv);
	testing::Environment* const env = ::testing::AddGlobalTestEnvironment(new MyEnvironment);

	return RUN_ALL_TESTS();
}
