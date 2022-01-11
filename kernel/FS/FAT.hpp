namespace FAT
{
	struct FormatParameters
	{
		u8 fatsCount;
		u8 sectorsPerCluster;
	};

	bool Init();
}
