#if VULKAN
#define PUSHCONST [[vk::push_constant]] 
#else
#define PUSHCONST
#endif
