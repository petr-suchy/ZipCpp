#pragma once

#include "Archive.h"
#include "WritableSourceStream.h"
#include "NullInputStream.h"

namespace Zip {

	// Create an instance of output archive stream.
	template<typename OutputStream>
	Archive MakeOutputArchive(OutputStream outputStream)
	{
		return Archive(
			[outputStream]()
			{
				auto writableSource = std::make_shared<
					WritableSourceStream<
						NullInputStream::SharedPtr,
						OutputStream
					>
				>(std::make_shared<NullInputStream>(), outputStream);

				Error error;

				zip_source_t* zipSrcPtr = zip_source_function_create(
					&WritableSourceStream<
						NullInputStream::SharedPtr,
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