#pragma once

#include "ZipHandle.h"
#include "EntryStreamImpl.h"

namespace Zip {

	class ReadableEntryStream {
	public:

		ReadableEntryStream(
			EntryStreamImpl::SharedPtr impl
		) :
			_impl(impl),
			_eof(false),
			_fail(false),
			_nread(0)
		{}

		bool eof() { return _eof; }
		bool fail() { return _fail; }
		bool good() { return !_eof && !_fail; }
		size_t gcount() { return _nread; }

		ZipFileHandle::WeakPtr getFileHandle()
		{
			return _impl->getFileHandle();
		}

		void clear()
		{
			_eof = false;
			_fail = false;
		}

		void read(char* buf, size_t nbytes)
		{
			_nread = 0;

			if (_eof || _fail) {
				return;
			}

			auto nread = _impl->read(buf, nbytes);

			if (nread < 0) {
				_fail = true;
				return;
			}

			_nread = (size_t) nread;

			if (_nread == 0) {
				_eof = true;
			}

		}

	private:

		EntryStreamImpl::SharedPtr _impl;

		bool _eof;
		bool _fail;
		size_t _nread;

	};

}