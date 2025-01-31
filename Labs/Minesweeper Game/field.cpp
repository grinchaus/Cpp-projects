#include "field.h"

#include <QRandomGenerator>

void Field::showInputDialog()
{
	m_save = false;
	QDialog dialog(this);
	dialog.setWindowTitle(tr("New game"));
	dialog.setWindowIcon(QIcon(":/icons/icon.png"));

	QSpinBox *widthEdit = new QSpinBox(&dialog);
	widthEdit->setValue(10);
	widthEdit->setMinimum(1);
	QSpinBox *heightEdit = new QSpinBox(&dialog);
	heightEdit->setValue(10);
	heightEdit->setMinimum(1);

	QSpinBox *minesEdit = new QSpinBox(&dialog);
	minesEdit->setValue(10);
	minesEdit->setMinimum(1);

	QFormLayout formLayout(&dialog);
	formLayout.addRow(tr("Field width:"), widthEdit);
	formLayout.addRow(tr("Field height:"), heightEdit);
	formLayout.addRow(tr("Number of mines:"), minesEdit);

	QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
	formLayout.addRow(&buttonBox);

	QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

	auto validateInput = [&]()
	{
		int width = widthEdit->value();
		int height = heightEdit->value();
		int mines = minesEdit->value();
		int maxMines = width * height - 2;
		if (mines > maxMines)
		{
			buttonBox.button(QDialogButtonBox::Ok)->setText(tr("Invalid size"));
			buttonBox.button(QDialogButtonBox::Ok)->setEnabled(false);
		}
		else
		{
			buttonBox.button(QDialogButtonBox::Ok)->setText(tr("Ok"));
			buttonBox.button(QDialogButtonBox::Ok)->setEnabled(true);
		}
	};
	QObject::connect(widthEdit, QOverload< int >::of(&QSpinBox::valueChanged), validateInput);
	QObject::connect(heightEdit, QOverload< int >::of(&QSpinBox::valueChanged), validateInput);
	QObject::connect(minesEdit, QOverload< int >::of(&QSpinBox::valueChanged), validateInput);
	validateInput();
	if (dialog.exec() == QDialog::Accepted)
	{
		m_row = widthEdit->value();
		m_col = heightEdit->value();
		m_mine = minesEdit->value();
	}
}

Field::Field(QWidget *parent) :
	QWidget(parent), m_firstClick(false), m_save(true), m_row(10), m_col(10), m_mine(10), m_openButtons(0)
{
	gridLayout = new QGridLayout();
	panelLayout = new QHBoxLayout();
	mainLayout = new QVBoxLayout(this);
	createToolbar();

	infoLabel = new QLabel("🙂", this);
	infoLabel->setAlignment(Qt::AlignCenter);
	QFont font = infoLabel->font();
	font.setPointSize(16);
	infoLabel->setFont(font);
	infoLabel->setFixedWidth(200);

	panelLayout->addWidget(infoLabel);

	mainLayout->addLayout(panelLayout);
	mainLayout->addLayout(gridLayout);
	setLayout(mainLayout);
	buildField();
}

void Field::createToolbar()
{
	toolBar = findChild< QToolBar * >("ToolBar");
	if (toolBar)
	{
		delete toolBar;
	}

	toolBar = new QToolBar(this);
	toolBar->setObjectName("ToolBar");

	mainLayout->insertWidget(0, toolBar);

	QAction *newGameAction = new QAction(tr("New Game"), this);
	QAction *customGameAction = new QAction(tr("Custom Game"), this);
	QAction *toEnglish = new QAction("English", this);
	QAction *toRussian = new QAction("Русский", this);

	toolBar->addAction(newGameAction);
	toolBar->addAction(customGameAction);
	toolBar->addAction(toEnglish);
	toolBar->addAction(toRussian);

	connect(newGameAction, &QAction::triggered, this, &Field::startNewGame);
	connect(customGameAction, &QAction::triggered, this, &Field::startCustomGame);

	connect(toEnglish, &QAction::triggered, this, &Field::toEnglish);
	connect(toRussian, &QAction::triggered, this, &Field::toRussian);
}

void Field::toEnglish()
{
	qApp->removeTranslator(&m_translator);
	recreateUI();
}

