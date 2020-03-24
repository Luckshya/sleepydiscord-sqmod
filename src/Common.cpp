#include "Common.hpp"

#include <regex>

using namespace SqMod;

namespace SqDiscord
{
	struct myEnum {
		const char* identifier;
		int value;
	};

	static const myEnum discord_Events[] = {
		{ "Ready", ON_READY },
		{ "Message", ON_MESSAGE }
	};

	void DRegisterEnum(HSQUIRRELVM vm, const char* name, const myEnum * data, int count) {
		using namespace Sqrat;

		Enumeration e(vm);

		for (int n = 0; n < count; ++n, ++data) {
			e.Const(data->identifier, data->value);
		}

		ConstTable(vm).Enum(name, e);
	}

	void DRegister_Constants(Sqrat::Table& discordcn) {
		DRegisterEnum(discordcn.GetVM(), "SqDiscordEvent", discord_Events, 2);
	}

	SQInteger Regex_Match(HSQUIRRELVM vm) {
		const int top = sq_gettop(vm);

		if (top <= 1)
		{
			return sq_throwerror(vm, "Missing the regex string");
		}

		else if (top <= 2)
		{
			return sq_throwerror(vm, "Missing the string value");
		}

		StackStrF regex_string(vm, 2);
		if (SQ_FAILED(regex_string.Proc(false)))
		{
			return regex_string.mRes; // Propagate the error!
		}

		StackStrF value_string(vm, 3);
		if (SQ_FAILED(value_string.Proc(true)))
		{
			return value_string.mRes; // Propagate the error!
		}

		try {
			std::regex r(regex_string.mPtr);
			std::string s(value_string.mPtr);

			std::smatch string_flag;

			bool matches = std::regex_match(s, r);

			sq_pushbool(vm, matches);
		}
		catch (std::regex_error& e) {
			return sq_throwerror(vm, e.what());
		}
		catch (...) {
			return sq_throwerror(vm, "unknown error");
		}

		return 1;
	}
}