// SPDX-License-Identifier: GPL-2.0-only

#include <dlfcn.h>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <atomic>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <utility>
#include <vector>

#include "wintypes.h"
#include "IB25Decoder.h"

static std::atomic_bool has_stop_signal{false};

static void signal_handler(int signo)
{
	has_stop_signal = true;
}

static void usage(const std::string& argv)
{
	std::cerr << "usage: " << argv << " [-b B25Decoder.so] input output\n";
	std::exit(0);
}

int main(int argc, char* argv[])
{
	void* hmodule_ = nullptr;
	IB25Decoder* b25dec_ = nullptr;
	std::string b25declib_ = "./B25Decoder.so";
	auto file = std::make_pair<std::string, std::string>("", "");
	auto fp = std::make_pair<std::FILE*, std::FILE*>(nullptr, nullptr);

	while (true)
	{
		auto c = getopt(argc, argv, "b:");
		if (c == -1) break;
		switch (c)
		{
		case 'b':
			b25declib_ = optarg;
			break;
		case 'h':
		default:
			usage(argv[0]);
		}
	}

	argc -= optind;
	if (argc != 2)
	{
		usage(argv[0]);
	}

	file.first = argv[optind];		// input
	file.second = argv[optind+1];	// output

	try
	{
		// モジュールロード
		hmodule_ = dlopen(b25declib_.c_str(), RTLD_LAZY);
		if (!hmodule_)
		{
			std::ostringstream os;
			os << "dlopen error: " << dlerror();
			throw std::runtime_error(os.str());
		}

		auto *(*f)() = (IB25Decoder *(*)())dlsym(hmodule_, "CreateB25Decoder");
		auto error = dlerror();
		if (error)
		{
			std::ostringstream os;
			os << "dlsym CreateB25Decoder error: " << error;
			throw std::runtime_error(os.str());
		}

		b25dec_ = f();
		auto code = b25dec_->Initialize();
		if (!code)
		{
			throw std::runtime_error("B25Decoder Initialize error");
		}

		if (file.first == "-")
		{
			fp.first = stdin;
		}
		else
		{
			fp.first = std::fopen(file.first.c_str(), "rb");
			if (!fp.first)
			{
				std::ostringstream os;
				os << "fail to open input file: " << file.first;
				throw std::runtime_error(os.str());
			}
		}

		if (file.second == "-")
		{
			fp.second = stdout;
		}
		else
		{
			fp.second = std::fopen(file.second.c_str(), "wb");
			if (!fp.second)
			{
				std::ostringstream os;
				os << "fail to output file: " << file.second;
				throw std::runtime_error(os.str());
			}
		}

		struct sigaction sa = {};
		sa.sa_handler = signal_handler;
		sigemptyset(&sa.sa_mask);
		sigaction(SIGINT, &sa, nullptr);
		sigaction(SIGTERM, &sa, nullptr);
		sigaction(SIGPIPE, &sa, nullptr);

		uint8_t* dstbuf = nullptr;
		uint32_t dstsize = 0;
		std::vector<uint8_t> srcbuf(188*512);
		while (!has_stop_signal)
		{
			auto srcsize = std::fread(srcbuf.data(), sizeof(uint8_t), srcbuf.size(), fp.first);
			std::clog << "read size = " << srcsize << '\n';
			if (srcsize == 0) break;

			auto code = b25dec_->Decode(srcbuf.data(), srcsize, &dstbuf, &dstsize);
			if (!code)
			{
				std::ostringstream os;
				os << "b25 decode error: code = " << code;
				std::clog << os.str()<< '\n';
			}
			else
			{
				std::clog << "dst size = " << dstsize << '\n';
				if (dstsize > 0)
				{
					auto writesize = std::fwrite(dstbuf, sizeof(uint8_t), dstsize, fp.second);
					std::clog << "write size = " << writesize << '\n';
				}
			}
		}

		code = b25dec_->Flush(&dstbuf, &dstsize);
		if (!code)
		{
			std::ostringstream os;
			os << "b25 flush error: code = " << code;
			std::clog << os.str()<< '\n';
		}
		else
		{
			std::clog << "flush size = " << dstsize << '\n';
			if (dstsize > 0)
			{
				auto writesize = std::fwrite(dstbuf, sizeof(uint8_t), dstsize, fp.second);
				std::clog << "write size = " << writesize << '\n';
			}
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

	if (fp.first)
	{
		std::fclose(fp.first);
	}

	if (fp.second)
	{
		std::fclose(fp.second);
	}

	if (b25dec_)
	{
		b25dec_->Release();
	}

	if (hmodule_)
	{
		dlclose(hmodule_);
	}

	return 0;
}
