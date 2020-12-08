#include <string>

namespace BlockNDN
{
	class SerialUtils
	{
	private:
	public:
		const char* port = "/dev/ttyUSB0";
		SerialUtils();
        std::string SerialRead(int len);
        void SerialWrite(char prefix[], size_t len);
	};
}