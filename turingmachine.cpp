#include "turingmachine.h"
#include "ui_turingmachine.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QMainWindow>
#include <QTimer>

TuringMachine::TuringMachine(const QString& Alphabet,
                              const QString& AddAlphabet,
                              QWidget* parent):
    QWidget(parent),
    ui(new Ui::TuringMachine),
    m_alphabet(Alphabet),
    m_addAlphabet(AddAlphabet)
{
    ui->setupUi(this);

    for (QChar ch : m_alphabet) {
        if (!ch.isSpace()) {
            m_allowed.insert(ch);
        }
    }
    for (QChar ch : m_addAlphabet) {
        if (!ch.isSpace()) {
            m_allowed.insert(ch);
        }
    }

    CreateTable();

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &TuringMachine::executeStep);

    ui->pause->setEnabled(false);
    ui->stop->setEnabled(false);

    UpdateSpeedDisplay();
}

TuringMachine::~TuringMachine()
{
    delete ui;
}

void TuringMachine::CreateTable() {
    QStringList main_symbols;
    for (QChar ch : m_alphabet) {
        if (!ch.isSpace()) {
            main_symbols << ch;
        }
    }
    main_symbols.removeDuplicates();
    QStringList add_symbols;
    for (QChar ch : m_addAlphabet) {
        if (!ch.isSpace()) {
            add_symbols << ch;
        }
    }

    add_symbols.removeDuplicates();

    QString blank = "λ";

    QStringList symbols;
    symbols << main_symbols;
    if (!symbols.contains(blank)) {
        symbols.append(blank);
    } else {
        symbols.removeAll(blank);
        symbols.append(blank);
    }
    symbols << add_symbols;

    symbols.removeDuplicates();

    m_allSymbols = symbols;


    QTableWidget* tw = ui->table;
    tw->clear();
    tw->setColumnCount(1 + m_allSymbols.size());
    tw->setRowCount(1);

    QStringList headers;
    headers << "Состояние";
    headers.append(m_allSymbols);
    tw->setHorizontalHeaderLabels(headers);

    tw->setItem(0, 0, new QTableWidgetItem("q0"));

    tw->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tw->verticalHeader()->setVisible(false);
}

void TuringMachine::CreateTape(const QString& input) {
    const int padding = VISIBLE_COLS / 2;
    QString blank = "λ";

    m_tapeSymbols.clear();
    for (int i = 0; i < padding; ++i) {
        m_tapeSymbols << blank;
    }
    for (QChar ch : input) {
        m_tapeSymbols << QString(ch);
    }
    for (int i = 0; i < padding; ++i) {
        m_tapeSymbols << blank;
    }

    QTableWidget* tw = ui->tape;
    tw->clear();
    tw->setRowCount(1);
    tw->setColumnCount(VISIBLE_COLS);

    tw->horizontalHeader()->setVisible(false);
    tw->verticalHeader()->setVisible(false);\
    tw->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tw->setSelectionMode(QAbstractItemView::NoSelection);
    tw->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tw->verticalHeader()->setDefaultSectionSize(40);
    tw->setFixedHeight(42);

    m_index = padding;

    UpdateView();

    m_initInput = input;
    m_initIndex = padding;
}

void TuringMachine::UpdateView() {
    const int half = VISIBLE_COLS / 2;
    QString blank = "λ";
    QTableWidget* tw = ui->tape;

    while (m_index - half < 0) {
        m_tapeSymbols.prepend(blank);
        ++m_index;
    }
    while (m_index + half >= m_tapeSymbols.size()) {
        m_tapeSymbols.append(blank);
    }

    for (int col = 0; col < VISIBLE_COLS; ++col) {
        int tapePos = m_index - half + col;
        QTableWidgetItem* item = new QTableWidgetItem(m_tapeSymbols[tapePos]);
        item->setTextAlignment(Qt::AlignCenter);

        if (col == half) {
            item->setBackground(QBrush(QColor(0xE0, 0xF0, 0xFF)));
        } else {
            item->setBackground(QBrush(QColor(Qt::white)));
        }
        tw->setItem(0, col, item);
    }
}

void TuringMachine::on_add_state_clicked() {
    QTableWidget* tw = ui->table;
    int new_row = tw->rowCount();
    tw->insertRow(new_row);
    tw->setItem(new_row, 0, new QTableWidgetItem(QString("q%1").arg(++m_nextState)));
}

