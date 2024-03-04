
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING 1

#include <iostream>
#include <filesystem>
#include <xstring>
#include <vector>
#include <windows.h>
#include <locale>
#include <codecvt>
#include <fstream>
#include <sstream>

//#pragma warning(suppress : 4996)

enum class ERefactoringClass : short
{
	FNamesFix = 0,
	IncludesFix,
};

using std::codecvt_utf8;

std::wstring s2ws(const std::string& str)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX = std::wstring_convert<convert_typeX, wchar_t>();

	return converterX.from_bytes(str);
}

std::string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX = std::wstring_convert<convert_typeX, wchar_t>();

	return converterX.to_bytes(wstr);
}

static const std::string MainPath = "C:\\Users\\CMD\\Desktop\\TempFl\\Temp\\Refactoring";

static const std::string IncludesMainPath = "C:\\Users\\CMD\\Desktop\\TempFl\\Temp\\Refactoring\\Source\\T1Game";

static std::vector<std::string> filesPaths = std::vector<std::string>();

static std::wstring headerFileExt = s2ws(".h");
static std::wstring cppFileExt = s2ws(".cpp");
static std::wstring NamesStaticFileName = s2ws("NamesStatic.h");

static std::filesystem::path NamesStaticFilePath = std::filesystem::path("None");

static std::wstring csFileExt = s2ws(".cs");
static std::string GameName = "T1Game";
static std::vector<std::string> includeDirectoriesPaths = std::vector<std::string>();
static std::string BuildCS_File_path = "";

struct IncludeFileInfo
{
	std::string includeFileName;
	std::string includeFileRelativePath;
	bool bIsPrivate;

	IncludeFileInfo() : includeFileName(), includeFileRelativePath()
	{
		bIsPrivate = false;
	}
};

static std::vector<IncludeFileInfo> includeFilesInfosArr = std::vector<IncludeFileInfo>();
static std::vector<std::string> includeFilesNames = std::vector<std::string>();

void RecursiveCheckDirectories2(std::string NewDirectory)
{
	const std::filesystem::path path{ NewDirectory.c_str() };
	for (const std::filesystem::directory_entry& dir_entry : std::filesystem::directory_iterator{ path })
	{
		if (dir_entry.is_directory())
		{
			std::string DirStr = ws2s(std::wstring(dir_entry.path().relative_path().c_str()));
			std::size_t foundNum = DirStr.find(GameName);
			if (foundNum != std::string::npos)
			{
				std::string subSTR = DirStr.substr(foundNum + GameName.length(), DirStr.length());
				//subSTR.replace(0, subSTR.length(), "\");
				if (subSTR.length() > 1)
				{
					includeDirectoriesPaths.push_back(subSTR);
				}
				//std::cout << "RecursiveCheckDirectories2 - relative_path - " << subSTR << '\n';
			}
			
			RecursiveCheckDirectories2(ws2s(std::wstring(dir_entry.path().c_str())));
		}
		else if (dir_entry.is_character_file() || dir_entry.is_regular_file())
		{
			if (dir_entry.path().extension() == headerFileExt || dir_entry.path().extension() == cppFileExt)
			{
				filesPaths.push_back(ws2s(std::wstring(dir_entry.path().c_str())));

				if (dir_entry.path().extension() == headerFileExt)
				{
					std::filesystem::path pathRt = dir_entry.path().relative_path();
					std::filesystem::path FileNameP = dir_entry.path().filename();
					std::string FileNameStr = ws2s(std::wstring(FileNameP.c_str()));
					includeFilesNames.push_back(FileNameStr);
					//std::cout << "RecursiveCheckDirectories2 - fileName - " << FileNameStr << '\n';
					pathRt._Remove_filename_and_separator();
					std::string DirStr = ws2s(std::wstring(pathRt.c_str()));
					if (!DirStr.empty())
					{
						std::size_t foundNum = DirStr.find(GameName);
						if (foundNum != std::string::npos)
						{
							std::string subSTR = DirStr.substr(foundNum + GameName.length(), DirStr.length());

							IncludeFileInfo NvFileInfo = IncludeFileInfo();
							NvFileInfo.includeFileName = FileNameStr;
							NvFileInfo.includeFileRelativePath = subSTR;
							std::replace(NvFileInfo.includeFileRelativePath.begin(), NvFileInfo.includeFileRelativePath.end(), '\\', '/');
							NvFileInfo.includeFileRelativePath += "/";
							NvFileInfo.includeFileRelativePath = NvFileInfo.includeFileRelativePath.substr(1, NvFileInfo.includeFileRelativePath.length());
							includeFilesInfosArr.push_back(NvFileInfo);
							//subSTR.replace(0, subSTR.length(), "\");
							//includeDirectoriesPaths.push_back(subSTR);
							std::cout << "RecursiveCheckDirectories2 - relative_path - " << NvFileInfo.includeFileRelativePath << '\n';
						}
					}
				}
			}
			else if (dir_entry.path().extension() == csFileExt)
			{
				BuildCS_File_path = ws2s(std::wstring(dir_entry.path().c_str()));
			}
		}
	}
}

