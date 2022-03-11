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
/// 信号与槽连接调用该函数
/// </summary>
/// <returns>该对象</returns>
TcpController* TcpController::getInstance()
{
	if (instance == nullptr)
		instance = new TcpController();
	return instance;
}

TcpController* TcpController::instance = nullptr;

/// <summary>
/// 开启服务器
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
	qDebug() << "服务器成功启动";
}

/// <summary>
/// 开启服务器
/// </summary>
/// <param name="addr">IP地址</param>
/// <param name="port">端口号</param>
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
	qDebug() << "服务器成功启动";
	//wait();
}

/// <summary>
/// 关闭服务器
/// </summary>
void TcpController::closeServer()
{
	this->close();
	qDebug() << "服务器成功关闭";
}

/// <summary>
/// 发送消息给Model
/// </summary>
/// <param name="msg">消息</param>
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
/// 重载客户端连接函数，将每个客户端放入一个线程中
/// </summary>
/// <param name="socketDescriptor">客户端套接字指针</param>
void TcpController::incomingConnection(qintptr socketDescriptor)
{
	//建立线程
	TcpThread* thread = new TcpThread(socketDescriptor);
	//创建连接
	createConnections(thread);
	//加入列表，以防需要指定客户端发送
	threadList.append(thread);
	thread->start();
}

/// <summary>
/// 创建与model的连接
/// </summary>
/// <param name="model">model对象</param>
void TcpController::createConnections(TcpThread* thread)
{
	//this对象为信号
	connect(this, SIGNAL(sendMsgToThread(QString)), thread, SIGNAL(sendMsgToModel(QString)));
	connect(this, SIGNAL(sendFilePathToThread(QString)), thread, SIGNAL(sendFilePathToModel(QString)));
	//thread对象为信号
	connect(thread, SIGNAL(disconnected()), this, SLOT(deleteThread()));
	connect(thread, SIGNAL(sendMsgToController(QString)), this, SLOT(sendMsgToView(QString)));
}

/// <summary>
/// 除断开连接的客户端
/// </summary>
void TcpController::deleteThread()
{
	QThread* thread = (QThread*)sender();
	for (int i = 0; i < threadList.count(); i++)
	{
		if (thread == threadList[i])
		{
			qDebug() << "线程" + QString::number(i) + "已退出";
			threadList[i]->deleteLater();
			threadList[i] = nullptr;
			threadList.removeAt(i);
		}
	}
}