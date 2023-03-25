#pragma once

#include <zipconf.h>
#include <zip.h>

#include <memory>

namespace Zip {

	class ZipFileHandle {
	public:

		typedef zip_file_t* RawPtr;
		typedef std::shared_ptr<ZipFileHandle> SharedPtr;
		typedef std::weak_ptr<ZipFileHandle> WeakPtr;

		ZipFileHandle(RawPtr zipFilePtr) :
			_zipFilePtr(zipFilePtr)
		{}

		~ZipFileHandle()
		{
			if (_zipFilePtr) {
				zip_fclose(_zipFilePtr);
			}
		}

		RawPtr get()
		{
			return _zipFilePtr;
		}

		zip_int64_t read(void *buf, zip_uint64_t nbytes)
		{
			return zip_fread(
				_zipFilePtr,
				buf,
				nbytes
			);
		}

	private:

		RawPtr _zipFilePtr;

	};

}