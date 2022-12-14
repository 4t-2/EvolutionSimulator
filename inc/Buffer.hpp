#pragma once

class Buffer
{
	public:
		unsigned char *data = nullptr;
		int			   size;

		Buffer(int size);
		Buffer(const Buffer &buffer);
		Buffer();
		~Buffer();

		void operator=(Buffer &buffer);

		void printBits();
};
