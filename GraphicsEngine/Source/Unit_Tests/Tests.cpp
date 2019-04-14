#include <Catch2/catch.hpp>
#define RHI_API
#define CORE_API 
#define FROMTEST
#include <glm/glm.hpp>
#include "Core/Types/FString.h"

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




