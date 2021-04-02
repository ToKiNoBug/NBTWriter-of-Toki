#include <iostream>
#include <fstream>
#include <cstring>
using namespace std;
#define TwinStackSize 128
namespace NBT{
	const char idEnd=0;
	const char idByte=1;
	const char idShort=2;
	const char idInt=3;
	const char idLong=4;
	const char idFloat=5;
	const char idDouble=6;
	const char idByteArray=7;
	const char idString=8;
	const char idList=9;
	const char idCompound=10;
	const char idIntArray=11;
	const char idLongArray=12;


template <typename T>
void IE2BE(T &Val)
{
	if (sizeof(T)<=1)return;
	char *s=(char*)&Val;
	for(short i=0;i*2+1<sizeof(T);i++)
	swap(s[i],s[sizeof(T)-1-i]);
}
template <typename T>
T IE2BE(T *Val)
{
	if (sizeof(T)<=1)return *Val;
	
	T Res=*Val;
	char *s=(char*)&Res;
	for(short i=0;i*2+1<sizeof(T);i++)
	swap(s[i],s[sizeof(T)-1-i]);
	return Res;
}

bool isSysBE()
{
	short S=1;char *temp=(char*)&S;
	if(temp[0]==0&&temp[1]==1)
	return true;
	return false;
}

void disp(const char*Str)
{
	cout<<Str<<endl;
	return;
}


class NBTWriter
{
	private:
		//Vars
		bool isOpen;
		bool isBE;
		fstream *File;
		unsigned long long ByteCount;
		short top;
		char CLA[TwinStackSize];
		int Size[TwinStackSize];
		//StackFun
		void pop();
		void push(char typeId,int size);
		bool isEmpty();
		bool isFull();
		char readType();
		char readSize();
		//WriterFun
		void elementWritten();
		void endList();
		int writeEnd();
		bool typeMatch(char typeId);
		//AutoFiller
		int emergencyFill();
	public:
		//Construct&deConstruct
		NBTWriter(const char*path);
		~NBTWriter();
		//Vars
		bool allowEmergencyFill;
		//WriterFun
		bool isInList();
		bool isInCompound();
		unsigned long long close();
		bool isListFinished();
		char CurrentType();
		//WriteAbstractTags
		template <typename T>
		int writeSingleTag(char typeId,const char*Name,T value);
		//int writeArrayHead(char typeId,const char*Name,int arraySize);
		//WriteSpecialTags
		int writeCompound(const char*Name);
		int writeListHead(const char*Name,char typeId,int listSize);
		int endCompound();
		int writeString(const char*Name,const char*value);
		//WriteRealSingleTags
		int writeByte(const char*Name,char value);
		int writeShort(const char*Name,short value);
		int writeInt(const char*Name,int value);
		int writeLong(const char*Name,long long value);
		int writeFloat(const char*Name,float value);
		int writeDouble(const char*Name,double value);
		//WriteArrayHeads
		int writeLongArrayHead(const char*Name,int arraySize);
		int writeByteArrayHead(const char*Name,int arraySize);
		int writeIntArrayHead(const char*Name,int arraySize);
};

NBTWriter::NBTWriter(const char*path)
{
	allowEmergencyFill=true;
	isBE=isSysBE();
	ByteCount=0;
	File=new fstream(path,ios::out|ios::binary);
		char temp[3]={10,0,0};
		File->write(temp,3);ByteCount+=3;
	isOpen=true;	
	for(top=0;top<TwinStackSize;top++)
	{
		CLA[top]=114;
		Size[top]=114514;
	}
	
	top=-1;

}

NBTWriter::~NBTWriter()
{
	if(isOpen)close();
	delete File;
	return;
}

unsigned long long NBTWriter::close()
{
	if(isOpen)
	{
		if(!isEmpty())emergencyFill();
		
	File->write(&idEnd,1);ByteCount+=1;
	File->close();}
	return ByteCount;
}

bool NBTWriter::isEmpty()
{
	return (top==-1);
}

bool NBTWriter::isFull()
{
	return top>=TwinStackSize;
}

bool NBTWriter::isListFinished()
{
	return (Size[top]<=0);
}

char NBTWriter::readType()
{
	return CLA[top];
}

bool NBTWriter::isInCompound()
{
	return isEmpty()||(readType()==0);
}

bool NBTWriter::isInList()
{
	return !isInCompound();
}

bool NBTWriter::typeMatch(char typeId)
{
	return(readType()==typeId);
}

void NBTWriter::endList()
{
	if(isInList()&&isListFinished())
	{
		pop();
		disp("结束了一个列表");
		elementWritten();	
	}
	return;
}

void NBTWriter::pop()
{
	if(!isEmpty()){
	top--;
	disp("popped");}
	else
	disp("pop失败：空栈不能继续pop");
	return;
}

void NBTWriter::push(char typeId,int size)
{
	if(!isFull())
	{
		top++;
		CLA[top]=typeId;
		Size[top]=size;
		cout<<"push成功，栈顶CLA="<<(short)CLA[top]<<"，Size="<<Size[top]<<endl;
	}
	else
	disp("push失败，满栈不能继续push");
	return;
}

void NBTWriter:: elementWritten()
{
	if(isInList()&&!isListFinished())
	Size[top]--;
	if(isListFinished())
	endList();
	return;
}

int NBTWriter::writeEnd()
{
	File->write(&idEnd,1);
	return 1;
}

template <typename T>
int NBTWriter::writeSingleTag(char typeId,const char*Name,T value)
{
	if (!isOpen){disp("文件没有打开，不能写入");return 0;}
	int ThisCount=0;short realNameL=strlen(Name),writeNameL=realNameL;
	if(!isBE)
	{
		IE2BE(writeNameL);IE2BE(value);//value不需要读取，只需要写入 
	}
	
	
	if (isInCompound())//写入为完整的Tag 
	{
		cout<<"写入为文件夹内的id"<<(short)typeId<<endl; 
		File->write(&typeId,sizeof(char));ThisCount+=sizeof(char);
		File->write((char*)&writeNameL,sizeof(short));ThisCount+=sizeof(short);
		File->write(Name,realNameL);ThisCount+=realNameL;
		File->write((char*)&value,sizeof(T));ThisCount+=sizeof(T);
	}
	
	if (isInList()&&typeMatch(typeId))//写入为列表中的tag 
	{
		cout<<"写入为列表中的id"<<(short)typeId<<endl;
		File->write((char*)&value,sizeof(T));ThisCount+=sizeof(T);
		elementWritten();
		
	}
	ByteCount+=ThisCount;
	if(!ThisCount)disp("writeSingleTag函数写入失败"); 
	return ThisCount;
}

int NBTWriter::writeCompound(const char*Name)
{
	if (!isOpen)return 0;
	int ThisCount=0;short realNameL=strlen(Name),writeNameL=realNameL;
	if(!isBE)
	{
		IE2BE(writeNameL); 
	}
	if(isInCompound())
	{
		File->write(&idCompound,sizeof(char));ThisCount+=sizeof(char);
		File->write((char*)&writeNameL,sizeof(short));ThisCount+=sizeof(short);
		File->write(Name,realNameL);ThisCount+=realNameL;
		push(idEnd,0);
		ByteCount+=ThisCount;
		return ThisCount;
	}
	
	if (isInList()&&typeMatch(idCompound))//写入为列表中的tag
	{
		//writeNothing
		push(idEnd,0);
		ByteCount+=ThisCount;
		return ThisCount;
	}
	
	cout<<"条件不符合，没有写入Compound\n";
	ByteCount+=ThisCount;
	return ThisCount;
	
}

int NBTWriter::endCompound()
{
	if(!isOpen)return 0;
	int ThisCount=0;
	if(isInCompound())
	{
		ThisCount+=writeEnd();
		pop();//This pop means end a Compound
		elementWritten();//检查pop之后是否在List内 
		ByteCount+=ThisCount;
		disp("结束了一个文件夹");
		return ThisCount;
	}
	disp("错误：这里不能结束文件夹"); 
	return ThisCount;
}

int NBTWriter::writeListHead(const char*Name,char TypeId,int listSize)
{
	if(!isOpen){disp("失败：文件未打开");return 0;}
	int ThisCount=0;short realNameL=strlen(Name),writeNameL=realNameL;
	int writeListSize=listSize;//listSize->readListSize
	if(!isBE){IE2BE(writeNameL);IE2BE(writeListSize);}
	
	if(isInCompound())
	{
		File->write(&idList,sizeof(char));ThisCount+=sizeof(char);
		File->write((char*)&writeNameL,sizeof(short));ThisCount+=sizeof(short);
		File->write(Name,realNameL);ThisCount+=realNameL;
		File->write(&TypeId,sizeof(char));ThisCount+=sizeof(char);
		File->write((char*)&writeListSize,sizeof(int));ThisCount+=sizeof(int);
		push(TypeId,listSize);
		disp("成功在文件夹中创建List");
		ByteCount+=ThisCount;
		if(listSize==0)elementWritten();//开始写入空列表等同于完成写入空列表 
		return ThisCount;
	}
	
	if(isInList()&&typeMatch(idList))
	{
		File->write(&TypeId,sizeof(char));ThisCount+=sizeof(char);
		File->write((char*)&writeListSize,sizeof(int));ThisCount+=sizeof(int);
		push(TypeId,listSize);
		disp("成功在List中创建List");
		ByteCount+=ThisCount;
		if(listSize==0)elementWritten();//开始写入空列表等同于完成写入空列表
		return ThisCount;
	}
	disp("失败：未能创建List");
	return ThisCount;
	
}

int NBTWriter::writeByte(const char*Name,char value)
{
	return writeSingleTag(idByte,Name,value);
}

int NBTWriter::writeShort(const char*Name,short value)
{
	return writeSingleTag(idShort,Name,value);
}

int NBTWriter::writeInt(const char*Name,int value)
{
	return writeSingleTag(idInt,Name,value);
}

int NBTWriter::writeLong(const char*Name,long long value)
{
	return writeSingleTag(idLong,Name,value);
}

int NBTWriter::writeFloat(const char*Name,float value)
{
	return writeSingleTag(idFloat,Name,value);
}

int NBTWriter::writeDouble(const char*Name,double value)
{
	return writeSingleTag(idDouble,Name,value);
}

int NBTWriter::writeLongArrayHead(const char*Name,int arraySize)
{
	if(!isOpen){disp("失败：文件未打开");return 0;}
	int ThisCount=0;short realNameL=strlen(Name),writeNameL=realNameL;
	int writeArraySize=arraySize;//arraSize->readArraySize
	if(!isBE){IE2BE(writeNameL);IE2BE(writeArraySize);}
	
	if(isInCompound())
	{
		File->write(&idLongArray,sizeof(char));ThisCount+=sizeof(char);
		File->write((char*)&writeNameL,sizeof(short));ThisCount+=sizeof(short);
		File->write(Name,realNameL);ThisCount+=realNameL;
		//File->write(&idLong,sizeof(char));ThisCount+=sizeof(char);
		File->write((char*)&writeArraySize,sizeof(int));ThisCount+=sizeof(int);
		push(idLong,arraySize);
		disp("成功在文件夹中创建LongArray");
		ByteCount+=ThisCount;
		if(arraySize==0)elementWritten();//开始写入空列表等同于完成写入空列表
		return ThisCount;
	}
	
	if(isInList()&&typeMatch(idLongArray))
	{
		//File->write(&idLong,sizeof(char));ThisCount+=sizeof(char);
		File->write((char*)&writeArraySize,sizeof(int));ThisCount+=sizeof(int);
		push(idLong,arraySize);
		disp("成功在List中创建LongArray");
		ByteCount+=ThisCount;
		if(arraySize==0)elementWritten();//开始写入空列表等同于完成写入空列表
		return ThisCount;
	}
	disp("失败：未能创建LongArray");
	return ThisCount;
}

int NBTWriter::writeByteArrayHead(const char*Name,int arraySize)
{
	if(!isOpen){disp("失败：文件未打开");return 0;}
	int ThisCount=0;short realNameL=strlen(Name),writeNameL=realNameL;
	int writeArraySize=arraySize;//arraSize->readArraySize
	if(!isBE){IE2BE(writeNameL);IE2BE(writeArraySize);}
	
	if(isInCompound())
	{
		File->write(&idByteArray,sizeof(char));ThisCount+=sizeof(char);
		File->write((char*)&writeNameL,sizeof(short));ThisCount+=sizeof(short);
		File->write(Name,realNameL);ThisCount+=realNameL;
		//File->write(&idLong,sizeof(char));ThisCount+=sizeof(char);
		File->write((char*)&writeArraySize,sizeof(int));ThisCount+=sizeof(int);
		push(idByte,arraySize);
		disp("成功在文件夹中创建ByteArray");
		ByteCount+=ThisCount;
		if(arraySize==0)elementWritten();//开始写入空列表等同于完成写入空列表
		return ThisCount;
	}
	
	if(isInList()&&typeMatch(idByteArray))
	{
		//File->write(&idLong,sizeof(char));ThisCount+=sizeof(char);
		File->write((char*)&writeArraySize,sizeof(int));ThisCount+=sizeof(int);
		push(idByte,arraySize);
		disp("成功在List中创建ByteArray");
		ByteCount+=ThisCount;
		if(arraySize==0)elementWritten();//开始写入空列表等同于完成写入空列表
		return ThisCount;
	}
	disp("失败：未能创建ByteArray");
	return ThisCount;
}

int NBTWriter::writeIntArrayHead(const char*Name,int arraySize)
{
	if(!isOpen){disp("失败：文件未打开");return 0;}
	int ThisCount=0;short realNameL=strlen(Name),writeNameL=realNameL;
	int writeArraySize=arraySize;//arraySize->readArraySize
	if(!isBE){IE2BE(writeNameL);IE2BE(writeArraySize);}
	
	if(isInCompound())
	{
		File->write(&idIntArray,sizeof(char));ThisCount+=sizeof(char);
		File->write((char*)&writeNameL,sizeof(short));ThisCount+=sizeof(short);
		File->write(Name,realNameL);ThisCount+=realNameL;
		
		File->write((char*)&writeArraySize,sizeof(int));ThisCount+=sizeof(int);
		push(idInt,arraySize);
		disp("成功在文件夹中创建IntArray");
		ByteCount+=ThisCount;
		if(arraySize==0)elementWritten();//开始写入空列表等同于完成写入空列表
		return ThisCount;
	}
	
	if(isInList()&&typeMatch(idIntArray))
	{
		//File->write(&idLong,sizeof(char));ThisCount+=sizeof(char);
		File->write((char*)&writeArraySize,sizeof(int));ThisCount+=sizeof(int);
		push(idInt,arraySize);
		disp("成功在List中创建IntArray");
		ByteCount+=ThisCount;
		if(arraySize==0)elementWritten();//开始写入空列表等同于完成写入空列表
		return ThisCount;
	}
	disp("失败：未能创建IntArray");
	return ThisCount;
}

int NBTWriter::writeString(const char*Name,const char*value)
{
	if(!isOpen){disp("失败：文件未打开");return 0;}
	int ThisCount=0;
	short realNameL=strlen(Name),writeNameL=realNameL;
	short realValL=strlen(value),writeValL=realValL;	
	if(!isBE){IE2BE(writeNameL);IE2BE(writeValL);}
	
	if(isInCompound())
	{
		File->write(&idString,sizeof(char));ThisCount+=sizeof(char);
		File->write((char*)&writeNameL,sizeof(short));ThisCount+=sizeof(short);
		File->write(Name,realNameL);ThisCount+=realNameL;
		File->write((char*)&writeValL,sizeof(short));ThisCount+=sizeof(short);
		File->write(value,realValL);ThisCount+=realValL;
		disp("成功在文件夹中创建String");
		ByteCount+=ThisCount;
		elementWritten();
		return ThisCount;
	}
	
	if(isInList()&&typeMatch(idString))
	{
		File->write((char*)&writeValL,sizeof(short));ThisCount+=sizeof(short);
		File->write(value,realValL);ThisCount+=realValL;
		disp("成功在List中创建String");
		ByteCount+=ThisCount;
		elementWritten();
		return ThisCount;
	}
	disp("失败：未能创建String");
	return ThisCount;
}

char NBTWriter::CurrentType()
{
	return readType();
}

int NBTWriter::emergencyFill()
{
	if(!allowEmergencyFill)return 0;
	if(isEmpty())return 0;
	int ThisCount=0;
	while(!isEmpty())
	{
		if(isInCompound()){ThisCount+=endCompound();continue;}
		//所以这里必然在List或者列表里
		switch (readType())
		{
			case idEnd:
				cout<<"你为什么要创建一个tag_End的列表？这里我不管了！\n";
				continue;
			case idByte:
				cout<<"emergencyFill写入了Byte\n";
				ThisCount+=writeByte("autoByte",114);
				continue;
			case idShort:
				cout<<"emergencyFill写入了Short\n";
				ThisCount+=writeShort("autoShort",514);
				continue;
			case idInt:
				cout<<"emergencyFill写入了Int\n";
				ThisCount+=writeInt("autoInt",114514);
				continue;
			case idLong:
				cout<<"emergencyFill写入了Long\n";
				ThisCount+=writeLong("autoLong",1919810);
				continue;
			case idFloat:
				cout<<"emergencyFill写入了Float\n";
				ThisCount+=writeFloat("autoFloat",114.514f);
				continue;
			case idDouble:
				cout<<"emergencyFill写入了Double\n";
				ThisCount+=writeDouble("autoDouble",1919810.114514d);
				continue;
			case idByteArray:
				cout<<"emergencyFill写入了ByteArray\n";
				ThisCount+=writeByteArrayHead("autoByteArray",1);
				continue;
			case idString:
				cout<<"emergencyFill写入了String\n";
				ThisCount+=writeString("autoString","FuckYUUUUUUUUUUUUUUUUU!");
				continue;
			case idList:
				cout<<"emergencyFill写入了List\n";
				ThisCount+=writeListHead("autoList",NBT::idInt,1);
				continue;
			case idCompound:
				cout<<"emergencyFill写入了Compound\n";
				ThisCount+=writeCompound("autoCompound");
				continue;
			case idIntArray:
				cout<<"emergencyFill写入了IntArray\n";
				ThisCount+=writeIntArrayHead("autoIntArray",1);
				continue;
			case idLongArray:
				cout<<"emergencyFill写入了LongArray\n";
				ThisCount+=writeLongArrayHead("autoLongArray",1);
				continue;
			default:
				cout<<"我滴妈，你咋搞出来个不存在的id？管不了！\n";
				continue;
		}
	}
	ThisCount+=writeString("TokiNoBug'sWarning","There's sth wrong with ur NBTWriter, the file format is completed automatically instead of manually.");
	disp("紧急补全格式成功");
	return ThisCount;
}

//NameSpace NBT ends here
}
