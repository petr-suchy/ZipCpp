#pragma once

#include "Archive.h"

namespace Zip {

	class ArchiveSource : public Archive {
	protected:

		ArchiveSource(int flags) :
			Archive(flags)
		{}

		virtual ZipHandle::SharedPtr _openArchive(int flags)
		{
			Error error;

			zip_source_t* zipSrcPtr = zip_source_function_create(
				_getSourceDispath(),
				_getSourcePtr(),
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

			return std::make_shared<ZipHandle>(newZipPtr);
		}

	protected:

		virtual zip_source_callback _getSourceDispath() = 0;
		virtual void* _getSourcePtr() = 0;

	};

}