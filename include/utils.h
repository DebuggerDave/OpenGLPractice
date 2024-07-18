#ifndef OPENGL_PRACTICE_UTILS_H
#define OPENGL_PRACTICE_UTILS_H

#include <iostream>
#include <source_location>
#include <concepts>
#include <string>
#include <fstream>
#include <vector>

#define STRINGIFY_MACRO_EXPANSION(x) #x
#define STRINGIFY(x) STRINGIFY_MACRO_EXPANSION(x)
#define LOG(x) utils::err() << x << utils::endl;

namespace utils
{
	template <typename T>
	concept Streamable = requires(T t) { std::cout << t; };
	template <typename T>
	concept FStreamable = requires(T t) { std::fstream{} << t; };

	class ScopedDeleter
	{
	public:
		explicit ScopedDeleter(void (*deleter)()) noexcept;
		~ScopedDeleter();
		void removeDeleter();
	private:
		void (*deleter)() = nullptr;
	};

	// fast way to delete in O(c) time instead of O(n) time, does not maintain order
	template <typename T>
	void vecSwapPopBack(std::vector<T>& vec, const size_t index)
	{
		if (index >= vec.size())
			return;

		if ((index == (vec.size() - 1)) || (vec.size() == 1)) {
			vec.pop_back();
		} else {
			vec[index] = std::move(vec.back());
			vec.pop_back();
		}
	}

	float min(float x, float y);
	float max(float x, float y);

	// return the sign of a number
	int sign(float x);

	// read the file at path and output it to out
	bool readFile(const std::string& path, std::string& out);

	// custom free operator for shared pointers
	struct FreeDelete
	{
		void operator()(void *x);
	};

	// ---- custom logging ----
	class err
	{
	public:
		err(const std::source_location& source = std::source_location::current()) noexcept;

		// forward to std::cerr
		template <Streamable T>
		err operator<<(const T& t) const
		{
			std::cerr << t;
			return *this;
		}

		err& operator<<(err& (*func)(err& e));

		std::source_location source;
	};

	// print souce code location
	err& endl(err& e);

	// I don't want users to see the actual class
	namespace privy
	{
		class Log
		{
		public:
			Log() noexcept;
			// forward to std::ofstream
			template <FStreamable T>
			Log& operator<<(const T& t)
			{
				if (stream.is_open()) {
					stream << t;
				} else {
					LOG("Failed to open log file.");
				}

				return *this;
			}

		private:
			inline static const std::string log_path = "./log.txt";
			std::ofstream stream;
		};
	}
	static privy::Log log = {};
}

#endif