#pragma execution_character_set("utf-8")
#include "TcpController.h"

TcpController::TcpController()
{
}

TcpController::~TcpController()
{
	qDeleteAll(threadList);
	threadList.clear();
	delete instance;
	instance = nullptr;
}

/// <summary>
/// �ź�������ӵ��øú���
/// </summary>
/// <returns>�ö���</returns>
TcpController* TcpController::getInstance()
{
	if (instance == nullptr)
		instance = new TcpController();
	return instance;
}

TcpController* TcpController::instance = nullptr;

/// <summary>
/// ����������
/// </summary>
void TcpController::startServer()
{
	if (!this->isListening())
	{
		this->listen(QHostAddress(HostInfoHelper::getInstance()->localIPv4()), 8068);
	}
	else
	{
		sendMsgToThread("connected");
	}
	qDebug() << "�������ɹ�����";
}

/// <summary>
/// ����������
/// </summary>
/// <param name="addr">IP��ַ</param>
/// <param name="port">�˿ں�</param>
void TcpController::startServer(QString addr, QString port)
{
	if (!this->isListening())
	{
		this->listen(QHostAddress(addr), quint16(port.toUInt()));
	}
	else
	{
		sendMsgToThread("connected");
	}
	qDebug() << "�������ɹ�����";
	//wait();
}

/// <summary>
/// �رշ�����
/// </summary>
void TcpController::closeServer()
{
	this->close();
	qDebug() << "�������ɹ��ر�";
}

/// <summary>
/// ������Ϣ��Model
/// </summary>
/// <param name="msg">��Ϣ</param>
/// <param name="id">id</param>
void TcpController::receMsgFromView(QString msg, QString id)
{
	for (int i = 0; i < threadList.count(); i++)
	{
		if (id.toInt() == i || id == "")
			emit sendMsgToThread(msg);
	}
}

/// <summary>
/// ���ؿͻ������Ӻ�������ÿ���ͻ��˷���һ���߳���
/// </summary>
/// <param name="socketDescriptor">�ͻ����׽���ָ��</param>
void TcpController::incomingConnection(qintptr socketDescriptor)
{
	//�����߳�
	TcpThread* thread = new TcpThread(socketDescriptor);
	//��������
	createConnections(thread);
	//�����б��Է���Ҫָ���ͻ��˷���
	threadList.append(thread);
	thread->start();
}

/// <summary>
/// ������model������
/// </summary>
/// <param name="model">model����</param>
void TcpController::createConnections(TcpThread* thread)
{
	//this����Ϊ�ź�
	connect(this, SIGNAL(sendMsgToThread(QString)), thread, SIGNAL(sendMsgToModel(QString)));
	connect(this, SIGNAL(sendFilePathToThread(QString)), thread, SIGNAL(sendFilePathToModel(QString)));
	//thread����Ϊ�ź�
	connect(thread, SIGNAL(disconnected()), this, SLOT(deleteThread()));
	connect(thread, SIGNAL(sendMsgToController(QString)), this, SLOT(sendMsgToView(QString)));
}

/// <summary>
/// ���Ͽ����ӵĿͻ���
/// </summary>
void TcpController::deleteThread()
{
	QThread* thread = (QThread*)sender();
	for (int i = 0; i < threadList.count(); i++)
	{
		if (thread == threadList[i])
		{
			qDebug() << "�߳�" + QString::number(i) + "���˳�";
			threadList[i]->deleteLater();
			threadList[i] = nullptr;
			threadList.removeAt(i);
		}
	}
}