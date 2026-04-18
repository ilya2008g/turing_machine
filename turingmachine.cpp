#include "turingmachine.h"
#include "ui_turingmachine.h"

TuringMachine::TuringMachine(const QString& Alphabet,
                              const QString& AddAlphabet,
                              QWidget* parent):
    QWidget(parent),
    ui(new Ui::TuringMachine),
    m_alphabet(Alphabet),
    m_addAlphabet(AddAlphabet)
{
    ui->setupUi(this);
}

TuringMachine::~TuringMachine()
{
    delete ui;
}

void TuringMachine::CreateTable() {
    QStringList symbols;
    for (QChar ch : m_alphabet) {
        if (!ch.isSpace()) {
            symbols << ch;
        }
    }
    for (QChar ch : m_addAlphabet) {
        if (!ch.isSpace()) {
            symbols << ch;
        }
    }

    symbols.removeDuplicates();
}
