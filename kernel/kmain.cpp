#include"Terminal.hpp"
#include"Memory.hpp"
#include"Interrupt.hpp"
#include"Timer.hpp"
#include"Input/Keyboard.hpp"
#include"Input/Mouse.hpp"
#include"Input/PS2.hpp"
#include"Thread.hpp"

#include"ISA_DMA.hpp"

#include"Block/Block.hpp"
#include"Block/Dummy.hpp"
#include"Block/Floppy.hpp"
#include"Block/ATA.hpp"

#include"FS/FS.hpp"
#include"FS/BlkFS.hpp"
#include"FS/FAT.hpp"
#include"FS/VFS.hpp"
#include"Shell.hpp"
#include"Video/VGA.hpp"
#include"Video/Video.hpp"
#include"Video/Video_VGA.hpp"
#include"Input/Serial.hpp"

Video::Bitmap* screen = nullptr;

extern u32* _ctors_beg;
extern u32* _ctors_end;

int xx = 0;
void KillStack()
{
	// Print("X: %d\r", xx);
	xx++;
	if(xx >= 217)
		asm("xchg %bx, %bx");
	KillStack();
}

void AddKey(char c)
{
	if(c == '\n')
		Keyboard::AddEvent(Keyboard::KeyEvent { .type = Keyboard::KeyType::Pressed, .key = Keyboard::KeyCode::Enter } );
	else
		Keyboard::AddEvent(Keyboard::KeyEvent { .type = Keyboard::KeyType::Pressed, .ascii = c } );
}

int TestThread(void* args)
{
	Timer::Delay(500);
	for(auto c : "cd fdd\n")
		AddKey(c);

	Timer::Delay(500);
	for(auto c : "cd kernel\n")
		AddKey(c);

	Timer::Delay(500);
	for(auto c : "cat kmain.asm\n")
		AddKey(c);

	Timer::Delay(500);
	for(auto c : "splash\n")
		AddKey(c);

	return 0;
}

extern "C" void UserThread()
{
	u8* ptr = (u8*)0x800b8000;

	for(;;)
	{
		ptr[0]++;
	}
}

