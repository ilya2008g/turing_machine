#ifndef ALPHABET_H
#define ALPHABET_H

#include <QWidget>\

class TuringMachine;

QT_BEGIN_NAMESPACE
namespace Ui {
class alphabet;
}
QT_END_NAMESPACE

class alphabet : public QWidget
{
    Q_OBJECT

public:
    alphabet(QWidget *parent = nullptr);
    ~alphabet();

private slots:
    void on_nextButton_clicked();
    void on_alph_change(const QString& mainAlph, const QString& addAlph);

private:
    Ui::alphabet* ui;
    TuringMachine* m_tmWindow = nullptr;
};
#endif // ALPHABET_H
