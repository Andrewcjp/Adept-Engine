#define CATCH_CONFIG_RUNNER
#include <Catch2/catch.hpp>
#include "EngineTests.h"
#include "Core/Types/FString.h"
void TESTING::RunTests()
{
	int result = Catch::Session().run();
	ensure(result == 0);
}

TEST_CASE("FString Contains ", "[FString]")
{
	FString test = FString("hello");

	REQUIRE(test.Contains("h") == true);
}
TEST_CASE("FString Contains From String ", "[FString]")
{
	std::string data = "hello";
	FString test = FString(data);
	REQUIRE(test.Contains("h") == true);
}

TEST_CASE("FString Hash Test ", "[FString]")
{
	FString HashS = FString("h", true);
	FString Hash2 = FString("h", true);

	REQUIRE(HashS == Hash2);
}

TEST_CASE("FString Hash Test Temp", "[FString]")
{
	FString HashS = FString("hello", true);
	REQUIRE(HashS == FString("hello"));
}


TEST_CASE("Thread test", "[Threading]")
{
	REQUIRE(true);
}