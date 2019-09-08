#if VULKAN
#define PUSHCONSTANT [[vk::push_constant]]
#else
#define PUSHCONSTANT
#endif