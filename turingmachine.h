#ifndef TURINGMACHINE_H
#define TURINGMACHINE_H

#include <QWidget>
#include <QScrollBar>

namespace Ui {
class TuringMachine;
}

struct TuringRules {
public:
    QString newSymbol;
    QString direction;
    QString newState;
    bool valid = false;
};

class TuringMachine : public QWidget
{
    Q_OBJECT

public:
    explicit TuringMachine(const QString& Alphabet,
                           const QString& AddAlphabet,
                            QWidget* parent = nullptr);
    ~TuringMachine();

    void applyAlphabetUpdate(const QString& mainAlph, const QString& addAlph);

signals:
    void alph_change(const QString& mainAlph, const QString& addAlph);

private slots:
    void on_add_state_clicked();
    void on_del_state_clicked();
    void on_confirm_str_clicked();
    void on_play_clicked();
    void on_step_clicked();
    void on_pause_clicked();
    void on_stop_clicked();
    void on_inc_speed_clicked();
    void on_dec_speed_clicked();
    void on_change_alph_clicked();
    void executeStep();

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
    QStringList collectStateNames() const;
    QString validateAndLoadRules();
    TuringRules parseRules(const QString &text,
                           const QStringList& stateNames, bool &ok, QString &error) const;
    QMap<QString, TuringRules> m_transitions;
    QString m_curState;
    QString m_initState;
    QString m_initInput;
    int m_initIndex = VISIBLE_COLS / 2;
    QTimer* m_timer = nullptr;
    bool m_timerActive = false;
    bool m_simInit = false;
    int m_timerInterval = 500;

    void CreateTable();
    void CreateTape(const QString& input);
    void UpdateView();
    void moveLeft();
    void moveRight();
    void StartSimulation();
    void stopTimer();
    void resetSimulation();
    void updateStateDisplay();
    void UpdateSpeedDisplay();
    void restartTimer();
    void RebuildTable();
    void addColumns(const QStringList& oldMain, const QStringList& oldAdd,
                    const QStringList& newMain, const QStringList& newAdd);
};

#endif // TURINGMACHINE_H
