#pragma once

#include "ZipFileHandle.h"

#include <stdexcept>
#include <functional>

namespace Zip {

	class EntryStreamImpl {
	public:

		typedef std::shared_ptr<EntryStreamImpl> SharedPtr;
		typedef std::weak_ptr<EntryStreamImpl> WeakPtr;
	
		typedef std::function<void(ZipFileHandle::WeakPtr)> Deleter;

		EntryStreamImpl(
			ZipFileHandle::WeakPtr fileHandle,
			Deleter deleter
		) :
			_fileHandle(fileHandle),
			_deleter(deleter)
		{}

		~EntryStreamImpl()
		{
			_deleter(_fileHandle);
		}

		ZipFileHandle::WeakPtr getFileHandle()
		{
			return _fileHandle;
		}

		zip_int64_t read(void *buf, zip_uint64_t nbytes)
		{
			auto tempFileHandle = _fileHandle.lock();

			if (!tempFileHandle) {
				// archive has been destroyed
				return -1;
			}

			return tempFileHandle->read(buf, nbytes);
		}

	private:

		ZipFileHandle::WeakPtr _fileHandle;
		Deleter _deleter;

	};

}