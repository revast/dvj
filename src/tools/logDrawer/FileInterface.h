/*
 *
 * FileInterface.h
 *
 */

#ifndef	DEFINE_FILE_INTERFACE
#define	DEFINE_FILE_INTERFACE

#include <stdarg.h>
#include <stdio.h>
#include <vector>

class FileInterfaceObj
{

public:

					FileInterfaceObj();
					~FileInterfaceObj();

	void				Clear();
	void				AddArgument(const char* arg, ...);

	bool				ReadLine(FILE* file);
	void				ReadLine(const char* line, ...);

	void				WriteLine(FILE* file);

const	char*				GetLine() const;
	void				PrintLine(FILE* file) const;

	void				RewindLine(FILE* file);

	unsigned int			Size() const;
	char*				operator[](const unsigned int& index) const;

private:

	void				BuildLineFromArgv();

	std::vector<char*>		Argv;
	char*				Line;
	long				RewindPoint;

};

#endif	//DEFINE_FILE_INTERFACE
