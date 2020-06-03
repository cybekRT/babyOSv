char* tmp = "Dafuq, just random text~!";

int strlen(const char* str)
{
	unsigned len = 0;
	while(*str++)
		len++;

	return len;
}

void kmain()
{
	/*char* vid = (char*)0xa0000;

	for(unsigned a = 0; a < 320 * 3; a++)
	{
		vid[a] = 0x1;
		vid[320 * 197 + a] = 0x1;
	}

	for(unsigned a = 0; a < 200; a++)
	{
		vid[a * 320 + 0] = 0x1;
		vid[a * 320 + 1] = 0x1;
		vid[a * 320 + 2] = 0x1;

		vid[a * 320 + 0 + 317] = 0x1;
		vid[a * 320 + 1 + 317] = 0x1;
		vid[a * 320 + 2 + 317] = 0x1;
	}*/

	char* data = (char*)0xb8000;

	/*for(unsigned a = 0; a < 320*200; a++)
	{
		data[a * 2 + 0] = 0;
		//data[a * 2 + 1] = 0x80;
	}*/

//__asm("xchgb bx, bx");
	data[ 0] = 'K';
	data[ 2] = 'e';
	data[ 4] = 'r';
	data[ 6] = 'n';
	data[ 8] = 'C';
	data[10] = ' ';

	for(unsigned a = 0; a < strlen(tmp); a++)
	{
		data[80 * 2 + a * 2] = tmp[a];
	}

	for(;;)
	{
		__asm("cli");
		__asm("hlt");
	}
}