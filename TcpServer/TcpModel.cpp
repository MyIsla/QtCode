#pragma execution_character_set("utf-8")
#include "TcpModel.h"

/// <summary>
/// 创建TcpModel
/// </summary>
/// <param name="socketDescriptor">客户端套接字指针</param>
TcpModel::TcpModel(qintptr socketDescriptor)
{
	this->socketDescriptor = socketDescriptor;
	socket = new QTcpSocket();
	socket->setSocketDescriptor(socketDescriptor);
	qDebug() << "客户端" + socket->peerAddress().toString() + "连接";
	createConnections();
	sendMsgToClient("connected");
}

TcpModel::~TcpModel()
{
	qDebug() << "IP地址为:" + socket->peerAddress().toString() + "的客户端断开连接";
	delete socket;
	socket = nullptr;
}

/// <summary>
/// 发送消息
/// </summary>
/// <param name="msg">内容</param>
/// <param name="state">阶段，默认 = 0</param>
void TcpModel::sendMsgToClient(QString msg, int state)
{
	qDebug() << "Server send:" << msg << " The state:" << QString::number(state);
	QJsonObject jsonObj;
	jsonObj.insert("type", "message");
	jsonObj.insert("filename", "");
	jsonObj.insert("data", msg);
	jsonObj.insert("filesize", QString::number(msg.size()));
	jsonObj.insert("state", QString::number(state));
	jsonObj.insert("md5", QString(QCryptographicHash::hash(msg.toUtf8(), QCryptographicHash::Md5).toHex()));
	QJsonDocument jsonDoc(jsonObj);
	QByteArray buf = jsonDoc.toJson();
	socket->write(buf);
	socket->flush();
}

/// <summary>
/// 发送文件(先读入sendList，再发送头)
/// </summary>
/// <param name="filePath">文件路径</param>
void TcpModel::sendFileToClient(QString filePath)
{
	//读取文件信息
	QFileInfo fileInfo(filePath);
	QString type = fileInfo.suffix();
	QString fileName = fileInfo.baseName();
	//查看是否在列表中
	for (int i = 0; i < sendList.count(); i++)
	{
		if (sendList[i]->type == type && sendList[i]->fileName == fileName)
			return;
	}
	//创建在内存中
	QFile file;
	file.setFileName(filePath);
	file.open(QIODevice::ReadOnly);
	QByteArray buf = file.readAll();
	file.close();
	TcpData* data = new TcpData(type, fileName, file.size(), "", 0, buf);
	connect(data, SIGNAL(clearData(TcpData*)), this, SLOT(clearMember(TcpData*)));
	sendList.append(data);
	sendHeadToClient(type, fileName, file.size(), 0);
}

