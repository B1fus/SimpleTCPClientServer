#include "../include/synchroFile.h"


SynchroFile::SynchroFile(const std::string& path):
                         m_path(path){
    file.open(m_path);
    file.close();
}
void SynchroFile::write(const std::string& data){
    std::lock_guard<std::mutex> lock(m_writeMutex);
    file.open(m_path, std::ios::app);
    file << data;
    file.close();
}

SynchroFileWriter::SynchroFileWriter(std::shared_ptr<SynchroFile> syncFile) : m_syncFile(syncFile) {}

void SynchroFileWriter::write(const std::string& data) {
    m_syncFile->write(data);
}