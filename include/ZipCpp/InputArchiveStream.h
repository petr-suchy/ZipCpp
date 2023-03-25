#pragma once

#include "Archive.h"
#include "SeekableSourceStream.h"

namespace Zip {

	// Creates an instance of input archive stream.
	template<typename InputStream>
	Archive MakeInputArchive(InputStream inputStream)
	{
		return Archive(
			[inputStream]()
			{
				auto seekableSource = std::make_shared<
					SeekableSourceStream<InputStream>
				>(inputStream);

				Error error;

				zip_source_t* zipSrcPtr = zip_source_function_create(
					&SeekableSourceStream<InputStream>::dispatch,
					seekableSource.get(),
					error.getInternalStructPtr()
				);

				if (!zipSrcPtr) {

					throw std::runtime_error(
						"cannot create a zip archive source -> "
							+ error.getErrMessage()
					);

				}

				zip_t* newZipPtr = zip_open_from_source(
					zipSrcPtr,
					ZIP_RDONLY,
					error.getInternalStructPtr()
				);

				if (!newZipPtr) {

					zip_source_free(zipSrcPtr);

					throw std::runtime_error(
						"cannot open a zip archive from the data source -> "
							+ error.getErrMessage()
					);

				}

				return std::make_shared<ZipHandle>(newZipPtr, seekableSource);
			}
		);
	}

	// Creates a shared pointer to input archive stream.
	template<typename InputStream>
	Archive::SharedPtr MakeSharedInputArchive(InputStream inputStream)
	{
		return std::make_shared<Archive>(
			MakeInputArchive(inputStream)
		);
	}

}