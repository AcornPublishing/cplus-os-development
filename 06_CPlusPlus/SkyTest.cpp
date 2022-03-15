#include "SkyTest.h"
#include "SkyConsole.h"
#include "MultiBoot.h"
#include "TestInteger.h"
#include "PureCallExtend.h"
#include "ZetPlane.h"
#include "string.h"
#include "memory.h"
#include "kheap.h"

//���ͷ�Ʈ �ڵ鷯 �׽�Ʈ
void TestDivideByZero();
void TestBreakPoint();
void TestInvalidOpcode();

void TestInterrupt()
{
	//TestDivideByZero();
	//TestBreakPoint();
	TestInvalidOpcode();
}

int _divider = 0;
int _dividend = 100;
void TestDivideByZero()
{	
	int result = _dividend / _divider;

//����ó���� ���� �Ʒ� �ڵ尡 ����ȴٰ� �ص�
//result ����� �������� ���� �ƴϴ�
//���� ������ ����� ��ɾ�ܿ��� ���� ���� ���̸� 
//�߰��������� ������ �κ��� �����ߴ� �ؼ� �������� ����� ����ϴ� ���� ������.

	if(_divider != 0)
		result = _dividend / _divider;

	SkyConsole::Print("Result is %d, divider : %d\n", result, _divider);
}

void funcBreakPoint(void)
{
	__asm {
		align 4		
		__asm _emit 0xcc
		__asm _emit 0x00
		__asm _emit 0x00
		__asm _emit 0x00
	}
}

void TestBreakPoint()
{
	funcBreakPoint();
}

void TestInvalidOpcode()
{
	__asm {
		align 4
		__asm _emit 0xFF
		__asm _emit 0xFF
		__asm _emit 0xFF
		__asm _emit 0xFF
	}
}
//���ͷ�Ʈ �ڵ鷯 �׽�Ʈ ��


//C++ �׽�Ʈ 
void TestCPP11();
void TestPureFunctionCall();
void TestHeapCorruption();
void TestHeapCorruption2();
void TestInheritance();

void TestCPlusPlus()
{
	TestCPP11();
	//TestPureFunctionCall();
	//TestHeapCorruption();
	//TestHeapCorruption2();
	//TestInheritance();
}

void TestCPP11()
{
	SkyConsole::Print("\nCPP11 Test\n\n");

	auto func = [x = 5]() { return x; };
	auto n1 = func();
	SkyConsole::Print("Lamda n1:%d\n", n1);

	constexpr TestInteger size(10);
	int x[size];
	x[3] = 11;
	SkyConsole::Print("constexpr x[3]:%d\n", x[3]);
}

void TestPureFunctionCall()
{
	SkyConsole::Print("\nPure Function Call Test\n\n");
	PureCallExtend pureCall;
}

void TestHeapCorruption()
{
	SkyConsole::Print("\nHeap Corruption Test\n\n");
	Plane* pPlane = new Plane();

	//����� Ǫ�� ����
	*((char*)pPlane + sizeof(Plane) + 1) = (char)0;

	delete pPlane;
}

void TestHeapCorruption2()
{
	SkyConsole::Print("\nHeap Corruption Test2\n\n");
	Plane* pPlane = new Plane();

	//�����Լ� ���̺� �����߸���
	memset(pPlane, 0, sizeof(Plane));
	
	delete pPlane;
}

void TestInheritance()
{
	SkyConsole::Print("\nClass Inheritance Test\n\n");
	
	int i = 100;
	Plane* pPlane = new ZetPlane();
	pPlane->SetX(i);
	pPlane->SetY(i + 5);

	pPlane->IsRotate();

	SkyConsole::Print("Plane X : %d, Plane Y : %d\n", pPlane->GetX(), pPlane->GetY());

	delete pPlane;
}