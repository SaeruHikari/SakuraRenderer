#include "UI/SakuraRenderer.h"
#include <QtWidgets/QApplication>
#include <windows.h>
#include "Applications/SakuraRendererApplication.h"

#include "SakuraCore.h"
#pragma comment(lib, "SakuraCore.lib")

#include "SakuraCore_ECS.h"
#pragma comment(lib, "SakuraCore_ECS.lib")

int main(int argc, char* argv[])
{
	SakuraRendererApplication a(argc, argv);
	//InitWorld();
	if (a.Initialize())
		return a.Run();
	assert(0);
	return a.exec();
}
