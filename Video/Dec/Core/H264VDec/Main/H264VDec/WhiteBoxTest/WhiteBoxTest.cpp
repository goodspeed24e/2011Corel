#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "../../LibSmartPtr/SmartModule.h"
#include "../../LibUnitTest/GMOUnitTest.h"

#include "GMO/GMO_i.h"
#include "GMO/GMOBase.h"

using namespace std;
using boost::bind;
using IVI::SmartModule;
using namespace boost::unit_test;

typedef void* (*LPCreateComponentT)(void);
CGMOPtr<IGMediaObject> CreateComponentFromDLL(const char *DLLPath);
bool BasicTest(const char* DLLPath);

test_suite* init_unit_test_suite( int argc, char* argv [])
{
	test_suite* pSuite = NULL;

	if ( argc == 2 )
	{
		pSuite = BOOST_TEST_SUITE("H264VDec unit test");
		pSuite->add( BOOST_TEST_CASE( bind(&BasicTest, argv[1]) ) );
	}
	else
		cout << "Error! Test target file not specified!" << endl;

	return pSuite;
}

bool BasicTest(const char* DLLPath)
{
	CGMOPtr<IGMediaObject> pDecoder = CreateComponentFromDLL(DLLPath);

	UnitTest::BasicGMOTest(pDecoder);

	return true;
}

CGMOPtr<IGMediaObject> CreateComponentFromDLL(const char *DLLPath)
{
	cout << "Loading " << DLLPath << endl;
	static SmartModule hDLL = ::LoadLibrary(DLLPath);
	BOOST_REQUIRE(hDLL);

	LPCreateComponentT pCreateComponent = reinterpret_cast<LPCreateComponentT>(GetProcAddress(hDLL, "CreateComponent"));
	BOOST_REQUIRE(pCreateComponent);
	CGMOPtr<IGMediaObject> pDecoder;
	pDecoder.Attach( reinterpret_cast<IGMediaObject*>(pCreateComponent()) );
	BOOST_REQUIRE(pDecoder);

	return pDecoder;
}