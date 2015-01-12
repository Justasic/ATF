#include "file.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>

// Read from the file
size_t File::Read(void *buffer, size_t len)
{
	return read(this->fd, buffer, len);
}

// Write to the file
size_t File::Write(const void *buffer, size_t len)
{
	return write(this->fd, buffer, len);
}

// Get the length of the file
size_t File::Length()
{
	size_t len = 0;
	// Check if the cached value is there, otherwise re-read the length and set it.
	if (this->len == 0)
	{
		// Save our current position
		size_t pos = this->GetPosition();

		// Go to the end of the file
		lseek(this->fd, 0, SEEK_END);

		// Get the position (which is now the length)
		this->len = len = this->GetPosition();

		// Go back to where we were.
		lseek(this->fd, pos, SEEK_SET);
	}
	else
		len = this->len;

	// Return our value
	return len;
}

// Get the position we're currently at
size_t File::GetPosition()
{
	return lseek(this->fd, 0, SEEK_CUR);
}

// Set the position
size_t File::SetPosition(size_t offset, fsOrigin_t whence)
{
	int w = 0;
	switch(whence)
	{
		case FS_SEEK_CUR:
			w = SEEK_CUR;
			break;
		case FS_SEEK_END:
			w = SEEK_END;
			break;
		case FS_SEEK_SET:
			w = SEEK_SET;
		default:
			break;
	}

	return lseek(this->fd, offset, w);
}

// Flush the buffer
void File::Flush()
{
	if (syncfs(this->fd) == -1)
		// TODO: Throw an exception.
		;
}

// Flush userspace and kernel buffers -- forces a write to device
void File::KFlush()
{
	this->Flush();
	if (fsync(this->fd) == -1)
		// TODO: throw an exception.
		;
}

FILE *File::GetFILE()
{
	std::string flags;
	if (this->modes & (FS_READ | FS_WRITE))
		flags = "rw";
	else if ((this->modes & FS_READ) && !(this->modes & FS_WRITE))
		flags = "r";
	else if ((this->modes & FS_WRITE) && !(this->modes & FS_READ))
		flags = "w";

	if (this->modes & FS_APPEND)
		flags = "a";

	return fdopen(this->fd, flags.c_str());
}


///////////////////////////////////////////////////////////////////
////////////////// FILESYSTEM CLASS ///////////////////////////////
///////////////////////////////////////////////////////////////////

File *FileSystem::OpenFile(const std::string &path, fsMode_t mode)
{
	// Make sure the file exists
	if (!IsFile(path))
		return nullptr;

	// Calculate flags
	int flags = 0;

	if (mode & (FS_READ | FS_WRITE))
		flags |= O_RDWR;
	else if ((mode & FS_READ) && !(mode & FS_WRITE))
		flags |= O_RDONLY;
	else if ((mode & FS_WRITE) && !(mode & FS_READ))
		flags |= O_WRONLY;

	if (mode & FS_APPEND)
		flags |= O_APPEND;

	// Allocate a file
	File *f = new File;
	f->len = 0;
	f->modes = mode;
	// Open the file.
	f->fd = open(path.c_str(), flags);

	if (f->fd == -1)
		// we had an error. Errno was set.
		// TODO: Throw an exception here.
		return nullptr;

	// Successfully opened our file, return it.
	return f;
}

void FileSystem::CloseFile(File *f)
{
	close(f->fd);
	delete f;
}


bool FileSystem::IsDirectory(const std::string &str)
{
	struct stat fileinfo;

	if (stat(str.c_str(), &fileinfo) == 0)
	{
		if (S_ISDIR(fileinfo.st_mode))
			return false;
	}

	return true;
}

bool FileSystem::IsFile(const std::string &file)
{
	struct stat st_buf;
	memset(&st_buf, 0, sizeof(struct stat));

	if (stat(file.c_str(), &st_buf) != 0)
		return false;

	if (S_ISREG(st_buf.st_mode))
		return true;
	return false;
}

bool FileSystem::FileExists(const std::string &file)
{
	struct stat sb;

	if (stat(file.c_str(), &sb) == -1)
		return false;

	if (S_ISDIR(sb.st_mode))
		return false;

	FILE *input = fopen(file.c_str(), "r");

	if (input == NULL)
		return false;
	else
		fclose(input);

	// Do a final access check to make sure we can read it
	// otherwise it's pointless to report a file we can't read.
	if (access(file.c_str(), R_OK) == -1)
		return false;

	return true;
}

void FileSystem::MakeDirectory(const std::string &dir)
{
	char tmp[256];
	char *p = NULL;
	size_t len;

	snprintf(tmp, sizeof(tmp), "%s", dir.c_str());
	len = strlen(tmp);

	if(tmp[len - 1] == '/')
		tmp[len - 1] = 0;

	for(p = tmp + 1; *p; p++)
	{
		if(*p == '/')
		{
			*p = 0;
			mkdir(tmp, S_IRWXU);
			*p = '/';
		}
	}
	mkdir(tmp, S_IRWXU);
}

std::string FileSystem::GetCurrentDirectory()
{
	// The posix 2001 standard states that getcwd will call malloc() to
	// allocate a string, this is great because I hate static buffers.
	char *cwd = getcwd(NULL, 0);
	std::string str = cwd;
	str += "/";
	free(cwd);

	return str;
}


static inline std::vector<std::string> getdir(const std::string &dir)
{
	std::vector<std::string> files;
	DIR *dp;
	struct dirent *dirp;
	if ((dp  = opendir(dir.c_str())) == NULL)
	{
		// TODO: this should be an exception that is thrown or an error that is ignored.
		std::cout << "Error(" << errno << ") opening " << dir << std::endl;
		return files;
	}

	while ((dirp = readdir(dp)) != NULL)
	{
		std::string dirn = dirp->d_name;
		if (dirn != "." && dirn != "..")
			files.push_back(dir + "/" + dirn);
	}

	closedir(dp);
	return files;
}

std::vector<std::string> FileSystem::DirectoryList(const std::string &dir)
{
	std::vector<std::string> files;
	std::vector<std::string> tmp;
	if (IsDirectory(dir))
	{
		files = getdir(dir);
		for (auto file : files)
		{
			if (IsDirectory(file))
			{
				std::vector<std::string> tmp2 = DirectoryList(file);
				tmp.insert(tmp.end(), tmp2.begin(), tmp2.end());
			}
		}
	}

	// Append the vectors together
	files.insert(files.end(), tmp.begin(), tmp.end());

	return files;
}
