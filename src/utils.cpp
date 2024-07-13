#include "utils.h"

#include <iostream>
#include <source_location>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>

namespace utils {
	int sign(float x) {
		return (x > 0) - (x < 0);
	}

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

	err::err(const std::source_location& source) : source(source) {}

	err& err::operator<<(err& (*func)(err& e)) {
		return func(*this);
	}

	err& endl(err& e ) {
		std::cerr << " in '" << e.source.function_name() << ":" << e.source.line() << "'\n";
		return e;
	}

	privy::Log::Log() {
		std::remove(log_path.c_str());
		stream = std::ofstream{log_path, std::ofstream::out | std::ofstream::app};
	}
}