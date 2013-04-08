#include "abstracthttprequest.h"

const QString AbstractHttpRequest::kAppPathParam   = QString("App-Path");
const QString AbstractHttpRequest::kMountPathParam = QString("Mount-Path");


AbstractHttpRequest::AbstractHttpRequest()
{
}

AbstractHttpRequest::~AbstractHttpRequest()
{
}

QString AbstractHttpRequest::header(const QString & field) const
{
	QList<QString> list = headers(field);
	if (list.size() > 0)
		return list.at(0);

	return "";
}

QList<QString> AbstractHttpRequest::headers(const QString & field) const
{
	return headers().values(field.toLower());
}
