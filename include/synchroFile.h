#pragma once

#include <thread>
#include <fstream>
#include <chrono>
#include <ostream>
#include <iomanip>
#include <memory>

//file for synchronize writing from different threads
class SynchroFile{
private:
    std::string m_path;
    std::mutex m_writeMutex;
    std::ofstream file;

public:
    SynchroFile(const std::string& path);
    void write(const std::string& data);
};

//Writer to SynchroFile. Can be used in threads for writing.
class SynchroFileWriter{
private:
    std::shared_ptr<SynchroFile> m_syncFile;

public:
    SynchroFileWriter(std::shared_ptr<SynchroFile> syncFile);
    void write(const std::string& data);
};