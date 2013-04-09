#include "tst_httprequest.h"
#include <QtxMockCore>
#include <QtNetwork>


void TestHttpRequest::initTestCase()
{
}

void TestHttpRequest::init()
{
	_error = HttpRequest::NoError;
	
	_emitsReceived = false;
	_emitsReadyRead = false;
	_emitsComplete = false;
	_emitsError = false;
}

void TestHttpRequest::cleanup()
{
}

void TestHttpRequest::getMethod()
{
	MockIODevice* mock = new MockIODevice("data/get-method.io");
	
	HttpRequest request(mock);
	connect(&request, SIGNAL(received()), SLOT(onReceived()));
	connect(&request, SIGNAL(complete()), SLOT(onComplete()));
	connect(&request, SIGNAL(error(Ot::Net::Http::HttpRequest::ParseError)), SLOT(onError(Ot::Net::Http::HttpRequest::ParseError)));
	
	request.open(QIODevice::ReadOnly);
	mock->open(QIODevice::ReadOnly);
	
	
	QVERIFY(request.isValid());
	QCOMPARE(request.method(), QString("GET"));
	QCOMPARE(request.uri(), QString("/welcome"));
	QCOMPARE(request.version(), QString("1.1"));
	QCOMPARE(request.header("Accept"), QString("text/html"));

	QVERIFY(_emitsReceived);
	QVERIFY(_emitsComplete);
	QVERIFY(!_emitsError);
}

void TestHttpRequest::postMethod()
{
	MockIODevice* mock = new MockIODevice("data/post-method.io");
	
	HttpRequest request(mock);
	connect(&request, SIGNAL(received()), SLOT(onReceived()));
	connect(&request, SIGNAL(readyRead()), SLOT(onReadyRead()));
	connect(&request, SIGNAL(complete()), SLOT(onComplete()));
	connect(&request, SIGNAL(error(Ot::Net::Http::HttpRequest::ParseError)), SLOT(onError(Ot::Net::Http::HttpRequest::ParseError)));
	
	request.open(QIODevice::ReadOnly);
	mock->open(QIODevice::ReadOnly);
	
	
	QVERIFY(request.isValid());
	QCOMPARE(request.method(), QString("POST"));
	QCOMPARE(request.uri(), QString("/greeting"));
	QCOMPARE(request.version(), QString("1.1"));
	QCOMPARE(request.header("Content-Length"), QString("5"));
	
	QVERIFY(request.bytesAvailable() == 5);
	QByteArray bytes = request.readAll();
	QVERIFY(request.bytesAvailable() == 0);
	
	QCOMPARE(bytes, QByteArray("Hello"));
	
	QVERIFY(_emitsReceived);
	QVERIFY(_emitsReadyRead);
	QVERIFY(_emitsComplete);
	QVERIFY(!_emitsError);
}

void TestHttpRequest::invalidProtocolError()
{
	MockIODevice* mock = new MockIODevice("data/invalid-protocol.io");
	
	HttpRequest request(mock);
	connect(&request, SIGNAL(received()), SLOT(onReceived()));
	connect(&request, SIGNAL(complete()), SLOT(onComplete()));
	connect(&request, SIGNAL(error(Ot::Net::Http::HttpRequest::ParseError)), SLOT(onError(Ot::Net::Http::HttpRequest::ParseError)));
	
	request.open(QIODevice::ReadOnly);
	mock->open(QIODevice::ReadOnly);
	
	
	QVERIFY(!request.isValid());
	QVERIFY(_error == HttpRequest::BadRequestError);
	
	QVERIFY(!_emitsReceived);
	QVERIFY(_emitsError);
	QVERIFY(_emitsComplete);
}

void TestHttpRequest::requestEntityTooLargeError()
{
	MockIODevice* mock = new MockIODevice("data/request-entity-too-large.io");
	
	HttpRequest request(mock);
	request.setHeadersBufferLimit(16);
	
	connect(&request, SIGNAL(received()), SLOT(onReceived()));
	connect(&request, SIGNAL(complete()), SLOT(onComplete()));
	connect(&request, SIGNAL(error(Ot::Net::Http::HttpRequest::ParseError)), SLOT(onError(Ot::Net::Http::HttpRequest::ParseError)));
	
	request.open(QIODevice::ReadOnly);
	mock->open(QIODevice::ReadOnly);
	
	
	QVERIFY(!request.isValid());
	QVERIFY(_error == HttpRequest::RequestEntityTooLargeError);
	
	QVERIFY(!_emitsReceived);
	QVERIFY(_emitsError);
	QVERIFY(_emitsComplete);
}

void TestHttpRequest::requestUriTooLongError()
{
	MockIODevice* mock = new MockIODevice("data/request-uri-too-long.io");
	
	HttpRequest request(mock);
	request.setStartLineBufferLimit(8);
	
	connect(&request, SIGNAL(received()), SLOT(onReceived()));
	connect(&request, SIGNAL(complete()), SLOT(onComplete()));
	connect(&request, SIGNAL(error(Ot::Net::Http::HttpRequest::ParseError)), SLOT(onError(Ot::Net::Http::HttpRequest::ParseError)));
	
	request.open(QIODevice::ReadOnly);
	mock->open(QIODevice::ReadOnly);
	
	
	QVERIFY(!request.isValid());
	QVERIFY(_error == HttpRequest::RequestUriTooLongError);
	
	QVERIFY(!_emitsReceived);
	QVERIFY(_emitsError);
	QVERIFY(_emitsComplete);
}

void TestHttpRequest::httpVersionNotSupportedError()
{
	MockIODevice* mock = new MockIODevice("data/unsupported-http-version.io");
	
	HttpRequest request(mock);
	request.setStartLineBufferLimit(8);
	
	connect(&request, SIGNAL(received()), SLOT(onReceived()));
	connect(&request, SIGNAL(complete()), SLOT(onComplete()));
	connect(&request, SIGNAL(error(Ot::Net::Http::HttpRequest::ParseError)), SLOT(onError(Ot::Net::Http::HttpRequest::ParseError)));
	
	request.open(QIODevice::ReadOnly);
	mock->open(QIODevice::ReadOnly);
	
	
	QVERIFY(!request.isValid());
	QVERIFY(_error == HttpRequest::HttpVersionNotSupportedError);
	
	QVERIFY(!_emitsReceived);
	QVERIFY(_emitsError);
	QVERIFY(_emitsComplete);
}

void TestHttpRequest::onReceived()
{
    qDebug() << "TestHttpRequest::onReceived";
	
	_emitsReceived = true;
}

void TestHttpRequest::onReadyRead()
{
	qDebug() << "TestHttpRequest::onReadyRead";
	
	_emitsReadyRead = true;
}

void TestHttpRequest::onComplete()
{
	qDebug() << "TestHttpRequest::onComplete";
	
	_emitsComplete = true;
}

void TestHttpRequest::onError(HttpRequest::ParseError code)
{
	qDebug() << "TestHttpRequest::onError";
	
	_error = code;
	_emitsError = true;
}


QTEST_APPLESS_MAIN(TestHttpRequest)
