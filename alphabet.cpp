#include "alphabet.h"
#include "ui_alphabet.h"
#include "turingmachine.h"
#include <QMessageBox>

alphabet::alphabet(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::alphabet)
{
    ui->setupUi(this);

    connect(ui->SetAlphabet, &QPushButton::clicked, this, &alphabet::on_nextButton_clicked);
}

alphabet::~alphabet()
{
    delete ui;
}

void alphabet::on_nextButton_clicked() {
    QString Normal_Alphabet = ui->Alphabet->text().trimmed();
    QString Additional_Alphabet = ui->Add_Alphabet->text().trimmed();

    QSet<QChar> setMain, setAdd;
    for (QChar ch : Normal_Alphabet) {
        if (!ch.isSpace()){
            setMain.insert(ch);
        }
    }
    for (QChar ch : Additional_Alphabet) {
        if (!ch.isSpace()){
            setAdd.insert(ch);
        }
    }

    QSet<QChar> intersection = setMain;
    intersection.intersect(setAdd);
    if(!intersection.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Алфавиты не должны пересекаться.");
        return;
    }

    if (m_tmWindow) {
        m_tmWindow->applyAlphabetUpdate(Normal_Alphabet, Additional_Alphabet);
        this->hide();
    } else {
        m_tmWindow = new TuringMachine(Normal_Alphabet, Additional_Alphabet);
        connect(m_tmWindow, &TuringMachine::alph_change, this, &alphabet::on_alph_change);
        m_tmWindow->setAttribute(Qt::WA_DeleteOnClose);
        QPalette pal = QPalette();
        pal.setColor(QPalette::Window, Qt::lightGray);
        m_tmWindow->setPalette(pal);
        m_tmWindow->show();
        this->hide();
    }
}

void alphabet::on_alph_change(const QString& mainAlph, const QString& addAlph) {
    ui->Alphabet->setText(mainAlph);
    ui->Add_Alphabet->setText(addAlph);
    this->show();
    this->raise();
    this->activateWindow();
}
