#pragma once

#include <string>
#include <memory>
#include <vector>

#include "Error.h"
#include "ZipHandle.h"
#include "ArchiveEntry.h"

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

		ArchiveEntry entry(
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
				// open for reading
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

					auto entryStream = std::make_shared<
						ReadableEntryStream
					>(
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

					return entryStream;
				},

				// open for writing
				[weakHandle, entryPath, entryPwd] (zip_int64_t entryIndex)
				{
					auto tempHandle = weakHandle.lock();
					
					if (!tempHandle) {
						throw std::logic_error("archive has been destroyed");
					}

					auto ss = std::make_shared<std::stringstream>();

					if (entryPwd.empty()) {
						tempHandle->addEntry(entryPath, ss);
					}
					else {
						tempHandle->addEncryptedEntry(entryPath, entryPwd, ss);
					}

					return std::make_shared<
						WritableEntryStream
					>(weakHandle, ss);
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
			getHandle()->addEntry(
				entryPath,
				readableStream,
				flags
			);
		}

		template<typename InputStream>
		void addEncryptedEntry(
			const std::string& entryPath,
			const std::string& entryPwd,
			InputStream readableStream,
			int flags = 0
		)
		{
			return getHandle()->addEncryptedEntry(
				entryPath,
				entryPwd,
				readbaleStream,
				flags
			);
		}

		void discardAndClose()
		{
			getHandle()->discardAndClose();
		}

		void saveAndClose()
		{
			getHandle()->saveAndClose();
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
			return getHandle();
		}

	};

}