#pragma once

#include "Archive.h"
#include "WritableSourceStream.h"
#include "NullInputStream.h"

namespace Zip {

	template<typename OutputStream>
	class OutputArchiveStream : public Archive {
	public:

		OutputArchiveStream(OutputStream outputStreamPtr) :
			Archive(ZIP_CREATE),
			_outputStreamPtr(outputStreamPtr)
		{}

	protected:

		typedef WritableSourceStream<
			NullInputStream::SharedPtr,
			OutputStream
		> WritableSource;

		virtual ZipHandle::SharedPtr _openArchive(int flags)
		{
			auto writableSource = std::make_shared<
				WritableSource
			>(std::make_shared<NullInputStream>(), _outputStreamPtr);

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

		OutputStream _outputStreamPtr;

	};

	template<typename OutputStream>
	static OutputArchiveStream<OutputStream> MakeOutputArchive(OutputStream os)
	{
		return OutputArchiveStream<OutputStream>(os);
	}

}