#include "fbutton.h"

#include <QMouseEvent>

Fbutton::Fbutton(QWidget *parent) : QPushButton(parent), mine_(false), counter_(0), flag_(false), open_(false)
{
	setIconSize(QSize(40, 40));
	setStyleSheet("QPushButton { background-color: #cccccc; }");
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setMinimumSize(40, 40);
}

void Fbutton::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		emit leftClicked(row_, col_);
	}
	else if (event->button() == Qt::RightButton)
	{
		emit rightClicked(row_, col_);
	}
	else if (event->button() == Qt::MiddleButton)
	{
		emit midlClicked(row_, col_);
	}
}

void Fbutton::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::MiddleButton)
	{
		emit midlRelease(row_, col_);
	}
}
