#ifndef TURINGMACHINE_H
#define TURINGMACHINE_H

#include <QWidget>
#include <QScrollBar>

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

private slots:
    void on_add_state_clicked();
    void on_del_state_clicked();
    void on_confirm_str_clicked();

private:
    Ui::TuringMachine* ui;
    QString m_alphabet;
    QString m_addAlphabet;
    int m_nextState = 0;
    QStringList m_allSymbols;
    QSet<QChar> m_allowed;
    const int VISIBLE_COLS = 13;
    int m_index = 0;
    QStringList m_tapeSymbols;

    void CreateTable();
    void CreateTape(const QString& input);
    void UpdateView();
};

#endif // TURINGMACHINE_H
