#include "../inc/Buffer.hpp"

Buffer::Buffer(int size)
{
	this->size = size;
	data	   = new unsigned char[this->size];

	return;
}

Buffer::Buffer(const Buffer &buffer)
{
	size = buffer.size;
	delete[] data;
	data = new unsigned char[size];

	for (int i = 0; i < size; i++)
	{
		data[i] = buffer.data[i];
	}

	return;
}

Buffer::Buffer()
{
	data = nullptr;

	return;
}

Buffer::~Buffer()
{
	delete[] data;

	return;
}

void Buffer::operator=(Buffer &buffer)
{
	size = buffer.size;
	delete[] data;
	data = new unsigned char[size];

	for (int i = 0; i < size; i++)
	{
		data[i] = buffer.data[i];
	}

	return;
}
