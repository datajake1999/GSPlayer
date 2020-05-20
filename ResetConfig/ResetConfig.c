#include <windows.h>
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
            printf("Key not found.\n");
            return TRUE;
        } 
        else {
            printf("Error opening key.\n");
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

void DeleteEffectPresets()
{
	char FolderPath[MAX_PATH];
	char FilePath[MAX_PATH];
	WIN32_FIND_DATAA fd;
	HANDLE hFind;
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
		printf("No preset files found.\n");
		return;
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
		printf("Error deleting %s.\n", FilePath);
	}
	while (FindNextFile(hFind, &fd))
	{
		StringCchCopy(FilePath, MAX_PATH, FolderPath);
		StringCchCat(FilePath, MAX_PATH, "\\");
		StringCchCat(FilePath, MAX_PATH, fd.cFileName);
		if (DeleteFile(FilePath) == 0)
		{
			printf("Error deleting %s.\n", FilePath);
		}
	}
	//Close the handel to hFind
	FindClose(hFind);
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

   CMDResult = strcmp(lpszCommandLine, "/h");
   if (CMDResult == 0)
   {
      MessageBox(NULL, "Usage:\n/u - uninstaller friendly interface\n/s - silent mode\n/l - log errors to console\n/h - show help\n", "Information", MB_ICONINFORMATION | MB_OK);
      return 0;
   }

   if (strstr(lpszCommandLine, "/l"))
   {
      RedirectStdOut();
   }

   if (strstr(lpszCommandLine, "/s"))
   {
      DeleteEffectPresets();
      RegDelnode(HKEY_CURRENT_USER, TEXT("Software\\GreenSoftware\\GSPlayer"));
      return 0;
   }

   if (strstr(lpszCommandLine, "/u"))
   {
      if (MessageBox(NULL, "Do you wish to reset the GSPlayer configuration?", "", MB_ICONQUESTION | MB_YESNO) == IDYES)
      {
         DeleteEffectPresets();
         RegDelnode(HKEY_CURRENT_USER, TEXT("Software\\GreenSoftware\\GSPlayer"));
      }
      return 0;
   }

   if (MessageBox(NULL, "Are you sure you want to reset the GSPlayer configuration?", "Reset GSPlayer Configuration", MB_ICONQUESTION | MB_YESNO) == IDYES)
   {
      DeleteEffectPresets();
      bSuccess = RegDelnode(HKEY_CURRENT_USER, TEXT("Software\\GreenSoftware\\GSPlayer"));

      if(bSuccess)
         MessageBox(NULL, "Configuration Reset!", "Success", MB_ICONINFORMATION | MB_OK);
      else
         MessageBox(NULL, "There was a problem resetting the configuration.", "Error", MB_ICONERROR | MB_OK);
   }

   return 0;
}
