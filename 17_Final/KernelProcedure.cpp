#include "SkyOS.h"
#include "SkyRenderer.h"
#include "nic.h"
#include "SkyGUISystem.h"
#include "SkyTest.h"
#include "SkyDebugger.h"

bool systemOn = false;

void NativeConsole()
{
	ConsoleManager manager;

	char	commandBuffer[MAXPATH];

	while (1)
	{
		int driveId = StorageManager::GetInstance()->GetCurrentDriveId();
		char driveLetter = 'a' + driveId;
		std::string driveName;
		driveName += toupper(driveLetter);
		driveName += ":> ";

		SkyConsole::Print("%s", (char*)driveName.c_str());		
		memset(commandBuffer, 0, MAXPATH);

		SkyConsole::GetCommand(commandBuffer, MAXPATH - 2);
		SkyConsole::Print("\n");

		if (manager.RunCommand(commandBuffer) == true)
			break;
	}
}

DWORD WINAPI SystemIdle(LPVOID parameter)
{
	while (1)
	{
		ksleep(10);
	}

	return 0;
}

DWORD WINAPI SystemConsoleProc(LPVOID parameter)
{
	SkyConsole::Print("Console Mode Start!!\n");

	systemOn = true;
	StartPITCounter(100, I86_PIT_OCW_COUNTER_0, I86_PIT_OCW_MODE_SQUAREWAVEGEN);

	multiboot_info* pBootInfo = SkyModuleManager::GetInstance()->GetMultiBootInfo();
	StorageManager::GetInstance()->Initilaize(pBootInfo);

	kEnterCriticalSection();
	SkyDebugger::GetInstance()->LoadSymbol("DEBUG_ENGINE_DLL");
	kLeaveCriticalSection();

	StorageManager::GetInstance()->SetCurrentFileSystemByID('L');
	
	NativeConsole();

	SkyConsole::Print("Bye!!");

	return 0;
}
extern void SampleFillRect(ULONG* lfb0, int x, int y, int w, int h, int col);

DWORD WINAPI SystemGUIProc(LPVOID parameter)
{
	unsigned int* ebp = (unsigned int*)&parameter - 1;
	SkyConsole::Print("start ebp : %x\n", *ebp);
	SkyConsole::Print("parameter : %x\n", parameter);

	multiboot_info* pBootInfo = SkyModuleManager::GetInstance()->GetMultiBootInfo();

	kEnterCriticalSection();

	StartPITCounter(1000, I86_PIT_OCW_COUNTER_0, I86_PIT_OCW_MODE_SQUAREWAVEGEN);
	
	StorageManager::GetInstance()->Initilaize(pBootInfo);
	StorageManager::GetInstance()->SetCurrentFileSystemByID('L');

	SkyDebugger::GetInstance()->LoadSymbol("DEBUG_ENGINE_DLL");

	SampleFillRect((ULONG*)SkyGUISystem::GetInstance()->GetVideoRamInfo()._pVideoRamPtr, 1004, 0, 20, 20, 0x0000FF00);		
	

	SkyGUISystem::GetInstance()->InitGUIModule();

	systemOn = true;
	kLeaveCriticalSection();

	SkyGUISystem::GetInstance()->Run();

	return 0;
}

#define TS_WATCHDOG_CLOCK_POS		(0xb8000+(80-1)*2)
#define TIMEOUT_PER_SECOND		50
static bool m_bShowTSWatchdogClock = true;

DWORD WINAPI WatchDogProc(LPVOID parameter)
{
	Process* pProcess = (Process*)parameter;
	int pos = 0;
	char *addr = (char *)TS_WATCHDOG_CLOCK_POS, status[] = { '-', '\\', '|', '/', '-', '\\', '|', '/' };
	int first = GetTickCount();

	//SkyConsole::Print("\nWatchDogProc Start. Thread Id : %d\n", kGetCurrentThreadId());

	while (1)
	{

		int second = GetTickCount();
		if (second - first >= TIMEOUT_PER_SECOND)
		{
			if (++pos > 7)
				pos = 0;

			if (m_bShowTSWatchdogClock)
				*addr = status[pos];

			first = GetTickCount();
		}
		kEnterCriticalSection();
		Scheduler::GetInstance()->Yield();
		kLeaveCriticalSection();
	}

	return 0;
}

void SampleFillRect(ULONG* lfb0, int x, int y, int w, int h, int col)
{
	for (int k = 0; k < h; k++)
		for (int j = 0; j < w; j++)
		{
			int index = ((j + x) + (k + y) * 1024);
			lfb0[index] = col;
		}
}

void WatchDogLoop(Process* pProcess)
{
	int pos = 0;

	int colorStatus[] = { 0x00FF0000, 0x0000FF00, 0x0000FF };
	int first = GetTickCount();

	//?????? ???? ?????? ??????.
	ULONG* lfb = (ULONG*)SkyGUISystem::GetInstance()->GetVideoRamInfo()._pVideoRamPtr;

	while (1)
	{
		int second = GetTickCount();
		//1?? ?????? ?????? ????????.
		if (second - first >= TIMEOUT_PER_SECOND)
		{
			if (++pos > 2)
				pos = 0;

			if (m_bShowTSWatchdogClock)
			{
				SampleFillRect(lfb, 1004, 0, 20, 20, colorStatus[pos]);
			}

			first = GetTickCount();
		}

		//?????? ?????? ?????? ???????? ?????????? ?? ?????????? ????????.
		Scheduler::GetInstance()->Yield();
	}
}

DWORD WINAPI GUIWatchDogProc(LPVOID parameter)
{
	Process* pProcess = (Process*)parameter;	

	//?????? ?????? ?????? ?????? ???????? ??????.
	WatchDogLoop(pProcess);

	return 0;
}

void LoopProcessRemove(Process* pProcess)
{
	int static id = 0;
	int temp = id++;
	int first = GetTickCount();

	while (1)
	{

		//?????? ???? ???? ??????		
		kEnterCriticalSection();
		//PhysicalMemoryManager::EnablePaging(false);
		//PhysicalMemoryManager::EnablePaging(true);

		ProcessManager::GetInstance()->RemoveTerminatedProcess();

		//Scheduler::GetInstance()->Yield();
		kLeaveCriticalSection();

		int second = GetTickCount();
		if (second - first >= 400)
		{
			first = GetTickCount();
		}
	}
}

DWORD WINAPI ProcessRemoverProc(LPVOID parameter)
{
	Process* pProcess = (Process*)parameter;

	LoopProcessRemove(pProcess);

	return 0;
}