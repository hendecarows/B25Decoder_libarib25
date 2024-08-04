// SPDX-License-Identifier: GPL-2.0-only

#include <cstddef>
#include <cstdint>
#include "wintypes.h"
#include "B25Decoder.h"

extern "C" IB25Decoder* CreateB25Decoder()
{
	return dynamic_cast<IB25Decoder*>(CB25Decoder::GetInstance());
}

extern "C" IB25Decoder2* CreateB25Decoder2()
{
	return dynamic_cast<IB25Decoder2*>(CB25Decoder::GetInstance());
}

CB25Decoder* CB25Decoder::instance_ = nullptr;
std::mutex CB25Decoder::instance_mutex_;

CB25Decoder::CB25Decoder(void)
{}

CB25Decoder::~CB25Decoder(void)
{}

void CB25Decoder::Release(void)
{
	std::lock_guard<std::mutex> lock(instance_mutex_);

	{
		std::lock_guard<std::mutex> lock(mutex_);

		if (b25_)
		{
			b25_->release(b25_);
			b25_ = nullptr;
		}

		if (bcas_)
		{
			bcas_->release(bcas_);
			bcas_ = nullptr;
		}

		closelog();
	}

	if (instance_)
	{
		delete instance_;
		instance_ = nullptr;
	}
}

const BOOL CB25Decoder::Initialize(DWORD dwRound)
{
	std::lock_guard<std::mutex> lock(mutex_);

	if (b25_)
		return TRUE;

	auto code = 0;
	openlog("B25Decoder", LOG_PERROR | LOG_PID, LOG_USER);

	b25_ = create_arib_std_b25();
	if (!b25_)
	{
		syslog(LOG_ERR, "B25Decoder::create_arib_std_b25() error");
		goto LAST;
	}

	b25_->set_multi2_round(b25_, dwRound);
	b25_->set_strip(b25_, 0);
	b25_->set_emm_proc(b25_, 0);

	bcas_ = create_b_cas_card();
	if (!bcas_)
	{
		syslog(LOG_ERR, "B25Decoder::create_b_cas_card() error");
		goto LAST;
	}

	code = bcas_->init(bcas_);
	if (code < 0)
	{
		syslog(LOG_ERR, "B25Decoder::init() error code=%d", code);
		goto LAST;
	}

	code = b25_->set_b_cas_card(b25_, bcas_);
	if (code < 0)
	{
		syslog(LOG_ERR, "B25Decoder::set_b_cas_card() error code=%d", code);
		goto LAST;
	}

	return TRUE;

LAST:
	if (b25_)
	{
		b25_->release(b25_);
		b25_ = nullptr;
	}

	if (bcas_)
	{
		bcas_->release(bcas_);
		bcas_ = nullptr;
	}

	return FALSE;
}

const BOOL CB25Decoder::Decode(BYTE *pSrcBuf, const DWORD dwSrcSize, BYTE **ppDstBuf, DWORD *pdwDstSize)
{
	std::lock_guard<std::mutex> lock(mutex_);

	if (!pSrcBuf || !dwSrcSize || !ppDstBuf || !pdwDstSize)
		return FALSE;

	if (!b25_)
		return FALSE;

	ARIB_STD_B25_BUFFER sbuf;
	sbuf.data = pSrcBuf;
	sbuf.size = dwSrcSize;
	auto code = b25_->put(b25_, &sbuf);
	if (code < 0)
	{
		// EDCBで「録画開始処理に失敗しました」のエラーを避けるために、エラー時にはresetし初期化する。
		// これは、転送開始直後のエラーパケット(transport error indicator=1)を多量に含むTSを
		// putした場合に、code=-3(ARIB_STD_B25_ERROR_NON_TS_INPUT_STREAM)が発生し、
		// TSが出てこなくなるため。
		// code=-3以外のエラーを確認できておらず、それ以外の場合にもresetするのが良いか判断できないが、
		// 全て同じ扱いとする。

		syslog(LOG_WARNING, "B25Decoder::put() error code=%d", code);
		syslog(LOG_WARNING, "B25Decoder::reset()");
		b25_->reset(b25_);
		return FALSE;
	}

	ARIB_STD_B25_BUFFER dbuf;
	code = b25_->get(b25_, &dbuf);
	if (code < 0)
	{
		syslog(LOG_WARNING, "B25Decoder::get() error code=%d", code);
		return FALSE;
	}

	*ppDstBuf = dbuf.data;
	*pdwDstSize = dbuf.size;

	// syslog(LOG_DEBUG, "B25Decoder::dwSrcSize=%u", dwSrcSize);
	// syslog(LOG_DEBUG, "B25Decoder::dwDstSize=%u", *pdwDstSize);

	return TRUE;
}

const BOOL CB25Decoder::Flush(BYTE **ppDstBuf, DWORD *pdwDstSize)
{
	std::lock_guard<std::mutex> lock(mutex_);

	if (!b25_)
		return FALSE;

	auto code = b25_->flush(b25_);
	if (code < 0)
	{
		syslog(LOG_ERR, "B25Decoder::flush() error code=%d", code);
		return FALSE;
	}

	ARIB_STD_B25_BUFFER buf;
	b25_->get(b25_, &buf);
	*ppDstBuf = buf.data;
	*pdwDstSize = buf.size;

	return TRUE;
}

const BOOL CB25Decoder::Reset(void)
{
	std::lock_guard<std::mutex> lock(mutex_);

	if (!b25_)
		return FALSE;

	auto code = b25_->reset(b25_);
	if (code < 0)
	{
		syslog(LOG_ERR, "B25Decoder::reset() error code=%d", code);
		return FALSE;
	}

	return TRUE;
}

void CB25Decoder::DiscardNullPacket(const bool bEnable)
{
	b25_->set_strip(b25_, bEnable);
}

void CB25Decoder::DiscardScramblePacket(const bool bEnable)
{
}

void CB25Decoder::EnableEmmProcess(const bool bEnable)
{
	b25_->set_emm_proc(b25_, bEnable);
}

const DWORD CB25Decoder::GetDescramblingState(const WORD wProgramID)
{
	return 0;
}

void CB25Decoder::ResetStatistics(void)
{
}

const DWORD CB25Decoder::GetPacketStride(void)
{
	return 0;
}

const DWORD CB25Decoder::GetInputPacketNum(const WORD wPID)
{
	return 0;
}

const DWORD CB25Decoder::GetOutputPacketNum(const WORD wPID)
{
	return 0;
}

const DWORD CB25Decoder::GetSyncErrNum(void)
{
	return 0;
}

const DWORD CB25Decoder::GetFormatErrNum(void)
{
	return 0;
}

const DWORD CB25Decoder::GetTransportErrNum(void)
{
	return 0;
}

const DWORD CB25Decoder::GetContinuityErrNum(const WORD wPID)
{
	return 0;
}

const DWORD CB25Decoder::GetScramblePacketNum(const WORD wPID)
{
	return 0;
}

const DWORD CB25Decoder::GetEcmProcessNum(void)
{
	return 0;
}

const DWORD CB25Decoder::GetEmmProcessNum(void)
{
	return 0;
}

CB25Decoder* CB25Decoder::GetInstance()
{
	std::lock_guard<std::mutex> lock(CB25Decoder::instance_mutex_);

	if (instance_ == nullptr)
	{
		instance_ = new CB25Decoder();
		return instance_;
	}

	return nullptr;
}
