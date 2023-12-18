/* Really ain't no other option */

#include "script.h"
#include <fstream>
#include <iomanip>
#include <varargs.h>

#include <Windows.h>
#include <Psapi.h>

#define BUFFER_SIZE 2048

#define VER_MAX 1
#define VER_MIN 1

#define MOD_NAME "CenterSteeringPatch.asi"
#define LOG_NAME "CenterSteeringPatch.log"

void clearLog()
{
	std::ofstream ofFile(LOG_NAME, std::ofstream::out | std::ofstream::trunc);
	ofFile.close();
}

void rawLog(const std::string& szInfo, const std::string& szData)
{
	std::ofstream ofFile(LOG_NAME, std::ios_base::out | std::ios_base::app);

	if (ofFile.is_open())
	{
		SYSTEMTIME stCurrTime;
		GetLocalTime(&stCurrTime);

		ofFile << "|" <<
			std::setw(2) << std::setfill('0') << stCurrTime.wHour << ":" <<
			std::setw(2) << std::setfill('0') << stCurrTime.wMinute << ":" <<
			std::setw(2) << std::setfill('0') << stCurrTime.wSecond << "." <<
			std::setw(3) << std::setfill('0') << stCurrTime.wMilliseconds <<
			"|[" << szInfo << "] -- " << szData << "\n";

		ofFile.close();
	}
}

void writeLog(const char* szInfo, const char* szFormat, ...)
{
	char szBuf[BUFFER_SIZE];
	va_list args;
	va_start(args, szFormat);
	vsnprintf(szBuf, BUFFER_SIZE, szFormat, args);
	va_end(args);
	rawLog(std::string(szInfo), std::string(szBuf));
}

ULONG_PTR findPattern(const char* szPattern, const char* szMask)
{
	MODULEINFO stInfo;

	if (GetModuleInformation(GetCurrentProcess(), 
		GetModuleHandle(NULL), &stInfo, sizeof(MODULEINFO)))
	{
		SIZE_T index = 0;
		ULONG_PTR startAddr = (ULONG_PTR)stInfo.lpBaseOfDll;
		for (SIZE_T i = 0; i < stInfo.SizeOfImage; i++)
		{
			if (*(PBYTE)(startAddr + i) == (BYTE)szPattern[index] ||
				szMask[index] == '?')
			{
				if (szMask[index + 1] == NULL)
					return (ULONG_PTR)((startAddr + i) - (strlen(szMask) - 1));
				index++;
			}
			else
			{
				index = 0;
			}
		}
	}
	else
		writeLog("Error", "GetModuleInformation() failed! [%d]", GetLastError());

	return 0;
}

void main()
{
	/* This makes InfamousSabre mad, lmfao */

	bool bError = false;
	clearLog();
	writeLog("Started", "%s v%d.%d", MOD_NAME, VER_MAX, VER_MIN);
	
	/* Address of centering when getting out of car normally */
	writeLog("Operation", "Finding steering address 1...");
	ULONG_PTR address = findPattern("\x44\x89\xbb\x00\x00\x00\x00\x8b\x0d", "xxx????xx");
	if (address)
	{
		writeLog("Operation", "Found address! Patching %d bytes...", 7);
		memset((void*)address, 0x90, 7);

		/* Address of centering when diving out */
		writeLog("Operation", "Finding steering address 2...");
		address = findPattern("\x89\x82\x00\x00\x00\x00\x38\x81", "xx????xx");
		if (address)
		{
			writeLog("Operation", "Found address! Patching %d bytes...", 6);
			memset((void*)address, 0x90, 6);
			writeLog("Operation", "Done!");
			writeLog("Info", "If a crash occurs then wrong address got patched. Try again or wait for mod update.");
		}
		else
		{
			writeLog("Error", "Could not find address! Either try again or the game has been updated and the address changed.");
			bError = true;
		}
	}
	else
	{
		writeLog("Error", "Could not find address! Either try again or the game has been updated and the address changed.");
		bError = true;
	}

	if (!bError)
		writeLog("Finished", "Patching complete.");
	else
		writeLog("Finished", "Error patching addresses.");
	return;
}

void ScriptMain()
{
	srand(GetTickCount());
	main();
}