#include <tchar.h>
#include <Windows.h>


// Hack to convert char to TCHAR, using two buffers to be able to
// call twice in the same statement
wchar_t convertTemp1[256] = {0};
wchar_t convertTemp2[256] = {0};
bool convertBufferSwitch(false);
extern "C" wchar_t * utf8_to_wchar(const char* str, int len) {
//#ifdef _UNICODE
  wchar_t* temp = convertBufferSwitch ? convertTemp1 : convertTemp2;
  convertBufferSwitch = !convertBufferSwitch;
  memset(temp, 0, sizeof(convertTemp1));
  MultiByteToWideChar(CP_UTF8, 0, str, len, temp, 256);
  return temp;
//#else
//  return str;
//#endif
}

// Hack to convert TCHAR to char
char convertTemp3[256] = {0};
extern "C" char* wchar_to_utf8(wchar_t* str, int len) {
//#ifdef _UNICODE
  memset(convertTemp3, 0, sizeof(convertTemp3));
  WideCharToMultiByte(CP_UTF8, 0, str, len, convertTemp3, 256, 0, 0);
  return convertTemp3;
//#else
//  return str;
//#endif
}