#include "../inc/Buffer.hpp"

#include <stdio.h>

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

void Buffer::printBits()
{
	for (int x = 0; x < size; x++)
	{
		for (int i = 0; i < 8; i++)
		{
			printf("%d", !!((data[x] << i) & 0x80));
		}

		printf(" ");
	}

	printf("\n");
}
