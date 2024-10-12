#include<cstdio>
#include<cerrno>
#include<string>
#include<cstring>
#include<vector>
#include<algorithm>
#include<iostream>
#include<io.h>
#include<conio.h>

using namespace std;

int main(int argc, char** argv)
{
	vector<string> file_list;
	vector<string>::iterator index;
	
	if (argc != 2)
	{
		cout << "Usage: mydir directory_name" << endl;
		return 1;
	}
	char* inquiry_string;
	size_t length = strlen(argv[1]);
	inquiry_string = new char[length + 2];
	strcpy_s(inquiry_string, length+1, argv[1]);
	inquiry_string[length] = '*';
	inquiry_string[length+1] = '\0';

	_finddata_t file;

	intptr_t inquiry_handel;

	if ((inquiry_handel=_findfirst(inquiry_string, &file)) == EOF)
	{
		char errmsg[100];
		strerror_s(errmsg, 99, errno);
		cout << errmsg << endl;
		return 2;
	}
	do
	{
		if(!(file.attrib & _A_HIDDEN))
			file_list.push_back(file.name);
	} 	
	while (_findnext(inquiry_handel, &file) != EOF);
	_findclose(inquiry_handel);
	sort(file_list.begin(), file_list.end());
	index = file_list.begin();
	while (index != file_list.end())
	{
		cout << *index << endl;
		index++;
	}
	delete inquiry_string;
	return 0;
}