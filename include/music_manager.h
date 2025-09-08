#pragma once

#include <filesystem>
#include <functional>
#include <initializer_list>
#include <memory>
#include <string>
#include <vector>
#include "Music.h"

namespace music_engine {
    
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
        static MusicManager &get_instance();

        // Delete copy constructor and copy assignment operator to ensure singleton uniqueness.
        MusicManager(const MusicManager &) = delete;
        MusicManager &operator=(const MusicManager &) = delete;

        /**
         * @brief Asynchronously scans the specified music directories to build the music database.
         *
         * This function starts a background thread to perform the scan, so it does not block the calling thread.
         * If a previous scan is still in progress, this function will return false immediately.
         * At least one valid directory path must be set via set_directory_paths() before calling this function.
         *
         * @param on_scan_finished (Optional) A callback function that will be invoked when the scan is complete.
         * The callback receives a size_t argument representing the total number of music found.
         * Defaults to nullptr, meaning no callback will be executed.
         * @return bool Returns true if the scan task was successfully started;
         * returns false if a scan is already in progress or if no directories have been set.
         */
        bool start_scan(const std::function<void(size_t)> &on_scan_finished = nullptr);

        /**
         * @brief Checks if a scan is currently in progress.
         * @return bool Returns true if scanning, false otherwise.
         */
        bool is_scanning() const;

        /**
         * @brief Retrieves all musics currently in the database.
         *
         * This function is thread-safe. It returns a copy of all music objects in the database.
         * Be aware that this may have a performance cost if the database is very large.
         *
         * @return std::vector<Music> A vector containing information for all musics. Returns an empty vector if the
         * database is empty.
         */
        std::vector<Music> get_all_musics() const;

        /**
         * @brief Searches for musics based on a query string.
         *
         * The search is case-insensitive and matches any music whose title contains the query string.
         * This function is thread-safe.
         *
         * @param query The string to search for.
         * @return std::vector<Music> A vector of musics that match the query. Returns an empty vector if no matches are
         * found.
         */
        std::vector<Music> search_musics(const std::string &query) const;

        /**
         * @brief Gets a list of all music filenames in the database.
         * @return std::vector<std::string> A vector of strings, where each string is a music's filename.
         */
        std::vector<std::string> get_music_filenames() const;

        /**
         * @brief Sets a single music library directory path.
         *
         * @note This method overwrites any previously set paths.
         *
         * @param directory_path The path to the directory to be scanned.
         */
        void set_directory_paths(const std::filesystem::path &directory_path);

        /**
         * @brief Sets multiple music library directory paths from a vector.
         *
         * @note This method overwrites any previously set paths.
         *
         * @param directory_paths A vector of paths to the directories to be scanned.
         */
        void set_directory_paths(const std::vector<std::filesystem::path> &directory_paths);

        /**
         * @brief Sets multiple music library directory paths using an initializer list.
         *
         * @note This method overwrites any previously set paths.
         * Example: `set_directory_paths({"path/to/music1", "path/to/music2"});`
         *
         * @param directory_paths An initializer_list of paths to be scanned.
         */
        void set_directory_paths(std::initializer_list<std::filesystem::path> directory_paths);

        /**
         * @brief Exports all music information from the current database to a text file.
         *
         * This function formats the details of each music (title, artist, album, etc.) and writes them to the specified
         * file. If the file already exists, its content will be overwritten. This operation is thread-safe.
         *
         * @param output_path The full path of the target file to write to.
         * @return bool Returns true if the export is successful, false if file creation fails (e.g., due to
         * permissions).
         */
        bool export_database_to_file(const std::filesystem::path &output_path) const;

        /**
         * @brief Sets the list of supported music file extensions.
         *
         * Calling this function overwrites the default or previously set list of extensions.
         * The provided extensions are automatically converted to lowercase and prefixed with a '.' if missing.
         * If an empty list is provided, the current settings will be retained and a warning will be logged.
         *
         * @param extensions A vector of strings containing supported extensions (e.g., "mp3", ".flac").
         */
        void set_supported_extensions(const std::vector<std::string> &extensions);

        /**
         * @brief Gets the cover art for a specific music object.
         *
         * This method is a convenient facade over the CoverArtCache. It will
         * retrieve the cover art from the cache, or load it from the file if
         * it's the first time being requested.
         *
         * @param music The music object to get the cover art for.
         * @return A shared_ptr to the cover art data (vector<char>), or nullptr if no cover art exists.
         */
        std::shared_ptr<const std::vector<char>> get_cover_art(const Music& music) const;

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl_;

        // Constructor and destructor are private to enforce the singleton pattern.
        MusicManager();
        ~MusicManager();
    };

} // namespace music_engine