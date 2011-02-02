// System includes
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <assert.h>

// Self
#include "file_utils.h"

using namespace std;

#ifdef _WIN32
// Windows
#define ALLOWABLE_DIRECTORY_DELIMITERS "/\\"
#define DIRECTORY_DELIMITER '\\'
#define FILENAMES_CASE_SENSITIVE 0
#elif defined(__MWERKS__)
// mac os <=9
#define ALLOWABLE_DIRECTORY_DELIMITERS ":"
#define DIRECTORY_DELIMITER ':'
#define FILENAMES_CASE_SENSITIVE 0
#else
// UNIX
#define ALLOWABLE_DIRECTORY_DELIMITERS "/"
#define DIRECTORY_DELIMITER '/'
#define FILENAMES_CASE_SENSITIVE 1
#endif

string
filename_sans_directory(string filename)
{
  size_t lastDirDelim= filename.find_last_of(ALLOWABLE_DIRECTORY_DELIMITERS);
  // No directory delimiter, so return filename
  if (lastDirDelim == string::npos) return filename;
  // Return everything after the delimiter
  return filename.substr(lastDirDelim+1);
}

string
filename_directory(string filename)
{
  size_t lastDirDelim= filename.find_last_of(ALLOWABLE_DIRECTORY_DELIMITERS);
  // No directory delimiter, so return nothing
  if (lastDirDelim == string::npos) return "";
  // Return everything up to just before the last delimiter
  return filename.substr(0, lastDirDelim);
}

string
filename_sans_suffix(string filename)
{
  // Find the last '.'
  size_t lastDot= filename.find_last_of(".");
  if (lastDot == string::npos) return filename;
  
  // Find the last directory delimiter
  size_t lastDirDelim= filename.find_last_of(ALLOWABLE_DIRECTORY_DELIMITERS);
  
  if (lastDirDelim != string::npos &&
      lastDot < lastDirDelim) {
    // The last dot was inside the directory name, so return as is
    return filename;
  }

  // Return everything up to the last dot
  return filename.substr(0, lastDot);
}

string
filename_suffix(string filename)
{
  // Find the last '.'
  size_t lastDot= filename.find_last_of(".");
  if (lastDot == string::npos) return "";
  
  // Find the last directory delimiter
  size_t lastDirDelim= filename.find_last_of(ALLOWABLE_DIRECTORY_DELIMITERS);
  
  if (lastDirDelim != string::npos &&
      lastDot < lastDirDelim) {
    // The last dot was inside the directory name, so return as is
    return "";
  }

  // Return everything after to the last dot
  return filename.substr(lastDot+1);
}

string
filename_canonicalize_directory_delims(string filename)
{
  for (size_t i= 0; i< filename.length(); i++) {
    if (filename[i] != DIRECTORY_DELIMITER &&
        strchr(ALLOWABLE_DIRECTORY_DELIMITERS, filename[i])) {
      filename[i] = DIRECTORY_DELIMITER;
    }
  }
  return filename;
}

string
filename_canonicalize(string filename)
{
  string ret;
  for (size_t i= 0; i< filename.length(); i++) {
    if (filename[i] != DIRECTORY_DELIMITER &&
        strchr(ALLOWABLE_DIRECTORY_DELIMITERS, filename[i])) {
      ret += DIRECTORY_DELIMITER;
    } else if (!FILENAMES_CASE_SENSITIVE && isupper(filename[i])) {
      ret += tolower(filename[i]);
    } else {
      ret += filename[i];
    }
  }
  return filename_simplify(filename);
}

string
filename_remove_trailing_directory_delim(string filename)
{
  if (filename.length() &&
      strchr(ALLOWABLE_DIRECTORY_DELIMITERS, filename[filename.length()-1])) {
    // Trim trailing /
    filename= filename.substr(0, filename.length()-1);
  }
  return filename;
}


