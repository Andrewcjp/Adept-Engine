#pragma once

static NvPmApiManager S_NVPMManager;
extern NvPmApiManager *GetNvPmApiManager() { return &S_NVPMManager; }
const NvPmApi *GetNvPmApi() { return S_NVPMManager.Api(); }