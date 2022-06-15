namespace Device
{
	struct Instance
	{
		Status Read(void* buffer, u32 bufferSize, u32* dataRead);
		Status Write(void* buffer, u32 bufferSize, u32* dataWritten);
		void* priv;
	};

	bool Init();


}
