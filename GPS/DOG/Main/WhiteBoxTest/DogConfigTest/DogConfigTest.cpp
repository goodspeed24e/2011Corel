#pragma warning (disable:4819)
//boost_1_35_0\include\boost\utility\enable_if.hpp : warning C4819: The file contains a character that cannot be represented in the current code page (950). Save the file in Unicode format to prevent data loss

#include <tchar.h>

#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>

#include <vector>
#include <string>

#include "DogConfig.h"
#include "DogCategory.h"
#include "SubCategory.h"
using namespace dog;
using namespace dogsub;



namespace DogConfigFileTest {

class CDogConfigFileTest : public CDogConfig
{
public:
	bool ReadLine(const std::string& line) { return CDogConfig::ReadLine(line); }
	std::string GetReadLineError() { return m_ReadLineErrMsg; }

	void SetProperty(unsigned int function, const TProperty& property) { CDogConfig::SetProperty(function, property); }
	void SetProperty(unsigned int function, unsigned int category, const TProperty& property) { CDogConfig::SetProperty(function, category, property); }
	void SetProperty(unsigned int function, unsigned int category, unsigned int subcategory, const TProperty& property) { CDogConfig::SetProperty(function, category, subcategory, property); }
};


void BasicTest()
{
	CDogConfigFileTest dogConfig;
	CDogConfig::TProperty property;

	//---------------------------------------------------
	// test case description:
	// initial status

	property = dogConfig.GetProperty(CDogConfig::PROFILER);
	BOOST_CHECK(property.bEnable == false);
	BOOST_CHECK(property.OutputList.empty());

	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_AUDIO);
	BOOST_CHECK(property.bEnable == false);
	BOOST_CHECK(property.OutputList.empty());

	{
		CDogConfig::TProperty propertySet;
		propertySet.bEnable = true;
		propertySet.OutputList.push_back(CDogConfig::DOG_VIEW);
		propertySet.OutputList.push_back(CDogConfig::DOG_LOG_FILE);
		dogConfig.SetProperty(CDogConfig::PROFILER, propertySet);
	}

	//---------------------------------------------------
	// test case description:
	// a child-node will inherit property from its parent 
	// if the child-node doesn't be specified a property.

	property = dogConfig.GetProperty(CDogConfig::PROFILER);
	BOOST_CHECK(property.bEnable == true);
	BOOST_CHECK(property.OutputList.size() == 2);
	if(property.OutputList.size() == 2 )
	{
		BOOST_CHECK(property.OutputList[0] == CDogConfig::DOG_VIEW);
		BOOST_CHECK(property.OutputList[1] == CDogConfig::DOG_LOG_FILE);
	}

	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_AUDIO);
	BOOST_CHECK(property.bEnable == true);
	BOOST_CHECK(property.OutputList.size() == 2);
	if(property.OutputList.size() == 2 )
	{
		BOOST_CHECK(property.OutputList[0] == CDogConfig::DOG_VIEW);
		BOOST_CHECK(property.OutputList[1] == CDogConfig::DOG_LOG_FILE);
	}


	//---------------------------------------------------
	// test case description:
	// a child-node will has its own property if the child-node has been specified a property.
	{
		CDogConfig::TProperty propertySet;
		propertySet.bEnable = false;
		propertySet.OutputList.push_back(CDogConfig::DOG_VIEW);
		dogConfig.SetProperty(CDogConfig::PROFILER, CAT_AUDIO, propertySet);
	}

	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_AUDIO);
	BOOST_CHECK(property.bEnable == false);
	BOOST_CHECK(property.OutputList.size() == 1);
	if(property.OutputList.size() == 1 )
	{
		BOOST_CHECK(property.OutputList[0] == CDogConfig::DOG_VIEW);
	}

	//---------------------------------------------------
	// test case description:
	// test Reset
	dogConfig.Reset();

	// back to the initial status
	property = dogConfig.GetProperty(CDogConfig::PROFILER);
	BOOST_CHECK(property.bEnable == false);
	BOOST_CHECK(property.OutputList.empty());

	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_AUDIO);
	BOOST_CHECK(property.bEnable == false);
	BOOST_CHECK(property.OutputList.empty());

	{
		CDogConfig::TProperty propertySet;
		propertySet.bEnable = true;
		propertySet.OutputList.push_back(CDogConfig::DOG_VIEW);
		propertySet.OutputList.push_back(CDogConfig::DOG_LOG_FILE);
		dogConfig.SetProperty(CDogConfig::PROFILER, propertySet);
	}
}

