#pragma once

#include "Archive.h"

namespace Zip {

	class ArchiveFile : public Archive {
	public:

		struct Existing {};
		struct ReadOnly {};
		struct Truncate {};

		// default - creates archive if it does not exist
		ArchiveFile(const std::string& filePath) :
			_filePath(filePath),
			// flags will be passed to _openArchive function
			Archive(ZIP_CREATE)
		{}

		// opens exsiting archive
		ArchiveFile(const std::string& filePath, Existing) :
			_filePath(filePath),
			Archive(0)
		{}

		// opens archive in read-only mode
		ArchiveFile(const std::string& filePath, ReadOnly) :
			_filePath(filePath),
			Archive(ZIP_RDONLY)
		{}

		// if archive exists, ignores its current contents
		ArchiveFile(const std::string& filePath, Truncate) :
			_filePath(filePath),
			Archive(ZIP_TRUNCATE)
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