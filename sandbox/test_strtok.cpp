#include <cstring>
#include <iostream>
#include <vector>

int main(void)
{
	// char s[] = "my string\t more\twords \t end";
	// char *p;
	// std::vector<std::string> tokens;

	// p = strtok(s, " \t");
	// while (p != NULL) {
	// 	tokens.push_back(p);
	// 	p = strtok(NULL, " ");  // pass NULL to retain the old string
	// }


	// std::string s = "  \t my string\t more\twords \t and \t\t \t   \t final\r";
	// std::string delimiters = " \t";
	std::string s = "";
	std::string delimiters = "\r";
	std::vector<std::string> tokens;
	size_t start = 0;
	size_t end = 0;

	while (start < s.size())
	{
		start = s.find_first_not_of(delimiters, end);	// start looking at 'end'
		if (start == std::string::npos)
			break;

		end = s.find_first_of(delimiters, start);		// start looking at 'start'
		tokens.push_back(s.substr(start, end - start));
	}
	std::cout << "Done tokenizing. tokens.size(): " << tokens.size() << std::endl;
	for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); it++)
		std::cout << *it << std::endl;
}