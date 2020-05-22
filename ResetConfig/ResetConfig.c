#include "logfile.h"
#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <strsafe.h>
#include <shlobj.h>
#pragma comment (lib, "shell32.lib")

//*************************************************************
//
//  RegDelnodeRecurse()
//
//  Purpose:    Deletes a registry key and all its subkeys / values.
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//
//*************************************************************

BOOL RegDelnodeRecurse (HKEY hKeyRoot, LPTSTR lpSubKey)
{
	LPTSTR lpEnd;
	LONG lResult;
	DWORD dwSize;
	TCHAR szName[MAX_PATH];
	HKEY hKey;
	FILETIME ftWrite;

	// First, see if we can delete the key without having
	// to recurse.

	lResult = RegDeleteKey(hKeyRoot, lpSubKey);

	if (lResult == ERROR_SUCCESS) 
	return TRUE;

	lResult = RegOpenKeyEx (hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);

	if (lResult != ERROR_SUCCESS) 
	{
		if (lResult == ERROR_FILE_NOT_FOUND) {
			fprintf(stderr, "Key not found.\n");
			LogFileWrite("Key not found.\n");
			return TRUE;
		} 
		else {
			fprintf(stderr, "Error opening key.\n");
			LogFileWrite("Error opening key.\n");
			return FALSE;
		}
	}

	// Check for an ending slash and add one if it is missing.

	lpEnd = lpSubKey + lstrlen(lpSubKey);

	if (*(lpEnd - 1) != TEXT('\\')) 
	{
		*lpEnd =  TEXT('\\');
		lpEnd++;
		*lpEnd =  TEXT('\0');
	}

	// Enumerate the keys

	dwSize = MAX_PATH;
	lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
	NULL, NULL, &ftWrite);

	if (lResult == ERROR_SUCCESS) 
	{
		do {

			*lpEnd = TEXT('\0');
			StringCchCat(lpSubKey, MAX_PATH * 2, szName);

			if (!RegDelnodeRecurse(hKeyRoot, lpSubKey)) {
				break;
			}

			dwSize = MAX_PATH;

			lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
			NULL, NULL, &ftWrite);

		} while (lResult == ERROR_SUCCESS);
	}

	lpEnd--;
	*lpEnd = TEXT('\0');

	RegCloseKey (hKey);

	// Try again to delete the key.

	lResult = RegDeleteKey(hKeyRoot, lpSubKey);

	if (lResult == ERROR_SUCCESS) 
	return TRUE;

	return FALSE;
}

//*************************************************************
//
//  RegDelnode()
//
//  Purpose:    Deletes a registry key and all its subkeys / values.
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//
//*************************************************************

BOOL RegDelnode (HKEY hKeyRoot, LPCTSTR lpSubKey)
{
	TCHAR szDelKey[MAX_PATH*2];

	StringCchCopy (szDelKey, MAX_PATH*2, lpSubKey);
	return RegDelnodeRecurse(hKeyRoot, szDelKey);

}

BOOL DeleteEffectPresets()
{
	char FolderPath[MAX_PATH];
	char FilePath[MAX_PATH];
	WIN32_FIND_DATAA fd;
	HANDLE hFind;
	BOOL error;
	//Get the path of the My Documents folder
	SHGetSpecialFolderPath(NULL, FolderPath, CSIDL_PERSONAL, FALSE);
	//Append the name of the preset folder
	StringCchCat(FolderPath, MAX_PATH, "\\preset");
	//Copy the preset folder path
	StringCchCopy(FilePath, MAX_PATH, FolderPath);
	//Append the preset search term
	StringCchCat(FilePath, MAX_PATH, "\\*.gpe");
	//Try to find the first file
	hFind = FindFirstFile(FilePath, &fd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "No preset files found.\n");
		LogFileWrite("No preset files found.\n");
		return FALSE;
	}
	/*
For the first file or any subsequent files:
1. Copy the folder path.
2. Append a backslash character.
3. Append the file name.
4. Delete the file.
*/
	StringCchCopy(FilePath, MAX_PATH, FolderPath);
	StringCchCat(FilePath, MAX_PATH, "\\");
	StringCchCat(FilePath, MAX_PATH, fd.cFileName);
	if (DeleteFile(FilePath) == 0)
	{
		fprintf(stderr, "Error deleting %s.\n", FilePath);
		LogFileWrite("Error deleting %s.\n", FilePath);
		error = TRUE;
	}
	while (FindNextFile(hFind, &fd))
	{
		StringCchCopy(FilePath, MAX_PATH, FolderPath);
		StringCchCat(FilePath, MAX_PATH, "\\");
		StringCchCat(FilePath, MAX_PATH, fd.cFileName);
		if (DeleteFile(FilePath) == 0)
		{
			fprintf(stderr, "Error deleting %s.\n", FilePath);
			LogFileWrite("Error deleting %s.\n", FilePath);
			error = TRUE;
		}
	}
	//Close the handel to hFind
	FindClose(hFind);
	return error;
}

