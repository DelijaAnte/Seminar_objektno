#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QMouseEvent>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QFileDialog>
#include <QTextStream>
#include <QColorDialog>
#include<QFontDialog>
#include <QUndoStack>
#include <QUndoCommand>

class ChangeTextCommand : public QUndoCommand {
public:
    ChangeTextCommand(QLabel* label, const QString& oldText, const QString& newText)
        : label(label), oldText(oldText), newText(newText) {}

    void undo() override {
        label->setText(oldText);
    }

    void redo() override {
        label->setText(newText);
    }

private:
    QLabel* label;
    QString oldText;
    QString newText;
};
class ChangeFontCommand : public QUndoCommand {
public:
    ChangeFontCommand(QLabel* label, const QFont& oldFont, const QFont& newFont)
        : label(label), oldFont(oldFont), newFont(newFont) {}

    void undo() override {
        label->setFont(oldFont);
    }

    void redo() override {
        label->setFont(newFont);
    }

private:
    QLabel* label;
    QFont oldFont;
    QFont newFont;
};
class MyDialog : public QDialog {
public:
    MyDialog();
    QVBoxLayout* mainLayout;
    QWidget* editWidget;
    QFormLayout* editLayout;
    QLineEdit* lineEdit;
    QDialogButtonBox* buttonBox;
};

class MyMainWindow : public QMainWindow {
public:
    MyMainWindow();
    QLabel* MyLabel;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    bool drawingEnabled;
    QColor drawingColor;
    QPoint previousPoint;
    QPoint currentPoint;
    QList<QLine> listOfLines;  // Nova lista za pohranu crtaćih linija, PREKO VEKTORA

public slots:
    void EditNoviTekstMenu();
    void FileSaveAsMenu();
    void ChangeTextColorMenu();
    void ChangeTextBackgroundMenu();
    void ChangeLabelBorderMenu();
    void ChangeFontMenu();
    void FileOpenMenu();
    void undo();
    void redo();
    void updateUndoRedoActions();
    void ChangeBackgroundColorAllMenu();
    void EnableDrawingOnBackground();
    void ClearingDrawing();

private:
    QUndoStack undoStack;
    QAction* EditUndo;
    QAction* EditRedo;
    QMenu* EditMenu;
    QAction* EditNoviTekst;
    QAction* DrawOnBackground;
    QAction* ChangeTextColor;
    QAction* ChangeTextBackground;
    QAction* ChangeLabelBorder;
    QAction* ChangeFont;
    QAction* ChangeBackgroundColorAll;
    QAction* ClearDrawing;
    QMenu* FileMenu;
    QAction* FileSaveAs;
    QAction* FileOpen;
};

MyDialog::MyDialog() {
    lineEdit = new QLineEdit;
    editLayout = new QFormLayout;
    editLayout->addRow(new QLabel(tr("Novi tekst:")), lineEdit);
    editWidget = new QWidget;
    editWidget->setLayout(editLayout);
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(editWidget);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    setWindowTitle("Promjena teksta");
}

void MyMainWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        MyLabel->move(event->position().toPoint());
    }

    if (drawingEnabled && event->button() == Qt::LeftButton) {
        previousPoint = event->pos();
        currentPoint = event->pos();
        update();
    }
}

void MyMainWindow::keyPressEvent(QKeyEvent *event) {
    switch(event->key()) {
    case Qt::Key_Left: MyLabel->move(MyLabel->pos().x() - 1, MyLabel->pos().y());
        break;
    case Qt::Key_Right: MyLabel->move(MyLabel->pos().x() + 1, MyLabel->pos().y());
        break;
    case Qt::Key_Up: MyLabel->move(MyLabel->pos().x(), MyLabel->pos().y() - 1);
        break;
    case Qt::Key_Down: MyLabel->move(MyLabel->pos().x(), MyLabel->pos().y() + 1);
        break;
//mogucnost kretanja i sa WSAD
    case Qt::Key_A : MyLabel->move(MyLabel->pos().x() - 1, MyLabel->pos().y());
        break;
    case Qt::Key_D: MyLabel->move(MyLabel->pos().x() + 1, MyLabel->pos().y());
        break;
    case Qt::Key_W: MyLabel->move(MyLabel->pos().x(), MyLabel->pos().y() - 1);
        break;
    case Qt::Key_S: MyLabel->move(MyLabel->pos().x(), MyLabel->pos().y() + 1);
        break;
    }
}

