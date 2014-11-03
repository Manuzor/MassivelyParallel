#pragma once

#ifdef MP_Wrapper_DllExport
#define MP_WrapperAPI __declspec(dllexport)
#else
#define MP_WrapperAPI __declspec(dllimport)
#endif
