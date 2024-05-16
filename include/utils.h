#ifndef OPENGL_PRACTICE_UTILS_H
#define OPENGL_PRACTICE_UTILS_H

#include <iostream>
#include <source_location>
#include <concepts>

#define STRINGIFY_MACRO_EXPANSION(x) #x
#define STRINGIFY(x) STRINGIFY_MACRO_EXPANSION(x)
#define LOG(x) utils::err() << x << utils::endl;

namespace utils {
	template <typename T>
	concept Streamable = requires(T t) { std::cout << t; };

	// custom logging class
	class err
	{
	public:
		err(const std::source_location& source = std::source_location::current());

		// forward stream to std::cerr
		template <Streamable T>
		err operator<<(const T& t) const {
			std::cerr << t;
			return *this;
		}

		err& operator<<(err& (*func)(err& e));

		std::source_location source;
	};

	// custom logs
	// ----------
	// print souce code location
	err& endl(err& e );

	bool readFile(const std::string& path, std::string& out);

	// custom free operator for shared pointers
	struct FreeDelete {
		void operator()(void* x);
	};
}

#endif