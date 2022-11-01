#pragma once

#include "ReadableSourceStream.h"

#define ZIP_SEEKABLE_SOURCE_STREAM_SUPPORTS \
	ZIP_SOURCE_SEEK, \
	ZIP_SOURCE_TELL, \
	ZIP_SOURCE_SUPPORTS

namespace Zip {

	template<typename InputStream>
	class SeekableSourceStream : public ReadableSourceStream<InputStream> {
	public:

		static zip_int64_t dispatch(
			void *userdata,
			void *data,
			zip_uint64_t len,
			zip_source_cmd_t cmd
		)
		{
			zip_int64_t result = -1;

			SeekableSourceStream* src = reinterpret_cast<
				SeekableSourceStream*
			>(userdata);

			switch (cmd) {

				case ZIP_SOURCE_SEEK: // set position for reading
					result = src->seek(data, len);
				break;

				case ZIP_SOURCE_TELL: // get read position
					result = src->tell();
				break;

				default:

					result = ReadableSourceStream<InputStream>::dispatch(
						userdata,
						data,
						len,
						cmd
					);

			}

			return result;
		}

		SeekableSourceStream(InputStream inputStreamPtr) :
			ReadableSourceStream<InputStream>(inputStreamPtr),
			_inputSize(-1) // undefined size
		{}

	protected:

		virtual zip_int64_t supports()
		{
			return zip_source_make_command_bitmap(
				ZIP_READABLE_SOURCE_STREAM_SUPPORTS,
				ZIP_SEEKABLE_SOURCE_STREAM_SUPPORTS,
				-1
			);
		}

		virtual zip_int64_t stat(zip_stat_t* zipStatPtr)
		{
			zip_int64_t result = ReadableSourceStream<InputStream>::stat(zipStatPtr);

			if (result != -1) {
				
				zip_int64_t inputSize = getInputSize();

				if (inputSize < 0) {
					return -1;
				}

				zipStatPtr->size = inputSize;
				zipStatPtr->valid |= ZIP_STAT_SIZE;

			}
			
			return result;
		}

		virtual zip_int64_t close()
		{
			// set undefined size whne closing the input stream
			_inputSize = -1;
			return ReadableSourceStream<InputStream>::close();
		}

		virtual zip_int64_t seek(void *data, zip_uint64_t len)
		{
			// is the input stream ready?
			if (_inputStreamPtr->fail()) {
				_lastError.setCode(ZIP_ER_SEEK);
				return -1;
			}

			zip_int64_t oldOffset = _inputStreamPtr->tellg();
			zip_int64_t inputSize = getInputSize();

			if (inputSize < 0) {
				return -1;
			}

			// validate arguments and compute a new offset
			zip_int64_t newOffset = zip_source_seek_compute_offset(
				oldOffset,
				inputSize,
				data,
				len,
				_lastError.getInternalStructPtr()
			);

			if (newOffset < 0) {
				return -1;
			}

			// set the new offset
			_inputStreamPtr->seekg(newOffset, _inputStreamPtr->beg);

			if (_inputStreamPtr->fail()) {
				_lastError.setCode(ZIP_ER_SEEK);
				return -1;
			}

			return 0;
		}

		virtual zip_int64_t tell()
		{
			// is the input stream ready?
			if (_inputStreamPtr->fail()) {
				_lastError.setCode(ZIP_ER_TELL);
				return -1;
			}

			return _inputStreamPtr->tellg();
		}

		zip_int64_t getInputSize()
		{
			zip_int64_t result = _inputSize;

			if (result < 0) {

				// when the function is called for the first time,
				// the size if obtained from the input stream

				// is the input stream ready?
				if (_inputStreamPtr->fail()) {
					_lastError.setCode(ZIP_ER_TELL);
					return -1;
				}

				// backup the current position in the stream
				zip_int64_t prevOffset = _inputStreamPtr->tellg();

				// set the position to the end
				_inputStreamPtr->seekg(0, _inputStreamPtr->end);

				if (_inputStreamPtr->fail()) {
					_lastError.setCode(ZIP_ER_SEEK);
					return -1;
				}

				// get once again the current position that is now equal to the size
				result = _inputStreamPtr->tellg();

				// restore the previous position
				_inputStreamPtr->seekg(prevOffset, _inputStreamPtr->beg);

				if (_inputStreamPtr->fail()) {
					_lastError.setCode(ZIP_ER_SEEK);
					return -1;
				}

				_inputSize = result;

			}

			return result;
		}

	private:

		zip_int64_t _inputSize;

	};

}