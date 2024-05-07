#include "utils.h"

#include <fstream>
#include <sstream>

namespace utils {
	bool readFile(const std::string& path, std::string& out) {
		out = {};
		std::string file_contents{};
		if (path.empty()) {
			LOG("File is empty")
			return false;
		}

		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try {
			file.open(path);
			std::stringstream stream;
			stream << file.rdbuf();
			file.close();
			file_contents = stream.str();
		}
		catch (std::ifstream::failure& e)
		{
			LOG("Unable to parse file")
			return false;
		}

		out = file_contents;
		return true;
	}

    void FreeDelete::operator()(void* x) { free(x); }
}