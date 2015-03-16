#pragma warning (disable:4819)
//boost_1_35_0\include\boost\utility\enable_if.hpp : warning C4819: The file contains a character that cannot be represented in the current code page (950). Save the file in Unicode format to prevent data loss

#include <stdio.h>

//#include <boost/config.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

using namespace boost;

boost::unit_test::test_suite* make_DogConfigFileTest_suite();


// test program entry point
unit_test::test_suite* init_unit_test_suite(int, char *[])
{
	unit_test_framework::test_suite* top_test_suite = BOOST_TEST_SUITE("main test suite");

	top_test_suite->add( make_DogConfigFileTest_suite());

	return top_test_suite;
}
