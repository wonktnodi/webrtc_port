#ifndef __MEDIA_ENGINE_UTILITY_H__
#define __MEDIA_ENGINE_UTILITY_H__

#ifdef __cplusplus
extern "C" {
#endif 


wchar_t * utf8_to_wchar(const char* str, int len);
char* wchar_to_utf8(wchar_t* str, int len);


#ifdef __cplusplus
}
#endif

#endif // __MEDIA_ENGINE_UTILITY_H__
