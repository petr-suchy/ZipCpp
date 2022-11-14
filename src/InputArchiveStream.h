#pragma once

#include "Archive.h"
#include "SeekableSourceStream.h"

namespace Zip {

	template<typename InputStream>
	class InputArchiveStream : public Archive {
	public:

		InputArchiveStream(InputStream inputStreamPtr) :
			Archive(ZIP_RDONLY),
			_inputStreamPtr(inputStreamPtr)
		{}

	protected:

		typedef SeekableSourceStream<
			InputStream
		> SeekableSource;

		virtual ZipHandle::SharedPtr _openArchive(int flags)
		{
			auto seekableSource = std::make_shared<
				SeekableSource
			>(_inputStreamPtr);

			_inputStreamPtr = nullptr;

			Error error;

			zip_source_t* zipSrcPtr = zip_source_function_create(
				&SeekableSource::dispatch,
				seekableSource.get(),
				error.getInternalStructPtr()
			);

			if (!zipSrcPtr) {

				throw std::runtime_error(
					std::string("cannot create a zip archive source -> ")
						+ error.getErrMessage()
				);

			}

			zip_t* newZipPtr = zip_open_from_source(
				zipSrcPtr,
				flags,
				error.getInternalStructPtr()
			);

			if (!newZipPtr) {

				zip_source_free(zipSrcPtr);

				throw std::runtime_error(
					std::string("cannot open a zip archive from the data source -> ")
						+ error.getErrMessage()
				);

			}

			return std::make_shared<ZipHandle>(newZipPtr, seekableSource);
		}

	private:

		InputStream _inputStreamPtr;

	};

	// create an instance of input archive stream
	template<typename InputStream>
	static InputArchiveStream<
		InputStream
	> MakeInputArchive(InputStream is)
	{
		return InputArchiveStream<InputStream>(is);
	}

	// create a shared pointer to input archive stream
	template<typename InputStream>
	static std::shared_ptr<
		InputArchiveStream<InputStream>
	> MakeSharedInputArchive(InputStream is)
	{
		return std::make_shared<
			InputArchiveStream<InputStream>
		>(is);
	}

}