#ifdef _Test
#include <UnitTest++.h>
#endif

#include <iostream>
using namespace std;

int main(int argc, const char** argv)
{
	cout << "Hello " << argv[0] << endl;

#ifdef _Test
	return UnitTest::RunAllTests();
#endif
	return 0;
}

#ifdef _Test

TEST(DoIt)
{
	CHECK(true);
}

#endif