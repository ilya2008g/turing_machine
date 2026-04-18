#ifndef ALPHABET_H
#define ALPHABET_H

#include <QWidget>

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

private:
    Ui::alphabet *ui;
};
#endif // ALPHABET_H