void BasicTest2()
{
	CDogConfigFileTest dogConfig;
	CDogConfig::TProperty property;

	//---------------------------------------------------
	// test case description:
	// specified a property to sub-category directly without specifying to category
	{
		CDogConfig::TProperty propertySet1;
		propertySet1.bEnable = true;
		propertySet1.OutputList.push_back(CDogConfig::DOG_VIEW);
		propertySet1.OutputList.push_back(CDogConfig::DOG_LOG_FILE);
		dogConfig.SetProperty(CDogConfig::PROFILER, propertySet1);

		CDogConfig::TProperty propertySet2;
		propertySet2.bEnable = true;
		propertySet2.OutputList.push_back(CDogConfig::DOG_VIEW);
		dogConfig.SetProperty(CDogConfig::PROFILER, CAT_AUDIO, CAT_AUDIO_AC3, propertySet2);
	}

	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_AUDIO);
	BOOST_CHECK(property.bEnable == true);
	BOOST_CHECK(property.OutputList.size() == 2);
	if(property.OutputList.size() == 2 )
	{
		BOOST_CHECK(property.OutputList[0] == CDogConfig::DOG_VIEW);
		BOOST_CHECK(property.OutputList[1] == CDogConfig::DOG_LOG_FILE);
	}

	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_AUDIO, CAT_AUDIO_AC3);
	BOOST_CHECK(property.bEnable == true);
	BOOST_CHECK(property.OutputList.size() == 1);
	if(property.OutputList.size() == 1 )
	{
		BOOST_CHECK(property.OutputList[0] == CDogConfig::DOG_VIEW);
	}
}

void ParsingTest1()
{
	CDogConfigFileTest dogConfig;
	CDogConfig::TProperty property;
	bool ret;

	ret = dogConfig.ReadLine("profiler = all");
	BOOST_CHECK(ret);

	property = dogConfig.GetProperty(CDogConfig::PROFILER);
	BOOST_CHECK(property.bEnable == true);
	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_AUDIO);
	BOOST_CHECK(property.bEnable == true);
	
	ret = dogConfig.ReadLine("profiler = none");
	BOOST_CHECK(ret);

	property = dogConfig.GetProperty(CDogConfig::PROFILER);
	BOOST_CHECK(property.bEnable == false);
	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_AUDIO);
	BOOST_CHECK(property.bEnable == false);

 	ret = dogConfig.ReadLine("profiler = all,  -video");
	BOOST_CHECK(ret);

	property = dogConfig.GetProperty(CDogConfig::PROFILER);
	BOOST_CHECK(property.bEnable == true);
	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_VIDEO);
	BOOST_CHECK(property.bEnable == false);

 	ret = dogConfig.ReadLine("profiler = none, +audio");
	BOOST_CHECK(ret);

	property = dogConfig.GetProperty(CDogConfig::PROFILER);
	BOOST_CHECK(property.bEnable == false);
	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_AUDIO);
	BOOST_CHECK(property.bEnable == true);


 	ret = dogConfig.ReadLine("profiler = demux, video, audio");
 	ret = dogConfig.ReadLine("profiler.output = file, debugView");
	BOOST_CHECK(ret);
	
	property = dogConfig.GetProperty(CDogConfig::PROFILER);
	BOOST_CHECK(property.bEnable == false);
	BOOST_CHECK(property.OutputList.size() == 2);
	if(property.OutputList.size() == 2 )
	{
		BOOST_CHECK(property.OutputList[0] == CDogConfig::DOG_LOG_FILE);
		BOOST_CHECK(property.OutputList[1] == CDogConfig::DEBUG_VIEW);
	}

	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_DEMUX);
	BOOST_CHECK(property.bEnable == true);
	BOOST_CHECK(property.OutputList.size() == 2);
	if(property.OutputList.size() == 2 )
	{
		BOOST_CHECK(property.OutputList[0] == CDogConfig::DOG_LOG_FILE);
		BOOST_CHECK(property.OutputList[1] == CDogConfig::DEBUG_VIEW);
	}

	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_VIDEO);
	BOOST_CHECK(property.bEnable == true);
	BOOST_CHECK(property.OutputList.size() == 2);
	if(property.OutputList.size() == 2 )
	{
		BOOST_CHECK(property.OutputList[0] == CDogConfig::DOG_LOG_FILE);
		BOOST_CHECK(property.OutputList[1] == CDogConfig::DEBUG_VIEW);
	}

	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_AUDIO);
	BOOST_CHECK(property.bEnable == true);
	BOOST_CHECK(property.OutputList.size() == 2);
	if(property.OutputList.size() == 2 )
	{
		BOOST_CHECK(property.OutputList[0] == CDogConfig::DOG_LOG_FILE);
		BOOST_CHECK(property.OutputList[1] == CDogConfig::DEBUG_VIEW);
	}


	ret = dogConfig.ReadLine("log = none");
	ret &= dogConfig.ReadLine("log.output = file");
	BOOST_CHECK(ret);

	property = dogConfig.GetProperty(CDogConfig::EVENT_LOG);
	BOOST_CHECK(property.bEnable == false);
	BOOST_CHECK(property.OutputList.size() == 1);
	if(property.OutputList.size() == 1 )
	{
		BOOST_CHECK(property.OutputList[0] == CDogConfig::DOG_LOG_FILE);
	}
}

