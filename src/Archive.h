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

		EntryList getEntryList()
		{
			EntryList entryList;

			zip_int64_t numOfEntries = zip_get_num_entries(
				getHandle()->get(),
				0
			);

			for (zip_int64_t index = 0; index < numOfEntries; index++) {

				struct zip_stat stat;

				int result = zip_stat_index(
					getHandle()->get(),
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

		ArchiveEntry getEntry(
			const std::string& entryPath,
			const std::string& entryPwd = "",
			int flags = ZIP_FL_NOCASE | ZIP_FL_ENC_GUESS
		)
		{
			// get a file index for the given name
			zip_int64_t entryIndex = zip_name_locate(
				getHandle()->get(),
				entryPath.c_str(),
				flags
			);

			auto weakHandle = getWeakHandle();

			return ArchiveEntry (
				entryIndex,
				// open entry function
				[weakHandle, entryPwd] (zip_int64_t entryIndex)
				{
					auto tempHandle = weakHandle.lock();
					
					if (!tempHandle) {
						throw std::logic_error("archive has been destroyed");
					}

					ZipFileHandle::SharedPtr fileHandle;

					if (entryPwd.empty()) {

						fileHandle = tempHandle->openEntry(
							entryIndex
						);
						
					}
					else {

						fileHandle = tempHandle->openEncryptedEntry(
							entryIndex,
							entryPwd
						);
						
					}

					auto streamImpl = std::make_shared<EntryStreamImpl>(
						fileHandle,
						// deleter
						[weakHandle] (
							ZipFileHandle::WeakPtr fileHandle
						)
						{
							auto tempHandle = weakHandle.lock();

							if (!tempHandle) {
								return;
							}

							auto tempFileHandle = fileHandle.lock();

							if (tempFileHandle) {
								tempHandle->closeEntry(tempFileHandle->get());
							}
						}
					);

					return ReadableEntryStream(streamImpl);
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
				getHandle(),
				entryIndex,
				ZIP_EM_AES_256,
				entryPassword.c_str()
			);

			if (failed) {

				throw std::runtime_error(
					std::string("cannot set encryption for archive entry -> ")
						+ zip_strerror(getHandle())
				);

			}

		}

		void discardAndClose()
		{
			_handle->discardAndClose();
		}

		void saveAndClose()
		{
			_handle->saveAndClose();
		}

	protected:

		Archive(
			int flags
		) :
			_flags(flags),
			_handle(nullptr)
		{}

		virtual ZipHandle::SharedPtr _openArchive(int flags) = 0;

	private:

		ZipHandle::SharedPtr _handle;
		int _flags;
		bool _hasBeenSaved;

		ZipHandle::SharedPtr getHandle()
		{
			if (!_handle) {
				_handle = _openArchive(_flags);
			}

			return _handle;
		}

		ZipHandle::WeakPtr getWeakHandle()
		{
			return _handle;
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

			zip_source_t* zipSrcPtr = zip_source_function(
				getHandle()->get(),
				&ReadableSourceStream<InputStream>::dispatch,
				srcPtr.get()
			);

			if (!zipSrcPtr) {
				throw std::runtime_error(
					"cannot create zip archive data source"
				);
			}

			zip_int64_t entryIndex = zip_file_add(
				getHandle()->get(),
				entryPath.c_str(),
				zipSrcPtr,
				ZIP_FL_OVERWRITE | flags
			);

			if (entryIndex < 0) {

				zip_source_free(zipSrcPtr);

				throw std::runtime_error(
					std::string("cannot add entry to zip archive -> ")
						+ zip_strerror(getHandle()->get())
				);

			}

			getHandle()->attachSourceForSaving(srcPtr);

			return entryIndex;
		}

	};

}