string
filename_simplify(string filename)
{
  // Replace ./ with nothing
  // Replace xxx/..[/] with nothing
  unsigned int i= 0;
  while (i < filename.length()) {
    if (i+1 < filename.length() &&
        filename[i] == '.' &&
        strchr(ALLOWABLE_DIRECTORY_DELIMITERS, filename[i+1])) {
      // Delete "./"
      // (This will probably get modded down)
      filename= filename.substr(0,i) + filename.substr(i+2);
    }
    else if (i+1 == filename.length() &&
             filename[i] == '.') {
      // Delete trailing "."
      filename= filename.substr(0,i);
    }
    else if (i+2 < filename.length() &&
             filename[i] == '.' &&
             filename[i+1] == '.' &&
             strchr(ALLOWABLE_DIRECTORY_DELIMITERS, filename[i+2])) {
      // Delete "../"
      string prefix=
        filename_directory(
          filename_remove_trailing_directory_delim(filename.substr(0,i)));
      filename= prefix + filename.substr(i+3);
      i= prefix.length();
    }
    else if (i+2 == filename.length() &&
             filename[i] == '.' &&
             filename[i+1] == '.') {
      filename=
        filename_directory(
          filename_remove_trailing_directory_delim(filename.substr(0,i)));
    }
    else {
      // Skip until after next delimiter
      while (i < filename.length()) {
        if (strchr(ALLOWABLE_DIRECTORY_DELIMITERS, filename[i])) {
          i++;
          break;
        }
        i++;
      }
    }
  }
  return filename;
}

#ifndef _WIN32
#define STRICMP strcasecmp
#else
#define STRICMP stricmp
#endif

bool
filenames_equal(string f1, string f2)
{
  f1= filename_canonicalize_directory_delims(f1);
  f2= filename_canonicalize_directory_delims(f2);

  if (FILENAMES_CASE_SENSITIVE) {
    return f1 == f2;
  } else {
    return !STRICMP(f1.c_str(), f2.c_str());
  }
}

#ifdef _WIN32
#define stat _stat
#define fstat _fstat
#endif

bool
filename_exists(string filename)
{
  struct stat statbuf;
  int ret= stat(filename.c_str(), &statbuf);
  return (ret == 0);
}

string
make_filename(string directory, string filename)
{
  if (filename_is_absolute(filename)) return filename;
  if (directory.length() != 0 &&
      !strchr(ALLOWABLE_DIRECTORY_DELIMITERS,
              directory[directory.length()-1])) {
    directory += DIRECTORY_DELIMITER;
  }
  string ret= filename_simplify(directory + filename);
  return ret;
}

bool
filename_is_relative(string filename)
{
  if (filename_is_absolute(filename)) return false;
  filename= filename_remove_trailing_directory_delim(filename);
  // If exename contains a '/' but doesn't start with one, it's a relative
  // path
  if (string::npos == filename.find_first_of(ALLOWABLE_DIRECTORY_DELIMITERS))
    return false;
  return true;
}

bool
filename_is_absolute(string filename)
{
#ifdef _WIN32
  if (filename.length() >= 2 &&
      isalpha(filename[0]) &&
      filename[1] == ':') {
    return true;
  }
#endif
  if (filename.length() &&
      strchr(ALLOWABLE_DIRECTORY_DELIMITERS, filename[0])) {
    return true;
  }
  return false;
}

bool
filename_is_directory(string filename)
{
  struct stat file_stat;
  
#ifdef _WIN32
  if (filename.length() &&
      isalpha(filename[0]) &&
      (!strcmp(filename.c_str()+1, ":/") ||
       !strcmp(filename.c_str()+1, ":\\") ||
       !strcmp(filename.c_str()+1, ":")))     return true;
#endif    
  
  if (stat(filename.c_str(), &file_stat) < 0) return false;
  return !!(file_stat.st_mode & S_IFDIR);
}

#ifdef _WIN32
#define getcwd _getcwd
#include <direct.h>
#endif

string
get_current_working_directory()
{
  int dirlen= 1000;
  char *dir= (char*)malloc(dirlen);
  assert(dir);
  while (1) {
    if (getcwd(dir, dirlen-1)) break;
    if (errno != ERANGE) {
      free(dir);
      return "";
    }
    dirlen *= 2;
    dir= (char*)realloc(dir, dirlen);
  }
  string ret= dir;
  free(dir);
  return ret;
}

