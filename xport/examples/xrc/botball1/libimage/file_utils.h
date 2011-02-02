#ifndef FILE_UTILS_H
#define FILE_UTILS_H

// System includes
#include <string>
#include <vector>
#include <iostream>
#include <stdio.h>

// Manipulate paths and filenames
std::string filename_sans_directory(std::string filename);
std::string filename_directory(std::string filename);
std::string filename_sans_suffix(std::string filename);
std::string filename_suffix(std::string filename);
std::string filename_canonicalize_directory_delims(std::string filename);
std::string filename_canonicalize(std::string filename);
std::string filename_simplify(std::string filename);
bool filenames_equal(std::string f1, std::string f2);
std::string filename_remove_trailing_directory_delim(std::string filename);
std::string make_filename(std::string directory, std::string filename);
bool filename_is_relative(std::string filename);
bool filename_is_absolute(std::string filename);
bool filename_is_directory(std::string filename);
bool file_copy(const std::string &src, const std::string &dest);

std::string getline(std::istream &in);

// Query files and directories
bool filename_exists(std::string filename);
std::vector<std::string> directory_get_filenames(std::string directory,
                                                 bool recurse= false);

// Get current working directory
std::string get_current_working_directory();

// Get full pathname for this executable
std::string find_exe_filename(std::string argv_0,
                              std::string initialWorkingDirectory);

std::string getline(std::istream &in);
std::string getline(FILE *in);

std::string getline_backup(FILE *in);
bool reverse_search(FILE *in, int c);

long long file_tell(FILE *in);
int file_seek(FILE *in, long long idx);

std::string file_to_string(const std::string &filename);

#endif
