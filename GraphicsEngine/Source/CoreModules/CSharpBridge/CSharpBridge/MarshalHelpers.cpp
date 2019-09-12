#include "MarshalHelpers.h"

using namespace System::Runtime::InteropServices;
String ^ MarshalHelpers::ConvertString(std::string s)
{
	IntPtr bfr = IntPtr(const_cast<char*>(s.c_str()));
	String^ strValue = Marshal::PtrToStringAnsi(bfr);
	return strValue;
}
