#include <gtest/gtest.h>
#include "../incl/Utils.hpp"
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

// Test Utils::split with char delimiter
TEST(UtilsTest, SplitWithCharDelimiter)
{
	std::string input = "a,b,c";
	char delimiter = ',';
	std::vector<std::string> result = Utils::split(input, delimiter);
	ASSERT_EQ(result.size(), 3);
	EXPECT_EQ(result[0], "a");
	EXPECT_EQ(result[1], "b");
	EXPECT_EQ(result[2], "c");
}

// Test Utils::split with string delimiter
TEST(UtilsTest, SplitWithStringDelimiter)
{
	std::string input = "a--b--c";
	std::string delimiter = "--";
	std::vector<std::string> result = Utils::split(input, delimiter);
	ASSERT_EQ(result.size(), 3);
	EXPECT_EQ(result[0], "a");
	EXPECT_EQ(result[1], "b");
	EXPECT_EQ(result[2], "c");
}

// Test Utils::fileExists
TEST(UtilsTest, FileExists)
{
	std::string filename = "test_file.txt";
	std::ofstream file(filename);
	file.close();
	EXPECT_TRUE(Utils::fileExists(filename));
	unlink(filename.c_str());
	EXPECT_FALSE(Utils::fileExists(filename));
}

// Test Utils::isDirectory
TEST(UtilsTest, IsDirectory)
{
	std::string dirname = "test_dir";
	mkdir(dirname.c_str(), 0755);
	EXPECT_TRUE(Utils::isDirectory(dirname));
	rmdir(dirname.c_str());
	EXPECT_FALSE(Utils::isDirectory(dirname));
}

// Test Utils::listDirectory
TEST(UtilsTest, ListDirectory)
{
	std::string dirname = "test_dir";
	mkdir(dirname.c_str(), 0755);
	std::ofstream file1(dirname + "/file1.txt");
	std::ofstream file2(dirname + "/file2.txt");
	file1.close();
	file2.close();

	std::vector<std::string> files = Utils::listDirectory(dirname);
	EXPECT_NE(std::find(files.begin(), files.end(), "file1.txt"), files.end());
	EXPECT_NE(std::find(files.begin(), files.end(), "file2.txt"), files.end());

	unlink((dirname + "/file1.txt").c_str());
	unlink((dirname + "/file2.txt").c_str());
	rmdir(dirname.c_str());
}

// Test Utils::sanitizeFilename
TEST(UtilsTest, SanitizeFilename)
{
	std::string input = "file@name#with*invalid|chars.txt";
	std::string sanitized = Utils::sanitizeFilename(input);
	EXPECT_EQ(sanitized, "file_name_with_invalid_chars.txt");
}

// Test Utils::uriIsSafe
TEST(UtilsTest, UriIsSafe)
{
	EXPECT_TRUE(Utils::uriIsSafe("/safe/path"));
	EXPECT_FALSE(Utils::uriIsSafe("/unsafe/../../path"));
}