void RecursiveCheckDirectories(std::string NewDirectory)
{
	const std::filesystem::path path{ NewDirectory.c_str() };
	for (const std::filesystem::directory_entry& dir_entry : std::filesystem::directory_iterator{ path })
	{
		if (dir_entry.is_directory())
		{
			//dir_entry.is_regular_file()
			RecursiveCheckDirectories(ws2s(std::wstring(dir_entry.path().c_str())));
		}
		else if(dir_entry.is_character_file() || dir_entry.is_regular_file())
		{
			if (dir_entry.path().filename() != NamesStaticFileName)
			{
				if (dir_entry.path().extension() == headerFileExt || dir_entry.path().extension() == cppFileExt)
				{
					filesPaths.push_back(ws2s(std::wstring(dir_entry.path().c_str())));
				}
			}
			else
			{
				NamesStaticFilePath = dir_entry.path();
			}
		}
	}
}

static std::string currentLine = std::string();

//static std::string NameStaticIncludeLine = "#include <T1Game/NamesStatic.h>";
static std::string NameStaticIncludeLine = "#include <T1Game/NamesStatic.h>";
static std::string NameToFind = "FName(TEXT(|";
//NameStaticIncludeLine.replace()
static std::vector<std::string> LinesToWriteIntoNamesStaticFile = std::vector<std::string>();

static std::string EndPointsSTR = "|));";
static std::string StartSTR = "static FName StaticNameGlobal_";
static std::string MidSTR = " = FName(TEXT(|";
static std::string PrefixSTR = "StaticNameGlobal_";
static std::string QuotesDivider = "|";

void DoOPS_WithFile(const std::string& FilePathStr)
{
	//return;

	std::ifstream in(FilePathStr, std::ios::out);
	if (!in.is_open()) {
		std::cerr << "Error: Unable to open file \"" << FilePathStr << "\" for reading!" << std::endl;
		return;
	}

	//std::istream& LineCur = std::getline(in, currentLine);
	//LineCur.
	bool bIncludeAdded = false;
	bool bNameDeclFinded = false;
	std::vector<std::string> FileLines = std::vector<std::string>();
	while (std::getline(in, currentLine)) {
		//std::cout << currentLine << '\n';

		for (size_t i = 0; i < 8; ++i)
		{
			std::size_t foundNum = currentLine.find(NameToFind);
			if (foundNum != std::string::npos)
			{
				std::size_t foundQuotesNext = currentLine.find('"', foundNum + NameToFind.size());
				if (foundQuotesNext != std::string::npos)
				{
					std::string subSTR = currentLine.substr(foundNum + NameToFind.size(), foundQuotesNext - (foundNum + NameToFind.size()));

					std::string CompiledSTR = StartSTR + subSTR + MidSTR + subSTR + EndPointsSTR;

					//std::cout << CompiledSTR << '\n';
					LinesToWriteIntoNamesStaticFile.push_back(CompiledSTR);
					bNameDeclFinded = true;

					std::string ReplaceSTR = PrefixSTR + subSTR;

					std::string subSTR2 = currentLine.substr(0, foundNum);
					std::string subSTR3 = currentLine.substr(foundQuotesNext + 3, currentLine.length());

					CompiledSTR = subSTR2 + ReplaceSTR + subSTR3;
					//log
					//std::cout << CompiledSTR << '\n';

					currentLine = CompiledSTR;
				}
			}
		}

		FileLines.push_back(currentLine);
		currentLine.clear();
	}

	in.close();

	if (bNameDeclFinded)
	{
		std::ofstream out(FilePathStr, std::ios::out);
		for (std::string& LineSTR : FileLines)
		{
			if (!bIncludeAdded)
			{
				std::size_t foundNum = LineSTR.find("#include");
				if (foundNum != std::string::npos)
				{
					out << NameStaticIncludeLine << '\n';
					bIncludeAdded = true;
				}
			}
			out << LineSTR << '\n';
		}
		out.close();

		std::cout << "File write operation complete, p - " << FilePathStr << '\n';
	}
}