void TuringMachine::on_del_state_clicked() {
    QTableWidget* tw = ui->table;
    int last_row = tw->rowCount() - 1;
    if (last_row > 0) {
        tw->removeRow(last_row);
        --m_nextState;
    } else {
        QMessageBox::information(this, "Ошибка", "Нельзя удалить единственное состояние.");
    }
}

void TuringMachine::on_confirm_str_clicked() {
    QString input = ui->w_string->text().trimmed();
    if (input.isEmpty()) {
        QMessageBox::information(this, "Запрет", "Строка не может быть пустой.");
        return;
    }

    for (QChar ch : input) {
        if (!m_allowed.contains(ch)) {
            QMessageBox::information(this, "Запрет", QString("Обнаружен недопустимый символ: '%1'").arg(ch));
            return;
        }
    }
    CreateTape(input);
}

void TuringMachine::on_play_clicked() {
    if (!m_simInit) {
        QString error = validateAndLoadRules();
        if (!error.isEmpty()) {
            QMessageBox::warning(this, "Обнаружены ошибки", error);
        }
        if (m_tapeSymbols.isEmpty()) {
            QMessageBox::warning(this, "Пустая строка", "Сначала введите строку");
        }
        StartSimulation();
    }

    if (!m_timerActive) {
        m_timer->start(500);
        m_timerActive = true;
        ui->play->setEnabled(false);
        ui->pause->setEnabled(true);
    }
}

void TuringMachine::on_step_clicked() {
    if (m_timerActive) {
        stopTimer();
    }
    if (!m_simInit) {
        QString error = validateAndLoadRules();
        if (!error.isEmpty()) {
            QMessageBox::warning(this, "Обнаружены ошибки", error);
        }
        if (m_tapeSymbols.isEmpty()) {
            QMessageBox::warning(this, "Пустая строка", "Сначала введите строку");
        }
        StartSimulation();
    }
    executeStep();
}

void TuringMachine::on_pause_clicked() {
    if (m_timerActive) {
        stopTimer();
    }
}

void TuringMachine::on_stop_clicked() {
    stopTimer();
    if (m_simInit) {
        resetSimulation();
    }
}

void TuringMachine::on_inc_speed_clicked() {
    int newInterval = m_timerInterval - 100;
    if (newInterval < 20) newInterval = 20;
    if (newInterval != m_timerInterval) {
        m_timerInterval = newInterval;
        restartTimer();
        UpdateSpeedDisplay();
    }
}

void TuringMachine::on_dec_speed_clicked() {
    int newInterval = m_timerInterval + 100;
    if (newInterval > 2000) newInterval = 2000;
    if (newInterval != m_timerInterval) {
        m_timerInterval = newInterval;
        restartTimer();
        UpdateSpeedDisplay();
    }
}

void TuringMachine::executeStep() {
    if (!m_simInit || m_tapeSymbols.isEmpty() || m_curState.isEmpty()) return;

    QString curSymbol = m_tapeSymbols[m_index];
    QString key = m_curState + ":" + curSymbol;

    if (!m_transitions.contains(key)) {
        stopTimer();
        QMessageBox::information(this, "Остановка", QString("Нет правила для (%1, %2). Машина остановлена.")
                                                        .arg(m_curState, curSymbol));
        return;
    }

    const TuringRules& rule = m_transitions[key];

    if (!rule.newSymbol.isEmpty()) {
        m_tapeSymbols[m_index] = rule.newSymbol;
    }

    if (rule.direction == "L") {
        moveLeft();
    } else if (rule.direction == "R") {
        moveRight();
    } else {
        UpdateView();
    }

    if (!rule.newState.isEmpty()) {
        m_curState = rule.newState;
    }

    updateStateDisplay();
}

void TuringMachine::StartSimulation() {
    QStringList states = collectStateNames();
    if (states.isEmpty()) return;
    m_initState = states.first();
    m_curState = m_initState;

    m_index = m_initIndex;
    UpdateView();
    updateStateDisplay();

    m_simInit = true;
    ui->play->setEnabled(true);
    ui->step->setEnabled(true);
    ui->pause->setEnabled(false);
    ui->stop->setEnabled(true);

    m_timerActive = false;
}

void TuringMachine::stopTimer() {
    if (m_timer ->isActive()) {
        m_timer->stop();
    }
    m_timerActive = false;
    ui->play->setEnabled(true);
    ui->pause->setEnabled(false);
}

void TuringMachine::resetSimulation() {
    if (!m_initInput.isEmpty()) {
        CreateTape(m_initInput);
    }
    m_curState = m_initState;
    updateStateDisplay();
    m_simInit = false;

    ui->pause->setEnabled(false);
    ui->stop->setEnabled(false);
}