void ParsingTest2()
{
	CDogConfigFileTest dogConfig;
	CDogConfig::TProperty property;
	bool ret;

 	ret = dogConfig.ReadLine("profiler.audio = all");
	BOOST_CHECK(ret);

	property = dogConfig.GetProperty(CDogConfig::PROFILER);
	BOOST_CHECK(property.bEnable == false);
	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_AUDIO);
	BOOST_CHECK(property.bEnable == true);
	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_AUDIO, CAT_AUDIO_AC3);
	BOOST_CHECK(property.bEnable == true);

 	ret = dogConfig.ReadLine("profiler.audio = none");
	BOOST_CHECK(ret);

	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_AUDIO);
	BOOST_CHECK(property.bEnable == false);
	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_AUDIO, CAT_AUDIO_AC3);
	BOOST_CHECK(property.bEnable == false);

 	ret = dogConfig.ReadLine("profiler.audio = all,  -ac3");
	BOOST_CHECK(ret);

	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_AUDIO);
	BOOST_CHECK(property.bEnable == true);
	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_AUDIO, CAT_AUDIO_AC3);
	BOOST_CHECK(property.bEnable == false);

 	ret = dogConfig.ReadLine("profiler.audio = none, +ddplus, dts");
	BOOST_CHECK(ret);

	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_AUDIO);
	BOOST_CHECK(property.bEnable == false);
	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_AUDIO, CAT_AUDIO_DDPLUS);
	BOOST_CHECK(property.bEnable == true);
	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_AUDIO, CAT_AUDIO_DTS);
	BOOST_CHECK(property.bEnable == true);

	ret = dogConfig.ReadLine("profiler.audio = none");
	ret &= dogConfig.ReadLine("profiler.audio = ac3, ddplus, dts");
	ret &= dogConfig.ReadLine("profiler.audio.output = dogview, file");
	BOOST_CHECK(ret);

	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_AUDIO);
	BOOST_CHECK(property.bEnable == false);
	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_AUDIO, CAT_AUDIO_AC3);
	BOOST_CHECK(property.bEnable == true);
	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_AUDIO, CAT_AUDIO_DDPLUS);
	BOOST_CHECK(property.bEnable == true);
	property = dogConfig.GetProperty(CDogConfig::PROFILER, CAT_AUDIO, CAT_AUDIO_DTS);
	BOOST_CHECK(property.bEnable == true);
	BOOST_CHECK(property.OutputList.size() == 2);
	if(property.OutputList.size() == 2 )
	{
		BOOST_CHECK(property.OutputList[0] == CDogConfig::DOG_VIEW);
		BOOST_CHECK(property.OutputList[1] == CDogConfig::DOG_LOG_FILE);
	}

	//dogConfig.ReadLine("profiler.video.output = dogview");
}