vector<string>
directory_get_filenames(string directory, bool recurse)
{
  vector<string> todo;
  vector<string> ret;

  // Trim trailing "/"
  if (directory.length() > 0 &&
      strchr(ALLOWABLE_DIRECTORY_DELIMITERS,
             directory[directory.length()-1])) {
    directory= directory.substr(0, directory.length()-1);
  }

  todo.push_back(directory);

  while (todo.size()) {
    directory= todo.back();
    todo.pop_back();
    
    DIR *d= opendir(directory.c_str());
    
    if (!d) continue;

    struct dirent *ent;
    while ((ent= readdir(d))) {
      string fullname= directory + DIRECTORY_DELIMITER + ent->d_name;
      if (!strcmp(ent->d_name, ".")) continue;
      if (!strcmp(ent->d_name, "..")) continue;

      ret.push_back(fullname);
      if (recurse && filename_is_directory(fullname.c_str()))
        todo.push_back(fullname);
    }
    closedir(d);
  }
  return ret;
}

#if 0
string
find_exe_filename(string argv_0, string initialWorkingDirectory)
{
  if (filename_is_absolute(argv_0)) {
    return argv_0;
  }
  if (filename_is_relative(argv_0)) {
    return make_filename(initialWorkingDirectory,
                         argv_0);
  }
  // Search the path
  const char *path= getenv("PATH");
  if (!path) return "";
  string pathStr= path;
  vector<string> pathItems= string_get_tokens(pathStr, ":");
  for (unsigned int i= 0; i< pathItems.size(); i++) {
    string candidate= make_filename(pathItems[i], argv_0);
    if (filename_is_relative(candidate)) {
      candidate= make_filename(initialWorkingDirectory, candidate);
    }
    if (filename_exists(candidate)) return candidate;
  }
  return "";
}
#endif

bool
file_copy(const string &src, const string &dest)
{
  FILE *in= fopen(src.c_str(), "r");
  if (!in) return false;
  FILE *out= fopen(dest.c_str(), "w");
  if (!out) {
    fclose(in);
    return false;
  }
  char buf[1024];
  bool ok= true;
  while (1) {
    unsigned int len= fread(buf, 1, 1024, in);
    if (len == 0) {
      if (!feof(in)) ok= false;
      break;
    }
    if (len != fwrite(buf, 1, len, out)) {
      ok= false;
      break;
    }
  }
  fclose(in);
  fclose(out);
  return ok;
}

string getline(istream &in)
{
  string ret;
  while (1) {
    int c= in.get();
    if (c == EOF) break;
    ret += (char)c;
    if (c == '\n') break;
  }
  return ret;
}

string getline(FILE *in)
{
  string ret;
  while (1) {
    int c= getc(in);
    if (c == EOF) break;
    ret += (char)c;
    if (c == '\n') break;
  }
  return ret;
}

string getline_backup(FILE *in) {
  if (!reverse_search(in, '\n')) return "";
  if (reverse_search(in, '\n')) getc(in);
  long long tmp= file_tell(in);
  string ret= getline(in);
  file_seek(in, tmp);
  return ret;
}

bool reverse_search(FILE *in, int c) {
  long long pos;
  int bufsize= 100;
  char buf[bufsize];
  pos= file_tell(in);
  while (1) {
    if (bufsize > pos) pos= bufsize;
    if (!bufsize) return false;
    pos -= bufsize;
    file_seek(in, pos);
    fread(buf, 1, bufsize, in);
    for (int i= bufsize-1; i >= 0; i--) {
      if (buf[i] == c) {
        pos += i;
        file_seek(in, pos);
        return true;
      }
    }
  }
}

long long file_tell(FILE *in)
{
  // UGH.  Change this to work for 64-bit offsets
  return ftell(in);
}

int file_seek(FILE *in, long long idx)
{
  // UGH.  Change this to work for 64-bit offsets
  return fseek(in, (long)idx, SEEK_SET);
}

string file_to_string(const string &filename)
{
  FILE *in= fopen(filename.c_str(), "r");
  if (!in) return "";
  string ret;
  while (1) {
    int c= getc(in);
    if (c == EOF) break;
    ret += (char)c;
  }
  fclose(in);
  return ret;
}
