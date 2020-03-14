#if RUNTESTS
#define CATCH_CONFIG_RUNNER
#include <Catch2/catch.hpp>
#include "EngineTests.h"
#include "Core/Types/FString.h"
#include "RHI/RHIInterGPUStagingResource.h"
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
//RHI tests to create resources with different sizes etc.
//ensures that changes don't break a currently unused code path.
//Many of these will assert in the code or debug layer rather than here.
TEST_CASE("Create RHI Buffer", "[RHI]")
{
	RHIBuffer* B = RHI::CreateRHIBuffer(ERHIBufferType::Vertex);
	B->CreateVertexBuffer(10, 10);
	REQUIRE(B != nullptr);
	EnqueueSafeRHIRelease(B);
}
//
//TEST_CASE("Create FrameBuffer with texture depth of 10 Buffer", "[RHI]")
//{
//	RHIFrameBufferDesc 	D = RHIFrameBufferDesc::CreateColourDepth(100, 100);
//	D.TextureDepth = 10;
//	FrameBuffer* B = RHI::CreateFrameBuffer(RHI::GetDefaultDevice(), D);
//	REQUIRE(B != nullptr);
//	EnqueueSafeRHIRelease(B);
//}InterGPUDesc D;

TEST_CASE("InterGPUDesc", "[RHI]")
{
	InterGPUDesc D;
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		REQUIRE(D.Mask.GetFlagValue(i));
	}
}

TEST_CASE("InterGPUDesc Mask GPU 1", "[RHI]")
{
	InterGPUDesc D;
	D.Mask.SetFlagValue(1, 0);
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		if (i == 1)
		{
			REQUIRE(!D.Mask.GetFlagValue(i));
		}
		else
		{
			REQUIRE(D.Mask.GetFlagValue(i));
		}

	}
}
#endif