#pragma once

#include "Archive.h"
#include "WritableSourceStream.h"

namespace Zip {

	// Creates an instance of input/output archive stream.
	template<typename InputStream, typename OutputStream>
	Archive MakeArchive(InputStream inputStream, OutputStream outputStream)
	{
		return Archive(
			[inputStream, outputStream]()
			{
				auto writableSource = std::make_shared<
					WritableSourceStream<
						InputStream,
						OutputStream
					>
				>(inputStream, outputStream);

				Error error;

				zip_source_t* zipSrcPtr = zip_source_function_create(
					&WritableSourceStream<
						InputStream,
						OutputStream
					>::dispatch,
					writableSource.get(),
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
					ZIP_CREATE,
					error.getInternalStructPtr()
				);

				if (!newZipPtr) {

					zip_source_free(zipSrcPtr);

					throw std::runtime_error(
						"cannot open a zip archive from the data source -> "
							+ error.getErrMessage()
					);

				}

				return std::make_shared<ZipHandle>(newZipPtr, writableSource);
			}
		);
	}

}