void Field::toRussian()
{
	qApp->removeTranslator(&m_translator);
	m_translator.load(":/translate/minesweeper_ru.qm");
	qApp->installTranslator(&m_translator);
	recreateUI();
}

void Field::recreateUI()
{
	//    QMenuBar->clear();
	createToolbar();
}

void Field::dbg()
{
	QPushButton *dbgButton = new QPushButton(tr("dbg"), this);
	panelLayout->addWidget(dbgButton);
	connect(dbgButton, &QPushButton::pressed, this, &Field::onDbgButtonClicked);
	connect(dbgButton, &QPushButton::clicked, this, &Field::onDbgButtonReleased);
}

void Field::onDbgButtonClicked()
{
	for (int row = 0; row < m_row; ++row)
	{
		for (int col = 0; col < m_col; ++col)
		{
			if (buttons[row][col]->mine_ && !buttons[row][col]->open_)
			{
				buttons[row][col]->setIcon(QIcon(":/icons/flag.png"));
			}
		}
	}
}

void Field::onDbgButtonReleased()
{
	for (int row = 0; row < m_row; ++row)
	{
		for (int col = 0; col < m_col; ++col)
		{
			if (buttons[row][col]->mine_ && !buttons[row][col]->flag_ && !buttons[row][col]->open_)
			{
				buttons[row][col]->setIcon(QIcon());
			}
		}
	}
}

void Field::buildField()
{
	QSettings settings("m_save.ini", QSettings::IniFormat);
	settings.beginGroup("GameSettings");
	bool flag = settings.value("flag", true).toBool();
	if (!flag)
	{
		m_row = settings.value("m_row", 10).toInt();
		m_col = settings.value("m_col", 10).toInt();
		m_mine = settings.value("m_mine", 10).toInt();
		m_openButtons = settings.value("m_openButtons", 0).toInt();
		m_firstClick = settings.value("m_firstClick", false).toBool();
		settings.endGroup();
	}
	buttons.resize(m_row);
	gridLayout->setSpacing(0);
	for (int i = 0; i < m_row; ++i)
	{
		buttons[i].resize(m_col);
	}
front:
	for (int row = 0; row < m_row; ++row)
	{
		for (int col = 0; col < m_col; ++col)
		{
			buttons[row][col] = new Fbutton(this);
			buttons[row][col]->row_ = row;
			buttons[row][col]->col_ = col;
			gridLayout->addWidget(buttons[row][col], row, col);
			connect(buttons[row][col], &Fbutton::leftClicked, this, &Field::onLeftClicked);
			connect(buttons[row][col], &Fbutton::rightClicked, this, &Field::onRightClicked);
			connect(buttons[row][col], &Fbutton::midlClicked, this, &Field::onMidlClicked);
			connect(buttons[row][col], &Fbutton::midlRelease, this, &Field::onMidlRelease);
		}
	}
	if (flag)
		return;
	for (int row = 0; row < m_row; ++row)
	{
		for (int col = 0; col < m_col; ++col)
		{
			QString groupKey = QString("button_%1_%2").arg(row).arg(col);
			settings.beginGroup(groupKey);
			if (!settings.contains("mine_") || !settings.contains("flag_") || !settings.contains("open_") ||
				!settings.contains("counter_") || !settings.contains("row_") || !settings.contains("col_"))
			{
				flag = true;
				settings.clear();
				goto front;
			}
			buttons[row][col]->mine_ = settings.value("mine_", false).toBool();
			buttons[row][col]->flag_ = settings.value("flag_", false).toBool();
			buttons[row][col]->open_ = settings.value("open_", false).toBool();
			buttons[row][col]->counter_ = settings.value("counter_", 0).toInt();
			buttons[row][col]->row_ = settings.value("row_", row).toInt();
			buttons[row][col]->col_ = settings.value("col_", col).toInt();
			settings.endGroup();
			if (buttons[row][col]->flag_)
			{
				buttons[row][col]->setIcon(QIcon(":/icons/flag.png"));
			}
			if (buttons[row][col]->open_)
			{
				buttons[row][col]->setStyleSheet("QPushButton { color: black; background-color: #bfbfbf; }");
				if (buttons[row][col]->counter_ > 0)
				{
					buttons[row][col]->setText(QString::number(buttons[row][col]->counter_));
				}
			}
		}
	}
	settings.clear();
}