void TuringMachine::updateStateDisplay() {
    QTableWidget* tw = ui->table;
    QStringList stateNames = collectStateNames();
    int curRow = stateNames.indexOf(m_curState);

    for (int row = 0; row < tw->rowCount(); ++row) {
        bool isActive = (row == curRow);
        for (int col = 0; col < tw->columnCount(); ++col) {
            QTableWidgetItem* item = tw->item(row, col);
            if (item) {
                if (isActive) {
                    item->setBackground(QBrush(QColor(0xE0, 0xF0, 0xFF)));
                } else {
                    item->setBackground(QBrush(QColor(Qt::white)));
                }
            }
        }
    }
}

void TuringMachine::moveLeft() {
    --m_index;
    UpdateView();
}

void TuringMachine::moveRight() {
    ++m_index;
    UpdateView();
}

QStringList TuringMachine::collectStateNames() const {
    QStringList names;
    QTableWidget* tw = ui->table;
    for (int row = 0; row < tw->rowCount(); ++row) {
        QTableWidgetItem* item = tw->item(row, 0);
        if (item) {
            names << item->text().trimmed();
        }
    }
    return names;
}

QString TuringMachine::validateAndLoadRules() {
    QTableWidget* tw = ui->table;

    QStringList stateNames = collectStateNames();
    m_transitions.clear();
    QStringList errors;

    for(int row = 0; row < tw->rowCount(); ++row) {
        QString curState = tw->item(row, 0)->text().trimmed();
        for (int col = 1; col < tw->columnCount(); ++col) {
            QString symbol = m_allSymbols[col - 1];
            QTableWidgetItem* cell = tw->item(row, col);
            QString text = cell ? cell->text().trimmed() : QString();

            if (text.isEmpty()) continue;

            bool ok = false;

            QString error;
            TuringRules rule = parseRules(text, stateNames, ok, error);
            if (!ok) {
                errors << QString("Строка %1, столбец '%2': %3").arg(row + 1).arg(symbol).arg(error);
                continue;
            }

            QString key = curState + ":" + symbol;
            m_transitions[key] = rule;
        }
    }

    if (!errors.isEmpty()) {
        return errors.join("\n");
    }

    return QString();
}

TuringRules TuringMachine::parseRules(const QString &text,
                       const QStringList& stateNames, bool &ok, QString &error) const {
    ok = true;
    error.clear();
    TuringRules rule;
    rule.valid = false;

    QStringList InitParts = text.split(',', Qt::KeepEmptyParts);
    QStringList parts;
    for (const QString& p : InitParts) {
        QString trim = p.trimmed();
        if (!trim.isEmpty()) {
            parts.append(trim);
        }
    }

    if (parts.size() > 3) {
        ok = false;
        error = "Слишком много инструкций (нужно 3)";
        return rule;
    }

    QString newSymbol, direction, newState;

    for (const QString& part : parts) {
        if (part.isEmpty()) continue;

        if (part == "L" || part == "R") {
            if (!direction.isEmpty()) {
                ok = false;
                error = "Указано два направления";
                return rule;
            }
            direction = part;
        } else if (stateNames.contains(part)) {
            if (!newState.isEmpty()) {
                ok = false;
                error = "Указано два состояния";
                return rule;
            }
            newState = part;
        } else if (m_allSymbols.contains(part)) {
            if (!newSymbol.isEmpty()) {
                ok = false;
                error = "Указано два элемента";
                return rule;
            }
            newSymbol = part;
        } else {
            ok = false;
            error = QString("Неизвестный компонент '%1'").arg(part);
            return rule;
        }
    }

    rule.newSymbol = newSymbol;
    rule.direction = direction;
    rule.newState = newState;

    if (newSymbol.isEmpty() && direction.isEmpty() && newState.isEmpty()) {
        ok = true;
        return rule;
    }

    rule.valid = true;
    return rule;
}

void TuringMachine::restartTimer() {
    if (m_timerActive) {
        m_timer->stop();
        m_timer->setInterval(m_timerInterval);
        m_timer->start();
    } else {
        m_timer->setInterval(m_timerInterval);
    }
}

void TuringMachine::UpdateSpeedDisplay() {
    QString tooltip = QString("Интервал: %1 мс").arg(m_timerInterval);
    ui->inc_speed->setToolTip(tooltip);
    ui->dec_speed->setToolTip(tooltip);
}
