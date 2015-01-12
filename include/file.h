#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include "str.h"
#include "tinyformat.h"

inline const char *GetHighestSize(unsigned long long int &size)
{
    static const char *sizes[] = { "B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };
    int si = 0;
    for (; 1024 < size; si++)
        size /= 1024;

    if (si > sizeof(sizes))
        return "(hello future!)";
    else
        return sizes[si];
}

// mode parm for Seek
typedef enum {
	FS_SEEK_CUR,
	FS_SEEK_END,
	FS_SEEK_SET
} fsOrigin_t;

typedef enum {
	FS_READ,
	FS_WRITE,
	FS_APPEND
} fsMode_t;


class File
{
	// The file descriptor for this file
	int fd;
	// The length of the file (a cached variable)
	size_t len;
	// Modes originally set
	fsMode_t modes;
	// Mark this class as being a friend of the FileSystem class
	// This allows us to create these file objects via the FileSystem class.
	friend class FileSystem;
public:
	// Read from the file
	virtual size_t Read(void *buffer, size_t len);
	// Write to the file
	virtual size_t Write(const void *buffer, size_t len);
	// Get the length of the file
	virtual size_t Length();
	// Get the position we're currently at
	virtual size_t GetPosition();
	// Set the position
	virtual size_t SetPosition(size_t offset, fsOrigin_t whence);
	// Flush the buffer
	virtual void Flush();
	// Flush userspace and kernel buffers -- forces a write to device
	virtual void KFlush();
	// Get a libc FILE pointer
	virtual FILE *GetFILE();

	// String-style write functions
	template<typename... Args>
	int printf(const std::string &str, const Args&... args)
	{
		std::string tmp = tfm::format(str.c_str(), args...);
		this->Write(tmp.c_str(), tmp.length());
	}

	// Endian-portable, type safe, binary Read and write functions.
	// Useful for floats, ints, and other POD types.
	// These will always write as little-endian (eg, x86 compatible)
	template<typename T>
	size_t Read(T &t)
	{
		// Read into a temporary variable
		T tmp;
		size_t len = this->Read(&tmp, sizeof(T));
#ifdef BIGENDIAN
		// Reverse and copy
		memrev(&t, &tmp, sizeof(T));
#else   // We're already little-endian so just copy.
		t = tmp;
#endif
		return len;
	}

	template<typename T>
	size_t Write(T t)
	{
#ifdef BIGENDIAN
		// Copy temporarily
		T tmp = t;
		// Reverse and copy
		memrev(&t, &tmp, sizeof(T));
#endif
		return this->Write(&t, sizeof(T));
	}
};


class FileSystem
{
public:

	virtual File *OpenFile(const std::string &path, fsMode_t mode);
	virtual void CloseFile(File *f);


	// Static functions used for simple reasons
	static bool IsDirectory(const std::string &str);
	static bool IsFile(const std::string &str);
	static bool FileExists(const std::string &str);
	static void MakeDirectory(const std::string &str);
	static std::string GetCurrentDirectory();
	static std::vector<std::string> DirectoryList(const std::string &dir);
};
