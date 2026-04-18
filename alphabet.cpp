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

    TuringMachine* window = new TuringMachine(Normal_Alphabet, Additional_Alphabet);
    window->setAttribute(Qt::WA_DeleteOnClose);
    QPalette pal = QPalette();
    pal.setColor(QPalette::Window, Qt::lightGray);
    window->setPalette(pal);
    window->show();
    this->hide();
}
