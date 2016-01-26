#include <windows.h>

BOOL APIENTRY DllMain(HMODULE /*module*/, DWORD  call_reason, void* /*reserved*/) {
  switch (call_reason) {
  case DLL_PROCESS_ATTACH:
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
  case DLL_PROCESS_DETACH:
    break;
  }
  return TRUE;
}

