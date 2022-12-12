#pragma once

class Buffer
{
	public:
		Buffer(int size);
		Buffer(const Buffer&buffer);
		Buffer();
		~Buffer();

		void operator=(Buffer &buffer);

		unsigned char *data = nullptr;
		int			   size;
};
