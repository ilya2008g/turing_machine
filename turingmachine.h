#ifndef TURINGMACHINE_H
#define TURINGMACHINE_H

#include <QWidget>

namespace Ui {
class TuringMachine;
}

class TuringMachine : public QWidget
{
    Q_OBJECT

public:
    explicit TuringMachine(const QString& Alphabet,
                           const QString& AddAlphabet,
                            QWidget* parent = nullptr);
    ~TuringMachine();

private:
    Ui::TuringMachine* ui;
    QString m_alphabet;
    QString m_addAlphabet;

    void CreateTable();
};

#endif // TURINGMACHINE_H