/**
* The following function shows one way that a GUI application can redirect the stdout 
* stream to a new console    window. After the following function has been called, the 
* GUI program can use C runtime functions like printf to display information on the 
* console window. This can be useful during debugging.
*/

void RedirectStdOut(void)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	// Create a new console window.
	if (!AllocConsole()) return;

	// Set the screen buffer to be larger than normal (this is optional).
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
	{
		csbi.dwSize.Y = 1000; // any useful number of lines...
		SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), csbi.dwSize);
	}


	// Redirect "stdin" to the console window.
	if (!freopen("CONIN$", "r", stdin)) return;


	// Redirect "stderr" to the console window.
	if (!freopen("CONOUT$", "w", stderr)) return;

	// Redirect "stdout" to the console window.
	if (!freopen("CONOUT$", "w", stdout)) return;


	// Turn off buffering for "stdout" ("stderr" is unbuffered by default).

	setbuf(stdout, NULL);
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCommandLine, int cmdShow)
{
	BOOL bSuccess;
	int CMDResult;
	char choice;

	CMDResult = strcmp(lpszCommandLine, "/h");
	if (CMDResult == 0)
	{
		MessageBox(NULL, "Usage:\nno arguments - GUI mode\n/u - uninstaller friendly interface\n/s - silent mode\n/f - log errors to file\n/l - log errors to console\n/c - console mode (must be combined with /l)\n/h - show help\n", "Information", MB_ICONINFORMATION | MB_OK);
		return 0;
	}

	if (strstr(lpszCommandLine, "/l"))
	{
		RedirectStdOut();
	}

	if (strstr(lpszCommandLine, "/s"))
	{
		if (strstr(lpszCommandLine, "/f"))
		{
			LogFileOpen("ResetConfig.log");
		}
		DeleteEffectPresets();
		RegDelnode(HKEY_CURRENT_USER, TEXT("Software\\GreenSoftware\\GSPlayer"));
		if (strstr(lpszCommandLine, "/f"))
		{
			LogFileClose();
		}
		return 0;
	}

	if (strstr(lpszCommandLine, "/c"))
	{
		if (strstr(lpszCommandLine, "/f"))
		{
			LogFileOpen("ResetConfig.log");
		}
		printf("Are you sure you want to reset the GSPlayer configuration? Y/N\n");
		scanf("%c", &choice);
		if (choice == 'y' || choice == 'Y')
		{
			if (DeleteEffectPresets() == TRUE)
			printf("1 Or more presets couldn't be deleted.\n");
			bSuccess = RegDelnode(HKEY_CURRENT_USER, TEXT("Software\\GreenSoftware\\GSPlayer"));
			if(bSuccess)
			printf("Configuration Reset!\n");
			else
			printf("There was a problem resetting the configuration.\n");
		}
		else if (choice == 'n' || choice == 'N')
		{
			printf("Configuration has not been reset.\n");
		}
		if (strstr(lpszCommandLine, "/f"))
		{
			LogFileClose();
		}
		printf("Press any key to exit.\n");
		getch();
		return 0;
	}

	if (strstr(lpszCommandLine, "/u"))
	{
		if (MessageBox(NULL, "Do you wish to reset the GSPlayer configuration?", "", MB_ICONQUESTION | MB_YESNO) == IDYES)
		{
			if (strstr(lpszCommandLine, "/f"))
			{
				LogFileOpen("ResetConfig.log");
			}
			DeleteEffectPresets();
			RegDelnode(HKEY_CURRENT_USER, TEXT("Software\\GreenSoftware\\GSPlayer"));
			if (strstr(lpszCommandLine, "/f"))
			{
				LogFileClose();
			}
		}
		return 0;
	}

	if (MessageBox(NULL, "Are you sure you want to reset the GSPlayer configuration?", "Reset GSPlayer Configuration", MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
		if (strstr(lpszCommandLine, "/f"))
		{
			LogFileOpen("ResetConfig.log");
		}
		if (DeleteEffectPresets() == TRUE)
		MessageBox(NULL, "1 Or more presets couldn't be deleted.", "Warning", MB_ICONEXCLAMATION | MB_OK);
		bSuccess = RegDelnode(HKEY_CURRENT_USER, TEXT("Software\\GreenSoftware\\GSPlayer"));

		if(bSuccess)
		MessageBox(NULL, "Configuration Reset!", "Success", MB_ICONINFORMATION | MB_OK);
		else
		MessageBox(NULL, "There was a problem resetting the configuration.", "Error", MB_ICONERROR | MB_OK);
		if (strstr(lpszCommandLine, "/f"))
		{
			LogFileClose();
		}
	}

	return 0;
}
