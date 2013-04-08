#include "httprequest.h"


const QString HttpRequest::kServerSoftwareParam    = QString("Server-Software");
const QString HttpRequest::kServerAddressParam     = QString("Server-Address");
const QString HttpRequest::kServerPortParam        = QString("Server-Port");
const QString HttpRequest::kRemoteAddressParam     = QString("Remote-Address");
const QString HttpRequest::kProtocolParam          = QString("Protocol");
//const QString HttpRequest::kAppPathParam           = AbstractHttpRequest::kAppPathParam;
//const QString HttpRequest::kMountPathParam         = AbstractHttpRequest::kMountPathParam;
const QString HttpRequest::kAppPathParam   = QString("App-Path");
const QString HttpRequest::kMountPathParam = QString("Mount-Path");

const quint16 HttpRequest::kStartLineBufferLimit =    2048;
const quint16 HttpRequest::kHeadersBufferLimit   =   16384; /* 16 KB */


HttpRequest::HttpRequest(QIODevice* ioDevice)
	: _ioDevice(ioDevice),
	  _bytesRead(0),
	  _complete(false),
	  _section(NoSection),
	  _parseError(NoError),
	  _startLineBufferLimit(kStartLineBufferLimit),
	  _headersBufferLimit(kHeadersBufferLimit)
{
	connect(_ioDevice, SIGNAL(readyRead()), SLOT(onReadyRead()));
}

HttpRequest::~HttpRequest()
{
}

bool HttpRequest::isValid()
{
	return (_section != Invalid);
}

QString HttpRequest::version() const
{
	return _version;
}

QString HttpRequest::method() const
{
	return _method;
}

QString HttpRequest::uri() const
{
	return _uri;
}

QMultiHash<QString, QString> HttpRequest::headers() const
{
	return _headers;
}

void HttpRequest::setVersion(const QString & version)
{
	_version = version;
}

void HttpRequest::setMethod(const QString & method)
{
	_method = method;
}

void HttpRequest::setUri(const QString & uri)
{
	_uri = uri;
}

void HttpRequest::addHeader(const QString & field, const QString & value)
{
	_headers.insert(field.trimmed().toLower(), value.trimmed());
}

QString HttpRequest::parameter(const QString & key) const
{
	return _parameters.value(key);
}

void HttpRequest::setParameter(const QString & key, const QString & value)
{
	_parameters.insert(key, value);
}

void HttpRequest::setStartLineBufferLimit(quint16 limit)
{
	_startLineBufferLimit = limit;
}

void HttpRequest::setHeadersBufferLimit(quint16 limit)
{
	_headersBufferLimit = limit;
}

qint64 HttpRequest::bytesAvailable() const
{
	return _buffer.size();
}

qint64 HttpRequest::expectedContentLength()
{
	QString contentLength = header("Content-Length");
	if (contentLength.isEmpty())
		return 0;
	
	return contentLength.toLongLong();
}

qint64 HttpRequest::readData(char* data, qint64 maxSize)
{
	// The request is complete, there can never be more bytes available.  Return
	// -1 to indicate this fact to the I/O subsystem.
	if ((_buffer.size() == 0) && _complete)
		return -1;
	
	qint64 size = maxSize;
	if (_buffer.size() < maxSize)
		size = _buffer.size();
		
	memcpy(data, _buffer.data(), size);
	_buffer.remove(0, size);
	
	return size;
}

qint64 HttpRequest::writeData(const char* /* data */, qint64 /* maxSize */)
{
	// An HTTP request is a read-only channel.  If an attempt is made to write
	// data to it, return -1 to indicate an error.
	return -1;
}

