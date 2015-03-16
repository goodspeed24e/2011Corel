#if !defined(__IVI_DIRECTSHOW_DEMUX_ENUMERATIONS_H__)
#define __IVI_DIRECTSHOW_DEMUX_ENUMERATIONS_H__

enum EDEMUX_FILTER_CONFIGURATION_COMMAND
{
    E_DFCC_OPERATION_MODE,
    E_DFCC_CHANNEL_SWITCH
}; // EDEMUX_FILTER_CONFIGURATION_COMMAND

enum EDEMUX_FILTER_OPERATION_MODE
{
    E_DFOM_AUTO_MODE,
    E_DFOM_MANUAL_MODE
}; // EDEMUX_FILTER_OPERATION_MODE

enum ECHANNEL_SWITCH_PROTECTION_COMMAND
{
    E_CSPC_BEGIN_CHANNEL_SWITCH_PROTECTION,
    E_CSPC_END_CHANNEL_SWITCH_PROTECTION
}; // ECHANNEL_SWITCH_COMMAND

enum EPROGRAM_SPECIFIC_INFO_COMMAND
{
    E_PSIC_PROGRAM_STREAM_INFO,			// used to retrieve program stream info
    E_PSIC_TRANSPORT_STREAM_INFO		// used to retrieve transport stream info
}; // EPROGRAM_SPECIFIC_INFO_COMMAND

enum EMEDIA_DATA_OUTPUT_FORMAT
{
    E_MDOF_TS_TRANSPORT_PACKET               = 0x0000, // passing through TS packets according to its pid
    E_MDOF_TS_ELEMENTARY_STREAM              = 0x0001,
    E_MDOF_TS_MPEG2_PSI                      = 0x0002,
    E_MDOF_TS_TRANSPORT_PAYLOAD              = 0x0003, // output TS payload. That's, no pealing off any header/data in the payload
    E_MDOF_TS_TRANSPORT_STREAM               = 0x0004, // passing through the intact transport stream received.
    E_MDOF_TS_ELEMENTARY_STREAM_IN_SL_PACKET = 0x0005,
    E_MDOF_TS_PCR_PID_STREAM                 = 0x0006, // Internal usage.Try to get PCR value in specific ID.
    E_MDOF_TS_PSI_PARSER                     = 0x0007,// Internal usage .Try to parse psi section data  in specific ID.

    E_MDOF_PS_PROGRAM_STREAM_MAP            = 0x0010,                 
    E_MDOF_PS_PROGRAM_ELEMENTARY_STREAM     = 0x0011,
    E_MDOF_PS_PROGRAM_DIRECTORY_PES_PACKET  = 0x0012,
    E_MDOF_PS_PROGRAM_PACK_HEADER           = 0x0013,
    E_MDOF_PS_PROGRAM_PES_STREAM            = 0x0014,
    E_MDOF_PS_PROGRAM_SYSTEM_HEADER         = 0x0015,
    E_MDOF_PS_PROGRAM_PACK                  = 0x0016,

    E_MDOF_NONE = 0xFFFF
};


#endif