static std::string IncludeLineBase = "#include";

void DoOPS_WithFile2(const std::string& FilePathStr)
{
	//return;

	std::ifstream in(FilePathStr, std::ios::out);
	if (!in.is_open()) {
		std::cerr << "Error: Unable to open file \"" << FilePathStr << "\" for reading!" << std::endl;
		return;
	}

	//std::istream& LineCur = std::getline(in, currentLine);
	//LineCur.
	bool bIncludeAdded = false;
	bool bNameDeclFinded = false;
	std::vector<std::string> FileLines = std::vector<std::string>();
	while (std::getline(in, currentLine)) {
		//std::cout << currentLine << '\n';

		//for (size_t i = 0; i < 8; ++i)
		{
			std::size_t foundNum = currentLine.find(std::string("//"));
			if (foundNum != std::string::npos)
			{
				FileLines.push_back(currentLine);
				currentLine.clear();
				continue;
			}
			foundNum = currentLine.find(IncludeLineBase);
			if (foundNum != std::string::npos)
			{
				for (std::string& IncludeSTR : includeFilesNames)
				{
					std::size_t foundIncludeNext = currentLine.find(IncludeSTR);
					if (foundIncludeNext != std::string::npos)
					{
						std::string IncludeRelPath = "";
						for (IncludeFileInfo& IncludeFileInfoStruct : includeFilesInfosArr)
						{
							if (IncludeFileInfoStruct.includeFileName == IncludeSTR)
							{
								IncludeRelPath = IncludeFileInfoStruct.includeFileRelativePath;
								break;
							}
						}
						currentLine = IncludeLineBase + " " + QuotesDivider + /*IncludeRelPath + */IncludeSTR + QuotesDivider;
						std::replace(currentLine.begin(), currentLine.end(), '|', '"');
						std::replace(currentLine.begin(), currentLine.end(), '\\', '/');
						bNameDeclFinded = true;
						break;
					}
				}
				/*std::size_t foundQuotesNext = currentLine.find('"', foundNum + NameToFind.size());
				if (foundQuotesNext != std::string::npos)
				{
					std::string subSTR = currentLine.substr(foundNum + NameToFind.size(), foundQuotesNext - (foundNum + NameToFind.size()));

					std::string CompiledSTR = StartSTR + subSTR + MidSTR + subSTR + EndPointsSTR;

					//std::cout << CompiledSTR << '\n';
					LinesToWriteIntoNamesStaticFile.push_back(CompiledSTR);
					bNameDeclFinded = true;

					std::string ReplaceSTR = PrefixSTR + subSTR;

					std::string subSTR2 = currentLine.substr(0, foundNum);
					std::string subSTR3 = currentLine.substr(foundQuotesNext + 3, currentLine.length());

					CompiledSTR = subSTR2 + ReplaceSTR + subSTR3;
					//log
					//std::cout << CompiledSTR << '\n';

					currentLine = CompiledSTR;
				}*/
			}
		}

		FileLines.push_back(currentLine);
		currentLine.clear();
	}

	in.close();

	if (bNameDeclFinded)
	{
		std::ofstream out(FilePathStr, std::ios::out);
		for (std::string& LineSTR : FileLines)
		{
			/*if (!bIncludeAdded)
			{
				std::size_t foundNum = LineSTR.find("#include");
				if (foundNum != std::string::npos)
				{
					out << NameStaticIncludeLine << '\n';
					bIncludeAdded = true;
				}
			}*/
			out << LineSTR << '\n';
		}
		out.close();

		std::cout << "File write operation complete, p - " << FilePathStr << '\n';
	}
}