/// <summary>
/// 创建连接
/// </summary>
/// <param name="socket">套接字</param>
void TcpModel::createConnections()
{
	connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
	connect(socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
}

/// <summary>
/// 发送Json头
/// </summary>
/// <param name="type">文件类型</param>
/// <param name="fileName">文件名</param>
/// <param name="fileSize">文件大小</param>
/// <param name="state">状态</param>
/// <param name="md5">md5</param>
void TcpModel::sendHeadToClient(QString type, QString fileName, quint64 fileSize, int state, QString md5)
{
	//服务器为发送头的状态
	if (state == 0)
	{
		bool get = false;
		for (int i = 0; i < sendList.count(); i++)
		{
			if (sendList[i]->type == type && sendList[i]->fileName == fileName &&
				sendList[i]->fileSize == fileSize)
			{
				QByteArray block = {};
				if (sendList[i]->nowSize + TCPMODEL_BUF_SIZE > sendList[i]->fileSize)
					block = sendList[i]->buf.mid(sendList[i]->nowSize, sendList[i]->fileSize - sendList[i]->nowSize);
				else
					block = sendList[i]->buf.mid(sendList[i]->nowSize, TCPMODEL_BUF_SIZE);
				md5 = QString(QCryptographicHash::hash(block, QCryptographicHash::Md5).toHex());
				get = true;
				break;
			}
		}
		if (!get) return;
	}
	QJsonObject jsonObj;
	jsonObj.insert("type", type);
	jsonObj.insert("filename", fileName);
	jsonObj.insert("data", "");
	jsonObj.insert("filesize", QString::number(fileSize));
	jsonObj.insert("state", QString::number(state));
	jsonObj.insert("md5", md5);
	QJsonDocument jsonDoc(jsonObj);
	QByteArray buf = jsonDoc.toJson();
	socket->write(buf);
	socket->flush();
}

/// <summary>
/// 发送数据
/// </summary>
/// <param name="type">类型</param>
/// <param name="fileName">文件名</param>
/// <param name="fileSize">文件大小</param>
void TcpModel::sendDataToClient(QString type, QString fileName, quint64 fileSize)
{
	for (int i = 0; i < sendList.count(); i++)
	{
		if (sendList[i]->type == type && sendList[i]->fileName == fileName &&
			sendList[i]->fileSize == fileSize)
		{
			sendList[i]->resetDataTime();
			QByteArray buf = {};
			if (sendList[i]->nowSize + TCPMODEL_BUF_SIZE > sendList[i]->fileSize)
				buf = sendList[i]->buf.mid(sendList[i]->nowSize, sendList[i]->fileSize - sendList[i]->nowSize);
			else
			{
				buf = sendList[i]->buf.mid(sendList[i]->nowSize, TCPMODEL_BUF_SIZE);
			}
			sendList[i]->nowSize += TCPMODEL_BUF_SIZE;
			if (sendList[i]->nowSize >= sendList[i]->fileSize)
			{
				clearMember(sendList[i]);
			}
			socket->write(buf);
			socket->flush();
			return;
		}
	}

}

/// <summary>
/// 对接到的json数据进行二次解析
/// </summary>
/// <param name="obj">Json对象</param>
void TcpModel::receJsonFromClient(QJsonObject obj)
{
	//没有type字段
	if (!obj.contains("type")) return;
	QString type = obj.value("type").toString();
	//没有filename字段
	if (!obj.contains("filename")) return;
	QString filename = obj.value("filename").toString();
	//没有data字段
	if (!obj.contains("data")) return;
	QString data = obj.value("data").toString();
	//没有filesize字段
	if (!obj.contains("filesize")) return;
	quint64 filesize = obj.value("filesize").toString().toInt();
	//没有state字段
	if (!obj.contains("state")) return;
	int state = obj.value("state").toString().toInt();
	//没有md5字段
	if (!obj.contains("md5"))return;
	QString md5 = obj.value("md5").toString();
	//判断是消息还是文件
	if (type == "message")
	{
		receMsgFromClient(data, filename, md5, state);
		return;
	}
	//判断此条消息的状态
	switch (state)
	{
	case 0: receHeadFromClient(type, filename, filesize, md5); break;//接收到头文件
	case 1:	sendDataToClient(type, filename, filesize); break;//发数据
	case 2: sendHeadToClient(type, filename, filesize, 0); break;//发送下一个头文件
	default:
		break;
	}
}

/// <summary>
/// 解析Json得到的是头
/// </summary>
/// <param name="type">类型</param>
/// <param name="fileName">文件名/设备名</param>
/// <param name="fileSize">文件大小</param>
/// <param name="md5">md5</param>
void TcpModel::receHeadFromClient(QString type, QString fileName, quint64 fileSize, QString md5)
{
	//查看接收的列表中有无记录
	for (int i = 0; i < receList.count(); i++)
	{
		if (receList[i]->type == type && receList[i]->fileName == fileName &&
			receList[i]->fileSize == fileSize)
		{
			//有记录，更改MD5为了使下一次发送来的数据可以进行验证
			receList[i]->md5 = md5;
			sendHeadToClient(receList[i]->type, receList[i]->fileName,
				receList[i]->fileSize, 1, receList[i]->md5);
			return;
		}
	}
	//无记录，新建记录
	TcpData* data = new TcpData(type, fileName, fileSize, md5);
	connect(data, SIGNAL(clearData(TcpData*)), this, SLOT(clearMember(TcpData*)));
	receList.append(data);
	//列表中的最后一个
	sendHeadToClient(receList[receList.count() - 1]->type, receList[receList.count() - 1]->fileName,
		receList[receList.count() - 1]->fileSize, 1, receList[receList.count() - 1]->md5);
}

/// <summary>
/// 解析Json得到的是消息
/// </summary>
/// <param name="msg">消息</param>
/// <param name="name"></param>
/// <param name="md5"></param>
/// <param name="state"></param>
void TcpModel::receMsgFromClient(QString msg, QString name, QString md5, int state)
{
	qDebug() << "Client send:" << msg << " The state:" << QString::number(state);
#if TCPMODEL_MD5_CHECK
	if (QString(QCryptographicHash::hash(msg.toUtf8(),QCryptographicHash::Md5).toHex()) != md5)
	{
		sendMsgToClient("md5 error");
		qDebug() << "md5校验错误";
		return;
	}
#endif // TCPMODEL_MD5_CHECK
	if (state == 0)
	{
		sendMsgToClient(msg, 1);
		analysisData(name, msg.toUtf8());
	}
	else if (state == 1)
	{
		sendMsgToClient(msg, 2);
		return;
	}
}

/// <summary>
/// 接到数据
/// </summary>
void TcpModel::receDataFromClient(QByteArray buf)
{
	qDebug() << "Clinet buf size:" << buf.size();
	bool get = false;
	for (int i = 0; i < receList.count(); i++)
	{
		qDebug() << "第" + QString::number(i) + "次:";
		qDebug() << "head md5:" + receList[i]->md5;
		qDebug() << "now md5:" + QString(QCryptographicHash::hash(buf, QCryptographicHash::Md5).toHex());
		if (receList[i]->md5 == QString(QCryptographicHash::hash(buf, QCryptographicHash::Md5).toHex()))
		{
			//qDebug() << "head md5:" + receList[i]->md5;
			//qDebug() << "now md5:" + QString(QCryptographicHash::hash(buf, QCryptographicHash::Md5).toHex());
			//qDebug() << "bufsize:" + QString::number(buf.size());
			//qDebug() << "data:" + buf;
			receList[i]->resetDataTime();
			receList[i]->nowSize += buf.size();
			receList[i]->buf += buf;
			get = true;
			sendHeadToClient(receList[i]->type, receList[i]->fileName,
				receList[i]->fileSize, 2, receList[i]->md5);
			if (receList[i]->fileSize <= receList[i]->nowSize)
			{
				analysisData(receList[i]->fileName, receList[i]->buf, receList[i]->type);
				clearMember(receList[i]);
			}
			break;
		}
	}
	qDebug() << " \n";
	if (!get)
	{
		qDebug() << "md5错误";
		return;
	}
}

/// <summary>
/// 初步解析数据
/// </summary>
/// <param name="type">类型</param>
/// <param name="fileName">名字</param>
/// <param name="buf">数据</param>
void TcpModel::analysisData(QString fileName, QByteArray buf, QString type)
{
	if (type == "message")
		analysisMSG(fileName, buf);
	else if (type == "bmp")
		analysisBMP(fileName, buf);
	else if (type == "png")
		analysisPNG(fileName, buf);
	else if (type == "csv")
		analysisCSV(fileName, buf);
	else if (type == "json")
		analysisJson(fileName, buf);
}

/// <summary>
/// 解析信息
/// </summary>
/// <param name="fileName">设备名/文件名</param>
/// <param name="buf">数据</param>
void TcpModel::analysisMSG(QString fileName, QString msg)
{
	emit sendMsgToThread(msg);
}

/// <summary>
/// 解析bmp数据
/// </summary>
/// <param name="fileName">设备名/文件名</param>
/// <param name="buf">数据</param>
void TcpModel::analysisBMP(QString fileName, QByteArray buf)
{
	QBitmap b;
	b.loadFromData(buf);
	QStringList stringList = fileName.split("_");
	QString num = stringList[0].replace(QRegExp("[a-z]|[A-Z]"), "");
}

/// <summary>
/// 解析png数据
/// </summary>
/// <param name="fileName">设备名/文件名</param>
/// <param name="buf">数据</param>
void TcpModel::analysisPNG(QString fileName, QByteArray buf)
{
	QPixmap p;
	p.loadFromData(buf);
	QStringList stringList = fileName.split("_");
	QString num = stringList[0].replace(QRegExp("[a-z]|[A-Z]"), "").toInt();
}

/// <summary>
/// 解析csv数据
/// </summary>
/// <param name="fileName">设备名/文件名</param>
/// <param name="buf">数据</param>
void TcpModel::analysisCSV(QString fileName, QByteArray buf)
{
	buf.replace("\r", "");
	QString path = QApplication::applicationDirPath() +  "/csv.csv";
	QFile f(path);
	if (f.exists()) f.remove();
	if (!f.open(QIODevice::WriteOnly))
	{
		qDebug() << "打开失败";
	}
	f.write(buf,buf.size());
	f.close();
}

/// <summary>
/// 解析json数据
/// </summary>
/// <param name="fileName">设备名/文件名</param>
/// <param name="buf">数据</param>
void TcpModel::analysisJson(QString fileName, QByteArray buf)
{
	QString path = QApplication::applicationDirPath() + "/json.json";
	QFile f(path);
	if (f.exists()) f.remove();
	if (!f.open(QIODevice::WriteOnly))
	{
		qDebug() << "打开失败";
	}
	f.write(buf, buf.size());
	f.close();
}

/// <summary>
/// 接收到消息处理(初步解析，此处已经规定好格式)
/// </summary>
void TcpModel::readyRead()
{
	QByteArray buf = socket->readAll();
	//qDebug() << "lcy client:\n" + buf + "\n";
	//协商好 传输的json格式/直接是数据流
	//如果是Json格式，做进一步解析
	if (QJsonDocument::fromJson(buf).isObject())
	{
		receJsonFromClient(QJsonDocument::fromJson(buf).object());
	}
	//如果不为json格式，那就是数据
	else
	{
		if (blockSize == 0)
		{
			blockSize = QByteArrayHelper::QByteArrayToInt(buf, 0);
			blockBuf += buf.mid(4, buf.size() - 4);
		}
		else
		{
			blockBuf += buf;
		}
		if (blockBuf.size() < blockSize)
			return;
		receDataFromClient(blockBuf);
		blockBuf = {};
		blockSize = 0;
	}
}

/// <summary>
/// 删除对象
/// </summary>
/// <param name="obj">对象</param>
void TcpModel::clearMember(TcpData* obj)
{
	//找接收列表
	for (int i = 0; i < receList.count(); i++)
	{
		if (receList[i] == obj)
		{
			if (receList[i]->nowSize >= receList[i]->fileSize)
			{
				qDebug() << receList[i]->fileName + "." + receList[i]->type + "接收完成";
			}
			else
			{
				qDebug() << receList[i]->fileName + "." + receList[i]->type + "接收失败";
			}
			receList[i]->deleteLater();
			receList[i] = nullptr;
			receList.removeAt(i);
		}
	}
	//找发送列表
	for (int i = 0; i < sendList.count(); i++)
	{
		if (sendList[i] == obj)
		{
			if (sendList[i]->nowSize >= sendList[i]->fileSize)
			{
				qDebug() << sendList[i]->fileName + "." + sendList[i]->type + "发送完成";
			}
			else
			{
				qDebug() << sendList[i]->fileName + "." + sendList[i]->type + "发送失败";
			}
			sendList[i]->deleteLater();
			sendList[i] = nullptr;
			sendList.removeAt(i);
		}
	}
}

