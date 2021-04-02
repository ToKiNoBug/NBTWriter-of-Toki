#include <iostream>
#include "NBTWriter.h"

using namespace std;
using namespace NBT;

int main()
{
	
	NBTWriter W("Test.nbt");
			W.writeCompound("This is a Compound");
				W.writeListHead("This is a List",NBT::idString,4);
					W.writeString("1","This File is Written by Toki's NBTWriter");
					W.writeString("2","Now I'm filling a List containing 4 String");
					W.writeString("3","Here I will end the program without finishing this List, see what will happen");
	W.close();
	return 0;
}
