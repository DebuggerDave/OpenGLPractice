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
		inline err(const std::source_location& source = std::source_location::current()) : source(source) {}

		// forward stream to std::cerr
		template <Streamable T>
		inline err operator<<(const T& t) const {
			std::cerr << t;
			return *this;
		}

		inline err& operator<<(err& (*func)(err& e)) {
			return func(*this);
		}

		std::source_location source;
	};

	// custom logs
	// ----------
	// print souce code location
	inline err& endl(err& e ) {
		std::cerr << " in '" << e.source.function_name() << ":" << e.source.line() << "'\n";
		return e;
	}

	bool readFile(const std::string& path, std::string& out);

	// custom free operator for shared pointers
	struct FreeDelete {
		void operator()(void* x);
	};
}

#endif