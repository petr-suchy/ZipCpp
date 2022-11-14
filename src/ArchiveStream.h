#pragma once

#include "Archive.h"
#include "WritableSourceStream.h"

namespace Zip {

	template<typename InputStream, typename OutputStream>
	class ArchiveStream : public Archive {
	public:

		ArchiveStream(InputStream inputStreamPtr, OutputStream outputStreamPtr) :
			Archive(ZIP_CREATE),
			_inputStreamPtr(inputStreamPtr),
			_outputStreamPtr(outputStreamPtr)
		{}

	protected:

		typedef WritableSourceStream<
			InputStream,
			OutputStream
		> WritableSource;

		virtual ZipHandle::SharedPtr _openArchive(int flags)
		{
			auto writableSource = std::make_shared<
				WritableSource
			>(_inputStreamPtr, _outputStreamPtr);

			_inputStreamPtr = nullptr;
			_outputStreamPtr = nullptr;

			Error error;

			zip_source_t* zipSrcPtr = zip_source_function_create(
				&WritableSource::dispatch,
				writableSource.get(),
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

			return std::make_shared<ZipHandle>(newZipPtr, writableSource);
		}

	private:

		InputStream _inputStreamPtr;
		OutputStream _outputStreamPtr;

	};

	// create an instance of archive stream
	template<typename IStream, typename OStream>
	static ArchiveStream<
		IStream, OStream
	> MakeArchive(IStream is, OStream os)
	{
		return ArchiveStream<IStream, OStream>(is, os);
	}

	// create a shared pointer to archive stream
	template<typename IStream, typename OStream>
	static std::shared_ptr<
		ArchiveStream<IStream, OStream>
	> MakeSharedArchive(IStream is, OStream os)
	{
		return std::make_shared<
			ArchiveStream<IStream, OStream>
		>(is, os);
	}

}