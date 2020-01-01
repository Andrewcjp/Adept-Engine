#define D3D12_MAKE_COARSE_SHADING_RATE(x,y) ((x) << 2 | (y))

#define D3D12_GET_COARSE_SHADING_RATE_X_AXIS(x) ((x) >> 2 )
#define D3D12_GET_COARSE_SHADING_RATE_Y_AXIS(y) ((y) & 3 )

static const int RATE_1X = 0x0;
static const int RATE_2X = 0x1;
static const int RATE_4X = 0x2;

static const int SHADING_RATE_1X1 = 0x0;
static const int SHADING_RATE_2X2 = D3D12_MAKE_COARSE_SHADING_RATE(RATE_2X, RATE_2X);
static const int SHADING_RATE_4X4 = D3D12_MAKE_COARSE_SHADING_RATE(RATE_4X, RATE_4X);