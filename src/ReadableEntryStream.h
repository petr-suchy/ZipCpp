#pragma once

#include "zipconf.h"
#include "zip.h"

namespace Zip {

	class ReadableEntryStream {
	public:

		ReadableEntryStream(zip_file_t* zipFilePtr) :
			_zipFilePtr(zipFilePtr),
			_bytesRead(0),
			_failFlag(false)
		{}

		~ReadableEntryStream()
		{
			zip_fclose(_zipFilePtr);
		}

		size_t gcount()
		{
			return _bytesRead;
		}

		bool fail()
		{
			return _failFlag;
		}

		void read(char* readBuffer, size_t bytesToRead)
		{
			_bytesRead = 0;

			if (_failFlag) {
				return;
			}

			zip_int64_t bytesRead = zip_fread(
				_zipFilePtr,
				readBuffer,
				bytesToRead
			);

			if (bytesRead < 0) {
				_failFlag = true;
			}
			else {
				_bytesRead = (size_t) bytesRead;
			}

		}

	private:

		zip_file_t* _zipFilePtr;
		size_t _bytesRead;
		bool _failFlag;

	};

}