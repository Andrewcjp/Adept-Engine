#define D3D12_MAKE_COARSE_SHADING_RATE(x,y) ((x) << 2 | (y))

#define D3D12_GET_COARSE_SHADING_RATE_X_AXIS(x) ((x) >> 2 )
#define D3D12_GET_COARSE_SHADING_RATE_Y_AXIS(y) ((y) & 3 )

static const int RATE_1X = 0x0;
static const int RATE_2X = 0x1;
static const int RATE_4X = 0x2;