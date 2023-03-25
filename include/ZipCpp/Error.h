#pragma once

#include <string>

#include <zipconf.h>
#include <zip.h>

namespace Zip {

	// this class wraps structure zip_error_t for storing information about errors

	class Error {
	public:

		Error()
		{
			// initialize zip_error structure
			zip_error_init(&_zipError);
		}

		Error(int zipErrCode)
		{
			// initialize zip_error structure, set the zip error code,
			// and set the system error code to the current errno
			zip_error_init_with_code(&_zipError, zipErrCode);
		}

		~Error()
		{
			// clean up zip_error structure
			zip_error_fini(&_zipError);
		}

		zip_error_t* getInternalStructPtr()
		{
			return &_zipError;
		}

		int getSysCode()
		{
			// get operating system error part of zip_error
			return zip_error_code_system(&_zipError);
		}

		int getZipCode()
		{
			// get libzip error part of zip_error
			return zip_error_code_zip(&_zipError);
		}

		void setCode(int zipErrCode, int sysErrCode = 0)
		{
			// set the zip error code to zipErrcode
			// and the system error code to sysErrCode
			zip_error_set(&_zipError, zipErrCode, sysErrCode);
		}

		std::string getErrMessage()
		{
			// get error message string
			return std::string(zip_error_strerror(&_zipError));
		}

		zip_int64_t convToSourceErr(void *data, zip_uint64_t len)
		{
			// convert zip_error to return value suitable for ZIP_SOURCE_ERROR
			return zip_error_to_data(&_zipError, data, len);
		}

	private:
		zip_error_t _zipError;
	};

}