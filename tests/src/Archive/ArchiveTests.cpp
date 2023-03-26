#include <boost/test/unit_test.hpp>
#include "../VsTestExplorer.h"

#include <ZipCpp/ZipCpp.h>

BOOST_AUTO_TEST_SUITE(Archive__Archive)

BOOST_AUTO_TEST_CASE(testImportExport)
{

	std::stringstream ss;

	// import entries

	{
		auto ar = Zip::MakeOutputArchive(&ss);

		std::istringstream test1("Hello!");
		std::istringstream test2("Hi!");

		ar.entry("test1.txt").importFrom(test1);
		ar.entry("test2.txt") << test2;

		BOOST_TEST(ss.str().length() == 0);

		ar.saveAndClose();

		BOOST_TEST(ss.str().length() != 0);
	}

	// export entries

	{
		auto ar = Zip::MakeInputArchive(&ss);

		std::ostringstream test1;
		std::ostringstream test2;

		ar.entry("test1.txt").exportTo(test1);
		ar.entry("test2.txt") >> test2;

		BOOST_TEST(test1.str() == "Hello!");
		BOOST_TEST(test2.str() == "Hi!");
	}

}

BOOST_AUTO_TEST_SUITE_END()