#pragma once

#include "ZipHandle.h"

#include <functional>

namespace Zip {

	class ReadableEntryStream {
	public:

		typedef std::shared_ptr<ReadableEntryStream> SharedPtr;
		typedef std::weak_ptr<ReadableEntryStream> WeakPtr;
	
		typedef std::function<void(ZipFileHandle::WeakPtr)> Deleter;

		ReadableEntryStream(
			ZipFileHandle::WeakPtr fileHandle,
			Deleter deleter
		) :
			_fileHandle(fileHandle),
			_deleter(deleter),
			_eof(false),
			_fail(false),
			_nread(0)
		{}

		~ReadableEntryStream()
		{
			_deleter(_fileHandle);
		}

		bool eof() { return _eof; }
		bool fail() { return _fail; }
		bool good() { return !_eof && !_fail; }
		size_t gcount() { return _nread; }

		ZipFileHandle::WeakPtr getFileHandle()
		{
			return _fileHandle;
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

			auto tempFileHandle = _fileHandle.lock();

			if (!tempFileHandle) {
				// archive has been destroyed
				_fail = true;
				return;
			}

			auto nread = tempFileHandle->read(buf, nbytes);

			if (nread < 0) {
				// failed to read data
				_fail = true;
				return;
			}

			_nread = (size_t) nread;

			if (_nread == 0) {
				// end of file
				_eof = true;
			}

		}

	private:

		ZipFileHandle::WeakPtr _fileHandle;
		Deleter _deleter;

		bool _eof;
		bool _fail;
		size_t _nread;

	};

}