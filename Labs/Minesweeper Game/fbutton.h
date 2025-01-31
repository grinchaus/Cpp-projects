#ifndef FBUTTON_H
#define FBUTTON_H

#include <QPushButton>

class Fbutton : public QPushButton
{
	Q_OBJECT

  public:
	bool mine_;
	int counter_;
	bool flag_;
	bool open_;
	int row_, col_;

	Fbutton(QWidget* parent = nullptr);

  protected:
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
  signals:
	void leftClicked(int row_, int col_);
	void rightClicked(int row_, int col_);
	void midlClicked(int row_, int col_);
	void midlRelease(int row_, int col_);
};

#endif	  // FBUTTON_H
