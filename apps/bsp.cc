#include <iostream>

#include "bsp.hh"

int main()
{
	BSPTree tree(10 * 10 * 10);
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 10; j++)
			for (int k = 0; k < 10; k++)
				tree.insert(0, 0, 0);
	std::cout << tree.num << std::endl;
}
