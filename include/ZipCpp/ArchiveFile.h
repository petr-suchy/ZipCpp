#pragma once

#include "Archive.h"

namespace Zip {

	class ArchiveFile : public Archive {
	public:

		enum class Mode {
			Existing,	// open an existing archive
			Create,		// create archive if it does not exist
			ReadOnly,	// open archive only for reading
			Truncate	// if archive exists, ignores its current contents
		};

		ArchiveFile(const std::string& filePath, Mode mode = Mode::Create) :
			Archive(
				[filePath, mode]()
				{
					int flags;
					int zipErrCode;

					switch (mode) {

						case Mode::Existing: flags = 0; break;
						case Mode::Create: flags = ZIP_CREATE; break;
						case Mode::ReadOnly: flags = ZIP_RDONLY; break;
						case Mode::Truncate: flags = ZIP_TRUNCATE; break;

						default:
							throw std::logic_error("invalid archive open mode");
					}

					zip_t* newZipPtr = zip_open(
						filePath.c_str(),
						flags,
						&zipErrCode
					);

					if (!newZipPtr) {

						throw std::runtime_error(
							std::string("cannot open zip archive file -> ")
							+ Error(zipErrCode).getErrMessage()
						);

					}

					return std::make_shared<ZipHandle>(newZipPtr);
				}
			)
		{}

	};

}