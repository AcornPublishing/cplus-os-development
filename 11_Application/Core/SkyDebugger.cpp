#include "SkyDebugger.h"
#include "SkyConsole.h"
#include "ProcessManager.h"
#include "Thread.h"
#include "SkyModuleManager.h"
#include "Exception.h"
#include "SkyMockInterface.h"
#include "MapFile.h"
#include "StorageManager.h"
#include "stdio.h"
//�޸� �Ҵ�
#include "kheap.h"

SkyDebugger* SkyDebugger::m_pDebugger = nullptr;

SKY_FILE_Interface g_FileInterface =
{
	fread,
	fopen,
	fwrite,
	fclose,
	feof,
	ferror,
	fflush,
	freopen,
	fseek,
	ftell,
	fgetc,
	fgets,
};

u32int sky_kmalloc(u32int sz)
{
	return kmalloc(sz);
}

void sky_kfree(void *p)
{
	kfree(p);
}

static SKY_ALLOC_Interface g_allocInterface =
{
	sky_kmalloc,
	sky_kfree,
};


void sky_printf(const char* str, ...)
{
	char *p = new char[128];
	va_list ap;

	va_start(ap, str);
	(void)vsprintf(p, str, ap);
	va_end(ap);

	SkyConsole::Print(p);
	delete p;
}

SKY_Print_Interface g_printInterface =
{
	sky_printf,
};

extern SKY_FILE_Interface g_FileInterface;
extern SKY_ALLOC_Interface g_allocInterface;
extern SKY_Print_Interface g_printInterface;

typedef void(*PSetSkyMockInterface)(SKY_ALLOC_Interface, SKY_FILE_Interface, SKY_Print_Interface);
typedef I_MapFileReader*(*PGetDebugEngineDLL)();


SkyDebugger::SkyDebugger()
{
	m_pMapReader = nullptr;
	m_symbolInit = false;
}


SkyDebugger::~SkyDebugger()
{
}

void SkyDebugger::TraceStack(unsigned int maxFrames)
{
	//���� ��Ȳ
	//  ù��° �Ķ���� maxFrames
	//  Trace �Լ��� �����Ų ȣ���Լ� �����ּ�
	//  ȣ���Լ��� EBP(���� EBP �������Ͱ� �̰��� ����Ű�� �ִ�)
	unsigned int* ebp = &maxFrames - 2;
	SkyConsole::Print("Stack trace:\n");
	//���� �½�ũ�� ����.
	Thread* pTask = ProcessManager::GetInstance()->GetCurrentTask();

	for (unsigned int frame = 0; frame < maxFrames; ++frame)
	{
		unsigned int eip = ebp[1];
		if (eip == 0)
			//�Լ� �����ּҰ� 0�̸� �ݽ��� ����� ������.
			break;
		// ���� ȣ���Լ��� �������������� �̵��Ѵ�.
		ebp = reinterpret_cast<unsigned int *>(ebp[0]);
		unsigned int * arguments = &ebp[2];
		SkyConsole::Print("  0x{%x}     \n", eip);
	}
}

void SkyDebugger::TraceStackWithSymbol(unsigned int maxFrames)
{
	//���� ��Ȳ
	//  ù��° �Ķ���� maxFrames
	//  Trace �Լ��� �����Ų ȣ���Լ� �����ּ�
	//  ȣ���Լ��� EBP(���� EBP �������Ͱ� �̰��� ����Ű�� �ִ�)
	unsigned int* ebp = &maxFrames - 2;
	SkyConsole::Print("Stack trace:\n");
	//���� �½�ũ�� ����.
	Thread* pTask = ProcessManager::GetInstance()->GetCurrentTask();

	int lineNumber = 0;
	DWORD resultAddress = 0;
	char function[256];
	char name[256];
	char fileName2[256];


	for (unsigned int frame = 0; frame < maxFrames; ++frame)
	{
		unsigned int eip = ebp[1];
		if (eip == 0)
			//�Լ� �����ּҰ� 0�̸� �ݽ��� ����� ������.
			break;
		// ���� ȣ���Լ��� �������������� �̵��Ѵ�.
		ebp = reinterpret_cast<unsigned int *>(ebp[0]);
		unsigned int * arguments = &ebp[2];

		if (m_symbolInit == true)
		{
			bool result = m_pMapReader->getAddressInfo(eip,
				name, fileName2, lineNumber, function, resultAddress);

			if(result == true)
				SkyConsole::Print("  %s     \n", function);
		}
		else
		{
			SkyConsole::Print("  0x{%x}     \n", eip);
		}
		
	}
}

//����׿��� ����� �ε��Ѵ�.
bool SkyDebugger::LoadSymbol(const char* moduleName)
{
	
//����� ��� ������ ã�´�.
	MODULE_HANDLE hwnd = SkyModuleManager::GetInstance()->LoadModuleFromMemory(moduleName);

	if (hwnd == nullptr)
	{
		HaltSystem("Memory Module Load Fail!!");
	}

//����� ���� ���� ���� SetSkyMockInterface, GetDebugEngineDLL �Լ��� ���´�.
	PSetSkyMockInterface SetSkyMockInterface = (PSetSkyMockInterface)SkyModuleManager::GetInstance()->GetModuleFunction(hwnd, "SetSkyMockInterface");
	PGetDebugEngineDLL GetDebugEngineDLLInterface = (PGetDebugEngineDLL)SkyModuleManager::GetInstance()->GetModuleFunction(hwnd, "GetDebugEngineDLL");

//����� ������ �÷��� �������� �������̽��� �ѱ��.
	SetSkyMockInterface(g_allocInterface, g_FileInterface, g_printInterface);

	if (!GetDebugEngineDLLInterface)
	{
		HaltSystem("Memory Module Load Fail!!");
	}
	
	m_pMapReader = GetDebugEngineDLLInterface();
	
	if (m_pMapReader == nullptr)
	{
		HaltSystem("Map Reader Creation Fail!!");
	}

	StorageManager::GetInstance()->SetCurrentFileSystemByID('L');
	
	bool result = m_pMapReader->readFile("SKYOS32_MAP");
	
	if (result == false)
	{
		HaltSystem("Map Read Fail!!");
	}

	m_pMapReader->setLoadAddress(0x100000);

	/*
	int lineNumber = 0;
	DWORD resultAddress = 0;
	char function[256];
	char name[256];
	char fileName2[256];

	result = pMapReader->getAddressInfo(0x00100df0,
		name, fileName2, lineNumber, function, resultAddress);

	SkyConsole::Print("%s\n", function);*/

	m_symbolInit = true;
	return true;
}

Module* SkyDebugger::FindModule(multiboot_info* bootinfo, const char* moduleName)
{
	uint32_t mb_flags = bootinfo->flags;
	if (mb_flags & MULTIBOOT_INFO_MODS)
	{
		uint32_t mods_count = bootinfo->mods_count;
		uint32_t mods_addr = (uint32_t)bootinfo->Modules;

		for (uint32_t mod = 0; mod < mods_count; mod++)
		{
			Module* module = (Module*)(mods_addr + (mod * sizeof(Module)));

			const char* module_string = (const char*)module->Name;

			SkyConsole::Print("Module Name : %s 0x%x 0x%x\n", module_string, module->ModuleStart, module->ModuleEnd);

			if (strcmp(module_string, moduleName) == 0)
			{
				return module;
			}
		}
	}

	return nullptr;
}