void RewriteBuildCSFile()
{
	if (BuildCS_File_path.empty())
		return;

	std::ifstream in(BuildCS_File_path, std::ios::out);
	if (!in.is_open()) {
		std::cerr << "Error: Unable to open file \"" << BuildCS_File_path << "\" for reading!" << std::endl;
		return;
	}
	std::vector<std::string> FileLines = std::vector<std::string>();
	while (std::getline(in, currentLine))
	{
		FileLines.push_back(currentLine);
	}
	in.close();
	//OUT
	std::ofstream out(BuildCS_File_path, std::ios::out);
	//out.write()
	std::string LineToCheck = "PrivateIncludePaths.Add(ModuleDirectory);";
	for (std::string& LineSTR : FileLines)
	{
		out << LineSTR << '\n';

		std::size_t LinePos = LineSTR.find(LineToCheck);
		if (LinePos != std::string::npos)
		{
			for (std::string& inclDirSTR : includeDirectoriesPaths)
			{
				std::replace(inclDirSTR.begin(), inclDirSTR.end(), '\\', '/');

				LinePos = inclDirSTR.find("Private");
				if (LinePos != std::string::npos)
				{
					{
						std::string BaseStr = "		PrivateIncludePaths.Add(ModuleDirectory";
						BaseStr = BaseStr + " + " + QuotesDivider + inclDirSTR + QuotesDivider + ");";
						std::replace(BaseStr.begin(), BaseStr.end(), '|', '"');
						out << BaseStr << '\n';
					}
				}
				else
				{
					LinePos = inclDirSTR.find("Public");
					if (LinePos != std::string::npos)
					{
						{
							std::string BaseStr = "		PublicIncludePaths.Add(ModuleDirectory";
							BaseStr = BaseStr + " + " + QuotesDivider + inclDirSTR + QuotesDivider + ");";
							std::replace(BaseStr.begin(), BaseStr.end(), '|', '"');
							out << BaseStr << '\n';
						}
					}
					else
					{
						{
							std::string BaseStr = "		PublicIncludePaths.Add(ModuleDirectory";
							BaseStr = BaseStr + " + " + QuotesDivider + inclDirSTR + QuotesDivider + ");";
							std::replace(BaseStr.begin(), BaseStr.end(), '|', '"');
							out << BaseStr << '\n';
						}

						{
							std::string BaseStr = "		PrivateIncludePaths.Add(ModuleDirectory";
							BaseStr = BaseStr + " + " + QuotesDivider + inclDirSTR + QuotesDivider + ");";
							std::replace(BaseStr.begin(), BaseStr.end(), '|', '"');
							out << BaseStr << '\n';
						}
					}
				}
			}
		}
	}
	/*out << '\n';
	for (std::string& LineSTR : LinesToWriteIntoNamesStaticFile)
	{
		out << LineSTR << '\n';
	}*/
	out.close();

	std::cout << "RewriteBuildCSFile end includeDirectoriesPaths size - " << includeDirectoriesPaths.size() << '\n';
}

static ERefactoringClass RefactoringClassType = ERefactoringClass::IncludesFix;

