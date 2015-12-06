#define NOMINMAX

#include <stdio.h>
#include "Kernel.h"
#include "Gui.h"
#include "Bootloader.h"

#include "citraimport\GPU\window\emu_window_glfw.h"

namespace VideoCore {
	void Init(EmuWindow* emu_window);
} // namespace


static u8 code[0x1000000];

KKernel* mykernel; //this is a citra hack

int main(int argc, char* argv[]) {
    /*FILE* fd = fopen("code.bin", "rb");
    size_t size = fread(code, 1, sizeof(code), fd);
    fclose(fd);*/

	//citra hacks

	EmuWindow_GLFW window;

	VideoCore::Init(&window);

	//citra hacks end

    //MainWindow* wndMain = new MainWindow();
	mykernel = new KKernel();

    Mem_Init(false);
    Mem_SharedMemInit();

	Boot(mykernel);

    //kernel->AddQuickCodeProcess(&code[0], size);
    mykernel->ThreadsRunTemp();
    return 0;
}
extern "C" void citraFireInterrupt(int id)
{
	LOG("cirta fire %02x",id);
	mykernel->FireInterrupt(id);
}
bool novideo = true;
extern "C" int citraPressedkey = 0;
extern "C" bool citraSettingSkipGSP = false;