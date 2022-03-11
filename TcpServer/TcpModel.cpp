#pragma execution_character_set("utf-8")
#include "TcpModel.h"

/// <summary>
/// ����TcpModel
/// </summary>
/// <param name="socketDescriptor">�ͻ����׽���ָ��</param>
TcpModel::TcpModel(qintptr socketDescriptor)
{
	this->socketDescriptor = socketDescriptor;
	socket = new QTcpSocket();
	socket->setSocketDescriptor(socketDescriptor);
	qDebug() << "�ͻ���" + socket->peerAddress().toString() + "����";
	createConnections();
	sendMsgToClient("connected");
}

TcpModel::~TcpModel()
{
	qDebug() << "IP��ַΪ:" + socket->peerAddress().toString() + "�Ŀͻ��˶Ͽ�����";
	delete socket;
	socket = nullptr;
}

/// <summary>
/// ������Ϣ
/// </summary>
/// <param name="msg">����</param>
/// <param name="state">�׶Σ�Ĭ�� = 0</param>
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
/// �����ļ�(�ȶ���sendList���ٷ���ͷ)
/// </summary>
/// <param name="filePath">�ļ�·��</param>
void TcpModel::sendFileToClient(QString filePath)
{
	//��ȡ�ļ���Ϣ
	QFileInfo fileInfo(filePath);
	QString type = fileInfo.suffix();
	QString fileName = fileInfo.baseName();
	//�鿴�Ƿ����б���
	for (int i = 0; i < sendList.count(); i++)
	{
		if (sendList[i]->type == type && sendList[i]->fileName == fileName)
			return;
	}
	//�������ڴ���
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
/// ��������
/// </summary>
/// <param name="socket">�׽���</param>
void TcpModel::createConnections()
{
	connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
	connect(socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
}

/// <summary>
/// ����Jsonͷ
/// </summary>
/// <param name="type">�ļ�����</param>
/// <param name="fileName">�ļ���</param>
/// <param name="fileSize">�ļ���С</param>
/// <param name="state">״̬</param>
/// <param name="md5">md5</param>
void TcpModel::sendHeadToClient(QString type, QString fileName, quint64 fileSize, int state, QString md5)
{
	//������Ϊ����ͷ��״̬
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
/// ��������
/// </summary>
/// <param name="type">����</param>
/// <param name="fileName">�ļ���</param>
/// <param name="fileSize">�ļ���С</param>
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
/// �Խӵ���json���ݽ��ж��ν���
/// </summary>
/// <param name="obj">Json����</param>
void TcpModel::receJsonFromClient(QJsonObject obj)
{
	//û��type�ֶ�
	if (!obj.contains("type")) return;
	QString type = obj.value("type").toString();
	//û��filename�ֶ�
	if (!obj.contains("filename")) return;
	QString filename = obj.value("filename").toString();
	//û��data�ֶ�
	if (!obj.contains("data")) return;
	QString data = obj.value("data").toString();
	//û��filesize�ֶ�
	if (!obj.contains("filesize")) return;
	quint64 filesize = obj.value("filesize").toString().toInt();
	//û��state�ֶ�
	if (!obj.contains("state")) return;
	int state = obj.value("state").toString().toInt();
	//û��md5�ֶ�
	if (!obj.contains("md5"))return;
	QString md5 = obj.value("md5").toString();
	//�ж�����Ϣ�����ļ�
	if (type == "message")
	{
		receMsgFromClient(data, filename, md5, state);
		return;
	}
	//�жϴ�����Ϣ��״̬
	switch (state)
	{
	case 0: receHeadFromClient(type, filename, filesize, md5); break;//���յ�ͷ�ļ�
	case 1:	sendDataToClient(type, filename, filesize); break;//������
	case 2: sendHeadToClient(type, filename, filesize, 0); break;//������һ��ͷ�ļ�
	default:
		break;
	}
}

/// <summary>
/// ����Json�õ�����ͷ
/// </summary>
/// <param name="type">����</param>
/// <param name="fileName">�ļ���/�豸��</param>
/// <param name="fileSize">�ļ���С</param>
/// <param name="md5">md5</param>
void TcpModel::receHeadFromClient(QString type, QString fileName, quint64 fileSize, QString md5)
{
	//�鿴���յ��б������޼�¼
	for (int i = 0; i < receList.count(); i++)
	{
		if (receList[i]->type == type && receList[i]->fileName == fileName &&
			receList[i]->fileSize == fileSize)
		{
			//�м�¼������MD5Ϊ��ʹ��һ�η����������ݿ��Խ�����֤
			receList[i]->md5 = md5;
			sendHeadToClient(receList[i]->type, receList[i]->fileName,
				receList[i]->fileSize, 1, receList[i]->md5);
			return;
		}
	}
	//�޼�¼���½���¼
	TcpData* data = new TcpData(type, fileName, fileSize, md5);
	connect(data, SIGNAL(clearData(TcpData*)), this, SLOT(clearMember(TcpData*)));
	receList.append(data);
	//�б��е����һ��
	sendHeadToClient(receList[receList.count() - 1]->type, receList[receList.count() - 1]->fileName,
		receList[receList.count() - 1]->fileSize, 1, receList[receList.count() - 1]->md5);
}

/// <summary>
/// ����Json�õ�������Ϣ
/// </summary>
/// <param name="msg">��Ϣ</param>
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
		qDebug() << "md5У�����";
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
/// �ӵ�����
/// </summary>
void TcpModel::receDataFromClient(QByteArray buf)
{
	qDebug() << "Clinet buf size:" << buf.size();
	bool get = false;
	for (int i = 0; i < receList.count(); i++)
	{
		qDebug() << "��" + QString::number(i) + "��:";
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
		qDebug() << "md5����";
		return;
	}
}

/// <summary>
/// ������������
/// </summary>
/// <param name="type">����</param>
/// <param name="fileName">����</param>
/// <param name="buf">����</param>
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
/// ������Ϣ
/// </summary>
/// <param name="fileName">�豸��/�ļ���</param>
/// <param name="buf">����</param>
void TcpModel::analysisMSG(QString fileName, QString msg)
{
	emit sendMsgToThread(msg);
}

/// <summary>
/// ����bmp����
/// </summary>
/// <param name="fileName">�豸��/�ļ���</param>
/// <param name="buf">����</param>
void TcpModel::analysisBMP(QString fileName, QByteArray buf)
{
	QBitmap b;
	b.loadFromData(buf);
	QStringList stringList = fileName.split("_");
	QString num = stringList[0].replace(QRegExp("[a-z]|[A-Z]"), "");
}

/// <summary>
/// ����png����
/// </summary>
/// <param name="fileName">�豸��/�ļ���</param>
/// <param name="buf">����</param>
void TcpModel::analysisPNG(QString fileName, QByteArray buf)
{
	QPixmap p;
	p.loadFromData(buf);
	QStringList stringList = fileName.split("_");
	QString num = stringList[0].replace(QRegExp("[a-z]|[A-Z]"), "").toInt();
}

/// <summary>
/// ����csv����
/// </summary>
/// <param name="fileName">�豸��/�ļ���</param>
/// <param name="buf">����</param>
void TcpModel::analysisCSV(QString fileName, QByteArray buf)
{
	buf.replace("\r", "");
	QString path = QApplication::applicationDirPath() +  "/csv.csv";
	QFile f(path);
	if (f.exists()) f.remove();
	if (!f.open(QIODevice::WriteOnly))
	{
		qDebug() << "��ʧ��";
	}
	f.write(buf,buf.size());
	f.close();
}

/// <summary>
/// ����json����
/// </summary>
/// <param name="fileName">�豸��/�ļ���</param>
/// <param name="buf">����</param>
void TcpModel::analysisJson(QString fileName, QByteArray buf)
{
	QString path = QApplication::applicationDirPath() + "/json.json";
	QFile f(path);
	if (f.exists()) f.remove();
	if (!f.open(QIODevice::WriteOnly))
	{
		qDebug() << "��ʧ��";
	}
	f.write(buf, buf.size());
	f.close();
}

/// <summary>
/// ���յ���Ϣ����(�����������˴��Ѿ��涨�ø�ʽ)
/// </summary>
void TcpModel::readyRead()
{
	QByteArray buf = socket->readAll();
	//qDebug() << "lcy client:\n" + buf + "\n";
	//Э�̺� �����json��ʽ/ֱ����������
	//�����Json��ʽ������һ������
	if (QJsonDocument::fromJson(buf).isObject())
	{
		receJsonFromClient(QJsonDocument::fromJson(buf).object());
	}
	//�����Ϊjson��ʽ���Ǿ�������
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
/// ɾ������
/// </summary>
/// <param name="obj">����</param>
void TcpModel::clearMember(TcpData* obj)
{
	//�ҽ����б�
	for (int i = 0; i < receList.count(); i++)
	{
		if (receList[i] == obj)
		{
			if (receList[i]->nowSize >= receList[i]->fileSize)
			{
				qDebug() << receList[i]->fileName + "." + receList[i]->type + "�������";
			}
			else
			{
				qDebug() << receList[i]->fileName + "." + receList[i]->type + "����ʧ��";
			}
			receList[i]->deleteLater();
			receList[i] = nullptr;
			receList.removeAt(i);
		}
	}
	//�ҷ����б�
	for (int i = 0; i < sendList.count(); i++)
	{
		if (sendList[i] == obj)
		{
			if (sendList[i]->nowSize >= sendList[i]->fileSize)
			{
				qDebug() << sendList[i]->fileName + "." + sendList[i]->type + "�������";
			}
			else
			{
				qDebug() << sendList[i]->fileName + "." + sendList[i]->type + "����ʧ��";
			}
			sendList[i]->deleteLater();
			sendList[i] = nullptr;
			sendList.removeAt(i);
		}
	}
}

