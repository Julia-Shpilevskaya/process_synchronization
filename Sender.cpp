#include <fstream>
#include <windows.h>
#include <conio.h>
#include <iostream>


int main(int argc, char* argv[])
{
	std::string bin_file_name = argv[1];
	std::fstream file;

	HANDLE hStartEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, L"Process_started");
	if (hStartEvent == NULL)
		return GetLastError();

	HANDLE hWriteReadySemaphore = OpenSemaphore(EVENT_ALL_ACCESS, FALSE, L"Input_semaphore_started");
	if (hWriteReadySemaphore == NULL)
		return GetLastError();

	HANDLE hReadReadySemaphore = OpenSemaphore(EVENT_ALL_ACCESS, FALSE, L"Output_semaphore_started");
	if (hReadReadySemaphore == NULL)
		return GetLastError();

	HANDLE hMutex = OpenMutex(SYNCHRONIZE, FALSE, L"mutex");

	SetEvent(hStartEvent);

	std::cout << "Event was started\n";

	int number;

	while (true)
	{
		std::cout << "Enter 1 to write message\nEnter 0 to exit process\n";
		std::cin >> number;

		if (number == 1)
		{
			WaitForSingleObject(hMutex, INFINITE);

			file.open(bin_file_name, std::ios::out | std::ios::app);

			std::string msg;
			std::cout << "Enter message to add(not more than 20 symbols)\n";
			std::cin >> msg;

			if (msg.length() > 20)
			{
				std::cout << "You entered more than 20 symbols. Try again\n";
				continue;
			}

			char message[21];

			for (int i = 0; i < msg.length(); i++)
				message[i] = msg[i];

			for (int i = msg.length(); i < 20; i++)
				message[i] = '\0';

			message[20] = '\n';

			//ReleaseSemaphore(hReadReadySemaphore, 1, NULL);

			if (ReleaseSemaphore(hWriteReadySemaphore, 1, NULL) != 1)
			{
				std::cout << "File is full\n";

				//ReleaseSemaphore(hReadReadySemaphore, 1, NULL);
				WaitForSingleObject(hReadReadySemaphore, INFINITE);

				//ReleaseSemaphore(hReadReadySemaphore, 1, NULL);
				//ReleaseSemaphore(hReadReadySemaphore, 1, NULL);
				ReleaseSemaphore(hWriteReadySemaphore, 1, NULL);
			}

			for (int i = 0; i < 21; i++)
				file << message[i];

			file.close();

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

	return 0;
}