void Field::setupBoard()
{
	placeMines();
	calculateAdjacentMines();
}

void Field::placeMines()
{
	int placedMines = 0;
	while (placedMines < m_mine)
	{
		int row = QRandomGenerator::global()->bounded(m_row);
		int col = QRandomGenerator::global()->bounded(m_col);
		if (!buttons[row][col]->mine_ && !buttons[row][col]->open_)
		{
			buttons[row][col]->mine_ = true;
			placedMines++;
		}
	}
}

void Field::calculateAdjacentMines()
{
	for (int row = 0; row < m_row; ++row)
	{
		for (int col = 0; col < m_col; ++col)
		{
			if (buttons[row][col]->mine_)
				continue;
			int count = 0;
			for (int i = -1; i <= 1; ++i)
			{
				for (int j = -1; j <= 1; ++j)
				{
					int newRow = row + i;
					int newCol = col + j;
					if (newRow >= 0 && newRow < m_row && newCol >= 0 && newCol < m_col)
					{
						if (buttons[newRow][newCol]->mine_)
						{
							count++;
						}
					}
				}
			}
			buttons[row][col]->counter_ = count;
		}
	}
}

void Field::onLeftClicked(int row, int col)
{
	if (buttons[row][col]->flag_ || buttons[row][col]->open_)
		return;
	if (buttons[row][col]->mine_)
	{
		buttons[row][col]->open_ = true;
		buttons[row][col]->setStyleSheet("QPushButton { color: black; background-color: red; }");
		buttons[row][col]->setIcon(QIcon(":/icons/mine.png"));
		lose();
	}
	else
	{
		revealCell(row, col);
		if (m_openButtons == m_row * m_col - m_mine)
		{
			win();
		}
	}
}

void Field::onRightClicked(int row, int col)
{
	if (buttons[row][col]->open_)
		return;
	buttons[row][col]->flag_ = !buttons[row][col]->flag_;
	buttons[row][col]->setIcon(buttons[row][col]->flag_ ? QIcon(":/icons/flag.png") : QIcon());
}

void Field::onMidlClicked(int row, int col)
{
	if (!buttons[row][col]->open_)
		return;
	int count = 0;
	for (int i = -1; i <= 1; ++i)
	{
		for (int j = -1; j <= 1; ++j)
		{
			int newRow = row + i;
			int newCol = col + j;
			if (newRow >= 0 && newRow < m_row && newCol >= 0 && newCol < m_col)
			{
				if (buttons[newRow][newCol]->flag_)
				{
					count++;
				}
			}
		}
	}
	if (count != buttons[row][col]->counter_)
	{
		for (int i = -1; i <= 1; i++)
		{
			for (int j = -1; j <= 1; j++)
			{
				int newRow = row + i;
				int newCol = col + j;
				if (newRow >= 0 && newRow < m_row && newCol >= 0 && newCol < m_col)
				{
					if (!buttons[newRow][newCol]->flag_ && !buttons[newRow][newCol]->open_)
					{
						buttons[newRow][newCol]->setStyleSheet("QPushButton { background-color: #dbdbdb; }");
					}
				}
			}
		}
		return;
	}
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			int newRow = row + i;
			int newCol = col + j;
			if (newRow >= 0 && newRow < m_row && newCol >= 0 && newCol < m_col)
			{
				onLeftClicked(newRow, newCol);
			}
		}
	}
}

void Field::onMidlRelease(int row, int col)
{
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			int newRow = row + i;
			int newCol = col + j;
			if (newRow >= 0 && newRow < m_row && newCol >= 0 && newCol < m_col)
			{
				if (!buttons[newRow][newCol]->flag_ && !buttons[newRow][newCol]->open_)
				{
					buttons[newRow][newCol]->setStyleSheet("QPushButton { background-color: #cccccc; }");
				}
			}
		}
	}
}

