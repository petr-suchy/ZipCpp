#pragma once

#include "SourceStream.h"
#include "ZipFileHandle.h"

#define ZIP_READABLE_SOURCE_STREAM_SUPPORTS \
	ZIP_SOURCE_OPEN, \
	ZIP_SOURCE_READ, \
	ZIP_SOURCE_CLOSE, \
	ZIP_SOURCE_STAT, \
	ZIP_SOURCE_ERROR, \
	ZIP_SOURCE_FREE

namespace Zip {

	template<typename InputStream>
	class ReadableSourceStream : public SourceStream {
	public:

		static zip_int64_t dispatch(
			void *userdata,
			void *data,
			zip_uint64_t len,
			zip_source_cmd_t cmd
		)
		{
			zip_int64_t result = -1;

			ReadableSourceStream* src = reinterpret_cast<
				ReadableSourceStream*
			>(userdata);

			switch (cmd) {

				case ZIP_SOURCE_SUPPORTS: // check whether source supports command
					result = src->supports();
				break;

				case ZIP_SOURCE_OPEN: // prepare for reading
					result = src->open();
				break;

				case ZIP_SOURCE_READ: // read data
					result = src->read(
						reinterpret_cast<char*>(data),
						len
					);
				break;

				case ZIP_SOURCE_CLOSE: // reading is done
					result = src->close();
				break;

				case ZIP_SOURCE_STAT: // get meta information
					result = src->stat(
						reinterpret_cast<zip_stat_t*>(data)
					);
				break;

				case ZIP_SOURCE_ERROR: // get error information
					result = src->error(data, len);
				break;

				case ZIP_SOURCE_FREE: // cleanup and free resources
					result = src->free();
				break;

				default: // invalid command
					src->_lastError.setCode(ZIP_ER_INVAL);

			}

			return result;
		}

		ReadableSourceStream(InputStream inputStreamPtr) :
			_inputStreamPtr(inputStreamPtr)
		{}

	protected:

		// generic pointer to the input stream
		InputStream _inputStreamPtr;
		// stores information about the last zip error
		Error _lastError;

		virtual zip_int64_t supports()
		{
			return zip_source_make_command_bitmap(
				ZIP_READABLE_SOURCE_STREAM_SUPPORTS,
				-1
			);
		}

		virtual zip_int64_t stat(zip_stat_t* zipStatPtr)
		{
			zip_stat_init(zipStatPtr);
			return 0;
		}

		virtual zip_int64_t open()
		{
			return 0;
		}

		virtual zip_int64_t read(char* buff, zip_uint64_t len)
		{
			_inputStreamPtr->read(buff, (size_t) len);

			if (
				_inputStreamPtr->fail() &&
				!_inputStreamPtr->eof()
			) {

				_lastError.setCode(ZIP_ER_READ);

				return -1;
			}

			return _inputStreamPtr->gcount();
		}

		virtual zip_int64_t close()
		{
			return 0;
		}

		virtual zip_int64_t error(void *data, zip_uint64_t len)
		{
			return _lastError.convToSourceErr(data, len);
		}

		virtual zip_int64_t free()
		{
			return 0;
		}

	};

}