#pragma once

#include <QObject>
#include <QTcpServer>
#include <QList>
#include <qhostinfo.h>

#include "TcpThread.h"
#include "HostInfoHelper.h"

/// <summary>
/// ������������/�رռ�ȷ�Ͽͻ������ӣ�����һ��TcpModel���������תվ
/// ����ͨ��TcpController����Model��Modelͨ��TcpController���ƽ���
/// </summary>
class TcpController : public QTcpServer
{
	Q_OBJECT
public:
	TcpController();
	~TcpController();

	static TcpController* getInstance();						//�ź�������ӵ��øú���
public slots:
	//From View
	void startServer();											//����������
	void startServer(QString addr, QString port);				//����������
	void closeServer();											//�رշ�����
	void receMsgFromView(QString msg, QString id = "");			//���յ��ӽ���������Ϣ������Ϊ�˷ֱ淢���ĸ��ͻ���,��ʱΪȫ����
protected:
	void incomingConnection(qintptr socketDescriptor)override;	//���ؿͻ������Ӻ�������ÿ���ͻ��˷���һ���߳���
private:
	static TcpController* instance;								//�����ź���۵�����
	QList<TcpThread*> threadList;								//ÿ��thread��Ӧ����һ���ͻ��˵��¼������б�洢���Ի�����ǵı���Ա����Ժ�����Ϣ����һ�Ŀͻ���
	void createConnections(TcpThread *thread);					//������thread������
private slots:
	//From Thread
	void deleteThread();										//ɾ���Ͽ����ӵĿͻ���
signals:
	//To Thread
	void sendMsgToThread(QString msg);							//������Ϣ��thread
	void sendFilePathToThread(QString filePath);				//�����ļ�·����Thread
	//To View
	void sendMsgToView(QString msg);							//������Ϣ��View
};