void Field::revealCell(int row, int col)
{
	if (buttons[row][col]->flag_ || buttons[row][col]->open_)
		return;
	buttons[row][col]->open_ = true;
	m_openButtons++;
	if (!m_firstClick)
	{
		setupBoard();
		m_save = true;
		m_firstClick = true;
	}
	buttons[row][col]->setStyleSheet("QPushButton { color: black; background-color: #bfbfbf; }");
	if (buttons[row][col]->counter_ > 0)
	{
		buttons[row][col]->setText(QString::number(buttons[row][col]->counter_));
	}
	else
	{
		revealAdjacentCells(row, col);
	}
}

void Field::revealAdjacentCells(int row, int col)
{
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			int newRow = row + i;
			int newCol = col + j;
			if (newRow >= 0 && newRow < m_row && newCol >= 0 && newCol < m_col)
			{
				revealCell(newRow, newCol);
			}
		}
	}
}

void Field::startNewGame()
{
	m_firstClick = false;
	m_openButtons = 0;
	buttons.clear();
	QLayoutItem *item;
	while ((item = gridLayout->takeAt(0)) != nullptr)
	{
		delete item->widget();
		delete item;
	}
	infoLabel->setText("🙂");
	buildField();
}

void Field::startCustomGame()
{
	showInputDialog();
	startNewGame();
}

void Field::win()
{
	for (int row = 0; row < m_row; ++row)
	{
		for (int col = 0; col < m_col; ++col)
		{
			buttons[row][col]->setEnabled(false);
			buttons[row][col]->open_ = true;
			if (buttons[row][col]->mine_)
			{
				buttons[row][col]->setIcon(QIcon(":/icons/flag.png"));
			}
		}
	}
	m_save = false;
	infoLabel->setText("😎");
}

void Field::lose()
{
	for (int row = 0; row < m_row; ++row)
	{
		for (int col = 0; col < m_col; ++col)
		{
			buttons[row][col]->setEnabled(false);
			if (buttons[row][col]->mine_ && !buttons[row][col]->open_)
			{
				buttons[row][col]->setStyleSheet("QPushButton { background-color: #cccccc; }");
				buttons[row][col]->setIcon(QIcon(":/icons/mine.png"));
			}
			else
			{
				revealCell(row, col);
			}
			buttons[row][col]->open_ = true;
		}
	}
	m_save = false;
	infoLabel->setText("😵");
}

void Field::saveGame()
{
	QSettings settings("m_save.ini", QSettings::IniFormat);
	settings.beginGroup("GameSettings");
	settings.setValue("flag", false);
	settings.setValue("m_row", m_row);
	settings.setValue("m_col", m_col);
	settings.setValue("m_mine", m_mine);
	settings.setValue("m_openButtons", m_openButtons);
	settings.setValue("m_firstClick", m_firstClick);
	settings.endGroup();
	for (int row = 0; row < m_row; ++row)
	{
		for (int col = 0; col < m_col; ++col)
		{
			QString groupKey = QString("button_%1_%2").arg(row).arg(col);
			settings.beginGroup(groupKey);
			settings.setValue("mine_", buttons[row][col]->mine_);
			settings.setValue("flag_", buttons[row][col]->flag_);
			settings.setValue("open_", buttons[row][col]->open_);
			settings.setValue("counter_", buttons[row][col]->counter_);
			settings.setValue("row_", buttons[row][col]->row_);
			settings.setValue("col_", buttons[row][col]->col_);
			settings.endGroup();
		}
	}
	if (!m_save)
	{
		settings.clear();
	}
}

void Field::closeEvent(QCloseEvent *event)
{
	saveGame();
	QWidget::closeEvent(event);
}

// void Field::resizeEvent(QResizeEvent *event)
//{
//   qDebug("dsadsad");
//   QWidget::resizeEvent(event);

//  int cellSize = qMin(width() / m_row, height() / m_col) - 5;
//  for (int i = 0; i < m_row; ++i)
//  {
//    for (int j = 0; j < m_col; ++j)
//    {
//      buttons[i][j]->setFixedSize(cellSize, cellSize);
//      buttons[i][j]->setIconSize(QSize(cellSize, cellSize));
//    }
//  }

//  int side = qMin(m_row * cellSize, m_col * cellSize);
//  resize(side, side);
//}
