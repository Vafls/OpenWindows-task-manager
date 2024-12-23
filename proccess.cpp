#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include <vector>
#include <conio.h>
#include <tchar.h>
#include <psapi.h>
#include <fstream>
#include <locale>
#include <codecvt>
#include <limits>

class ProcessManager {
public:
    void listProcesses();
    void showProcessDetails();
    void killProcess(DWORD processID);
    void updateProcesses();

private:
    std::vector<PROCESSENTRY32> getProcesses();
    std::vector<PROCESSENTRY32> processes;
};

std::vector<PROCESSENTRY32> ProcessManager::getProcesses() {
    std::vector<PROCESSENTRY32> processes;
    PROCESSENTRY32 pe32;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return processes;
    }
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return processes;
    }
    do {
        processes.push_back(pe32);
    } while (Process32Next(hProcessSnap, &pe32));
    CloseHandle(hProcessSnap);
    return processes;
}

void ProcessManager::updateProcesses() {
    processes = getProcesses();
}

void ProcessManager::listProcesses() {
    while (true) {
        system("cls");
        for (size_t i = 0; i < processes.size(); ++i) {
            std::wcout << processes[i].szExeFile << L" (PID: " << processes[i].th32ProcessID << L")" << std::endl;
        }
        std::wcout << L"Enter the ID of the process to kill it (ESC to exit): ";
        std::string input;
        char ch = _getch();
        if (ch == 27) { // ESC key
            break;
        }
        std::getline(std::cin, input);
        input.insert(input.begin(), ch); // Insert the first character read by _getch()
        try {
            DWORD pid = std::stoul(input);
            killProcess(pid);
        } catch (const std::exception& e) {
            std::wcout << L"Invalid input. Please enter a valid process ID." << std::endl;
        }
    }
}

void ProcessManager::showProcessDetails() {
    while (true) {
        std::wcout << L"Enter Process ID (ESC to exit): ";
        std::string input;
        char ch = _getch();
        if (ch == 27) { // ESC key
            break;
        }
        std::getline(std::cin, input);
        input.insert(input.begin(), ch); // Insert the first character read by _getch()
        try {
            DWORD pid = std::stoul(input);
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
            if (hProcess == NULL) {
                std::wcout << L"Could not open process." << std::endl;
                continue;
            }
            TCHAR processName[MAX_PATH] = TEXT("<unknown>");
            HMODULE hMod;
            DWORD cbNeeded;
            if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
                GetModuleBaseName(hProcess, hMod, processName, sizeof(processName) / sizeof(TCHAR));
            }
            std::wcout << L"Process ID: " << pid << std::endl;
            std::wcout << L"Process Name: " << processName << std::endl;
            CloseHandle(hProcess);
        } catch (const std::exception& e) {
            std::wcout << L"Invalid input. Please enter a valid process ID." << std::endl;
        }
    }
}

void ProcessManager::killProcess(DWORD processID) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processID);
    if (hProcess == NULL) {
        std::wcout << L"Could not open process." << std::endl;
        return;
    }
    if (!TerminateProcess(hProcess, 0)) {
        std::wcout << L"Could not terminate process." << std::endl;
    } else {
        std::wcout << L"Process terminated." << std::endl;
    }
    CloseHandle(hProcess);
}

int main() {
    // Set console output to UTF-8
    SetConsoleOutputCP(CP_UTF8);
    std::wcout.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));

    ProcessManager pm;
    pm.updateProcesses();
    DWORD pid;
    int choice;
    while (true) {
        system("cls");
        std::wcout << L"1. List Processes" << std::endl;
        std::wcout << L"2. Show Process Details" << std::endl;
        std::wcout << L"3. Update" << std::endl;
        std::wcout << L"4. Exit" << std::endl;
        std::wcout << L"Enter your choice: ";
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear the input buffer
        switch (choice) {
            case 1:
                pm.listProcesses();
                break;
            case 2:
                pm.showProcessDetails();
                break;
            case 3:
                pm.updateProcesses();
                break;
            case 4:
                return 0;
            default:
                std::wcout << L"Invalid choice. Please try again." << std::endl;
        }
    }
    return 0;
}