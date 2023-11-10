#include <fstream>
#include <iostream>
#include <windows.h>
#include <conio.h>
#include <string>


int main()
{
	std::string bin_file_name;
	std::cout << "Enter binary file name\n";
	std::cin >> bin_file_name;

	long long number_of_notes;
	std::cout << "Enter number of notes:\n";
	std::cin >> number_of_notes;

	std::fstream file;
	file.open(bin_file_name, std::ios::out);
	file.close();

	int number_of_senders;
	std::cout << "Enter number of Sender processes:\n";
	std::cin >> number_of_senders;

	HANDLE hWriteReadySemaphore = CreateSemaphore(NULL, 0, number_of_notes, L"Input_semaphore_started");
	if (hWriteReadySemaphore == NULL)
		return GetLastError();

	HANDLE hReadReadySemaphore = CreateSemaphore(NULL, 0, number_of_notes, L"Output_semaphore_started");
	if (hReadReadySemaphore == NULL)
		return GetLastError();

	HANDLE hMutex = CreateMutex(NULL, 0, L"mutex");

	HANDLE* hEventStarted = new HANDLE[number_of_senders];

	LPWSTR lpwstrSenderProcessCommandLine;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	for (int i = 0; i < number_of_senders; ++i)
	{
		std::string sender_cmd = "Process_synchronization_Sender.exe " + bin_file_name;
		std::wstring converting_sender_to_lpwstr = std::wstring(sender_cmd.begin(), sender_cmd.end());
		lpwstrSenderProcessCommandLine = &converting_sender_to_lpwstr[0];

		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);

		if (!CreateProcess(NULL, lpwstrSenderProcessCommandLine, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
		{
			std::cout << "The Sender process is not started.\n";
			return GetLastError();
		}

		hEventStarted[i] = CreateEvent(NULL, FALSE, FALSE, L"Process_started");

		if (hEventStarted[i] == NULL)
			return GetLastError();

		CloseHandle(pi.hProcess);
	}

	WaitForMultipleObjects(number_of_senders, hEventStarted, TRUE, INFINITE);

	int number;

	file.open(bin_file_name, std::ios::in);

	while (true)
	{
		std::cout << "\nEnter 1 to read message\nEnter 0 to exit process\n";
		std::cin >> number;

		if (number == 1)
		{
			std::string message;

			WaitForSingleObject(hWriteReadySemaphore, INFINITE);
			WaitForSingleObject(hMutex, INFINITE);

			std::getline(file, message);

			std::cout << message;

			ReleaseSemaphore(hReadReadySemaphore, 1, NULL);
			ReleaseMutex(hMutex);
		}
		else if (number == 0)
		{
			std::cout << "Process ended";
			break;
		}
		else if (number != 0 && number != 1)
			std::cout << "\nIncorrect value. Enter again\n";
	}

	file.close();

	CloseHandle(hWriteReadySemaphore);
	CloseHandle(hReadReadySemaphore);

	for (int i = 0; i < number_of_senders; i++)
		CloseHandle(hEventStarted[i]);

	return 0;
}