void HttpRequest::parse()
{
	// The HTTP request has previously been determined to be invalid.  No
	// further parsing will be attempted.
	if (Invalid == _section)
		return;
	
	// The parser has not read any message sections.  A request line is the
	// first expected section, which the parser will attempt to read.
	if (NoSection == _section)
	{
		int idx = _buffer.indexOf("\r\n");
		
		// If the delimiter signalling the end of the start line has not yet
		// been received, and the buffer size exceeds the limit, the request is
		// invalid.
		if ( (-1 == idx) && (_buffer.size() > _startLineBufferLimit) )
		{
			_section = Invalid;
			_parseError = RequestUriTooLongError;
			return;
		}
		
		// The delimiter signalling the end of the start line has not yet been
		// received.  However, the buffer limit hasn't been exceeded, so more
		// data is expected to be received over the connection.  Parsing will
		// resume when further data has been received.
		if (-1 == idx)
			return;
		
		
		QByteArray request_line = _buffer.left(idx);
		_buffer.remove(0, idx + 2);
		_bytesRead -= (idx + 2);

		// Attempt to parse out the method.  If the space character, which
		// separates it from the request URI, is not found, then the request is
		// invalid.
		idx = request_line.indexOf(' ');
		if (-1 == idx)
		{
			_section = Invalid;
			_parseError = BadRequestError;
			return;
		}
		
		setMethod(QString::fromUtf8(request_line.left(idx)));
		request_line.remove(0, idx + 1);
		
		// Attempt to parse out the request URI.  If the space character, which
		// separates it from the HTTP version, is not found, then the request is
		// invalid.
		idx = request_line.indexOf(' ');
		if (-1 == idx)
		{
			_section = Invalid;
			_parseError = BadRequestError;
			return;
		}
		
		setUri(QString::fromUtf8(request_line.left(idx)));
		request_line.remove(0, idx + 1);
		
		// The remainder of the request line should consist of the HTTP version.
		// It is expected to be in the following form: "HTTP/x.x".  x.x
		// signifies the version of the HTTP protocol in use, typically 1.0 or
		// 1.1.  If the HTTP version does not begin with "HTTP/", the request is
		// invalid.
		if (!request_line.startsWith("HTTP/"))
		{
			_section = Invalid;
			_parseError = BadRequestError;
			return;
		}
		
		request_line.remove(0, 5);
		setVersion(QString::fromUtf8(request_line));
		
		if (!_version.startsWith("1."))
		{
			_section = Invalid;
			_parseError = HttpVersionNotSupportedError;
			return;
		}
		
		_section = StartLine;
	}
	
	// The parser has read the request line.  The next section expected is the
	// message headers, which the parser will attempt to read.
	if (StartLine == _section)
	{
		int idx = _buffer.indexOf("\r\n\r\n");
		
		// If the delimiter signalling the end of headers has not yet
		// been received, and the buffer size exceeds the limit, the request is
		// invalid.
		if ( (-1 == idx) && (_buffer.size() > _headersBufferLimit) )
		{
			_section = Invalid;
			_parseError = RequestEntityTooLargeError;
			return;
		}

		// The delimiter signalling the end of headers has not yet been
		// received.  However, the buffer limit hasn't been exceeded, so more
		// data is expected to be received over the connection.  Parsing will
		// resume when further data has been received.
		if (-1 == idx)
			return;

		while (_buffer.size() > 0)
		{
			idx = _buffer.indexOf("\r\n");

			// The parser has encountered the end of headers.  The next section
			// expected is the message body, which the parser will attempt to read.
			if (0 == idx)
			{
				_buffer.remove(0, idx + 2);
				_bytesRead -= (idx + 2);
				
				_section = Headers;
				
				// The request headers have been received.  The request can now be
				// dispatched, and any interested object can read the body through
				// the QIODevice interface.
				emit received();
				
				break;
			}

			QByteArray header = _buffer.left(idx);
			parseHeader(header);

			_buffer.remove(0, idx + 2);
			_bytesRead -= (idx + 2);
		}
	}
	
	if (Headers == _section)
	{
		// The parser has read the headers.  Any remaining data consists as part
		// of the HTTP body.  This data will be read through the QIODevice
		// interface.
		_section = Body;
	}
}

void HttpRequest::parseHeader(const QByteArray & buffer)
{
	QByteArray field;
	QByteArray value;

	int idx = buffer.indexOf(':');
	if (-1 == idx)
		return;
		
	field = buffer.left(idx);
	value = buffer.mid(idx + 1);

	addHeader(QString::fromUtf8(field),
			  QString::fromUtf8(value));
}

void HttpRequest::onReadyRead()
{
	if (_ioDevice.isNull())
		return;
	
	QByteArray bytes = _ioDevice->readAll();
	_buffer.append(bytes);
	_bytesRead += bytes.size();
	
	
	parse();
		
	if (_section == Body)
	{
		if (_buffer.size() > 0) {
			emit readyRead();
		}
		if (_bytesRead == expectedContentLength()) {
			_complete = true;
			emit complete();
			return;
		}
	}
	else if (_section == Invalid)
	{
		disconnect(_ioDevice, 0, this, 0);
		
		emit error(_parseError);
		emit complete();
		return;
	}
}
