#pragma once

#include <string>
#include <memory>
#include <vector>

#include "Error.h"
#include "ArchiveEntry.h"
#include "ReadableSourceStream.h"

namespace Zip {

	class Archive {
	public:

		virtual ~Archive()
		{
			if (_isOpen) {
				// close the archive without writing changes to disk
				zip_discard(_zipPtr);
			}
		}

		void saveAndClose()
		{
			if (_isOpen) {

				// write changes to the disk and close the archive
				int result = zip_close(_zipPtr);

				if (result == -1) {

					throw std::runtime_error(
						std::string("can't close zip archive -> ")
							+ zip_strerror(_zipPtr)
					);

				}

				_zipPtr = nullptr;
				_isOpen = false;
				_hasBeenSavedAndClosed = true;
			}
		}

		ArchiveEntry getEntry(
			const std::string& entryPath,
			int flags = ZIP_FL_NOCASE | ZIP_FL_ENC_GUESS
		)
		{
			openArchiveOnlyOnce();

			zip_int64_t entryIndex = zip_name_locate(
				_zipPtr,
				entryPath.c_str(),
				flags
			);

			return ArchiveEntry (
				entryIndex,
				[this](zip_int64_t entryIndex) {

					if (!_zipPtr) {
						throw std::logic_error("archive has been closed");
					}

					zip_file_t* zipFilePtr = zip_fopen_index(
						_zipPtr,
						entryIndex,
						0
					);

					if (!zipFilePtr) {

						throw std::runtime_error(
							std::string("can't open archive entry for reading -> ")
								+ zip_strerror(_zipPtr)
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

			zip_int64_t entryIndex = zip_name_locate(
				_zipPtr,
				entryPath.c_str(),
				flags
			);

			return ArchiveEntry(
				entryIndex,
				[this, entryPassword](zip_int64_t entryIndex) {

					if (!_zipPtr) {
						throw std::logic_error("archive has been closed");
					}

					zip_file_t* zipFilePtr = zip_fopen_index_encrypted(
						_zipPtr,
						entryIndex,
						0,
						entryPassword.c_str()
					);

					if (!zipFilePtr) {

						throw std::runtime_error(
							std::string("can't open encrypted archive entry for reading -> ")
								+ zip_strerror(_zipPtr)
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
			_zipPtr(nullptr),
			_isOpen(false),
			_hasBeenSavedAndClosed(false)
		{}

		virtual zip_t* _openArchive(int flags) = 0;

	private:

		int _flags;
		zip_t* _zipPtr;
		bool _isOpen;
		bool _hasBeenSavedAndClosed;

		std::vector<
			std::shared_ptr<SourceStream>
		> _attachedSourcesForSaving;

		void openArchiveOnlyOnce()
		{
			if (_hasBeenSavedAndClosed) {
				throw std::logic_error("archive has been closed");
			}

			if (!_isOpen) {
				_zipPtr = _openArchive(_flags);
				_isOpen = true;
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
				_zipPtr,
				&ReadableSourceStream<InputStream>::dispatch,
				srcPtr.get()
			);

			if (!zipSrcPtr) {
				throw std::runtime_error("can't create the zip archive data source");
			}

			zip_int64_t entryIndex = zip_file_add(
				_zipPtr,
				entryPath.c_str(),
				zipSrcPtr,
				ZIP_FL_OVERWRITE | flags
			);

			if (entryIndex < 0) {

				zip_source_free(zipSrcPtr);

				throw std::runtime_error(
					std::string("can't add entry to zip archive -> ")
						+ zip_strerror(_zipPtr)
				);

			}

			_attachedSourcesForSaving.push_back(srcPtr);

			return entryIndex;
		}

	};

}