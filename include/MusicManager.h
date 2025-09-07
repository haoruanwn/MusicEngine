#pragma once

#include <filesystem>
#include <functional>
#include <initializer_list>
#include <memory>
#include <string>
#include <vector>
#include "Music.h"

namespace MusicEngine {
    
    /**
     * @class MusicManager
     * @brief A singleton class for managing a music library.
     *
     * MusicManager is responsible for scanning music files from specified directories,
     * parsing their metadata, and storing it in an internal database.
     * It provides features like asynchronous scanning, searching, retrieving music info, and exporting the database.
     * This class uses the Pimpl (Pointer to implementation) idiom to hide implementation details.
     */
    class MusicManager {
    public:
        /**
         * @brief Gets the global unique instance of MusicManager.
         * @return A reference to the static instance of MusicManager.
         */
        static MusicManager &getInstance();

        // Delete copy constructor and copy assignment operator to ensure singleton uniqueness.
        MusicManager(const MusicManager &) = delete;
        MusicManager &operator=(const MusicManager &) = delete;

        /**
         * @brief Asynchronously scans the specified music directories to build the music database.
         *
         * This function starts a background thread to perform the scan, so it does not block the calling thread.
         * If a previous scan is still in progress, this function will return false immediately.
         * At least one valid directory path must be set via setDirectoryPath() before calling this function.
         *
         * @param onScanFinished (Optional) A callback function that will be invoked when the scan is complete.
         * The callback receives a size_t argument representing the total number of music found.
         * Defaults to nullptr, meaning no callback will be executed.
         * @return bool Returns true if the scan task was successfully started;
         * returns false if a scan is already in progress or if no directories have been set.
         */
        bool startScan(const std::function<void(size_t)> &onScanFinished = nullptr);

        /**
         * @brief Checks if a scan is currently in progress.
         * @return bool Returns true if scanning, false otherwise.
         */
        bool isScanning() const;

        /**
         * @brief Retrieves all music currently in the database.
         *
         * This function is thread-safe. It returns a copy of all music objects in the database.
         * Be aware that this may have a performance cost if the database is very large.
         *
         * @return std::vector<Music> A vector containing information for all music. Returns an empty vector if the
         * database is empty.
         */
        std::vector<Music> getAllMusics() const;

        /**
         * @brief Searches for music based on a query string.
         *
         * The search is case-insensitive and matches any music whose title contains the query string.
         * This function is thread-safe.
         *
         * @param query The string to search for.
         * @return std::vector<Music> A vector of music that match the query. Returns an empty vector if no matches are
         * found.
         */
        std::vector<Music> searchMusics(const std::string &query) const;

        /**
         * @brief Gets a list of all music filenames in the database.
         * @return std::vector<std::string> A vector of strings, where each string is a music's filename.
         */
        std::vector<std::string> getMusicNames() const;

        /**
         * @brief Sets a single music library directory path.
         *
         * @note This method overwrites any previously set paths.
         *
         * @param directoryPath The path to the directory to be scanned.
         */
        void setDirectoryPath(const std::filesystem::path &directoryPath);

        /**
         * @brief Sets multiple music library directory paths from a vector.
         *
         * @note This method overwrites any previously set paths.
         *
         * @param directoryPaths A vector of paths to the directories to be scanned.
         */
        void setDirectoryPath(const std::vector<std::filesystem::path> &directoryPaths);

        /**
         * @brief Sets multiple music library directory paths using an initializer list.
         *
         * @note This method overwrites any previously set paths.
         * Example: `setDirectoryPath({"path/to/music1", "path/to/music2"});`
         *
         * @param directoryPaths An initializer_list of paths to be scanned.
         */
        void setDirectoryPath(std::initializer_list<std::filesystem::path> directoryPaths);

        /**
         * @brief Exports all music information from the current database to a text file.
         *
         * This function formats the details of each music (title, artist, album, etc.) and writes them to the specified
         * file. If the file already exists, its content will be overwritten. This operation is thread-safe.
         *
         * @param outputPath The full path of the target file to write to.
         * @return bool Returns true if the export is successful, false if file creation fails (e.g., due to
         * permissions).
         */
        bool exportDatabaseToFile(const std::filesystem::path &outputPath) const;

        /**
         * @brief Sets the list of supported music file extensions.
         *
         * Calling this function overwrites the default or previously set list of extensions.
         * The provided extensions are automatically converted to lowercase and prefixed with a '.' if missing.
         * If an empty list is provided, the current settings will be retained and a warning will be logged.
         *
         * @param extensions A vector of strings containing supported extensions (e.g., "mp3", ".flac").
         */
        void setSupportedExtensions(const std::vector<std::string> &extensions);

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl;

        // Constructor and destructor are private to enforce the singleton pattern.
        MusicManager();
        ~MusicManager();
    };

} // namespace MusicEngine
