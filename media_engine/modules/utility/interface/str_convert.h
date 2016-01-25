#ifndef __WEBRTC_UTILITY_STR_CONVERT_H__
#define __WEBRTC_UTILITY_STR_CONVERT_H__

#ifdef __cplusplus
extern "C" {
#endif

wchar_t * utf8_to_wchar(const char* str, int len);
char* wchar_to_utf8(wchar_t* str, int len);

#ifdef __cplusplus
}
#endif

#ifdef _UNICODE
#define TCHAR_TO_UTF8(str) wchar_to_utf8((str), -1)
#define UTF8_TO_TCHAR(str) utf8_to_wchar((str), -1)
#else
#define TCHAR_TO_UTF8(str) (str)
#define UTF8_TO_TCHAR(str) (str)
#endif




#endif // __WEBRTC_UTILITY_STR_CONVERT_H__
