#pragma once

template <typename T> class List
{
	private:
		T	*data;

	public:
		int length = 0;
		
		List(int max)
		{
			data = new T[max];

			for(int i = 0 ; i < max; i++)
			{
				data[i] = 0;
			}
		}

		~List()
		{
			delete[] data;
		}

		void add(T data)
		{
			this->data[length] = data;
			length++;
		}

		void pop(int index)
		{
			data[index] = 0;

			for(int i = index+1; i < length; i++)
			{
				data[i-1] = data[i];
			}

			length--;

			return;
		}

		int find(T item)
		{
			for(int i = 0; i < length; i++)
			{
				if(item == data[i])
				{
					return i;
				}
			}

			return -1;
		}

		void clear()
		{
			length = 0;
		}

		T get(int index)
		{
			return data[index];
		}
};
