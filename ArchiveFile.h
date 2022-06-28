#pragma once

#include "Archive.h"

namespace Zip {

	class ArchiveFile : public Archive {
	public:

		struct ReadOnly {};

		ArchiveFile(const std::string& filePath) :
			_filePath(filePath),
			Archive(ZIP_CREATE)
		{}

		ArchiveFile(const std::string& filePath, ReadOnly) :
			_filePath(filePath),
			Archive(ZIP_RDONLY)
		{}

	protected:

		virtual zip_t* _openArchive(int flags)
		{
			int zipErrCode;

			zip_t* newZipPtr = zip_open(
				_filePath.c_str(),
				flags,
				&zipErrCode
			);

			if (!newZipPtr) {

				throw std::runtime_error(
					std::string("can't open zip archive file -> ")
						+ Error(zipErrCode).getErrMessage()
				);

			}

			return newZipPtr;
		}

	private:

		std::string _filePath;

	};

}