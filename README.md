# NBTWriter-of-Toki
C++ NBT format writer library.<br>
<br>
## NBTWriter简介
是一个C++下写入未压缩nbt格式文件的库。它的功能非常简单————我需要生成投影文件，所以额外写了这它；结构也非常简单，只有一个头文件。<br>
NBT（二进制命名标签，Named Binary Tags） 格式为Minecraft中用于向文件中存储数据的一种存储格式。<br>
如果你不了解NBT格式，请翻wiki。<br>
<br>
## NBTWriter的优点和缺点？
#### 优点
在开发时我特别注意写出来的Nbt文件不能乱码。因此，无论你在调用的时候如何胡乱的写入格式，它都能保证写出来的文件至少能被正常读取，不会乱码。这应该会在debug中起到一些帮助。<br>
如果你在调用时违背了nbt的格式，那么nbtwriter不会按照你的要求写入错误的数据，而是什么都不会做。<br>
另外，如果你在还没有写完nbt格式就错误的停止写入、关闭文件，那么nbtwriter会自动补全格式，避免乱码。这个功能叫做**紧急补全格式**<br>
#### 缺点
功能比较少。只能写入未压缩的nbt文件，要想获得压缩的文件还需gzip；<br>
只能写入，不能读取，因为我目前用不到读取的功能；<br>
写入速度不会是最快的，因为我写了多层安全检查。<br>

## NBTWriter有哪些内容？
1.NBTWriter类，这是核心部分；<br>
2.isSystemBE函数，它用来判断系统是大端序还是小端序。（NBT格式只允许大端序，但大部分系统似乎都是小端序）<br>
3.IE2BE函数，用来将小端序的变量翻转为大端序。<br>
4.disp函数，用来输出运行状况的，没啥用，下个版本就删掉。<br>

#### NBTWriter类
NBTWriter类中你需要调用的公有成员（及函数）都有如下：<br>
```cpp
		NBTWriter(const char*path);//构造函数，构造时请给出文件的路径和地址
		~NBTWriter();
		//公有变量
		bool allowEmergencyFill;//是否允许使用紧急补全功能
		//WriterFun
		bool isInList();//判断目前是不是在向列表或者数组中写入东西
		bool isInCompound();//判断目前是不是在向Compound（文件夹？）中写入东西
		unsigned long long close();//关闭文档，返回总共写入的字节数。空的nbt文件有4个字节，所以关闭空文档会返回4
		bool isListFinished();//判断当前写入的列表是否已经被填满
		char CurrentType();//获取当前允许写入的tag类型（id）。如果不受限，那么返回0。只有List和Array会限制类型，Compound不限制类型。
		//WriteAbstractTags
		template <typename T>
		int writeSingleTag(char typeId,const char*Name,T value);//用来写入任何类型的简单标签（仅限Byte,Short,Int,Long,Float和Double），返回写入的字节数。
    //Name为tag名称，如果是在List或者Array中写入，那么Name无效，下同
		//int writeArrayHead(char typeId,const char*Name,int arraySize);//这个是用来写入任何类型的数组，但实际没用到，所以注释掉了
		//WriteSpecialTags
		int writeCompound(const char*Name);//开启一个Compound
		int writeListHead(const char*Name,char typeId,int listSize);//开启一个列表。Name为列表名称，typeId是列表包含的数据类型（列表只能容纳一种数据类型），listSize为列表的长度（元素数量）
		int endCompound();//结束Compound。（只有Compound需要手动结束，List和Array在写满之后自动结束
		int writeString(const char*Name,const char*value);//写入字符串
		//WriteRealSingleTags
		int writeByte(const char*Name,char value);//写入Byte标签（int8）
		int writeShort(const char*Name,short value);//写入Short标签（int16）
		int writeInt(const char*Name,int value);//写入Int标签（int32）
		int writeLong(const char*Name,long long value);//写入Long标签（int64）
		int writeFloat(const char*Name,float value);//写入Float标签（单精度浮点）
		int writeDouble(const char*Name,double value);//写入Double标签（双精度浮点
		//WriteArrayHeads
		int writeLongArrayHead(const char*Name,int arraySize);//开启Long数组，arraySize为数组元素个数
		int writeByteArrayHead(const char*Name,int arraySize);//开启Byte数组，arraySize为数组元素个数
		int writeIntArrayHead(const char*Name,int arraySize);//开启Int数组，arraySize为数组元素个数
```
<br>
<br>

**注意：因为List和Array类似于有序、定长的数组，所以一个List或者Array写满之后，NBTWriter会自动反应过来，哪怕是多个List和Compound循环套娃也能处理！**<br>
**只有Compound结构最为灵活，需要调用endCompound函数手动结束一个Compound！**
