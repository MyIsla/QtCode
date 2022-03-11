#pragma execution_character_set("utf-8")
#include "TcpThread.h"

TcpThread::TcpThread(qintptr socketDescriptor)
{
	this->socketDescriptor = socketDescriptor;
}

TcpThread::~TcpThread()
{
	this->quit();
	this->wait();
}

void TcpThread::run()
{
	TcpModel* model = new TcpModel(socketDescriptor);
	createConnection(model);
	this->exec();
}

void TcpThread::createConnection(TcpModel* model)
{
	//this对象为信号
	connect(this, SIGNAL(sendMsgToModel(QString)), model, SLOT(sendMsgToClient(QString)));
	connect(this, SIGNAL(sendFilePathToModel(QString)), model, SLOT(sendFileToClient(QString)));
	//model对象为信号
	connect(model, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
	connect(model, SIGNAL(sendMsgToThread(QString)), this, SIGNAL(sendMsgToController(QString)));
}