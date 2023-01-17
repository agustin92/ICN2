
#ifndef BkComDLL_h
#define BkComDLL_h


#ifdef BKCOMDLL_EXPORTS
#define BKCOMDLL_API __declspec(dllexport)
#else
#define BKCOMDLL_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C"
{
#endif


// einige Typen
struct BkComData
{
	HANDLE hCommPort;
	long nBaudrate;
	long nPortNum;
	long nTimeout;
	long nBKxTyp;
};
typedef struct BkComData BKCOMDLL_API BkComData;

BKCOMDLL_API typedef enum
{
	ComErrNotImplemented = -1,
	ComErrNo = 0,
	ComErrTimeout1 = 1,
	ComErrTimeout2 = 2,
	ComErrCRC = 3,
	ComErrTargetNr = 4,
	ComErrTableNr = 5,
	ComErrOffset = 6,
	ComErrDataLength = 7,
	ComErrMultipoint = 8,
	ComErrDataBuff = 9,
	ComErrStartPattern = 10,
	ComErrSendTel = 11,
	ComErrIdent = 12,
	ComErrRegResponse = 13,
	ComErrHexFileFault = 14,
	ComErrChecksumFault = 15,
	ComErrAddressMappingFault = 16,
	ComErrFaultInByte = 17,
	ComErrWrongAck = 18,
	ComErrWrongBtlAck = 19,
	ComErrReadBackFault = 20
} BkComErr;

BKCOMDLL_API typedef enum
{
	Baud_9600 = 9600,
	Baud_19200 = 19200,
	Baud_38400 = 38400,
	Baud_57600 = 57600

} BkComBaud;

BKCOMDLL_API typedef enum
{
	BKxType_RS485 = 1,
	BKxType_RS232 = 2
} BkComBKxType;


// Funktionen
BKCOMDLL_API long OpenBkComPort(BkComData *pAnBKCom);
BKCOMDLL_API HANDLE OpenBkComPortEx(long nBaudrate, long nCommPort, long nTimeout, long nBKxTyp);
BKCOMDLL_API long BK8xProcSyncReadReq(BkComData *pAnBKCom, long lMultiPoint, long FAR* lStatus, long FAR* cwRecLength, long FAR* lpRecBuff);
BKCOMDLL_API long BK8xProcSyncReadWriteReq(BkComData *pAnBKCom, long lMultiPoint, long FAR* lStatus, long cwSendLength, long FAR* lpSendBuff, long FAR* cwRecLength, long FAR* lpRecBuff);
BKCOMDLL_API void CloseBkComPort(BkComData *pAnBkCom); 

#ifdef __cplusplus
} // extern "C"
#endif


#endif
