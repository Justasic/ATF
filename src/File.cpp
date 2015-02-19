#define _XOPEN_SOURCE 700
#include "flux.h"
#include "file.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>

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
	Flux::string flags;
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

File *FileSystem::OpenFile(const Flux::string &path, fsMode_t mode)
{
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

	// Try adding O_CREAT to the list.
	if (f->fd == -1 && errno == ENOENT)
	{
		f->fd = open(path.c_str(), flags | O_CREAT);
		if (f->fd == -1)
			goto fail;
	}
	else if (f->fd == -1)
	{
fail:
		printf("Could not open file \"%s\": %s\n", path.c_str(), strerror(errno));
		delete f;
		// we had an error. Errno was set.
		// TODO: Throw an exception here.
		return nullptr;
	}

	f->path = path;

	// Successfully opened our file, return it.
	return f;
}

static inline std::vector<char> charset()
{
	//Change this to suit
	return std::vector<char>(
		{'0','1','2','3','4',
			'5','6','7','8','9',
			'A','B','C','D','E','F',
			'G','H','I','J','K',
			'L','M','N','O','P',
			'Q','R','S','T','U',
			'V','W','X','Y','Z',
			'a','b','c','d','e','f',
			'g','h','i','j','k',
			'l','m','n','o','p',
			'q','r','s','t','u',
			'v','w','x','y','z'
		});
};

static std::string random_string(size_t length)
{
	//0) create the character set.
	//   yes, you can use an array here,
	//   but a function is cleaner and more flexible
	const auto ch_set = charset();

	//1) create a non-deterministic random number generator
	std::default_random_engine rng(std::random_device{}());

	//2) create a random number "shaper" that will give
	//   us uniformly distributed indices into the character set
	std::uniform_int_distribution<> dist(0, ch_set.size() - 1);

	//3) create a function that ties them together, to get:
	//   a non-deterministic uniform distribution from the
	//   character set of your choice.
	auto randchar = [ch_set, &dist, &rng](){ return ch_set[dist(rng)]; };

	std::string str(length, 0);
	std::generate_n(str.begin(), length, randchar);
	return str;
}

File *FileSystem::OpenTemporaryFile(const Flux::string &templatepath)
{
	// First parse our path and fill in the template.
	auto str = random_string(8);
	File *f = OpenFile(templatepath + "." + str.c_str(), fsMode_t(FS_READ | FS_WRITE));
	return f;
}

void FileSystem::CloseFile(File *f)
{
	close(f->fd);
	delete f;
}

bool FileSystem::CopyFile(File *dest, File *src)
{

	printf("=============Copying %s to %s\n", src->GetPath().c_str(), dest->GetPath().c_str());

	if (!dest || !src)
		return false;

	// Allocate a temporary buffer.
	uint8_t *buf = new uint8_t[1024];

	// Copy the data
	size_t len = 0;
	do
	{
		len = src->Read(buf, 1024);
		dest->Write(buf, len);
	}
	while (len == 1024);

	delete [] buf;

	return true;
}


bool FileSystem::IsDirectory(const Flux::string &str)
{
	struct stat fileinfo;

	if (stat(str.c_str(), &fileinfo) == 0)
	{
		if (S_ISDIR(fileinfo.st_mode))
			return true;
	}

	return false;
}

bool FileSystem::IsFile(const Flux::string &file)
{
	struct stat st_buf;
	memset(&st_buf, 0, sizeof(struct stat));

	if (stat(file.c_str(), &st_buf) != 0)
		return false;

	if (S_ISREG(st_buf.st_mode))
		return true;
	return false;
}

bool FileSystem::FileExists(const Flux::string &file)
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

void FileSystem::MakeDirectory(const Flux::string &dir)
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

Flux::string FileSystem::GetCurrentDirectory()
{
	// The posix 2001 standard states that getcwd will call malloc() to
	// allocate a string, this is great because I hate static buffers.
	char *cwd = getcwd(NULL, 0);
	Flux::string str = cwd;
	str += "/";
	free(cwd);

	return str;
}


static inline Flux::vector getdir(const Flux::string &dir)
{
	Flux::vector files;
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
		Flux::string dirn = dirp->d_name;
		if (dirn != "." && dirn != "..")
			files.push_back(dir + "/" + dirn);
	}

	closedir(dp);
	return files;
}

Flux::vector FileSystem::DirectoryList(const Flux::string &dir)
{
	Flux::vector files;
	Flux::vector tmp;
	if (IsDirectory(dir))
	{
		files = getdir(dir);
		for (auto file : files)
		{
			if (IsDirectory(file))
			{
				Flux::vector tmp2 = DirectoryList(file);
				tmp.insert(tmp.end(), tmp2.begin(), tmp2.end());
			}
		}
	}

	// Append the vectors together
	files.insert(files.end(), tmp.begin(), tmp.end());

	return files;
}

Flux::string FileSystem::Dirname(const Flux::string &str)
{
	if (str.empty())
		return "";

	char *strr = strndup(str.c_str(), str.size());
	char *cstr = dirname(strr);
	Flux::string ret(cstr);
	free(strr);
	return ret;
}

Flux::string FileSystem::Basename(const Flux::string &str)
{
	if (str.empty())
		return "";

	char *strr = strndup(str.c_str(), str.size());
	char *cstr = basename(strr);
	Flux::string ret(cstr);
	free(strr);
	return ret;
}