extern "C" void kmain()
{
	ASSERT(sizeof(u64) == 8, "u64");
	ASSERT(sizeof(u32) == 4, "u32");
	ASSERT(sizeof(u16) == 2, "u16");
	ASSERT(sizeof(u8) == 1, "u8");
	ASSERT(sizeof(size_t) == 4, "size_t");

	Terminal::Init();
	VGA::SetCursor(false);
	Memory::Init();
	Interrupt::Init();

	// UserThread();
	// __asm("jmp $0x1B, $UserThread");

	// Print("Logical: %p\n", (void*)kmain);
	// Print("Physical: %p\n", Memory::Logical::GetPhysicalFromLogical((void*)kmain));
	// for(;;);

	/* Call global constructors */
	Print("Constructors: (%p - %p)\n", _ctors_beg, _ctors_end);
	for(u32* ptr = _ctors_beg; ptr != _ctors_end; ptr++)
	{
		void (*func)() = (void (*)())(*ptr);
		Print("- %p\n", func);
		ASSERT(func, "Invalid constructor function~!");
		if(func)
			func();
	}

	Timer::Init();
	Thread::Init();
	// PS2::Init();

	Print("Thread started~!\n");

	// Interrupt::Disable();
	KillStack();

	Serial::Init();
	// Keyboard::Init();
	// Mouse::Init();
	// Mouse::Test();

	const u8 mousePort = 1;
	Serial::Configure(mousePort, 1200, Serial::WordLength::Bits_7, Serial::StopBits::Bits_1, Serial::Parity::None, true);
	Serial::SetReady(mousePort, false, false);
	Timer::Delay(200);
	Serial::ClearBuffers(mousePort);
	Serial::SetReady(mousePort, true, true);

	// Serial::Test(mousePort);

	u8 mouseVal = 0;
	for(unsigned a = 0; a < 1; a++) {
		if(!Serial::ReadByte(mousePort, &mouseVal, 2000))
			Print("No serial mouse :/\n");
		else
			Print("Mouse: %x\n", mouseVal);
	}

	Print("Finished~!\n");
	Serial::ClearBuffers(mousePort);
	for(;;)
	{
		Serial::ReadByte(1, &mouseVal, -1);
		Print("Byte: %x\n", mouseVal);
	}

	if(0)
	{
		Print("=== Memory test ===\n");
		for(unsigned a = 0; a < 80*25; a++)
			Terminal::Print(" ");
		Terminal::SetXY(0, 0);
		Print("Memory test:\n");
		char c = 0;
		u32 zxc = 0;

		for(volatile unsigned a = 0; a < 1000; a++)
		{
			zxc++;
			if(a % 50 == 0)
			{
				// Terminal::SetXY(0, 1);
				Terminal::Clear();
				Terminal::SetXY(0, 0);
				Print("### Iteration: %d\n", zxc);
				Memory::PrintMemoryMap();
				// Timer::Delay(1000);
			}

			// Memory::Alloc(5);
			// if(a == 999)
			{
				const u32 cnt = 1000;
				static u8* ptr[cnt];

				for(unsigned x = 0; x < cnt; x++)
				{
					u32 size = (zxc % 123) + 5;
					// Print("&ptr[x] = %p\nAlloc~! ", &ptr[x]);
					ptr[x] = (u8*)Memory::Alloc(size);// Memory::Physical::AllocPage();
					// Print("Assign~! (%p, %p)\n", ptr[x], Memory::Logical::GetPhysicalFromLogical(ptr[x]));
					// ptr[x] = tmpPtr;
					// Print("Filling~! [%p, %p]\n", &ptr[x][0], &ptr[x][1023]);
					for(unsigned y = 0; y < size; y++)
					{
						const u32 v = 0xBAADF00D;
						u8* vv = (u8*)&v;
						ptr[x][y] = vv[y % 4];
					}
					// Print("Ok, -> %d (%d - %p)\n", zxc, x, ptr[x]);
				}

				// FIXME: memory is leaking...
				for(unsigned x = 0; x < cnt; x++)
					Memory::Free(ptr[x]); // Memory::Physical::FreePage(ptr[ (x * 3) % cnt ]);

				if(a == 999)
					a = 0;
			}

			// for(volatile unsigned b = 0; b < 9999; b++)
			{

			}

			// Print("%c\b", c++);
		}
	}

	Block::Init();
	// Block::Dummy::Init();
	ISA_DMA::Init();
	Floppy::Init();
	// ATA::Init();
	FS::Init();
	FAT::Init();
	// FS::BlkFS::Init();
	VFS::Init();
	// FIXME: memory allocator is crashing... :F

	if(VFS::Mount("fdd0r0", "fdd") != Status::Success)
	{
		Print("No floppy :<\n");
		// for(;;);
	}

	if(0)
	{
		Video::Init(&Video::vgaDriver);

		auto modes = Video::GetAvailableModes();

		Print("Available modes:\n");
		for(auto mode : modes)
		{
			Print("- [%c] %dx%d %dbpp\n", (mode.type == Video::Mode::Type::Graphical) ? 'G' : 'T', mode.width, mode.height, mode.bpp);
		}

		Video::SetMode(modes.Back());
		Video::ClearScreen();

		screen = Video::GetScreen();

		// Video::DrawRect(Video::Rect(10, 10, 180, 50), Video::Color(255, 0, 0));
		// Video::DrawRect(Video::Rect(40, 30, 180, 20), Video::Color(0, 255, 0, 128));
	}

	if(1)
	{
		static int mx = 20, my = 20;
		static bool mousePressed = false;

		auto hndl = [](const Mouse::Event* ev) {
			Print("Event: %d\n", (int)ev->type);

			if(ev->type == Mouse::EventType::Movement)
			{
				mx += ev->movement.x;
				my -= ev->movement.y;
			}

			if(ev->type == Mouse::EventType::ButtonClick && ev->button == Mouse::Button::Left)
				mousePressed = true;
			if(ev->type == Mouse::EventType::ButtonRelease && ev->button == Mouse::Button::Left)
				mousePressed = false;

			if(mx < 0)
				mx = 0;
			if(my < 0)
				my = 0;
			if(mx >= 320)
				mx = 320 - 1;
			if(my >= 200)
				my = 200 - 1;
		};
		Mouse::Register(hndl);

		// for(;;)
		// {
		// 	Print("Mouse: %dx%d     \r", mx, my);
		// }

		// VGA::Init();
		Video::Init(&Video::vgaDriver);

		auto modes = Video::GetAvailableModes();
		if(!Video::SetMode(modes[1]))
		{
			Print("Set mode failed~!\n");
			for(;;);
		}

		Video::ClearScreen();
		// Video::UpdateScreen();
		// for(;;);
		auto screen = Video::GetScreen();

		Video::Bitmap* img;
		Video::CreateBitmap(320, 200, &img);
		Video::DrawRect(img, Video::Rect(0, 0, 320, 200), Video::Color(0, 0, 0));

		for(unsigned a = 0; a < img->width * img->height; a++)
		{
			img->pixels[a] = Video::Color(0, 64, 0, 255);
		}

		Video::SetBlending(img, Video::BlendingMethod::Static);
		int imgAlpha = 0;

		int frame = 0, frameTimer = Timer::GetTicks();
		auto oldCursor = Video::Rect(0, 0, 0, 0);

		auto res = Video::LoadBitmap(String("/fdd/test.bmp"), &img);
		Print("Res: %d\n", res);

		Video::ClearScreen();
		Video::DrawBitmap(Video::Rect(0, 0, 64, 64), img, Video::Rect(64, 16, 0, 0), screen);
		Video::UpdateScreen();

		u32 cnt = 0;
		for(;;)
		{
			u32 x = cnt % (320 - 64);
			if( (cnt / (320 - 64)) % 2)
				x = 320 - 64 - x;
			u32 y = cnt % (200 - 64);
			if( (cnt / (200 - 64)) % 2)
				y = 200 - 64 - y;

			Video::ClearScreen();
			Video::DrawBitmap(Video::Rect(0, 0, 64, 64), img, Video::Rect(x, y, 0, 0), screen);
			Video::UpdateScreen();

			cnt++;
		}

		for(;;)
		{
			Video::ClearScreen();

			// Video::DrawRect(screen, Video::Rect(-2, -2, 5, 5), Video::Color(255, 0, 0));
			// Video::UpdateScreen();

			Video::DrawBitmap(Video::Rect(0, 0, 320, 200), img, Video::Rect(0, 0, 0, 0), screen);

			Video::Rect rect(mx - 2, my - 2, 5, 5);
			Video::Color color(255, 0, 0);
			Video::DrawRect(screen, rect, color);
			if(mousePressed)
				Video::DrawRect(img, rect, Video::Color(0, 64, 0));
			// Video::UpdateScreen(oldCursor);
			// Video::UpdateScreen(rect);
			Video::UpdateScreen();

			Video::SetBlendingStaticAlpha(img, imgAlpha);
			imgAlpha = (imgAlpha + 1) % 256;

			oldCursor = rect;
			// Thread::NextThread();

			frame++;
			if(Timer::GetTicks() >= frameTimer + 1000)
			{
				Print("FPS: %d (%d frames per %d)\n", frame*1000/u32(Timer::GetTicks() - frameTimer),
					frame, Timer::GetTicks() - frameTimer);

				frame = 0;
				frameTimer=Timer::GetTicks();
			}
		}
	}

#if 1
	// Thread::Thread* fakeKbdThread;
	// Thread::Create(&fakeKbdThread, (u8*)"FakeKbd", FakeKbdThread);
	// Thread::Start(fakeKbdThread);

	Thread::Thread* shellThread;
	Thread::Create(&shellThread, "Shell", Shell::Thread);
	Thread::Start(shellThread);

	if(0)
	{
		Thread::Thread* tT;
		Thread::Create(&tT, "test", TestThread);
		Thread::Start(tT);
	}
#endif

	Print("\n\nKernel running...\n");
	for(;;)
	{
		Thread::SetState(Thread::currentThread, Thread::State::Zombie);
		Thread::NextThread();
		Print("WTF~!\n");
	}
}