void MyMainWindow::paintEvent(QPaintEvent* event) {
    QMainWindow::paintEvent(event);

    if (drawingEnabled) {
        QPainter painter(this);
        painter.setPen(QPen(drawingColor, 20));
        for (const auto& line : listOfLines) {
            painter.drawLine(line);
        }
    }
}

void MyMainWindow::EditNoviTekstMenu() {
    MyDialog dialog;
    if (dialog.exec() == QDialog::Accepted) {
        QString oldText = MyLabel->text();
        QString newText = dialog.lineEdit->text();

        if (oldText != newText) {
            MyLabel->setText(newText);
            QUndoCommand* command = new ChangeTextCommand(MyLabel, oldText, newText);
            undoStack.push(command);
            updateUndoRedoActions();
        }
    }
}

void MyMainWindow::FileSaveAsMenu() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save As...", "", "FESB File (*.fsb)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream out(&file);
            out << "fesb file" << Qt::endl;
            out << MyLabel->text() << Qt::endl;
            out << MyLabel->pos().x() << Qt::endl;
            out << MyLabel->pos().y() << Qt::endl;
            out << pos().x() << Qt::endl;
            out << pos().y() << Qt::endl;
            out << size().width() << Qt::endl;
            out << size().height() << Qt::endl;
        } else {
            QMessageBox::information(this, "Unable to Open File", file.errorString());
        }
    }
}

void MyMainWindow::FileOpenMenu() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Geometry...", "", "FESB File (*.fsb)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream in(&file);
            QString str = in.readLine();
            if (str == "fesb file") {
                str = in.readLine();
                MyLabel->setText(str);
                int x, y, w, h;
                in >> x >> y;
                MyLabel->move(x, y);
                in >> x >> y >> w >> h;
                this->setGeometry(x, y, w, h);
            }
        } else {
            QMessageBox::information(this, "Unable to OpenFile", file.errorString());
        }
    }
}

void MyMainWindow::ChangeTextColorMenu() {
    QColor color = QColorDialog::getColor(MyLabel->palette().text().color(), this, tr("Select Text Color"));
    if (color.isValid()) {
        QPalette palette = MyLabel->palette();
        palette.setColor(QPalette::WindowText, color);
        MyLabel->setPalette(palette);
    }
}

void MyMainWindow::ChangeTextBackgroundMenu() {
    QColor color = QColorDialog::getColor(MyLabel->palette().window().color(), this, tr("Select Text Background Color"));
    if (color.isValid()) {
        QString styleSheet = QString("color: %1; background-color: %2;").arg(MyLabel->palette().text().color().name(), color.name());
        MyLabel->setStyleSheet(styleSheet);
    }
}

void MyMainWindow::ChangeLabelBorderMenu() {
    QColor color = QColorDialog::getColor(MyLabel->palette().window().color(), this, tr("Select Label Border Color"));
    if (color.isValid()) {
        MyLabel->setStyleSheet(QString("border: 2px solid %1;").arg(color.name()));
    }
}
void MyMainWindow::ChangeFontMenu() {
    bool ok;
    QFont font = QFontDialog::getFont(&ok, MyLabel->font(), this, tr("Select Font"));
    if (ok) {
        // Stvaranje QUndoCommand i dodavanje na undoStack
        QUndoCommand* command = new ChangeFontCommand(MyLabel, MyLabel->font(), font);
        undoStack.push(command);

        MyLabel->setFont(font);

        updateUndoRedoActions();
    }
}

void MyMainWindow::undo() {
    undoStack.undo();
    updateUndoRedoActions();
}

void MyMainWindow::redo() {
    undoStack.redo();
    updateUndoRedoActions();
}

void MyMainWindow::updateUndoRedoActions() {
    EditUndo->setEnabled(undoStack.canUndo());
    EditRedo->setEnabled(undoStack.canRedo());
}

void MyMainWindow::ChangeBackgroundColorAllMenu() {
    QColor color = QColorDialog::getColor(MyLabel->palette().window().color(), this, tr("Select Background Color for All"));

    if (color.isValid()) {
        MyLabel->setAutoFillBackground(true);
        QPalette palette = MyLabel->palette();
        palette.setColor(QPalette::Window, color);
        MyLabel->setPalette(palette);

        palette = this->palette();
        palette.setColor(QPalette::Window, color);
        this->setPalette(palette);
    }
}

void MyMainWindow::EnableDrawingOnBackground() {
    QColorDialog colorDialog(this);
    colorDialog.setCurrentColor(drawingColor);
    if (colorDialog.exec() == QDialog::Accepted) {
        drawingEnabled = true;
        drawingColor = colorDialog.currentColor();
        setCursor(Qt::CrossCursor);
    }
}

void MyMainWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (drawingEnabled && event->button() == Qt::LeftButton) {
        drawingEnabled = false;
        setCursor(Qt::ArrowCursor);//promjeni izgled kursora ako smo usli u crtanje
    }
}

void MyMainWindow::ClearingDrawing() {
    drawingEnabled = false;
    listOfLines.clear();//ocisti crtez sa liste linija
    update();
}

void MyMainWindow::mouseMoveEvent(QMouseEvent* event) {
    if (drawingEnabled && (event->buttons() & Qt::LeftButton)) {
        previousPoint = currentPoint;
        currentPoint = event->pos();
        listOfLines.append(QLine(previousPoint, currentPoint));  // Dodajte crtež u listu linija
        update();
    }
}

MyMainWindow::MyMainWindow() {
    MyLabel = new QLabel(this);
    MyLabel->setText("Hello World!");
    MyLabel->move(10, 20);

    EditNoviTekst = new QAction(tr("&Novi tekst..."), this);
    EditNoviTekst->setShortcut(tr("CTRL+N"));
    connect(EditNoviTekst, &QAction::triggered, this, &MyMainWindow::EditNoviTekstMenu);

    EditMenu = menuBar()->addMenu(tr("&Edit"));
    EditMenu->addAction(EditNoviTekst);

    EditUndo = undoStack.createUndoAction(this, tr("&Undo"));
    EditUndo->setShortcut(tr("CTRL+Z"));
    connect(EditUndo, &QAction::triggered, this, &MyMainWindow::undo);
    EditMenu->addAction(EditUndo);

    EditRedo = undoStack.createRedoAction(this, tr("&Redo"));
    EditRedo->setShortcut(tr("CTRL+Y"));
    connect(EditRedo, &QAction::triggered, this, &MyMainWindow::redo);
    EditMenu->addAction(EditRedo);

    FileSaveAs = new QAction(tr("&Save As..."), this);
    connect(FileSaveAs, &QAction::triggered, this, &MyMainWindow::FileSaveAsMenu);

    FileOpen = new QAction(tr("&Open..."), this);
    FileOpen->setShortcut(tr("CTRL+O"));
    connect(FileOpen, &QAction::triggered, this, &MyMainWindow::FileOpenMenu);

    FileMenu = menuBar()->addMenu(tr("&File"));
    FileMenu->addAction(FileSaveAs);
    FileMenu->addAction(FileOpen);

    ChangeTextColor = new QAction(tr("Change Text Color..."), this);
    connect(ChangeTextColor, &QAction::triggered, this, &MyMainWindow::ChangeTextColorMenu);
    EditMenu->addAction(ChangeTextColor);

    ChangeTextBackground = new QAction(tr("Change Text Background..."), this);
    connect(ChangeTextBackground, &QAction::triggered, this, &MyMainWindow::ChangeTextBackgroundMenu);
    EditMenu->addAction(ChangeTextBackground);

    ChangeLabelBorder = new QAction(tr("Change Label Border..."), this);
    connect(ChangeLabelBorder, &QAction::triggered, this, &MyMainWindow::ChangeLabelBorderMenu);
    EditMenu->addAction(ChangeLabelBorder);

    ChangeFont = new QAction(tr("Change Font..."), this);
    connect(ChangeFont, &QAction::triggered, this, &MyMainWindow::ChangeFontMenu);
    EditMenu->addAction(ChangeFont);

    //promjena pozadinske boje
    ChangeBackgroundColorAll = new QAction(tr("Change Background Color (All)"), this);
    connect(ChangeBackgroundColorAll, &QAction::triggered, this, &MyMainWindow::ChangeBackgroundColorAllMenu);
    EditMenu->addAction(ChangeBackgroundColorAll);
    //DrawOnBackground

    DrawOnBackground=new QAction(tr("Draw on Background"), this);
    connect(DrawOnBackground,&QAction::triggered, this, &MyMainWindow::EnableDrawingOnBackground);
    EditMenu->addAction(DrawOnBackground);

    //brisanje bojanja
    ClearDrawing = new QAction(tr("Clear Drawing"), this);
    connect(ClearDrawing, &QAction::triggered, this, &MyMainWindow::ClearingDrawing);
    EditMenu->addAction(ClearDrawing);

}




int main(int argc, char **argv) {
    QApplication app(argc, argv);
    MyMainWindow mainWindow;
    mainWindow.resize(300, 150);
    mainWindow.show();// sluzi z aprikaz glavnog prozora
    return app.exec();//petlja  aobradu podataka, zavrsava s pozivom exit!
}
