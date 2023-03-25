#pragma once

#include "SeekableSourceStream.h"

#define ZIP_WRITABLE_SOURCE_STREAM_SUPPORTS \
	ZIP_SOURCE_BEGIN_WRITE, \
	ZIP_SOURCE_COMMIT_WRITE, \
	ZIP_SOURCE_ROLLBACK_WRITE, \
	ZIP_SOURCE_WRITE, \
	ZIP_SOURCE_SEEK_WRITE, \
	ZIP_SOURCE_TELL_WRITE, \
	ZIP_SOURCE_REMOVE

namespace Zip {

	template<typename InputStream, typename OutputStream>
	class WritableSourceStream : public SeekableSourceStream<InputStream> {
	public:

		static zip_int64_t dispatch(
			void *userdata,
			void *data,
			zip_uint64_t len,
			zip_source_cmd_t cmd
		)
		{
			zip_int64_t result = -1;

			WritableSourceStream* src = reinterpret_cast<
				WritableSourceStream*
			>(userdata);

			switch (cmd) {

				case ZIP_SOURCE_BEGIN_WRITE: // prepare for writing
					result = src->beginWrite();
				break;

				case ZIP_SOURCE_COMMIT_WRITE: // writing is done
					result = src->commitWrite();
				break;

				case ZIP_SOURCE_ROLLBACK_WRITE: // discard written changes
					result = src->rollbackWrite();
				break;

				case ZIP_SOURCE_WRITE: // write data
					result = src->write(
						reinterpret_cast<char*>(data),
						len
					);
				break;

				case ZIP_SOURCE_SEEK_WRITE: // set position for writing
					result = src->seekWrite(data, len);
				break;

				case ZIP_SOURCE_TELL_WRITE: // get write position
					result = src->tellWrite();
				break;

				case ZIP_SOURCE_REMOVE: // remove file
					result = src->remove();
				break;

				default:

					result = SeekableSourceStream<InputStream>::dispatch(
						userdata,
						data,
						len,
						cmd
					);

			}

			return result;
		}

		WritableSourceStream(InputStream inputStreamPtr, OutputStream outputStreamPtr) :
			SeekableSourceStream<InputStream>(inputStreamPtr),
			_outputStreamPtr(outputStreamPtr),
			_outputSize(0)
		{}

	protected:

		OutputStream _outputStreamPtr;
		zip_int64_t _outputSize;

		virtual zip_int64_t supports()
		{
			return zip_source_make_command_bitmap(
				ZIP_READABLE_SOURCE_STREAM_SUPPORTS,
				ZIP_SEEKABLE_SOURCE_STREAM_SUPPORTS,
				ZIP_WRITABLE_SOURCE_STREAM_SUPPORTS,
				-1
			);
		}

		virtual zip_int64_t beginWrite()
		{
			_outputSize = 0;
			return 0;
		}

		virtual zip_int64_t commitWrite()
		{
			return 0;
		}

		virtual zip_int64_t rollbackWrite()
		{
			return 0;
		}

		virtual zip_int64_t write(const char *data, zip_uint64_t len)
		{
			if (_outputStreamPtr->fail()) {
				lastError().setCode(ZIP_ER_WRITE);
				return -1;
			}

			zip_int64_t oldWritePos = _outputStreamPtr->tellp();

			_outputStreamPtr->write(data, len);

			if (_outputStreamPtr->fail()) {
				lastError().setCode(ZIP_ER_WRITE);
				return -1;
			}

			zip_int64_t newWritePos = _outputStreamPtr->tellp();
			zip_int64_t written = newWritePos - oldWritePos;

			_outputSize += written;

			return written;
		}

		virtual zip_int64_t seekWrite(void *data, zip_uint64_t len)
		{
			// is the output stream ready?
			if (_outputStreamPtr->fail()) {
				lastError().setCode(ZIP_ER_SEEK);
				return -1;
			}

			zip_int64_t oldOffset = _outputStreamPtr->tellp();

			// validate arguments and compute a new offset
			zip_int64_t newOffset = zip_source_seek_compute_offset(
				oldOffset,
				_outputSize,
				data,
				len,
				lastError().getInternalStructPtr()
			);

			if (newOffset < 0) {
				return -1;
			}

			_outputStreamPtr->seekp(newOffset, _outputStreamPtr->beg);

			if (_outputStreamPtr->fail()) {
				lastError().setCode(ZIP_ER_SEEK);
				return -1;
			}

			return 0;
		}

		virtual zip_int64_t tellWrite()
		{
			// is the output stream ready?
			if (_outputStreamPtr->fail()) {
				lastError().setCode(ZIP_ER_TELL);
				return -1;
			}

			return _outputStreamPtr->tellp();
		}

		virtual zip_int64_t remove()
		{
			return 0;
		}

	private:

		Error& lastError()
		{
			return ReadableSourceStream<InputStream>::_lastError;
		}

	};

}