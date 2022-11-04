#pragma once

#include <string>
#include <memory>
#include <vector>

#include "Error.h"
#include "ZipHandle.h"
#include "ArchiveEntry.h"
#include "ReadableSourceStream.h"

namespace Zip {

	class Archive {
	public:

		typedef struct zip_stat EntryInfo;
		typedef std::vector<EntryInfo> EntryList;

		virtual ~Archive()
		{
			// zip handle MUST be destroyed before the attached sources
			_zipPtr = nullptr;
		}

		EntryList getEntryList()
		{
			openArchiveOnlyOnce();

			EntryList entryList;

			zip_int64_t numOfEntries = zip_get_num_entries(
				_zipPtr->get(),
				0
			);

			for (zip_int64_t index = 0; index < numOfEntries; index++) {

				struct zip_stat stat;

				int result = zip_stat_index(
					_zipPtr->get(),
					index,
					0,
					&stat
				);

				if (result == 0) {
					entryList.push_back(stat);
				}

			}

			return entryList;
		}

		void saveAndClose()
		{
			_zipPtr->saveAndClose();
		}

		ArchiveEntry getEntry(
			const std::string& entryPath,
			int flags = ZIP_FL_NOCASE | ZIP_FL_ENC_GUESS
		)
		{
			openArchiveOnlyOnce();

			// get a file index for the given name
			zip_int64_t entryIndex = zip_name_locate(
				_zipPtr->get(),
				entryPath.c_str(),
				flags
			);

			ZipHandle::WeakPtr weakZipPtr = _zipPtr;

			return ArchiveEntry (
				entryIndex,
				// opens archive entry
				[weakZipPtr] (zip_int64_t entryIndex)
				{

					auto zipPtr = weakZipPtr.lock();

					if (!zipPtr) {
						throw std::logic_error("archive has been closed");
					}

					zip_file_t* zipFilePtr = zip_fopen_index(
						zipPtr->get(),
						entryIndex,
						0
					);

					if (!zipFilePtr) {

						throw std::runtime_error(
							std::string("can't open archive entry for reading -> ")
								+ zip_strerror(zipPtr->get())
						);

					}

					return ReadableEntryStream(zipFilePtr);
				}
			);

		}

		ArchiveEntry getEncryptedEntry(
			const std::string& entryPath,
			const std::string& entryPassword,
			int flags = ZIP_FL_NOCASE | ZIP_FL_ENC_GUESS
		)
		{
			openArchiveOnlyOnce();

			// get a file index for the given name
			zip_int64_t entryIndex = zip_name_locate(
				_zipPtr->get(),
				entryPath.c_str(),
				flags
			);

			return ArchiveEntry(
				entryIndex,
				// opens encrypted archive entry
				[this, entryPassword] (zip_int64_t entryIndex) {

					if (!_zipPtr->get()) {
						throw std::logic_error("archive has been closed");
					}

					zip_file_t* zipFilePtr = zip_fopen_index_encrypted(
						_zipPtr->get(),
						entryIndex,
						0,
						entryPassword.c_str()
					);

					if (!zipFilePtr) {

						throw std::runtime_error(
							std::string("can't open encrypted archive entry for reading -> ")
								+ zip_strerror(_zipPtr->get())
						);

					}

					return ReadableEntryStream(zipFilePtr);
				}
			);

		}
		

		template<typename InputStream>
		void addEntry(
			const std::string& entryPath,
			InputStream readableStream,
			int flags = 0
		)
		{
			attachSourceForSaving(
				entryPath,
				std::make_shared<
					Zip::ReadableSourceStream<InputStream>
				>(readableStream),
				flags
			);
		}

		template<typename InputStream>
		void addEncryptedEntry(
			const std::string& entryPath,
			const std::string& entryPassword,
			InputStream readableStream,
			int flags = 0
		)
		{
			zip_int64_t entryIndex = attachSourceForSaving(
				entryPath,
				std::make_shared<
					Zip::ReadableSourceStream<InputStream>
				>(readableStream),
				flags
			);

			int failed = zip_file_set_encryption(
				_zipPtr,
				entryIndex,
				ZIP_EM_AES_256,
				entryPassword.c_str()
			);

			if (failed) {

				throw std::runtime_error(
					std::string("can't set encryption for archive entry -> ")
						+ zip_strerror(_zipPtr)
				);

			}

		}

	protected:

		Archive(
			int flags
		) :
			_flags(flags),
			_zipPtr(std::make_shared<ZipHandle>(nullptr))
		{}

		virtual ZipHandle::SharedPtr _openArchive(int flags) = 0;

	private:

		int _flags;
		ZipHandle::SharedPtr _zipPtr;

		std::vector<
			std::shared_ptr<SourceStream>
		> _attachedSourcesForSaving;

		void openArchiveOnlyOnce()
		{
			if (_zipPtr->isSaved()) {
				throw std::logic_error("archive has been closed");
			}

			if (!_zipPtr->isOpen()) {
				_zipPtr = _openArchive(_flags);
			}
		}

		template<typename InputStream>
		zip_int64_t attachSourceForSaving(
			const std::string entryPath,
			std::shared_ptr<
				ReadableSourceStream<InputStream>
			> srcPtr,
			int flags = 0
		)
		{
			openArchiveOnlyOnce();

			zip_source_t* zipSrcPtr = zip_source_function(
				_zipPtr->get(),
				&ReadableSourceStream<InputStream>::dispatch,
				srcPtr.get()
			);

			if (!zipSrcPtr) {
				throw std::runtime_error("can't create zip archive data source");
			}

			zip_int64_t entryIndex = zip_file_add(
				_zipPtr->get(),
				entryPath.c_str(),
				zipSrcPtr,
				ZIP_FL_OVERWRITE | flags
			);

			if (entryIndex < 0) {

				zip_source_free(zipSrcPtr);

				throw std::runtime_error(
					std::string("can't add entry to zip archive -> ")
						+ zip_strerror(_zipPtr->get())
				);

			}

			_attachedSourcesForSaving.push_back(srcPtr);

			return entryIndex;
		}

	};

}