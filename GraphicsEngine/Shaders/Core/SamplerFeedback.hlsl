#if HARDWARE_SUPPORT_SAMPLER_FEEDBACK
#define FEEDBACKTEXTURE(Type) FeedbackTexture2D<Type>
#define FEEDBACK_WRITE(FeedBack,TextureTarget,Sampler,UV) FeedBack.WriteSamplerFeedback(TextureTarget, Sampler, UV);

#define SFS_TYPE_MIPSUSED SAMPLER_FEEDBACK_MIP_REGION_USED
#define SFS_TYPE_MINMIP SAMPLER_FEEDBACK_MIN_MIP

#else
#define FEEDBACKTEXTURE(Type) RWTexture2D<float> 
#define FEEDBACK_BIND Texture2D SFS_MipTexture:register(t100);
#define FEEDBACK_WRITE(FeedBack,TextureTarget,Sampler,UV) float mip = TextureTarget.CalculateLevelOfDetail(Sampler,UV); uint W, H = 0; FeedBack.GetDimensions(W, H); FeedBack[(UV*uint2(W, H) % uint2(W, H))] = min(FeedBack[UV*uint2(W, H)],mip);
//check width max on this?
#define SFS_TYPE_MIPSUSED 0
#define SFS_TYPE_MINMIP 0
#endif