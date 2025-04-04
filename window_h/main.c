#include <windows.h>

DWORD WINAPI ThreadFunc(void* data) {
  // Do stuff.  This will be the first function called on the new thread.
  // When this function returns, the thread goes away.  See MSDN for more details.
  return 0;
}

int main() {
  HANDLE thread = CreateThread(NULL, 0, ThreadFunc, NULL, 0, NULL);
  if (thread) {
    // Optionally do stuff, such as wait on the thread.
  }
}