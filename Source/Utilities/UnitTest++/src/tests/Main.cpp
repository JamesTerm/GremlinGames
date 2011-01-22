#include "../UnitTest++.h"
#include "../TestReporterStdout.h"


int main(int, char const *[])
{
    return UnitTest::RunAllTests();
}

/*
TEST(DoIt)
{
	CHECK(false);
}

#define FAILURE_TEST(num, name) TEST(name)	\
{								\
	CHECK(num < 3);					\
}


FAILURE_TEST(1, myBad1)
FAILURE_TEST(2, myBad2)
FAILURE_TEST(3, myBad3)
*/