#ifndef FIELD_H
#define FIELD_H

#include "fbutton.h"

#include <QApplication>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QPixmap>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QToolBar>
#include <QTranslator>
#include <QVBoxLayout>
#include <QWidget>

class Field : public QWidget
{
	Q_OBJECT

  private:
	bool m_firstClick;
	bool m_save;
	QTranslator m_translator;
	int m_row;
	int m_col;
	int m_mine;
	int m_openButtons;
	QHBoxLayout *panelLayout;
	QVBoxLayout *mainLayout;
	QToolBar *toolBar;
	QGridLayout *gridLayout;
	QLabel *infoLabel;
	QVector< QVector< Fbutton * > > buttons;

  public:
	Field(QWidget *parent = nullptr);

	~Field()
	{
		buttons.clear();
		delete gridLayout;
		delete toolBar;
		delete infoLabel;
		delete panelLayout;
		delete mainLayout;
	}

	void dbg();

  protected:
	void closeEvent(QCloseEvent *event) override;
	//    void resizeEvent(QResizeEvent* event) override;

  private slots:
	void onLeftClicked(int row, int col);
	void onRightClicked(int row, int col);
	void onMidlClicked(int row, int col);
	void onMidlRelease(int row, int col);
	void startNewGame();
	void startCustomGame();
	void onDbgButtonClicked();
	void onDbgButtonReleased();
	void toEnglish();
	void toRussian();

  private:
	void setupBoard();
	void placeMines();
	void calculateAdjacentMines();
	void revealCell(int row, int col);
	void revealAdjacentCells(int row, int col);
	void buildField();
	void showInputDialog();
	void win();
	void lose();
	void saveGame();
	void recreateUI();
	void createToolbar();
};

#endif	  // FIELD_H
