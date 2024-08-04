// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <syslog.h>

#include <cstddef>
#include <cstdint>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <mutex>

#include "wintypes.h"
#include "IB25Decoder.h"
#include "arib_std_b25.h"

#define ARIB_STD_B25_ERROR_INVALID_PARAM          -1
#define ARIB_STD_B25_ERROR_NO_ENOUGH_MEMORY       -2
#define ARIB_STD_B25_ERROR_NON_TS_INPUT_STREAM    -3
#define ARIB_STD_B25_ERROR_NO_PAT_IN_HEAD_16M     -4
#define ARIB_STD_B25_ERROR_NO_PMT_IN_HEAD_32M     -5
#define ARIB_STD_B25_ERROR_NO_ECM_IN_HEAD_32M     -6
#define ARIB_STD_B25_ERROR_EMPTY_B_CAS_CARD       -7
#define ARIB_STD_B25_ERROR_INVALID_B_CAS_STATUS   -8
#define ARIB_STD_B25_ERROR_ECM_PROC_FAILURE       -9
#define ARIB_STD_B25_ERROR_DECRYPT_FAILURE       -10
#define ARIB_STD_B25_ERROR_PAT_PARSE_FAILURE     -11
#define ARIB_STD_B25_ERROR_PMT_PARSE_FAILURE     -12
#define ARIB_STD_B25_ERROR_ECM_PARSE_FAILURE     -13
#define ARIB_STD_B25_ERROR_CAT_PARSE_FAILURE     -14
#define ARIB_STD_B25_ERROR_EMM_PARSE_FAILURE     -15
#define ARIB_STD_B25_ERROR_EMM_PROC_FAILURE      -16

class CB25Decoder : public IB25Decoder2
{
public:
	// CB25Decoder
	CB25Decoder(void);
	virtual ~CB25Decoder(void);
	void Release(void);

	// IB25Decoder
	virtual const BOOL Initialize(DWORD dwRound = 4);
	virtual const BOOL Decode(BYTE *pSrcBuf, const DWORD dwSrcSize, BYTE **ppDstBuf, DWORD *pdwDstSize);
	virtual const BOOL Flush(BYTE **ppDstBuf, DWORD *pdwDstSize);
	virtual const BOOL Reset(void);

	// IB25Decoder2
	virtual void DiscardNullPacket(const bool bEnable = true);
	virtual void DiscardScramblePacket(const bool bEnable = true);	// 未実装
	virtual void EnableEmmProcess(const bool bEnable = true);

	// 以下は未実装
	// 処理パケットの統計データはゼロ返却
	virtual const DWORD GetDescramblingState(const WORD wProgramID);
	virtual void ResetStatistics(void);
	virtual const DWORD GetPacketStride(void);
	virtual const DWORD GetInputPacketNum(const WORD wPID = TS_INVALID_PID);
	virtual const DWORD GetOutputPacketNum(const WORD wPID = TS_INVALID_PID);
	virtual const DWORD GetSyncErrNum(void);
	virtual const DWORD GetFormatErrNum(void);
	virtual const DWORD GetTransportErrNum(void);
	virtual const DWORD GetContinuityErrNum(const WORD wPID = TS_INVALID_PID);
	virtual const DWORD GetScramblePacketNum(const WORD wPID = TS_INVALID_PID);
	virtual const DWORD GetEcmProcessNum(void);
	virtual const DWORD GetEmmProcessNum(void);

	static CB25Decoder* GetInstance();

private:
	static CB25Decoder* instance_;
	static std::mutex instance_mutex_;

	std::mutex mutex_;
	ARIB_STD_B25 *b25_ = nullptr;
	B_CAS_CARD *bcas_ = nullptr;
};
