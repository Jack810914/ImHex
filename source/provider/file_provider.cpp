#include "providers/file_provider.hpp"

#include <cstdio>

#include <sys/stat.h>
#include <time.h>

#include "utils.hpp"

namespace hex::prv {

    FileProvider::FileProvider(std::string_view path) : Provider(), m_path(path) {
        this->m_fileStatsValid = stat(path.data(), &this->m_fileStats) == 0;

        this->m_file = fopen(path.data(), "r+b");

        this->m_readable = true;
        this->m_writable = true;

        if (this->m_file == nullptr) {
            this->m_file = fopen(path.data(), "rb");
            this->m_writable = false;
        }

    }

    FileProvider::~FileProvider() {
        if (this->m_file != nullptr)
            fclose(this->m_file);
    }


    bool FileProvider::isAvailable() {
        return this->m_file != nullptr;
    }

    bool FileProvider::isReadable() {
        return isAvailable() && this->m_readable;
    }

    bool FileProvider::isWritable() {
        return isAvailable() && this->m_writable;
    }


    void FileProvider::read(u64 offset, void *buffer, size_t size) {
        if ((offset + size) > this->getSize() || buffer == nullptr || size == 0)
            return;

        _fseeki64(this->m_file, offset, SEEK_SET);
        fread(buffer, 1, size, this->m_file);
    }

    void FileProvider::write(u64 offset, void *buffer, size_t size) {
        if (buffer == nullptr || size == 0)
            return;

        _fseeki64(this->m_file, offset, SEEK_SET);
        fwrite(buffer, 1, size, this->m_file);
    }

    size_t FileProvider::getSize() {
        _fseeki64(this->m_file, 0, SEEK_END);
        return _ftelli64(this->m_file);
    }

    std::vector<std::pair<std::string, std::string>> FileProvider::getDataInformation() {
        std::vector<std::pair<std::string, std::string>> result;

        result.emplace_back("File path", this->m_path);
        result.emplace_back("Size", hex::toByteString(this->getSize()));

        if (this->m_fileStatsValid) {
            result.emplace_back("Creation time", ctime(&this->m_fileStats.st_ctime));
            result.emplace_back("Last access time", ctime(&this->m_fileStats.st_atime));
            result.emplace_back("Last modification time", ctime(&this->m_fileStats.st_mtime));
        }

        return result;
    }

}