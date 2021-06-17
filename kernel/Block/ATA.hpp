namespace ATA
{
	struct IDENTIFY_DEVICE_DATA {
		struct {
			u16 Reserved1 : 1;
			u16 Retired3 : 1;
			u16 ResponseIncomplete : 1;
			u16 Retired2 : 3;
			u16 FixedDevice : 1;
			u16 RemovableMedia : 1;
			u16 Retired1 : 7;
			u16 DeviceType : 1;
		} GeneralConfiguration;
		u16 NumCylinders;
		u16 SpecificConfiguration;
		u16 NumHeads;
		u16 Retired1[2];
		u16 NumSectorsPerTrack;
		u16 VendorUnique1[3];
		u8  SerialNumber[20];
		u16 Retired2[2];
		u16 Obsolete1;
		u8  FirmwareRevision[8];
		u8  ModelNumber[40];
		u8  MaximumBlockTransfer;
		u8  VendorUnique2;
		struct {
			u16 FeatureSupported : 1;
			u16 Reserved : 15;
		} TrustedComputing;
		struct {
			u8  CurrentLongPhysicalSectorAlignment : 2;
			u8  ReservedByte49 : 6;
			u8  DmaSupported : 1;
			u8  LbaSupported : 1;
			u8  IordyDisable : 1;
			u8  IordySupported : 1;
			u8  Reserved1 : 1;
			u8  StandybyTimerSupport : 1;
			u8  Reserved2 : 2;
			u16 ReservedWord50;
		} Capabilities;
		u16 ObsoleteWords51[2];
		u16 TranslationFieldsValid : 3;
		u16 Reserved3 : 5;
		u16 FreeFallControlSensitivity : 8;
		u16 NumberOfCurrentCylinders;
		u16 NumberOfCurrentHeads;
		u16 CurrentSectorsPerTrack;
		u32  CurrentSectorCapacity;
		u8  CurrentMultiSectorSetting;
		u8  MultiSectorSettingValid : 1;
		u8  ReservedByte59 : 3;
		u8  SanitizeFeatureSupported : 1;
		u8  CryptoScrambleExtCommandSupported : 1;
		u8  OverwriteExtCommandSupported : 1;
		u8  BlockEraseExtCommandSupported : 1;
		u32  UserAddressableSectors;
		u16 ObsoleteWord62;
		u16 MultiWordDMASupport : 8;
		u16 MultiWordDMAActive : 8;
		u16 AdvancedPIOModes : 8;
		u16 ReservedByte64 : 8;
		u16 MinimumMWXferCycleTime;
		u16 RecommendedMWXferCycleTime;
		u16 MinimumPIOCycleTime;
		u16 MinimumPIOCycleTimeIORDY;
		struct {
			u16 ZonedCapabilities : 2;
			u16 NonVolatileWriteCache : 1;
			u16 ExtendedUserAddressableSectorsSupported : 1;
			u16 DeviceEncryptsAllUserData : 1;
			u16 ReadZeroAfterTrimSupported : 1;
			u16 Optional28BitCommandsSupported : 1;
			u16 IEEE1667 : 1;
			u16 DownloadMicrocodeDmaSupported : 1;
			u16 SetMaxSetPasswordUnlockDmaSupported : 1;
			u16 WriteBufferDmaSupported : 1;
			u16 ReadBufferDmaSupported : 1;
			u16 DeviceConfigIdentifySetDmaSupported : 1;
			u16 LPSAERCSupported : 1;
			u16 DeterministicReadAfterTrimSupported : 1;
			u16 CFastSpecSupported : 1;
		} AdditionalSupported;
		u16 ReservedWords70[5];
		u16 QueueDepth : 5;
		u16 ReservedWord75 : 11;
		struct {
			u16 Reserved0 : 1;
			u16 SataGen1 : 1;
			u16 SataGen2 : 1;
			u16 SataGen3 : 1;
			u16 Reserved1 : 4;
			u16 NCQ : 1;
			u16 HIPM : 1;
			u16 PhyEvents : 1;
			u16 NcqUnload : 1;
			u16 NcqPriority : 1;
			u16 HostAutoPS : 1;
			u16 DeviceAutoPS : 1;
			u16 ReadLogDMA : 1;
			u16 Reserved2 : 1;
			u16 CurrentSpeed : 3;
			u16 NcqStreaming : 1;
			u16 NcqQueueMgmt : 1;
			u16 NcqReceiveSend : 1;
			u16 DEVSLPtoReducedPwrState : 1;
			u16 Reserved3 : 8;
		} SerialAtaCapabilities;
		struct {
			u16 Reserved0 : 1;
			u16 NonZeroOffsets : 1;
			u16 DmaSetupAutoActivate : 1;
			u16 DIPM : 1;
			u16 InOrderData : 1;
			u16 HardwareFeatureControl : 1;
			u16 SoftwareSettingsPreservation : 1;
			u16 NCQAutosense : 1;
			u16 DEVSLP : 1;
			u16 HybridInformation : 1;
			u16 Reserved1 : 6;
		} SerialAtaFeaturesSupported;
		struct {
			u16 Reserved0 : 1;
			u16 NonZeroOffsets : 1;
			u16 DmaSetupAutoActivate : 1;
			u16 DIPM : 1;
			u16 InOrderData : 1;
			u16 HardwareFeatureControl : 1;
			u16 SoftwareSettingsPreservation : 1;
			u16 DeviceAutoPS : 1;
			u16 DEVSLP : 1;
			u16 HybridInformation : 1;
			u16 Reserved1 : 6;
		} SerialAtaFeaturesEnabled;
		u16 MajorRevision;
		u16 MinorRevision;
		struct {
			u16 SmartCommands : 1;
			u16 SecurityMode : 1;
			u16 RemovableMediaFeature : 1;
			u16 PowerManagement : 1;
			u16 Reserved1 : 1;
			u16 WriteCache : 1;
			u16 LookAhead : 1;
			u16 ReleaseInterrupt : 1;
			u16 ServiceInterrupt : 1;
			u16 DeviceReset : 1;
			u16 HostProtectedArea : 1;
			u16 Obsolete1 : 1;
			u16 WriteBuffer : 1;
			u16 ReadBuffer : 1;
			u16 Nop : 1;
			u16 Obsolete2 : 1;
			u16 DownloadMicrocode : 1;
			u16 DmaQueued : 1;
			u16 Cfa : 1;
			u16 AdvancedPm : 1;
			u16 Msn : 1;
			u16 PowerUpInStandby : 1;
			u16 ManualPowerUp : 1;
			u16 Reserved2 : 1;
			u16 SetMax : 1;
			u16 Acoustics : 1;
			u16 BigLba : 1;
			u16 DeviceConfigOverlay : 1;
			u16 FlushCache : 1;
			u16 FlushCacheExt : 1;
			u16 WordValid83 : 2;
			u16 SmartErrorLog : 1;
			u16 SmartSelfTest : 1;
			u16 MediaSerialNumber : 1;
			u16 MediaCardPassThrough : 1;
			u16 StreamingFeature : 1;
			u16 GpLogging : 1;
			u16 WriteFua : 1;
			u16 WriteQueuedFua : 1;
			u16 WWN64Bit : 1;
			u16 URGReadStream : 1;
			u16 URGWriteStream : 1;
			u16 ReservedForTechReport : 2;
			u16 IdleWithUnloadFeature : 1;
			u16 WordValid : 2;
		} CommandSetSupport;
		struct {
			u16 SmartCommands : 1;
			u16 SecurityMode : 1;
			u16 RemovableMediaFeature : 1;
			u16 PowerManagement : 1;
			u16 Reserved1 : 1;
			u16 WriteCache : 1;
			u16 LookAhead : 1;
			u16 ReleaseInterrupt : 1;
			u16 ServiceInterrupt : 1;
			u16 DeviceReset : 1;
			u16 HostProtectedArea : 1;
			u16 Obsolete1 : 1;
			u16 WriteBuffer : 1;
			u16 ReadBuffer : 1;
			u16 Nop : 1;
			u16 Obsolete2 : 1;
			u16 DownloadMicrocode : 1;
			u16 DmaQueued : 1;
			u16 Cfa : 1;
			u16 AdvancedPm : 1;
			u16 Msn : 1;
			u16 PowerUpInStandby : 1;
			u16 ManualPowerUp : 1;
			u16 Reserved2 : 1;
			u16 SetMax : 1;
			u16 Acoustics : 1;
			u16 BigLba : 1;
			u16 DeviceConfigOverlay : 1;
			u16 FlushCache : 1;
			u16 FlushCacheExt : 1;
			u16 Resrved3 : 1;
			u16 Words119_120Valid : 1;
			u16 SmartErrorLog : 1;
			u16 SmartSelfTest : 1;
			u16 MediaSerialNumber : 1;
			u16 MediaCardPassThrough : 1;
			u16 StreamingFeature : 1;
			u16 GpLogging : 1;
			u16 WriteFua : 1;
			u16 WriteQueuedFua : 1;
			u16 WWN64Bit : 1;
			u16 URGReadStream : 1;
			u16 URGWriteStream : 1;
			u16 ReservedForTechReport : 2;
			u16 IdleWithUnloadFeature : 1;
			u16 Reserved4 : 2;
		} CommandSetActive;
		u16 UltraDMASupport : 8;
		u16 UltraDMAActive : 8;
		struct {
			u16 TimeRequired : 15;
			u16 ExtendedTimeReported : 1;
		} NormalSecurityEraseUnit;
		struct {
			u16 TimeRequired : 15;
			u16 ExtendedTimeReported : 1;
		} EnhancedSecurityEraseUnit;
		u16 CurrentAPMLevel : 8;
		u16 ReservedWord91 : 8;
		u16 MasterPasswordID;
		u16 HardwareResetResult;
		u16 CurrentAcousticValue : 8;
		u16 RecommendedAcousticValue : 8;
		u16 StreamMinRequestSize;
		u16 StreamingTransferTimeDMA;
		u16 StreamingAccessLatencyDMAPIO;
		u32  StreamingPerfGranularity;
		u32  Max48BitLBA[2];
		u16 StreamingTransferTime;
		u16 DsmCap;
		struct {
			u16 LogicalSectorsPerPhysicalSector : 4;
			u16 Reserved0 : 8;
			u16 LogicalSectorLongerThan256Words : 1;
			u16 MultipleLogicalSectorsPerPhysicalSector : 1;
			u16 Reserved1 : 2;
		} PhysicalLogicalSectorSize;
		u16 InterSeekDelay;
		u16 WorldWideName[4];
		u16 ReservedForWorldWideName128[4];
		u16 ReservedForTlcTechnicalReport;
		u16 WordsPerLogicalSector[2];
		struct {
			u16 ReservedForDrqTechnicalReport : 1;
			u16 WriteReadVerify : 1;
			u16 WriteUncorrectableExt : 1;
			u16 ReadWriteLogDmaExt : 1;
			u16 DownloadMicrocodeMode3 : 1;
			u16 FreefallControl : 1;
			u16 SenseDataReporting : 1;
			u16 ExtendedPowerConditions : 1;
			u16 Reserved0 : 6;
			u16 WordValid : 2;
		} CommandSetSupportExt;
		struct {
			u16 ReservedForDrqTechnicalReport : 1;
			u16 WriteReadVerify : 1;
			u16 WriteUncorrectableExt : 1;
			u16 ReadWriteLogDmaExt : 1;
			u16 DownloadMicrocodeMode3 : 1;
			u16 FreefallControl : 1;
			u16 SenseDataReporting : 1;
			u16 ExtendedPowerConditions : 1;
			u16 Reserved0 : 6;
			u16 Reserved1 : 2;
		} CommandSetActiveExt;
		u16 ReservedForExpandedSupportandActive[6];
		u16 MsnSupport : 2;
		u16 ReservedWord127 : 14;
		struct {
			u16 SecuritySupported : 1;
			u16 SecurityEnabled : 1;
			u16 SecurityLocked : 1;
			u16 SecurityFrozen : 1;
			u16 SecurityCountExpired : 1;
			u16 EnhancedSecurityEraseSupported : 1;
			u16 Reserved0 : 2;
			u16 SecurityLevel : 1;
			u16 Reserved1 : 7;
		} SecurityStatus;
		u16 ReservedWord129[31];
		struct {
			u16 MaximumCurrentInMA : 12;
			u16 CfaPowerMode1Disabled : 1;
			u16 CfaPowerMode1Required : 1;
			u16 Reserved0 : 1;
			u16 Word160Supported : 1;
		} CfaPowerMode1;
		u16 ReservedForCfaWord161[7];
		u16 NominalFormFactor : 4;
		u16 ReservedWord168 : 12;
		struct {
			u16 SupportsTrim : 1;
			u16 Reserved0 : 15;
		} DataSetManagementFeature;
		u16 AdditionalProductID[4];
		u16 ReservedForCfaWord174[2];
		u16 CurrentMediaSerialNumber[30];
		struct {
			u16 Supported : 1;
			u16 Reserved0 : 1;
			u16 WriteSameSuported : 1;
			u16 ErrorRecoveryControlSupported : 1;
			u16 FeatureControlSuported : 1;
			u16 DataTablesSuported : 1;
			u16 Reserved1 : 6;
			u16 VendorSpecific : 4;
		} SCTCommandTransport;
		u16 ReservedWord207[2];
		struct {
			u16 AlignmentOfLogicalWithinPhysical : 14;
			u16 Word209Supported : 1;
			u16 Reserved0 : 1;
		} BlockAlignment;
		u16 WriteReadVerifySectorCountMode3Only[2];
		u16 WriteReadVerifySectorCountMode2Only[2];
		struct {
			u16 NVCachePowerModeEnabled : 1;
			u16 Reserved0 : 3;
			u16 NVCacheFeatureSetEnabled : 1;
			u16 Reserved1 : 3;
			u16 NVCachePowerModeVersion : 4;
			u16 NVCacheFeatureSetVersion : 4;
		} NVCacheCapabilities;
		u16 NVCacheSizeLSW;
		u16 NVCacheSizeMSW;
		u16 NominalMediaRotationRate;
		u16 ReservedWord218;
		struct {
			u8 NVCacheEstimatedTimeToSpinUpInSeconds;
			u8 Reserved;
		} NVCacheOptions;
		u16 WriteReadVerifySectorCountMode : 8;
		u16 ReservedWord220 : 8;
		u16 ReservedWord221;
		struct {
			u16 MajorVersion : 12;
			u16 TransportType : 4;
		} TransportMajorVersion;
		u16 TransportMinorVersion;
		u16 ReservedWord224[6];
		u32  ExtendedNumberOfUserAddressableSectors[2];
		u16 MinBlocksPerDownloadMicrocodeMode03;
		u16 MaxBlocksPerDownloadMicrocodeMode03;
		u16 ReservedWord236[19];
		u16 Signature : 8;
		u16 CheckSum : 8;
	} __attribute__((packed));

	struct MBRPartitionEntry
	{
		struct CHSAddress {
			u8 head;
			u8 sector : 6;
			u8 cylinderHigh : 2;
			u8 cylinderLow;
		} __attribute__((packed));

		u8 bootable;
		CHSAddress chsFirst;
		u32 type : 8;
		CHSAddress chsLast;
		u32 lbaFirst;
		u32 lbaSize;
	} __attribute__((packed));

	struct MBR
	{
		u8 boot[446];
		MBRPartitionEntry partitions[4];
		u16 signature;
	} __attribute__((packed));

	bool Init();
}