void ParsingTest3()
{
	CDogConfigFileTest dogConfig;
	CDogConfig::TProperty property;
	bool ret;

 	ret = dogConfig.ReadLine("log.audio = 1,3,5,7,9, 100, 200");
	BOOST_CHECK(ret);

	property = dogConfig.GetProperty(CDogConfig::EVENT_LOG, CAT_AUDIO);
	BOOST_CHECK(property.bEnable == false);
	property = dogConfig.GetProperty(CDogConfig::EVENT_LOG, CAT_AUDIO, 1);
	BOOST_CHECK(property.bEnable == true);
	property = dogConfig.GetProperty(CDogConfig::EVENT_LOG, CAT_AUDIO, 3);
	BOOST_CHECK(property.bEnable == true);
	property = dogConfig.GetProperty(CDogConfig::EVENT_LOG, CAT_AUDIO, 100);
	BOOST_CHECK(property.bEnable == true);
	property = dogConfig.GetProperty(CDogConfig::EVENT_LOG, CAT_AUDIO, 200);
	BOOST_CHECK(property.bEnable == true);

 	ret = dogConfig.ReadLine("log.100 = 2,4,6,8,10");
	BOOST_CHECK(ret);

	property = dogConfig.GetProperty(CDogConfig::EVENT_LOG, 100);
	BOOST_CHECK(property.bEnable == false);
	property = dogConfig.GetProperty(CDogConfig::EVENT_LOG, 100, 2);
	BOOST_CHECK(property.bEnable == true);
	property = dogConfig.GetProperty(CDogConfig::EVENT_LOG, 100, 4);
	BOOST_CHECK(property.bEnable == true);
	property = dogConfig.GetProperty(CDogConfig::EVENT_LOG, 100, 6);
	BOOST_CHECK(property.bEnable == true);
	property = dogConfig.GetProperty(CDogConfig::EVENT_LOG, 100, 8);
	BOOST_CHECK(property.bEnable == true);

 	ret = dogConfig.ReadLine("log.100.2.output = file");
	BOOST_CHECK(ret);

	property = dogConfig.GetProperty(CDogConfig::EVENT_LOG, 100, 2);
	BOOST_CHECK(property.OutputList.size() == 1);
	if(property.OutputList.size() == 1 )
	{
		BOOST_CHECK(property.OutputList[0] == CDogConfig::DOG_LOG_FILE);
	}
}

void ParsingErrorTest()
{
	CDogConfigFileTest dogConfig;
	CDogConfig::TProperty property;
	bool ret;
	std::string ErrMsg;

	ret = dogConfig.ReadLine("profiler.xyz = all");
	ErrMsg = dogConfig.GetReadLineError();
	BOOST_CHECK(!ret);
	BOOST_CHECK(!dogConfig.GetReadLineError().empty());

	std::string configString = 
		"log.audio = all\n"
		"log.audio = ac3, xyz, dts\n"
		"log.audio = truehd"
		" \n\n";

	ret = dogConfig.ReadFromString(configString.c_str());
	ErrMsg = "";
	for(size_t i=0; i<dogConfig.GetErrorMessage().size(); ++i)
	{
		ErrMsg += dogConfig.GetErrorMessage()[i];
		ErrMsg +="\n";
	}
	printf(ErrMsg.c_str());
	BOOST_CHECK(!ret);
	BOOST_CHECK(!dogConfig.GetErrorMessage().empty());

}


} // namespace DogConfigFileTest

//============================================================================
boost::unit_test::test_suite* make_DogConfigFileTest_suite()
{
	using namespace DogConfigFileTest;

	boost::unit_test_framework::test_suite* suite = NULL;
	suite = BOOST_TEST_SUITE("DogConfigFileTest");
	{
		suite->add( BOOST_TEST_CASE( BasicTest ));
		suite->add( BOOST_TEST_CASE( BasicTest2 ));
		suite->add( BOOST_TEST_CASE( ParsingTest1 ));
		suite->add( BOOST_TEST_CASE( ParsingTest2 ));
		suite->add( BOOST_TEST_CASE( ParsingTest3 ));
		suite->add( BOOST_TEST_CASE( ParsingErrorTest ));
	}
	return suite;
}
