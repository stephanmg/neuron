#ifndef ocfile_h
#define ocfile_h

#include <OS/string.h>
class File;
class FileChooser;

class OcFile {
public:
	OcFile();
	virtual ~OcFile();
	bool open(const char* filename, const char* type);
	void set_name(const char* s);
	const char* get_name() {return filename_.string();}
	const char* dir();
	void close();
	void print(const char* s) {fprintf(file(), "%s", s);}
	FILE* file();
	bool is_open() { return file_ ? true : false;}
	bool eof();
	void flush() { if (file_) { fflush(file_); }}
	bool mktemp();
	bool unlink();
	void file_chooser_style(const char* type, const char* path,
		const char* banner=nil, const char* filter=nil,
		const char* accept=nil, const char* cancel=nil);
	bool file_chooser_popup();
#ifdef WIN32
	void binary_mode();// sets open file to binary mode once only
#endif
private:
	FileChooser* fc_;
	enum { N, R, W, A};
	int chooser_type_;
	CopyString filename_;
	CopyString dirname_;
	FILE* file_;
#ifdef WIN32
	bool binary_;
	char mode_[3];
#endif
};

#ifdef WIN32
#define BinaryMode(ocfile) ocfile->binary_mode();
#else
#define BinaryMode(ocfile) /**/
#endif

#endif