int main()
{
	if (RefactoringClassType == ERefactoringClass::FNamesFix)
	{
		currentLine.reserve(270);

		std::replace(NameStaticIncludeLine.begin(), NameStaticIncludeLine.end(), '<', '"');
		std::replace(NameStaticIncludeLine.begin(), NameStaticIncludeLine.end(), '>', '"');
		std::replace(NameToFind.begin(), NameToFind.end(), '|', '"');
		std::replace(EndPointsSTR.begin(), EndPointsSTR.end(), '|', '"');
		std::replace(MidSTR.begin(), MidSTR.end(), '|', '"');

		RecursiveCheckDirectories(MainPath);
		/*const std::filesystem::path path{ MainPath.c_str() };
		for (const std::filesystem::directory_entry& dir_entry : std::filesystem::directory_iterator{ path })
		{
			//dir_entry.path->c_str();
			std::cout << dir_entry.path() << '\n';

			for (const std::filesystem::directory_entry& dir_entry_2nd : std::filesystem::directory_iterator{ dir_entry.path() })
			{
				std::cout << dir_entry_2nd.path() << '\n';
			}
		}*/

		if (filesPaths.size() < 1)
		{
			std::cout << "(filesPaths.size() < 1)" << '\n';
		}

		for (std::string& FilePathSTR : filesPaths)
		{
			//std::cout << FilePathSTR << '\n';
			DoOPS_WithFile(FilePathSTR);
		}

		if (LinesToWriteIntoNamesStaticFile.size() > 1)
		{
			std::cout << "(LinesToWriteIntoNamesStaticFile.size() > 1) - " << LinesToWriteIntoNamesStaticFile.size() << '\n';
		}

		if (LinesToWriteIntoNamesStaticFile.size() > 0)
		{
			/*for (int i = LinesToWriteIntoNamesStaticFile.size() - 1; i >= 0; --i)
			{
				bool needDelete = false;
				for (std::string& LineSTR : LinesToWriteIntoNamesStaticFile)
				{
					if (LineSTR == LinesToWriteIntoNamesStaticFile[i])
					{
						needDelete = true;
						break;
					}
				}

				if (needDelete)
				{
					//std::cout << LinesToWriteIntoNamesStaticFile[i] << '\n';
					LinesToWriteIntoNamesStaticFile.erase(LinesToWriteIntoNamesStaticFile.begin() + i);
				}

				if (i == 0)
				{
					break;
				}
			}*/

			for (std::vector<std::string>::iterator it = LinesToWriteIntoNamesStaticFile.begin(); it != LinesToWriteIntoNamesStaticFile.end();)
			{
				bool needDelete = false;
				{
					int counterRt = 0;
					for (std::string& LineSTR : LinesToWriteIntoNamesStaticFile)
					{
						if (LineSTR == *it)
						{
							++counterRt;
							if (counterRt > 1)
							{
								needDelete = true;
								break;
							}
						}
					}
					counterRt = 0;
				}

				if (needDelete)
				{
					it = LinesToWriteIntoNamesStaticFile.erase(it);
					needDelete = false;
				}
				else
				{
					++it;
				}
			}
		}

		for (std::string& LineSTR : LinesToWriteIntoNamesStaticFile)
		{
			std::cout << LineSTR << '\n';
		}

		if (LinesToWriteIntoNamesStaticFile.size() > 1)
		{
			std::cout << "LinesToWriteIntoNamesStaticFile after clean - " << LinesToWriteIntoNamesStaticFile.size() << '\n';
		}

		if (LinesToWriteIntoNamesStaticFile.size() < 1)
		{
			std::cout << "(LinesToWriteIntoNamesStaticFile.size() < 1)" << '\n';
		}

		std::cout << NamesStaticFilePath.string() << '\n';

		std::cout << NameStaticIncludeLine << '\n';
		{
			//IN
			std::ifstream in(NamesStaticFilePath.string(), std::ios::out);
			if (!in.is_open()) {
				std::cerr << "Error: Unable to open file \"" << NamesStaticFilePath.string() << "\" for reading!" << std::endl;
				return 1;
			}
			std::vector<std::string> FileLines = std::vector<std::string>();
			while (std::getline(in, currentLine))
			{
				FileLines.push_back(currentLine);
			}
			in.close();
			//OUT
			std::ofstream out(NamesStaticFilePath.string(), std::ios::out);
			//out.write()
			for (std::string& LineSTR : FileLines)
			{
				out << LineSTR << '\n';
			}
			out << '\n';
			for (std::string& LineSTR : LinesToWriteIntoNamesStaticFile)
			{
				out << LineSTR << '\n';
			}
			out.close();
		}
	}
	else if (RefactoringClassType == ERefactoringClass::IncludesFix)
	{
		RecursiveCheckDirectories2(IncludesMainPath);

		for (std::string& FilePathSTR : filesPaths)
		{
			DoOPS_WithFile2(FilePathSTR);
		}

		RewriteBuildCSFile();
	}

	Sleep(1000000);
}

//#pragma warning(disable : 4996)